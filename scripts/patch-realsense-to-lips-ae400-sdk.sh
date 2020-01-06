#!/bin/bash
#
# This is a helper script to generate a LIPS AE400 development
# git repositofy by patching the required files/folders to
# your clean librealsense git repository
#
# mimimum RS2 version is 2.17.1
#
# For example, prepare a clean RS2 git repo with version v2.31.0
#
# $ git clone https://github.com/IntelRealSense/librealsense.git
# $ cd librealsense
# $ git checkout -b rs2.31.0 v2.31.0

if [ -z $1 ]; then
    echo "Usage: $0 <RS2 git dir>"
    exit 0
fi

AE4_GIT=$(pwd)
RS2_GIT=$1

echo "AE400 SRC GIT = $AE4_GIT"
echo "  RS2 SRC GIT = $RS2_GIT"
echo ""
echo "Processing ..."

rsync -avrhP --delete $AE4_GIT/src/linux/ $RS2_GIT/src/linux/
#rsync -avrhP --delete $AE4_GIT/src/linux/link_lips_prebuilt.cmake $RS2_GIT/src/linux/
#printf "\ninclude(src/linux/link_lips_prebuilt.cmake)\n" >> $RS2_GIT/src/linux/CMakeLists.txt

rsync -avrhP --delete $AE4_GIT/src/win/ $RS2_GIT/src/win/
#rsync -avrhP --delete $AE4_GIT/src/win/link_lips_prebuilt.cmake $RS2_GIT/src/win/
#printf "\ninclude(src/win/link_lips_prebuilt.cmake)\n" >> $RS2_GIT/src/win/CMakeLists.txt

rsync -avrhP --delete $AE4_GIT/third-party/lips/ $RS2_GIT/third-party/lips/

rsync -avrhP --delete $AE4_GIT/config/network.json $RS2_GIT/config/

rsync -avrhP --delete $AE4_GIT/CMake/install_network_config.cmake $RS2_GIT/CMake/
printf "\ninclude(CMake/install_network_config.cmake)\n" >> $RS2_GIT/CMake/install_config.cmake

rsync -avrhP --delete $AE4_GIT/wrappers/python/link_lips_prebuilt.cmake $RS2_GIT/wrappers/python/
printf "\ninclude(link_lips_prebuilt.cmake)\n" >> $RS2_GIT/wrappers/python/CMakeLists.txt

echo "Done!"
