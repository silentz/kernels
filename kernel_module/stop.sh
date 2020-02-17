#!/bin/bash

module="storage"
device="storage"

/sbin/rmmod "$module"
rm -f "/dev/$device"

