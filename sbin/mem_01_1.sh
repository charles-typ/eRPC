#!/bin/bash
sudo modprobe -a ib_uverbs mlx5_core mlx5_ib
ip addr add 10.10.10.223/32 dev ens9np1
ifconfig ens9np1 mtu 9000
ifconfig ens9np1 up

ip route add 10.10.10.0/24 dev ens9np1
arp -i ens9np1 -s 10.10.10.213 04:3f:72:d8:3b:d8
#arp -i ens9np1 -s 10.10.10.222 04:3f:72:a2:c5:32

#ping -c 3 10.10.10.222  # switch vm

ibv_devinfo
