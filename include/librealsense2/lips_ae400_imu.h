/* License: Apache 2.0. See LICENSE file in root directory.
   Copyright(c) 2020 LIPS Corporation. All Rights Reserved. */

#ifndef LIPS_AE400_IMU_H
#define LIPS_AE400_IMU_H

#include <stdint.h>

#if defined(_MSC_VER)
  // Microsoft
  #if defined(BUILD_SDK)
    /* We are building this library */
    #define API_DECLARE __declspec(dllexport)
  #else
    /* We are using this library */
    #define API_DECLARE __declspec(dllimport)
  #endif
#elif defined(__GNUC__)
  // GCC
  #if defined(BUILD_SDK)
    /* We are building this library */
    #define API_DECLARE __attribute__((visibility("default")))
  #else
    /* We are using this library */
    #define API_DECLARE
  #endif
#else
  // do nothing and hope for the best?
  #define API_DECLARE
  #pragma Warning Unknown OS to set dynamic link import/export.
#endif

/*
 *  Imu data structure
 */
struct lips_ae400_imu {
	/* Gyro X data */
	float gyro_x;
	/* Gyro Y data */
	float gyro_y;
	/* Gyro Z data */
	float gyro_z;

	/* Accel X data */
	float accel_x;
	/* Accel Y data */
	float accel_y;
	/* Accel Z data */
	float accel_z;
	/* AE400 timestamp (ms) */
	uint64_t timestamp;
};

namespace lips
{
    namespace ae400
    {
        //up to 100 IP addresses can be allocated for ae400 camera
        #define MAX_IP_INDEX 100

        /* get_imu_data
        *    retrieve IMU sensor data from AE400 camera
        * input:
        *  - ip_index: e.g. network.json "ip1" index is 0, "ip2" index is 1, "ip3" index is 2, and so on.
        *              IP addresses can be up to 100 allocated for ae400 cameram
        *  - data: pointer of data lips_ae_imu provided by the caller
        * output:
        *  - 0 = OKay, non-0 = failed
        */
        API_DECLARE int get_imu_data(int ip_index, lips_ae400_imu *data);
    }
}

#endif //LIPS_AE400_IMU_H