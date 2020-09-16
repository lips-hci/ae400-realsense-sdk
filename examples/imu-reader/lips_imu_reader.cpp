// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2019 LIPS Corporation. All Rights Reserved.
// Get imu data by enable_stream RS2_STREAM_DEPTH.

#include <iostream>
#include <librealsense2/rs.hpp>
#include <librealsense2/lips_ae400_imu.h>

#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define IMG_HD_WIDTH 1280
#define IMG_HD_HEIGHT 720
#define IMG_FPS_15 15
#define IMG_FPS 30
#define IMG_FPS_DOU 60
#define IMG_FPS_TRI 90

using namespace lips::ae400;
using namespace std;
enum showOp
{
	DEPTH = 1,
	IMAGE = 2,
	IR = 4
};

int getUserInput()
{
	int option = 0;
#if 0
	cout << "1) Depth only" << endl;
	cout << "2) Image only" << endl;
	cout << "3) IR only" << endl;
	cout << "4) Depth and Image" << endl;
	cout << "5) Depth and IR" << endl;
	cout << "6) Image and IR" << endl;
	cout << "7) All" << endl;
	cout << "0) Exit" << endl;
	cout << "Please input your choice : ";
#endif
	//cin >> option;
	option=1; //depth only
	switch (option)
	{
	case 1:
		return DEPTH;
		break;
	case 2:
		return IMAGE;
		break;
	case 3:
		return IR;
		break;
	case 4:
		return (DEPTH + IMAGE);
		break;
	case 5:
		return (DEPTH + IR);
		break;
	case 6:
		return (IMAGE + IR);
		break;
	case 7:
		return (DEPTH + IMAGE + IR);
		break;
	case 0:
		return 0;
		break;
	default:
		return getUserInput();
		break;
	};
}

void showResolution(int option)
{
	switch (option)
	{
	case DEPTH:
		cout << "720P ( 1280 x 720 ), FPS = 15" << endl;
		cout << "720P ( 1280 x 720 ), FPS = 30" << endl;
		cout << "VGA ( 640 x 480 ), FPS = 90" << endl;
		cout << "VGA ( 640 x 480 ), FPS = 60" << endl;
		cout << "VGA ( 640 x 480 ), FPS = 30" << endl;
		break;
	case IMAGE:
		cout << "720P ( 1280 x 720 ), FPS = 15" << endl;
		cout << "720P ( 1280 x 720 ), FPS = 30" << endl;
		cout << "VGA ( 640 x 480 ), FPS = 60" << endl;
		cout << "VGA ( 640 x 480 ), FPS = 30" << endl;
		break;
	case IR:
		cout << "720P ( 1280 x 720 ), FPS = 15" << endl;
		cout << "720P ( 1280 x 720 ), FPS = 30" << endl;
		cout << "VGA ( 640 x 480 ), FPS = 90" << endl;
		cout << "VGA ( 640 x 480 ), FPS = 60" << endl;
		cout << "VGA ( 640 x 480 ), FPS = 30" << endl;
		break;
	}
}

int main()
{

	int option = getUserInput();

	if (0 == option)
	{
		cout << "Exit program!" << endl;
		return 0;
	}
	double DepthWidthMultiple = 1;
	double DepthHeightMultiple = 1;
	double DepthFpsMultiple = 1;
	double RGBWidthMultiple = 1;
	double RGBHeightMultiple = 1;
	double RGBFpsMultiple = 1;
	double IRWidthMultiple = 1;
	double IRHeightMultiple = 1;
	double IRFpsMultiple = 1;

	//Contruct a pipeline which abstracts the device
	rs2::pipeline pipe;

	//Create a configuration for configuring the pipeline with a non default profile
	rs2::config cfg;

	rs2::log_to_console(RS2_LOG_SEVERITY_ERROR);
	if (option & DEPTH)
	{
		int answer = 1;
		//system("CLS");
		//std::cout << "Available options for Depth : " << endl;
		//showResolution(DEPTH);
		//std::cin >> answer;
		answer=5;//vga resolution
		switch (answer)
		{
		case 1:
			cfg.enable_stream(RS2_STREAM_DEPTH, IMG_HD_WIDTH, IMG_HD_HEIGHT, RS2_FORMAT_Z16, IMG_FPS_15);
			DepthWidthMultiple = 2;
			DepthHeightMultiple = 1.5;
			break;

		case 2:
			cfg.enable_stream(RS2_STREAM_DEPTH, IMG_HD_WIDTH, IMG_HD_HEIGHT, RS2_FORMAT_Z16, IMG_FPS);
			DepthWidthMultiple = 2;
			DepthHeightMultiple = 1.5;
			break;
		case 3:
			cfg.enable_stream(RS2_STREAM_DEPTH, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_Z16, IMG_FPS_TRI);
			DepthFpsMultiple = 3;
			break;
		case 4:
			cfg.enable_stream(RS2_STREAM_DEPTH, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_Z16, IMG_FPS_DOU);
			DepthFpsMultiple = 2;
			break;
		case 5:
			cfg.enable_stream(RS2_STREAM_DEPTH, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_Z16, IMG_FPS);
			break;
		default:
			std::cout << "Wrong input value, use default" << endl;
			cfg.enable_stream(RS2_STREAM_DEPTH, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_Z16, IMG_FPS);
			break;
		}

	}
	if (option & IMAGE)
	{
		int answer = 1;
		std::cout << "Available options for Depth : " << endl;
		showResolution(IMAGE);
		std::cin >> answer;
		switch (answer)
		{
		case 1:
			cfg.enable_stream(RS2_STREAM_COLOR, IMG_HD_WIDTH, IMG_HD_HEIGHT, RS2_FORMAT_RGB8, IMG_FPS_15);
			RGBWidthMultiple = 2;
			RGBHeightMultiple = 1.5;
			break;
		case 2:
			cfg.enable_stream(RS2_STREAM_COLOR, IMG_HD_WIDTH, IMG_HD_HEIGHT, RS2_FORMAT_RGB8, IMG_FPS);
			RGBWidthMultiple = 2;
			RGBHeightMultiple = 1.5;
			break;
		case 3:
			cfg.enable_stream(RS2_STREAM_COLOR, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_RGB8, IMG_FPS_DOU);
			RGBFpsMultiple = 2;
			break;
		case 4:
			cfg.enable_stream(RS2_STREAM_COLOR, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_RGB8, IMG_FPS);
			break;
		default:
			std::cout << "Wrong input value, use default" << endl;
			cfg.enable_stream(RS2_STREAM_COLOR, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_RGB8, IMG_FPS);
			break;
		}
	}
	if (option & IR)
	{
		int answer = 1;
		std::cout << "Available options for IR : " << endl;
		showResolution(IR);
		std::cin >> answer;
		switch (answer)
		{
		case 1:
			cfg.enable_stream(RS2_STREAM_INFRARED, IMG_HD_WIDTH, IMG_HD_HEIGHT, RS2_FORMAT_Y8, IMG_FPS_15);
			IRWidthMultiple = 2;
			IRHeightMultiple = 1.5;
			break;
		case 2:
			cfg.enable_stream(RS2_STREAM_INFRARED, IMG_HD_WIDTH, IMG_HD_HEIGHT, RS2_FORMAT_Y8, IMG_FPS);
			IRWidthMultiple = 2;
			IRHeightMultiple = 1.5;
			break;
		case 3:
			cfg.enable_stream(RS2_STREAM_INFRARED, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_Y8, IMG_FPS_TRI);
			IRFpsMultiple = 3;
			break;
		case 4:
			cfg.enable_stream(RS2_STREAM_INFRARED, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_Y8, IMG_FPS_DOU);
			IRFpsMultiple = 2;
			break;
		case 5:
			cfg.enable_stream(RS2_STREAM_INFRARED, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_Y8, IMG_FPS);
			break;
		default:
			std::cout << "Wrong input value, use default" << endl;
			cfg.enable_stream(RS2_STREAM_INFRARED, IMG_WIDTH, IMG_HEIGHT, RS2_FORMAT_Y8, IMG_FPS);
			break;
		}
	}
	pipe.start(cfg);
	rs2::frameset frames;
	while (true)
	{
		frames = pipe.wait_for_frames();
		//Get each frame
		rs2::frame depth_frame = frames.get_depth_frame();
		rs2::frame rgb_frame = frames.get_color_frame();
		rs2::frame ir_frame = frames.get_infrared_frame();

        lips_ae400_imu data;
		if (depth_frame && get_imu_data(0, &data) == 0)
		{
			printf("IMU accel (x,y,z,timestamp) = (%.3f, %.3f, %.3f, %llu)\n ",
                    data.accel_x, data.accel_y, data.accel_z, data.timestamp);
			printf("IMU gyro  (x,y,z,timestamp) = (%.3f, %.3f, %.3f, %llu)\n ",
                    data.gyro_x, data.gyro_y, data.gyro_z, data.timestamp);
		}
	}
	return 0;
}

