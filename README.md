## LIPSedge™ AE400 stereo camera
[LIPSedge™ AE400](https://www.lips-hci.com/product?product_id=29) stereo camera is powered by Intel® RealSense™ Technology and designed for industrial applications, such as robot applications, logistic/factory automation, and 3D monitoring/inspection.

### Overview
This software development kits is fully compatible to [Intel® RealSense™ SDK 2.0](https://github.com/IntelRealSense/librealsense) to help you connect to LIPSedge AE400 camera easily, the RealSense SDK already offers many tools, code samples, and wrappers for integration with existing 3rd-party technologies and software languages.

### Build
You need CMake to build SDK, please refer below RealSense wiki pages.
 - [build SDK on Ubuntu Linux](https://github.com/IntelRealSense/librealsense/blob/master/doc/installation.md)
 
 * for example, the command to build Release SDK and run installation
 ```
 $ mkdir build_Release
 $ cd build_Release
 $ cmake .. -DCMAKE_BUILD_TYPE=Release
 $ make -j4
 $ sudo make install
 ```
 - [build SDK on Windows](https://github.com/IntelRealSense/librealsense/blob/master/doc/installation_windows.md)
 
### Build wrappers for your applications
Please refer [build configurations](https://github.com/IntelRealSense/librealsense/wiki/Build-Configuration) to build the wrappers you need.
 - [wrappers overview from RealSense SDK](https://github.com/IntelRealSense/librealsense/tree/master/wrappers)
 - [OpenCV](https://github.com/IntelRealSense/librealsense/tree/master/wrappers/opencv)
 - [OpenNI2](https://github.com/IntelRealSense/librealsense/tree/master/wrappers/openni2)
 - [Python](https://github.com/IntelRealSense/librealsense/tree/master/wrappers/python)
 - [NVIDIA Isaac SDK](https://developer.nvidia.com/isaac-sdk) - Please check LIPS Github project [stereo_ae400](https://github.com/lips-hci/stereo_ae400).

* for example, the command to build Release SDK with wrappers
```
$ export OpenCV_DIR=/usr/local/share/OpenCV
$ cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_OPENNI2_BINDINGS=true \
  -DOPENNI2_DIR=/usr/include/openni2 \
  -DBUILD_PYTHON_BINDINGS=true \
  -DPYTHON_EXECUTABLE=/usr/bin/python3 \
  -DBUILD_PYTHON_DOCS=true \
  -DBUILD_CV_EXAMPLES=true
$ make -j4
$ sudo make install
```
