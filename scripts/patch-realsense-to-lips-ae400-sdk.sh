#!/bin/bash
#
# This is a helper script to generate a LIPS AE400 development
# git repositofy by patching the required files/folders to
# your clean librealsense git repository
#
# Recommend RS2 build is 2.36.0 or higher versions
#
# Assume you already clone project ae400-realsense-sdk
# All you have to do is,
# - prepare a clean RS2 git project, e.g. using build v2.36.0
# - run script to patch it
# - enjoy running RealSense2 SDK with AE400
#
# Quick Instruction
# $ git clone https://github.com/lips-hci/ae400-realsense-sdk.git
# $ cd ae400-realsense-sdk
#
# $ git clone https://github.com/IntelRealSense/librealsense.git
# $ cd librealsense
# $ git checkout -b rs2.36.0 v2.36.0
#
# $ cd ..
# $ ./scripts/patch-realsense-to-lips-ae400-sdk.sh librealsense
#
# patch is done, now you can build RealSense SDK for AE400 camera
# $ cd librealsense
# $ mkdir build && cd build
# $ cmake .. -DCMAKE_BUILD_TYPE=Release
# $ make -j4
# $ sudo make install

if [ -z $1 ]; then
    echo "Usage: $0 <RS2 git dir>"
    exit 0
fi

AE4_GIT=$(pwd)
RS2_GIT=$1
echo ""
echo "AE400 SRC GIT = $AE4_GIT"
echo "  RS2 SRC GIT = $RS2_GIT"
echo ""

if [ "$(which rsync 2> /dev/null)" != "" ]; then
echo "Processing by rsync ..."

rsync -avrhP --delete --exclude 'backend-*.h' $AE4_GIT/src/linux/ $RS2_GIT/src/linux/

rsync -avrhP --delete --exclude 'mf-*.h' $AE4_GIT/src/mf/ $RS2_GIT/src/mf/

rsync -avrhP --delete $AE4_GIT/third-party/lips/ $RS2_GIT/third-party/lips/

rsync -avrhP --delete $AE4_GIT/config/network.json $RS2_GIT/config/

rsync -avrhP --delete $AE4_GIT/CMake/install_network_config.cmake $RS2_GIT/CMake/
printf "\ninclude(CMake/install_network_config.cmake)\n" >> $RS2_GIT/CMake/install_config.cmake

rsync -avrhP --delete $AE4_GIT/wrappers/python/link_lips_prebuilt.cmake $RS2_GIT/wrappers/python/
printf "\ninclude(link_lips_prebuilt.cmake)\n" >> $RS2_GIT/wrappers/python/CMakeLists.txt

rsync -avrhP --delete $AE4_GIT/include/librealsense2/lips_ae400_imu.h $RS2_GIT/include/librealsense2/
rsync -avrhP --delete $AE4_GIT/examples/imu-reader/ $RS2_GIT/examples/imu-reader/
printf "\nadd_subdirectory(imu-reader)\n" >> $RS2_GIT/examples/CMakeLists.txt

getdevicetimems=$(grep -i '_device->get_device_time_ms()' "$RS2_GIT/src/global_timestamp_reader.cpp")
if [ "$getdevicetimems" != "" ]; then
echo "Patch src/global_timestamp_reader.cpp with new hardware time ms representation"
newhwtimems="duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()"
sed -i -e "s/_device->get_device_time_ms()/$newhwtimems/g" $RS2_GIT/src/global_timestamp_reader.cpp
fi

echo ""
echo "patch is done, enjoy it!"
echo ""
exit 0

fi

#
# Cannot find rsync, use common commands like cp & move
# Assume user is running in Git BASH (Git for Windows) environment
#
echo "Expect to running in Git BASH for Windows environment ..."
echo "Processing by rm&cp ..."

for dstfile in $RS2_GIT/src/linux/*; do
    [ "$(basename "$dstfile" | grep -i 'backend-.*\.h')" == "" ] && rm -vf $dstfile
done
for srcfile in $AE4_GIT/src/linux/*; do
    cp -vf $srcfile $RS2_GIT/src/linux/
done
#rsync -avrhP --delete --exclude 'backend-*.h' $AE4_GIT/src/linux/ $RS2_GIT/src/linux/

for dstfile in $RS2_GIT/src/mf/*; do
    [ "$(basename "$dstfile" | grep -i 'mf-.*\.h')" == "" ] && rm -vf $dstfile
done
for srcfile in $AE4_GIT/src/mf/*; do
    cp -vf $srcfile $RS2_GIT/src/mf/
done
#rsync -avrhP --delete --exclude 'mf-*.h' $AE4_GIT/src/mf/ $RS2_GIT/src/mf/

for srcfile in $AE4_GIT/third-party/lips/*; do
    cp -vrfP $srcfile $RS2_GIT/third-party/lips/
done
#rsync -avrhP --delete $AE4_GIT/third-party/lips/ $RS2_GIT/third-party/lips/

cp -vf $AE4_GIT/config/network.json $RS2_GIT/config/
#rsync -avrhP --delete $AE4_GIT/config/network.json $RS2_GIT/config/

cp -vf $AE4_GIT/CMake/install_network_config.cmake $RS2_GIT/CMake/
#rsync -avrhP --delete $AE4_GIT/CMake/install_network_config.cmake $RS2_GIT/CMake/
printf "\ninclude(CMake/install_network_config.cmake)\n" >> $RS2_GIT/CMake/install_config.cmake

cp -vf $AE4_GIT/wrappers/python/link_lips_prebuilt.cmake $RS2_GIT/wrappers/python/
#rsync -avrhP --delete $AE4_GIT/wrappers/python/link_lips_prebuilt.cmake $RS2_GIT/wrappers/python/
printf "\ninclude(link_lips_prebuilt.cmake)\n" >> $RS2_GIT/wrappers/python/CMakeLists.txt

cp -vf $AE4_GIT/include/librealsense2/lips_ae400_imu.h $RS2_GIT/include/librealsense2/
#rsync -avrhP --delete $AE4_GIT/include/librealsense2/lips_ae400_imu.h $RS2_GIT/include/librealsense2/
[ ! -e $RS2_GIT/examples/imu-reader ] && mkdir $RS2_GIT/examples/imu-reader
for srcfile in $AE4_GIT/examples/imu-reader/*; do
    cp -vrfP $srcfile $RS2_GIT/examples/imu-reader/
done
#rsync -avrhP --delete $AE4_GIT/examples/imu-reader/ $RS2_GIT/examples/imu-reader/
printf "\nadd_subdirectory(imu-reader)\n" >> $RS2_GIT/examples/CMakeLists.txt

getdevicetimems=$(grep -i '_device->get_device_time_ms()' "$RS2_GIT/src/global_timestamp_reader.cpp")
if [ "$getdevicetimems" != "" ]; then
echo "Patch src/global_timestamp_reader.cpp with new hardware time ms representation"
newhwtimems="duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()"
sed -i -e "s/_device->get_device_time_ms()/$newhwtimems/g" $RS2_GIT/src/global_timestamp_reader.cpp
fi

echo ""
echo "patch is done, enjoy it!"
echo ""