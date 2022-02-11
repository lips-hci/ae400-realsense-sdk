## License: Apache 2.0. See LICENSE file in root directory.
## Copyright(c) 2020 LIPS Corporation. All Rights Reserved.

###############################################
##      Open CV and Numpy integration        ##
###############################################

"""
LIPS Corporation

Author: Tim Cheng (timcheng@lips-hci.com)

This sample is a simple OpenCV viewer showing frame-rate number.

Keyboard:
    [o]     Save fps number and timestamp to .csv file (./fps_results.csv)
    [q\ESC] Quit
"""

import math
import time
import cv2
import numpy as np
import pyrealsense2 as rs
import csv

# Configure depth and color streams
pipeline = rs.pipeline()
config = rs.config()
config.enable_stream(rs.stream.depth, 640, 480, rs.format.z16, 30)
config.enable_stream(rs.stream.color, 640, 480, rs.format.bgr8, 30)

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
csvfilename = "fps_results_%dx%d.csv" %(w, h)
print("Help:")
print("- Press 'o' to export fps number to .csv file")
savetopath = "  (default save to file: %s)" %(csvfilename)
print(savetopath)
print("- Press 'q' or Esc to exit the program")

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
            cv2.setWindowTitle(WIN_NAME, "RealSense (%dx%d) %.2f FPS (%.2fms)" %(w, h, counter/dt, (dt*1000)/counter))
            if savetocsv:
                with open('./fps_results.csv', 'w', newline='') as csvfile:
                    writer = csv.writer(csvfile, delimiter=',')
                    writer.writerow([now, counter/dt, ((dt*1000)/counter)])
            start = time.time()
            counter = 0

        # Show images
        cv2.imshow(WIN_NAME, images)
        key = cv2.waitKey(1)

        # key handlers
        if key == ord("o"):
            with open('./fps_results.csv', 'w', newline='') as csvfile:
                writer = csv.writer(csvfile, delimiter=',')
                writer.writerow(['timestamp', 'fps', 'ms'])
            savetocsv = True
            text = "Start writing data to %s ..." %(csvfile.name)
            print(text)

        if key in (27, ord("q")) or cv2.getWindowProperty(WIN_NAME, cv2.WND_PROP_AUTOSIZE) < 0:
            if savetocsv:
                text = "Stop writing file %s ..." %(csvfile.name)
                print(text)
                csvfile.close()
                print("Done")
            break

finally:

    # Stop streaming
    pipeline.stop()
