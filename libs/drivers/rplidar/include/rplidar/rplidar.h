#ifndef __RPLIDAR_H__
#define __RPLIDAR_H__

#include <os/os.h>

int32_t rplidar_init();
int32_t rplidar_print_info();
void rplidar_enable_motor();
void rplidar_disable_motor();
int32_t rplidar_start_scan();
int32_t rplidar_stop_scan();
int32_t rplidar_run();

#endif
