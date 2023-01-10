// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2023 LIPS Corporation. All Rights Reserved.
//
// This example is to demo how to recover camera and realsense
// connection if network has problem
//
// Test case:
//   you can run this sample to query depth or color frames, and randomly
//   unplug ethernet cable from your PC/NB side to simulate broken network
//   or unstable network problems
//
//   This sample should reconnect camera and restart realsense tasks successfully
//   your depth streaming should go back
//
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <iostream>             // for cout


bool safe_stop_pipeline(rs2::pipeline &p)
{
    try
    {
        std::cout << "Trying to stop pipeline. Please wait..." << std::endl;
        std::cout << "pipeline Stopping" << std::endl;
        p.stop();
        std::cout << "pipeline Stopped" << std::endl;

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

// Hello RealSense example demonstrates the basics of connecting to a RealSense device
// and taking advantage of depth data
int main(int argc, char * argv[]) try
{
    rs2::log_to_console(RS2_LOG_SEVERITY_ERROR);
    std::cout << "Distance-read Example is Running..." << std::endl;

    int start_retry = 0;

    while (true) {
        // Create a Pipeline - this serves as a top-level API for streaming and processing frames
        rs2::pipeline p;

        try {
            // Configure and start the pipeline
            std::cout << "pipeline Starting" << std::endl;
            p.start();
            std::cout << "pipeline Started" << std::endl;
            start_retry = 0;
        }
        catch (const rs2::error & e)
        {
            std::cout << "pipeline start - FAIL" << std::endl;
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
            while (true)
            {
                // Block program until frames arrive
                rs2::frameset frames = p.wait_for_frames(5000);
                // Try to get a frame of a depth image
                rs2::depth_frame depth = frames.get_depth_frame();
                // Get the depth frame's dimensions
                auto width = depth.get_width();
                auto height = depth.get_height();
                // Query the distance from the camera to the object in the center of the image
                float dist_to_center = depth.get_distance(width / 2, height / 2);
                // Print the distance
                std::cout << "The camera is facing an object " << dist_to_center << " meters away \n";
            }
            return EXIT_SUCCESS;
        }
        catch (const rs2::error & e)
        {
            std::cerr << "RealSense error calling " << e.get_failed_function();
            std::cerr << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;

            if ( safe_stop_pipeline(p) )
            {
                //Next step is to re-start pipe
                std::cout << "Trying to restart pipeline. Please wait for 5 seconds..." << std::endl;
                continue;
            }
            else
            {
                std::cerr << "We're doomed! Cannot stop pipeline..." << std::endl;
                break;
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
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