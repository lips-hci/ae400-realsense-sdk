## License: Apache 2.0. See LICENSE file in root directory.
## Copyright(c) 2015-2017 Intel Corporation. All Rights Reserved.

###############################################
##      Open CV and Numpy integration        ##
###############################################

"""
This sample is a simple OpenCV viewer showing frame-rate number.

Keyboard:
    [o]     Save fps number and timestamp to .csv file (./fps_results.csv)
    [q\ESC] Quit
"""
from __future__ import print_function
import math
import time
import cv2
import numpy as np
import pyrealsense2 as rs
import csv
import sys

if sys.version_info[0] == 2: # python 2.x
	argOpen = 'wb'
	argAppd = 'ab'
	kwargs = {}
else:
	argOpen = 'w'
	argAppd = 'a'
	kwargs = {'newline':''}

# Configure depth and color streams
pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.depth, 1280, 720, rs.format.z16, 30)
config.enable_stream(rs.stream.color, 1280, 720, rs.format.bgr8, 30)

# Start streaming
pipeline.start(config)

# Get stream profile and camera intrinsics
profile = pipeline.get_active_profile()
depth_profile = rs.video_stream_profile(profile.get_stream(rs.stream.depth))
depth_intrinsics = depth_profile.get_intrinsics()
w, h = depth_intrinsics.width, depth_intrinsics.height

# Display frame-rate every 1 second
x = 1
counter = 0

WIN_NAME = 'RealSense'
cv2.namedWindow(WIN_NAME, cv2.WINDOW_AUTOSIZE)

# Start time
start = time.time()

# Default write-to-csv feature is OFF
savetocsv = False
writelines = 0
csvfilename = "fps_results_%dx%d.csv" %(w, h)

try:
    while True:

        # Wait for a coherent pair of frames: depth and color
        frames = pipeline.wait_for_frames()
        depth_frame = frames.get_depth_frame()
        color_frame = frames.get_color_frame()
        if not depth_frame or not color_frame:
            continue

        # Convert images to numpy arrays
        depth_image = np.asanyarray(depth_frame.get_data())
        color_image = np.asanyarray(color_frame.get_data())

        # Apply colormap on depth image (image must be converted to 8-bit per pixel first)
        depth_colormap = cv2.applyColorMap(cv2.convertScaleAbs(depth_image, alpha=0.03), cv2.COLORMAP_JET)


        # Stack both images horizontally
        images = np.hstack((color_image, depth_colormap))

        counter+=1
        # Time elapsed
        now = time.time()
        dt = now - start
        if (dt) > x :
            cv2.setWindowTitle(WIN_NAME, "RealSense (%dx%d) %.2f FPS (%.3fms)" %(w, h, counter/dt, (dt*1000)/counter))

            if savetocsv:
                strlocaltime = time.strftime("%Y-%m-%d.%H:%M:%S",time.localtime(now))
                with open(csvfilename, argAppd, **kwargs) as csvfile:
                    writer = csv.writer(csvfile, delimiter=',')
                    writer.writerow([strlocaltime,'{:.2f}'.format(counter/dt),'{:.3f}'.format((dt*1000)/counter)])
                    writelines+=1
                    print(" => write record %d : %s,%.2f,%.3f" %(writelines, strlocaltime, counter/dt, (dt*1000)/counter))

            start = time.time()
            counter = 0

        # Show images
        cv2.imshow(WIN_NAME, images)
        key = cv2.waitKey(1)

        # key handlers
        if key == ord("o"):
            with open(csvfilename, argOpen, **kwargs) as csvfile:
                writer = csv.writer(csvfile, delimiter=',')
                writer.writerow(['localtime', 'frames-per-second', 'milliseconds-per-frame'])
            savetocsv = True
            print("Start writing data to", csvfile.name, "...")

        if key in (27, ord("q")) or cv2.getWindowProperty(WIN_NAME, cv2.WND_PROP_AUTOSIZE) < 0:
            if savetocsv:
                print("Stop writing file", csvfile.name, "...")
                csvfile.close()
                print("Done")
            break

finally:

    # Stop streaming
    pipeline.stop()
