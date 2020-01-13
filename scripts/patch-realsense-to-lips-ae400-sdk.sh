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

if [ "$(which rsync)" != "" ]; then
echo "Processing by rsync ..."

rsync -avrhP --delete --exclude 'backend-*.h' $AE4_GIT/src/linux/ $RS2_GIT/src/linux/
#rsync -avrhP --delete $AE4_GIT/src/linux/link_lips_prebuilt.cmake $RS2_GIT/src/linux/
#printf "\ninclude(src/linux/link_lips_prebuilt.cmake)\n" >> $RS2_GIT/src/linux/CMakeLists.txt

rsync -avrhP --delete --exclude 'win-helpers.*' $AE4_GIT/src/win/ $RS2_GIT/src/win/
#rsync -avrhP --delete $AE4_GIT/src/win/link_lips_prebuilt.cmake $RS2_GIT/src/win/
#printf "\ninclude(src/win/link_lips_prebuilt.cmake)\n" >> $RS2_GIT/src/win/CMakeLists.txt

rsync -avrhP --delete $AE4_GIT/third-party/lips/ $RS2_GIT/third-party/lips/

rsync -avrhP --delete $AE4_GIT/config/network.json $RS2_GIT/config/

rsync -avrhP --delete $AE4_GIT/CMake/install_network_config.cmake $RS2_GIT/CMake/
printf "\ninclude(CMake/install_network_config.cmake)\n" >> $RS2_GIT/CMake/install_config.cmake

rsync -avrhP --delete $AE4_GIT/wrappers/python/link_lips_prebuilt.cmake $RS2_GIT/wrappers/python/
printf "\ninclude(link_lips_prebuilt.cmake)\n" >> $RS2_GIT/wrappers/python/CMakeLists.txt

echo "Done!"
exit 0

fi

#
# Cannot find rsync, use common commands like cp & move
#
echo "Processing by rm&cp ..."

for dstfile in $RS2_GIT/src/linux/*; do
    [ "$(basename "$dstfile" | grep -i 'backend-.*\.h')" == "" ] && rm -vf $dstfile
done
for srcfile in $AE4_GIT/src/linux/*; do
    cp -vf $srcfile $RS2_GIT/src/linux/
done
#rsync -avrhP --delete --exclude 'backend-*.h' $AE4_GIT/src/linux/ $RS2_GIT/src/linux/

for dstfile in $RS2_GIT/src/win/*; do
    [ "$(basename "$dstfile" | grep -i 'win-helpers..*')" == "" ] && rm -vf $dstfile
done
for srcfile in $AE4_GIT/src/win/*; do
    cp -vf $srcfile $RS2_GIT/src/win/
done
#rsync -avrhP --delete --exclude 'win-helpers.*' $AE4_GIT/src/win/ $RS2_GIT/src/win/

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

echo "Done!"
