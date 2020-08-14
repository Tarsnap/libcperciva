#!/bin/sh

out="speed_mbps.txt"

# Initialize
if [ ! -f "${out}" ] ; then
	printf "hash\tspeed MB/s\n" > "${out}"
	orig_mb=""
else
	# Get original speed
	orig_mb="$(head -n 2 "${out}" | tail -n 1 | cut -f 2)"
fi

# Compile, time, extract speed.
make CFLAGS="-O2 -g"
speed="$(./test_sha256 -t | grep "Speed = " | cut -f 3 -d " ")"
speed_mb="$(echo "scale=1; ${speed}/10^6" | bc)"

# Set an original speed for the first line.
if [ -z "${orig_mb}" ]; then
	orig_mb="${speed_mb}"
fi

# Save to file.
hash="$(git rev-parse --short HEAD)"
relative_mb="$(echo "scale=1; 100*${speed_mb}/${orig_mb}" | bc)"
printf "${hash}\t${speed_mb}\t${relative_mb}%%\n" >> "${out}"