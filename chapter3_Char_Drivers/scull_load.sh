#!/bin/bash

module_name="scull"
module_nb=4

insmod scull.ko || exit 1

rm -f /dev/${module_name}[0-${module_nb}]

major=$(awk "\$2 == \"${module_name}\" {print \$1}" /proc/devices)

for ((i=0; i<module_nb; i++)); do
    mknod "/dev/${module_name}${i}" c "${major}" "${i}"
done
