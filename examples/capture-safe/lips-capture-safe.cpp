// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2023 LIPS Corporation. All Rights Reserved.
//
// This example is to demo how to recover camera and realsense
// connection if network has problem
//
// Test case:
//   you can run this sample to query depth and color frames, and randomly
//   unplug ethernet cable from your PC/NB side to simulate broken network
//   or unstable network problems
//
//   This sample should reconnect camera and restart realsense tasks successfully
//   your depth streaming should go back
//
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "example.hpp"          // Include short list of convenience functions for rendering


bool safe_stop_pipeline(rs2::pipeline &p)
{
    try
    {
        std::cout << "Trying to stop pipeline. Please wait..." << std::endl;
        std::cout << "pipeline safe stopping..." << std::endl;
        p.stop();
        std::cout << "pipeline safe stopped!" << std::endl;

        return true;
    }
    catch(const rs2::error &e)
    {
        std::cout << "pipeline stop - FAIL!" << std::endl;
        std::cerr << "RealSense error calling " << e.get_failed_function();
        std::cerr << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    }

    return false;
}

// Capture Example demonstrates how to
// capture depth and color video streams and render them to the screen
int main(int argc, char * argv[]) try
{
    rs2::log_to_console(RS2_LOG_SEVERITY_ERROR);
    // Create a simple OpenGL window for rendering:
    window app(1280, 720, "RealSense Capture Example");

    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;
    // Declare rates printer for showing streaming rates of the enabled streams.
    //rs2::rates_printer printer;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    //rs2::pipeline pipe;

    int start_retry = 0;

    while (true)
    {
        // Declare rates printer for showing streaming rates of the enabled streams.
        rs2::rates_printer printer;

        // Declare RealSense pipeline, encapsulating the actual device and sensors
        rs2::pipeline pipe;

        try
        {
            // Start streaming with default recommended configuration
            // The default video configuration contains Depth and Color streams
            // If a device is capable to stream IMU data, both Gyro and Accelerometer are enabled by default
            pipe.start();
            std::cout << "pipeline safe started." << std::endl;
            start_retry = 0;
        }
        catch (const rs2::error & e)
        {
            std::cout << "pipeline start - FAIL!" << std::endl;
            std::cerr << "RealSense error calling " << e.get_failed_function();
            std::cerr << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;

            if( std::string("No device connected").compare(e.what()) == 0 )
            {
                std::cout << "[TODO] we can try rebooting remote camera (retry:" << ++start_retry << ")" << std::endl;
                if (start_retry > 2 )
                {
                    //TODO: try use web-gui to reboot remote device to fix this situation, the socket server may dead or in loop
                    std::cout << "[TODO] reboot the LIPSedge AE series camera" << std::endl;
                }
            }

            //since pipe didn't start correctedly, we don't have to stop it
            continue;
        }

        try
        {
            while (app)// Application still alive?
            {
                rs2::frameset data = pipe.wait_for_frames(5000).// Wait for next set of frames from the camera
                                    apply_filter(printer).     // Print each enabled stream frame rate
                                    apply_filter(color_map);   // Find and colorize the depth data

                // The show method, when applied on frameset, break it to frames and upload each frame into a gl textures
                // Each texture is displayed on different viewport according to it's stream unique id
                app.show(data);
            }
        }
        catch (const rs2::error & e)
        {
            std::cerr << "RealSense error calling " << e.get_failed_function();
            std::cerr << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;

            if (safe_stop_pipeline(pipe))
            {
                //Next step is to re-start pipe
                std::cout << "Trying to restart pipeline. Please wait for 5 seconds..." << std::endl;
                continue;
            }
            else
            {
                std::cout << "We're doomed! Cannot recover pipeline..." << std::endl;
                break;
            }
        }
    }

    std::cout << "Bye Bye!" << std::endl;
    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function();
    std::cerr << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}