#!/bin/bash
# use testnet settings,  if you need mainnet,  use ~/.zenxcore/zenxd.pid file instead
zenx_pid=$(<~/.zenxcore/testnet3/zenxd.pid)
sudo gdb -batch -ex "source debug.gdb" zenxd ${zenx_pid}
