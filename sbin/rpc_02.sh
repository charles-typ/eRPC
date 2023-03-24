#!/bin/bash
sudo modprobe -a ib_uverbs mlx5_core mlx5_ib
ip addr add 10.10.10.208/32 dev ens9
ifconfig ens9 mtu 9000
ifconfig ens9 up

ip route add 10.10.10.0/24 dev ens9
arp -i ens9 -s 10.10.10.207 04:3f:72:a2:b0:12

#ping -c 3 10.10.10.222  # switch vm

ibv_devinfo
