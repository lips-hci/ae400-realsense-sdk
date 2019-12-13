/* License: Apache 2.0. See LICENSE file in root directory.
   Copyright(c) 2019 LIPS Corporation. All Rights Reserved. */

/*!
 *  Imu data structure
 */
struct lips_ae400_imu {
	/*! Gyro X data */
	float gyro_x;
	/*! Gyro Y data */
	float gyro_y;
	/*! Gyro Z data */
	float gyro_z;

	/*! Accel X data */
	float accel_x;
	/*! Accel Y data */
	float accel_y;
	/*! Accel Z data */
	float accel_z;
	//AE400 timestamp (ms)
	long long timestamp;
};

#if defined(__WIN32__) || defined(WIN32) || defined(__WIN32)
__declspec(dllimport) struct lips_ae400_imu imu_data;
#else
extern struct lips_ae400_imu imu_data;
#endif
