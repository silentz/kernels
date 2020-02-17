#!/bin/bash

module="storage"
device="storage"
mode="666"

/sbin/insmod "$module.ko"
rm -f "/dev/$device"

major=$(cat /proc/devices | grep "$module" | awk '{print $1}')
mknod "/dev/$device" c $major 0
chmod "$mode" "/dev/$device"
