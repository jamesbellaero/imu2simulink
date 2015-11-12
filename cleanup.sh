#!/bin/sh
mv /home/debian/imu_data/imu_log.log /home/debian/imu_data/imu_log_old.log
mv /home/debian/imu_data/data_imu.txt /home/debian/imu_data/data_imu_old.txt
mv /home/debian/matlog.txt /home/debian/matlab_logs
mv /home/debian/*.elf /home/debian/old_elf
rm /home/debian/*.log
sleep 4
/home/debian/imu2simulink/imu2simulink -P 26101 -q -w
