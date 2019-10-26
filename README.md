# gd32vf103_freertos

在GD32VF103移植freertos，linux环境开发

GD32VF103驱动来自于https://github.com/riscv-mcu/GD32VF103_Demo_Suites

FreeRTOS移植层部分代码来自于https://github.com/nucleisys/n200-sdk

## 编译说明

tools/Nuclei/openocd目录内为官方提供的64位linux平台openocd工具，用来调试下载

tools/riscv-none-gcc/8.2.0-2.2-20190521-0004目录为官方提供的64位linux平台GNU工具链

### 编译

运行

```shell
make
```

将在根目录执行编译，最终在build目录生成gd32vf103.elf、gd32vf103.bin、gd32vf103.hex文件

### 调试

运行

```shell
sudo ./ocdsever.sh
```

将启动通过openocd打开一个gdbsever，供调试使用

重新打开一个新命令行终端，运行

```shell
make debug
```

等待程序下载至flash，即可开始调试。

#### 其他说明

目前makefile中使用insight代替gdb来调试，如果未安装此工具可以将

```makefile
GDB       = /opt/riscv-none-eabi-insight/bin/riscv-none-eabi-insight #使用insight代替gdb来调试
```

改为

```makefile
GDB       = $(PREFIX)gdb
```

使用
