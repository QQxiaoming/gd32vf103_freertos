######################################
# target
######################################
file ./build/gd32vf103.elf


set mem inaccessible-by-default off
set arch riscv:rv32
set remotetimeout 240

######################################
# 连接到ocdGDBsever,请修改主机地址
######################################
target extended-remote localhost:3333

monitor reset halt
monitor flash protect 0 0 last off

load

break main
continue

#monitor resume
#monitor shutdown
#quit
