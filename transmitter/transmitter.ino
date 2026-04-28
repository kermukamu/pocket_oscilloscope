#include <WiFi.h>
#include <WiFiUdp.h>

const int ar_pin = 34;
volatile uint32_t sample_count = 0;

// WiFi settings
const char* ssid = "wifi_example";
const char* password = "password_example";

// Target device
WiFiUDP udp;
IPAddress targetIP(192, 168, 0, 104);
const uint16_t targetPort = 5005;

// Sampling settings
const uint32_t fs = 5000;   // target sample rate in Hz
const int batch_size = 128; // samples per packet/buffer

// Double buffers
volatile uint16_t bufferA[batch_size];
volatile uint16_t bufferB[batch_size];

volatile uint16_t* activeBuffer = bufferA;
volatile uint16_t* readyBuffer = nullptr;

volatile int activeIndex = 0;
volatile bool bufferReady = false;
volatile uint32_t readySampleCount = 0;

// Hardware timer
hw_timer_t* timer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// UDP payload buffer for CSV
char payload[1024];

void IRAM_ATTR onSampleTimer() {
  portENTER_CRITICAL_ISR(&timerMux);

  // If main loop has not consumed previous buffer yet, drop samples
  if (bufferReady) {
    portEXIT_CRITICAL_ISR(&timerMux);
    return;
  }

  uint16_t val = analogRead(ar_pin);
  activeBuffer[activeIndex] = val;
  activeIndex++;
  sample_count++;

  if (activeIndex >= batch_size) {
    readyBuffer = activeBuffer;
    readySampleCount = sample_count;
    bufferReady = true;

    // Swap buffers
    if (activeBuffer == bufferA) {
      activeBuffer = bufferB;
    } else {
      activeBuffer = bufferA;
    }

    activeIndex = 0;
  }

  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());

  udp.begin(12345);

  // Optional ADC setup
  analogReadResolution(12);
  // analogSetPinAttenuation(ar_pin, ADC_11db); // if you want wider voltage range

  // Timer: 1 tick = 1 us
  timer = timerBegin(1000000);

  // Attach ISR
  timerAttachInterrupt(timer, &onSampleTimer);

  // Fire at fs Hz
  timerAlarm(timer, 1000000 / fs, true, 0);

  Serial.print("Sampling started at ");
  Serial.print(fs);
  Serial.println(" Hz");
}

void loop() {
  static uint16_t localCopy[batch_size];
  uint32_t localSampleCount = 0;
  bool haveBuffer = false;

  portENTER_CRITICAL(&timerMux);
  if (bufferReady && readyBuffer != nullptr) {
    for (int i = 0; i < batch_size; i++) {
      localCopy[i] = readyBuffer[i];
    }
    localSampleCount = readySampleCount;
    bufferReady = false;
    readyBuffer = nullptr;
    haveBuffer = true;
  }
  portEXIT_CRITICAL(&timerMux);

  if (haveBuffer) {
    send_batch_udp(localCopy, batch_size, localSampleCount);
  }
}

void send_batch_udp(const uint16_t* vals, int count, uint32_t totalSamples) {
  int offset = 0;

  offset += snprintf(payload + offset, sizeof(payload) - offset, "%lu", (unsigned long)totalSamples);

  for (int i = 0; i < count; i++) {
    int written = snprintf(payload + offset, sizeof(payload) - offset, ",%u", vals[i]);
    if (written < 0 || offset + written >= (int)sizeof(payload)) {
      break;
    }
    offset += written;
  }

  udp.beginPacket(targetIP, targetPort);
  udp.write((const uint8_t*)payload, offset);
  udp.endPacket();
}
