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

    sf::RenderWindow window(sf::VideoMode(1280, 720), std::string("inspect | spectroscopy software | ") + INSPECT_VERSION);

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

    sf::Image img;
    sf::Texture texture;

    if (!img.loadFromFile("spec.png"))
    {
    }

    texture.loadFromImage(img);
    texture.setSmooth(true);

    ImTextureID img_handle = gl_handle_to_imgui_id(texture.getNativeHandle());

    std::vector<float> brightness;
    std::vector<float> x_values;

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
                ImPlot::PlotImage("Spectrum", img_handle, ImPlotPoint(0, 0), ImPlotPoint(texture.getSize().x, texture.getSize().y), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));

                static double drag_tag = texture.getSize().y / 2;
                ImPlot::DragLineY(0, &drag_tag, ImVec4(1, 0, 0, 1), 1, ImPlotDragToolFlags_NoFit);
                ImPlot::TagY(drag_tag, ImVec4(1, 0, 0, 1), "");

                uint32_t rounded_row = (uint32_t)round(texture.getSize().y - drag_tag);

                const uint8_t *image_pixels = img.getPixelsPtr();
                const uint8_t *row = image_pixels + rounded_row * 4;

                brightness.clear();
                x_values.clear();
                for (uint32_t px = 0; px < texture.getSize().x; px++)
                {
                    sf::Color col = img.getPixel(px, rounded_row);
                    // uint32_t cur_byte = px * 4;
                    HSV var = rgbToHsv(col.r, col.g, col.b);
                    brightness.push_back(var.v);
                    x_values.push_back(px);
                }

                ImPlot::EndPlot();
            }
            ImGui::End();
        }
        if (ImGui::Begin("Spectrum"))
        {
            if (ImPlot::BeginPlot("##", ImVec2(-1, -1)))
            {
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 4);
                ImPlot::PlotLine("Test", x_values.data(), brightness.data(), brightness.size());
                ImPlot::EndPlot();
                ImPlot::PopStyleVar();
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