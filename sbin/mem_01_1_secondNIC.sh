#!/bin/bash
sudo modprobe -a ib_uverbs mlx5_core mlx5_ib
ip addr add 10.10.10.223/32 dev ens10
ifconfig ens10 mtu 9000
ifconfig ens10 up

ip route add 10.10.10.0/24 dev ens10
arp -i ens10 -s 10.10.10.213 04:3f:72:d8:3b:d8
#arp -i ens10 -s 10.10.10.222 04:3f:72:a2:c5:32

#ping -c 3 10.10.10.222  # switch vm

ibv_devinfo
