#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "imgui-SFML.h"
#include "imgui.h"
#include "implot.h"

#include "inspect_config.h"
#include "inspect.hpp"

int close_app(sf::RenderWindow &window);

int main(int argc, char *argv[])
{

    sf::RenderWindow window(sf::VideoMode(1280, 720), std::string("INSPECT - ") + INSPECT_VERSION);

    window.setFramerateLimit(INSPECT_MAX_FPS);
    window.setVerticalSyncEnabled(INSPECT_VSYNC);

    if (!ImGui::SFML::Init(window))
    {
        std::cout << "IMGUI SFML Window Init failed!" << std::endl;
        return -1;
    }

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::CreateContext();
    ImPlot::CreateContext();

    sf::Texture texture;

    if (!texture.loadFromFile("spec.png"))
    {
    }
    ImTextureID img_handle = gl_handle_to_imgui_id(texture.getNativeHandle());

    sf::Clock deltaClock;
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
            {
                return close_app(window);
            }

            if (event.type == sf::Event::Resized)
            {
                sf::FloatRect visibleArea(0.f, 0.f, event.size.width, event.size.height);
                window.setView(sf::View(visibleArea));
            }
        }

        // == UPDATE == //
        ImGui::SFML::Update(window, deltaClock.restart());

        // == DRAWING == //
        ImGui::DockSpaceOverViewport();

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Settings"))
            {

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (ImGui::Begin("Image Feed"))
        {
            if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend | ImPlotFlags_Equal | ImPlotFlags_CanvasOnly))
            {
                ImPlotAxisFlags ax_flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines;
                ImPlot::SetupAxes(0, 0, ax_flags, ax_flags);
                ImPlot::PlotImage("Spectrum", img_handle, ImPlotPoint(0, 0), ImPlotPoint(1, 1), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));
                ImPlot::EndPlot();
            }
            ImGui::End();
        }
        if (ImGui::Begin("Spectrum"))
        {
            if (ImPlot::BeginPlot("##", ImVec2(-1, -1)))
            {
                ImPlot::EndPlot();
            }

            ImGui::End();
        }

        window.clear(sf::Color(20, 20, 20));
        ImGui::SFML::Render(window);
        window.display();
    }

    return close_app(window);
}

int close_app(sf::RenderWindow &window)
{
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    // ImGui::SFML::Shutdown();

    window.close();

    return 0;
}