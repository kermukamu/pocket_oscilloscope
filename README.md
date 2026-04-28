A fun, quite barebones, MCU unit project for sampling, transmitting, and plotting received analog signal.

The MCU samples an analog signal, and transmits the data over Wi-Fi to a desktop client. The desktop client plots the data.

Relevant features:
> Real-time waveform plotting
> Wireless sample streaming over Wi-Fi (UDP)
> ESP32-based ADC acquisition
> Double-buffered sampling for reduced jitter
> Ring-buffered desktop visualization
> Cross-platform receiver (Linux / Windows)

The transmitter (MCU) code is set to sample at 5000 Hz, however much higher rates can be set by adjusting fs in transmitter.ino

Hardware Requirements:
> ESP32 development board and USB cable for uploading the program.
> Wi-Fi network
> To read negative voltage values, a biasing circuit is needed.

Software requirements:
> Arduino IDE or PlatformIO
> ESP32 board support package
> CMake
> C++17 compiler
> OpenGL
> GLFW

Dependencies included:
- Dear ImGui
- ImPlot
- Glad

ESP32 Setup (transmitter):
> Open transmitter/transmitter.ino
> Update Wi-Fi credentials
> Set the receiver IP address
> Upload to ESP32

Building receiver
> In receiver directory, run
```mkdir build
cd build
cmake ..
make
./esp32_scope
```
