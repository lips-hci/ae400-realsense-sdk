

/* License: Apache 2.0. See LICENSE file in root directory.
   Copyright(c) 2020 LIPS Corporation. All Rights Reserved. */

#ifdef ENABLE_AE400_IMU
#define BUILD_SDK
#else
#undef BUILD_SDK
#endif
#include <librealsense2/lips_ae400_imu.h>

namespace lips
{
    namespace ae400
    {
        lips_ae400_imu _query_imu(int index);

        int get_imu_data(int ip_index, lips_ae400_imu *data)
        {
            if (data && (0 <= ip_index && ip_index < MAX_IP_INDEX))
            {
                lips_ae400_imu front = _query_imu(ip_index);
                data->gyro_x = front.gyro_x;
                data->gyro_y = front.gyro_y;
                data->gyro_z = front.gyro_z;

                data->accel_x = front.accel_x;
                data->accel_y = front.accel_y;
                data->accel_z = front.accel_z;

                data->timestamp = front.timestamp;

                return 0;
            }

            return -1;
        }
    }
}
