// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "example.hpp"          // Include short list of convenience functions for rendering
#include <iostream>
#include <cstring>
#include <fstream>

// Helper functions
void register_glfw_callbacks(window& app, glfw_state& app_state);

bool bExit = false;
bool bSaveToCsv = false;

const int w = 640;
const int h = 480;

char csvfilename[60] = "lips_fps_results.csv";
std::fstream csvfile;

// Key handler
void onKeyRelease(int key)
{
    if (key == 79) //O/o
    {
        if (!bSaveToCsv)
        {
            csvfile.open(csvfilename, std::fstream::out | std::fstream::trunc);
            csvfile << "localtime, frames-per-second, milliseconds-per-frame\n";
            std::cout << "Start writing data to " << csvfilename << "..." << std::endl;
            bSaveToCsv = true;
        }
    }
    else if ((key == 32) || (key == 81)) // Escape or Q/q
    {
        if (bSaveToCsv)
        {
            // close csv file
            if (csvfile.is_open())
            {
                std::cout << "Stop writing file " << csvfilename << "..." << std::endl;
                csvfile.close();
            }
        }
        bExit = true;
    }
    return;
}

// Capture Example demonstrates how to
// capture depth and color video streams and render them to the screen
int main(int argc, char * argv[]) try
{
    rs2::log_to_console(RS2_LOG_SEVERITY_ERROR);
    // Create a simple OpenGL window for rendering:
    window app(1280, 720, "LIPSedge AE400 Example FPS-Viewer (key 'o' to save result into csv text)");

    // register callbacks to allow manipulation of the pointcloud
    app.on_key_release = onKeyRelease;

    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;
    // Declare rates printer for showing streaming rates of the enabled streams.
    //rs2::rates_printer printer;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;
    // Start streaming with default recommended configuration
    //pipe.start();

    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_DEPTH, w, h, RS2_FORMAT_Z16, 30);
    cfg.enable_stream(RS2_STREAM_COLOR, w, h, RS2_FORMAT_RGB8, 30);
    // Start streaming with the above configuration
    pipe.start(cfg);

    const int MAX_SECS = 1; // set to 1 seconds
    int counter = 0;

    int writelines = 0;
    std::sprintf(csvfilename + std::strlen(csvfilename) - 4, "_%dx%d.csv", w, h );
    std::cout << std::endl;

    auto start_time = std::chrono::steady_clock::now();
    while (app) // Application still alive?
    {
        if (bExit)
        {
            std::cout << "Exit program." << std::endl;
            break;
        }

        rs2::frameset data = pipe.wait_for_frames().    // Wait for next set of frames from the camera
                             //apply_filter(printer).     // Print each enabled stream frame rate
                             apply_filter(color_map);   // Find and colorize the depth data

        counter += 1;
        auto curr_time = std::chrono::steady_clock::now();
        double diffms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_time - start_time).count();
        float diff = diffms/1000.0;
        if (diff > MAX_SECS)
        {
            std::time_t t =  std::time(NULL);
            std::tm tm    = *std::localtime(&t);

            if (bSaveToCsv)
            {
                if (csvfile.is_open())
                {
                    writelines += 1;

                    std::cout << " => write record " << writelines << " : ";
                    std::cout << std::put_time(&tm, "%Y-%m-%d.%H:%M:%S");
                    std::cout << "," << std::setprecision(5) << counter/diff;
                    std::cout << "," << std::setprecision(6) << diffms/counter << std::endl;

                    csvfile << std::put_time(&tm, "%Y-%m-%d.%H:%M:%S");
                    csvfile << "," << std::setprecision(5) << counter/diff;
                    csvfile << "," << std::setprecision(6) << diffms/counter << std::endl;
                }
            }
            else
            {
                std::cout << std::put_time(&tm, "%Y-%m-%d.%H:%M:%S") << ", ";
                std::cout << "(" << w << "x" << h << ")";
                std::cout << ", " << std::setprecision(4) << counter/diff << " FPS";
                std::cout << " (" << std::setprecision(5) << diffms/counter << "ms)" << std::endl;
            }

            counter = 0;
            start_time = std::chrono::steady_clock::now();
        }

        // The show method, when applied on frameset, break it to frames and upload each frame into a gl textures
        // Each texture is displayed on different viewport according to it's stream unique id
        app.show(data);
    }

    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}