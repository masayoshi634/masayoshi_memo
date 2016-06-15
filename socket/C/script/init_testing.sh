#!/bin/sh

ip netns add test01
ip netns add test02

ip link add veth0 type veth peer name veth1
ip link add veth2 type veth peer name veth3

ip link set veth0 netns test01
ip link set veth3 netns test02

ip netns exec test01 ip addr add 192.168.254.1/24 dev veth0
ip netns exec test01 ip link set veth0 up
ip netns exec test01 ip link set lo up

ip netns exec test02 ip addr add 192.168.254.2/24 dev veth3
ip netns exec test02 ip link set veth3 up
ip netns exec test02 ip link set lo up

ip link set veth1 up
ip link set veth2 up
