#!/bin/bash
sudo modprobe -a ib_uverbs mlx5_core mlx5_ib
ip addr add 10.10.10.222/32 dev ens9np0
ifconfig ens9np0 mtu 9000
ifconfig ens9np0 up

ip route add 10.10.10.0/24 dev ens9np0
arp -i ens9np0 -s 10.10.10.213 04:3f:72:d8:3d:d8

#ping -c 3 10.10.10.222  # switch vm

ibv_devinfo
