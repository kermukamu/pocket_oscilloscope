#include "imgui.h"
#include "implot.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

#include "udp_receiver.hpp"
#include "ring_buffer.hpp"

int main() {
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";

    // Setup the window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ESP32 Scope", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    UdpReceiver receiver(5005);
    receiver.start();

    RingBuffer ring(5000);
    std::vector<float>plotData;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Get any newly received samples from UDP thread
        auto samples = receiver.takeSamples();
        for (int s : samples) {
            ring.push((float)s);
        }

        // Copy ring buffer into plot-ready ordered vector
        ring.copyOrdered(plotData);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Oscilloscope");

        if (ImPlot::BeginPlot("Signal")) {
            ImPlot::SetupAxes("Sample", "ADC");
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, (double)plotData.size(), ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 4095, ImGuiCond_Always);

            if (!plotData.empty()) {
                ImPlot::PlotLine("Channel A", plotData.data(), (int)plotData.size());
            }

            ImPlot::EndPlot();
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    receiver.stop();

    ImPlot::DestroyContext();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}