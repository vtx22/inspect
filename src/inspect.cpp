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
#include "utility.hpp"

#include "spectrum.hpp"

int close_app(sf::RenderWindow &window);

int main(int argc, char *argv[])
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8.0;

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

    Spectrum spectrum;
    spectrum.load_spectrum("spec2.jpg");
    spectrum.update_raw_data();

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

        static float alpha = 0.5;
        static bool interpolate = false;
        static bool low_pass = false;
        static int selector_width = 1;

        if (ImGui::Begin("Spectrum"))
        {
            static float row_ratios[3] = {0.25, 0.05, 0.7};
            static double x_min = 0;
            static double x_max = spectrum.get_image().width();
            static double drag_tag = spectrum.get_image().height() / 2;

            if (ImPlot::BeginSubplots("##", 3, 1, ImVec2(-1, -1), ImPlotSubplotFlags_NoResize | ImPlotFlags_NoFrame, row_ratios))
            {
                if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend | ImPlotFlags_Equal | ImPlotFlags_CanvasOnly))
                {
                    ImPlotAxisFlags ax_flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines;
                    ImPlot::SetupAxes(0, 0, ax_flags, ax_flags);
                    ImPlot::PlotImage("Spectrum", spectrum.get_image().handle, ImPlotPoint(0, 0), ImPlotPoint(spectrum.get_image().width(), spectrum.get_image().height()), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));

                    ImPlot::DragLineY(0, &drag_tag, ImVec4(1, 0, 0, 1), selector_width, ImPlotDragToolFlags_NoFit);
                    ImPlot::TagY(drag_tag, ImVec4(1, 0, 0, 1), " ");

                    spectrum.select_line(drag_tag, 1);

                    ImPlot::EndPlot();
                }

                if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend | ImPlotFlags_CanvasOnly | ImPlotFlags_NoInputs))
                {
                    ImPlotAxisFlags ax_flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines;
                    ImPlot::SetupAxes(0, 0, ax_flags, ax_flags);
                    ImPlot::SetupAxisLinks(ImAxis_X1, &x_min, &x_max);
                    ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, 1, 1);
                    ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 0, spectrum.get_line().width());
                    ImPlot::PlotImage("Spectrum", spectrum.get_line().handle, ImPlotPoint(0, 0), ImPlotPoint(spectrum.get_line().width(), spectrum.get_line().height()), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));

                    ImPlot::EndPlot();
                }

                if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend))
                {
                    ImPlot::SetupAxis(ImAxis_Y1, "Relative Brightness");
                    ImPlot::SetupAxis(ImAxis_X1, "Pixel");
                    ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, spectrum.get_image().width() - 1);
                    ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, 0, 1.1);
                    ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, 0, 1.1);
                    ImPlot::SetupAxisLinks(ImAxis_X1, &x_min, &x_max);
                    ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 4);

                    ImPlot::PlotLine("Spectrum Plot", spectrum.get_plot_x().data(), spectrum.get_plot_data().data(), spectrum.get_plot_data().size());

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
            if (ImGui::Checkbox("Smooth Image (Preview only)", &smooth))
            {
                spectrum.set_image_preview_smooth(smooth);
            }

            ImGui::SeparatorText("Data Selection");
            ImGui::SliderInt("Selector Width", &selector_width, 1, 10);

            ImGui::SeparatorText("Data Manipulation");

            if (ImGui::Checkbox("Low Pass Filter", &low_pass))
            {
                spectrum.set_lp_filtering(low_pass);
                spectrum.update_lp_filter(alpha);
            }

            if (!low_pass)
            {
                ImGui::BeginDisabled();
            }

            if (ImGui::SliderFloat("Filter Alpha", &alpha, 0.0001, 1))
            {
                spectrum.update_lp_filter(alpha);
            }

            if (!low_pass)
            {
                ImGui::EndDisabled();
            }

            if (ImGui::Checkbox("Interpolate", &interpolate))
            {
                spectrum.set_interpolation(interpolate);
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