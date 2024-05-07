#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <tuple>
#include <ranges>
#include <iterator>

#include <SFML/Graphics.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "imgui-SFML.h"
#include "imgui.h"
#include "implot.h"

#include "inspect_config.h"
#include "inspect_styles.hpp"

#include "utility.hpp"

#include "spectrum.hpp"

int close_app(sf::RenderWindow &window);

int main(int argc, char *argv[])
{
    sf::ContextSettings settings;
    settings.antialiasingLevel = INSPECT_ANTIALIASING_LEVEL;

    sf::RenderWindow window(sf::VideoMode(1280, 720), std::string("inspect | spectroscopy software | ") + INSPECT_VERSION, sf::Style::Default, settings);

    window.setFramerateLimit(INSPECT_MAX_FPS);
    window.setVerticalSyncEnabled(INSPECT_VSYNC);

    if (!ImGui::SFML::Init(window))
    {
        std::cout << "IMGUI SFML Window Init failed!" << std::endl;
        return -1;
    }

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("opensans.ttf", 20);

    if (!ImGui::SFML::UpdateFontTexture())
    {
        std::cout << "Could not load font!\n";
        return -1;
    }

    ImGui::CreateContext();
    ImPlot::CreateContext();

    set_style(ULTRA_DARK);

    Spectrum spectrum;
    spectrum.load_spectrum("spec2.jpg");

    bool data_updated = true;

    static double drag_tag = spectrum.get_image().height() / 2;
    static point_pair_t measure_points;

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
                if (ImGui::MenuItem("Open", ""))
                {
                    std::string path = open_file_explorer(0);
                    spectrum.load_spectrum(path.c_str());
                    drag_tag = spectrum.get_image().height() / 2;
                    data_updated = true;
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Style"))
            {
                if (ImGui::MenuItem("Default", ""))
                {
                    set_style(DEFAULT_STYLE);
                }
                if (ImGui::MenuItem("Ultra Dark", ""))
                {
                    set_style(ULTRA_DARK);
                }

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        static int selector_width = 1;

        static bool measure_markers = true;

        if (ImGui::Begin("Spectrum"))
        {
            static float row_ratios[3] = {0.25, 0.05, 0.7};
            static double x_min = 0;
            static double x_max = spectrum.get_image().width();

            if (ImPlot::BeginSubplots("##", 3, 1, ImVec2(-1, -1), ImPlotSubplotFlags_NoResize, row_ratios))
            {
                if (data_updated)
                {
                    ImPlot::SetNextAxesToFit();
                }
                if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend | ImPlotFlags_Equal | ImPlotFlags_CanvasOnly))
                {
                    ImPlotAxisFlags ax_flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks | ImPlotAxisFlags_NoGridLines;
                    ImPlot::SetupAxes(0, 0, ax_flags, ax_flags);
                    ImPlot::PlotImage("Spectrum", spectrum.get_image().handle, ImPlotPoint(0, 0), ImPlotPoint(spectrum.get_image().width(), spectrum.get_image().height()), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1, 1, 1, 1));

                    ImPlot::DragLineY(0, &drag_tag, ImVec4(1, 0, 0, 1), selector_width, ImPlotDragToolFlags_NoFit);
                    // ImPlot::TagY(drag_tag, ImVec4(1, 0, 0, 1), " ");

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

                if (data_updated)
                {
                    ImPlot::SetNextAxesToFit();
                    data_updated = false;
                }

                if (ImPlot::BeginPlot("##", ImVec2(-1, -1), ImPlotFlags_NoFrame | ImPlotFlags_NoLegend))
                {
                    ImPlot::SetupAxis(ImAxis_Y1, "Relative Brightness");
                    ImPlot::SetupAxis(ImAxis_X1, "Pixel");
                    ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 0, spectrum.get_image().width() - 1);
                    ImPlot::SetupAxisLimitsConstraints(ImAxis_Y1, -0.05, 1.1);
                    ImPlot::SetupAxisZoomConstraints(ImAxis_Y1, -0.05, 1.1);
                    ImPlot::SetupAxisLinks(ImAxis_X1, &x_min, &x_max);

                    ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 4);
                    auto spectrum_data = spectrum.get_plot_data();
                    ImPlot::PlotLine("Spectrum Plot", std::get<0>(spectrum_data).data(), std::get<1>(spectrum_data).data(), std::get<0>(spectrum_data).size());
                    ImPlot::PopStyleVar();

                    if (measure_markers)
                    {
                        for (size_t mp = 0; mp < measure_points.x.size(); mp++)
                        {
                            ImPlot::DragLineX(mp, &measure_points.x[mp], ImVec4(1, 1, 1, 1), 2);
                            std::string marker_name = "X" + std::to_string(mp);
                            ImPlot::TagX(measure_points.x[mp], ImVec4(1, 1, 1, 1), marker_name.c_str());
                        }

                        measure_points.y = spectrum.set_measure_markers(measure_points.x);
                    }

                    ImPlot::EndPlot();
                }

                ImPlot::EndSubplots();
            }

            ImGui::End();
        }

        static float alpha = 0.5;
        static bool interpolate = false;
        static bool low_pass = false;
        static bool calib_markers = false;

        if (ImGui::Begin("Toolbar"))
        {
            ImGui::SeparatorText("Image Settings");
            static bool smooth = false;
            if (ImGui::Checkbox("Smooth Image (Preview only)", &smooth))
            {
                spectrum.set_image_preview_smooth(smooth);
            }

            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::SeparatorText("Data Selection");
            ImGui::SliderInt("Selector Width", &selector_width, 1, 10);

            ImGui::Dummy(ImVec2(0.0f, 20.0f));
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

            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::BeginDisabled();
            ImGui::SeparatorText("Wavelength Calibration");
            if (ImGui::Checkbox("Show Markers##Calib", &calib_markers))
            {
            }
            ImGui::EndDisabled();

            ImGui::Dummy(ImVec2(0.0f, 20.0f));
            ImGui::SeparatorText("Measure");
            if (ImGui::Checkbox("Show Markers##Measure", &measure_markers))
            {
            }

            ImGui::SameLine();

            if (ImGui::Button("Add"))
            {
                measure_points.x.push_back(0);
                measure_points.y.push_back(0);
            }

            ImGui::SameLine();

            if (ImGui::Button("Clear All"))
            {
                measure_points.x.clear();
                measure_points.y.clear();
            }

            std::vector<size_t> to_delete;
            for (size_t mp = 0; mp < measure_points.x.size(); mp++)
            {
                ImGui::Text("M%lld : ", mp);
                ImGui::SameLine();

                ImGui::Text("X : %.2f | Y : %.3f", measure_points.x[mp], measure_points.y[mp]);

                ImGui::SameLine();
                std::string button_name = "Delete##" + std::to_string(mp);
                if (ImGui::Button(button_name.c_str()))
                {
                    to_delete.push_back(mp);
                }
            }

            delete_at_indices(measure_points.x, to_delete);
            delete_at_indices(measure_points.y, to_delete);

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
