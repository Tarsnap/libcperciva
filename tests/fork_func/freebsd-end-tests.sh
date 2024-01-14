#!/bin/sh

# Configure with /usr/local/etc/doas.conf containing:
#     permit nopass USERNAME as root cmd /sbin/sysctl
# where USERNAME is the your user name.
#
# In freebsd 12.4, the default value is 5; it could change in future versions.

doas /sbin/sysctl kern.timecounter.alloweddeviation=5
