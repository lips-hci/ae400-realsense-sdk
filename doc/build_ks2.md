### Branch/Version Information

This branch **ks2.43.0** is ONLY for camera model KS2.

| Branch         | LIPS SDK ver. | KS2 FW ver.   | Min. RS SDK ver.| Support status  |
| -------------- | ------------- | ------------- | --------------- | --------------- |
| ks2.43.0 :new: | v1.1.0.0      |               | v2.43.0         | Fix at v2.43.0  |

LIPS KS2 SDK v1.1.0.0
* This SDK ONLY supports camera KS2 and fix at RS v2.43.0
* In this version, you have to enable option '**BUILD_WITH_ZMQ=true**'

Simple build instruction
```
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_WITH_ZMQ=true
$ make
```
