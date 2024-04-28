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

#include "interpolation.hpp"

int close_app(sf::RenderWindow &window);

int main(int argc, char *argv[])
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 16.0;

    sf::RenderWindow window(sf::VideoMode(1280, 720), std::string("inspect | spectroscopy software | ") + INSPECT_VERSION, sf::Style::Default, settings);

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
    sf::Texture texture_rgb_line;

    if (!img.loadFromFile("spec2.jpg"))
    {
    }

    texture.loadFromImage(img);
    texture_rgb_line.create(texture.getSize().x, 1);

    ImTextureID img_handle = gl_handle_to_imgui_id(texture.getNativeHandle());

    std::vector<float> brightness, brightness_interp;
    std::vector<float> x_values, x_values_interp;

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

            ImGui::EndMainMenuBar();
        }
        static float alpha = 1;
        static bool interpolate = false;
        static bool low_pass = false;
        if (ImGui::Begin("Spectrum"))
        {
            static float row_ratios[3] = {0.25, 0.05, 0.7};
            static double x_min = 0;
            static double x_max = texture.getSize().x;

            if (ImPlot::BeginSubplots("##", 3, 1, ImVec2(-1, -1), ImPlotSubplotFlags_NoResize | ImPlotFlags_NoFrame, row_ratios))
            {
                if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend | ImPlotFlags_Equal | ImPlotFlags_CanvasOnly))
                {
                    ImPlotAxisFlags ax_flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines;
                    ImPlot::SetupAxes(0, 0, ax_flags, ax_flags);
                    ImPlot::PlotImage("Spectrum", img_handle, ImPlotPoint(0, 0), ImPlotPoint(texture.getSize().x, texture.getSize().y), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));

                    static double drag_tag = texture.getSize().y / 2;
                    ImPlot::DragLineY(0, &drag_tag, ImVec4(1, 0, 0, 1), 1, ImPlotDragToolFlags_NoFit);
                    ImPlot::TagY(drag_tag, ImVec4(1, 0, 0, 1), " ");

                    uint32_t rounded_row = (uint32_t)(texture.getSize().y - drag_tag);

                    brightness.clear();
                    x_values.clear();

                    const uint8_t *row_start = img.getPixelsPtr() + rounded_row * 4 * texture.getSize().x;
                    texture_rgb_line.update(row_start);

                    texture_rgb_line.create(texture.getSize().x, 1);
                    /*
                    for (uint8_t i = 0; i < 10; i++)
                    {

                        texture_rgb_line.update(row_start, texture.getSize().x, 1, 0, i);
                    }
                    */

                    for (uint32_t px = 0; px < texture.getSize().x; px++)
                    {
                        uint8_t r = *(row_start + px * 4);
                        uint8_t g = *(row_start + px * 4 + 1);
                        uint8_t b = *(row_start + px * 4 + 2);
                        HSV var = rgbToHsv(r, g, b);
                        brightness.push_back(var.v);
                        x_values.push_back(px);
                    }

                    ImPlot::EndPlot();
                }

                if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend | ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs))
                {
                    ImPlotAxisFlags ax_flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines;
                    ImPlot::SetupAxes(0, 0, ax_flags, ax_flags);
                    ImPlot::SetupAxisLinks(ImAxis_X1, &x_min, &x_max);
                    ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, 1, 1);
                    ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 0, texture.getSize().x);
                    ImPlot::PlotImage("Spectrum", gl_handle_to_imgui_id(texture_rgb_line.getNativeHandle()), ImPlotPoint(0, 0), ImPlotPoint(texture_rgb_line.getSize().x, texture_rgb_line.getSize().y), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));

                    ImPlot::EndPlot();
                }

                if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend))
                {
                    // ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, 0, 1.1);
                    // ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 0, texture.getSize().x);
                    ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, texture.getSize().x);
                    ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0, 1.1);
                    ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, 0, 1.1);
                    ImPlot::SetupAxisLinks(ImAxis_X1, &x_min, &x_max);
                    ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 4);

                    x_values.clear();
                    for (size_t i = 0; i < brightness.size(); i++)
                    {
                        x_values.push_back(i);
                    }

                    if (low_pass)
                    {
                        brightness = low_pass_filer(brightness, alpha);
                    }

                    // Interpolation
                    if (interpolate)
                    {
                        x_values_interp.clear();
                        brightness_interp = cubic_spline_interpolate(brightness, 10);

                        for (size_t i = 0; i < brightness_interp.size(); i++)
                        {
                            x_values_interp.push_back(1 + i * 1.0 / 10);
                        }
                        ImPlot::PlotLine("Spectrum Interpolated", x_values_interp.data(), brightness_interp.data(), brightness_interp.size());
                    }
                    else
                    {
                        ImPlot::PlotLine("Spectrum Plot", x_values.data(), brightness.data(), brightness.size());
                    }

                    //  ImPlot::PlotLine("Spectrum Reduced", x_values.data(), reduced_brightness_vector.data(), reduced_brightness_vector.size());
                    ImPlot::PopStyleVar();

                    ImPlot::EndPlot();
                }

                ImPlot::EndSubplots();
            }

            ImGui::End();
        }

        if (ImGui::Begin("Toolbar"))
        {
            ImGui::SeparatorText("Image Settings");
            static bool smooth = false;
            if (ImGui::Checkbox("Smooth Image", &smooth))
            {
                texture.setSmooth(smooth);
            }

            ImGui::SeparatorText("Data Manipulation");

            if (ImGui::Checkbox("Low Pass Filter", &low_pass))
            {
            }
            ImGui::SliderFloat("Filter Alpha", &alpha, 0.0001, 1);

            if (ImGui::Checkbox("Interpolate", &interpolate))
            {
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