#!/bin/sh

# Configure with /usr/local/etc/doas.conf containing:
#     permit nopass USERNAME as root cmd /sbin/sysctl
# where USERNAME is the your user name.

doas /sbin/sysctl kern.timecounter.alloweddeviation=0
