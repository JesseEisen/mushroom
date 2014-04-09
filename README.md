这是我毕业设计的项目， 用来控制蘑菇出菇房的环境， 桥梁是PLC， 通信走的是RS422可编程口， USB转RS422芯片是FT232RL。完全由C语言实现， 基于多线程架构， 使用了sqlite3做蘑菇房的配置， protobuf做消息通信。 请善待该项目和它的BUGs， 目前打算完善串口通信部分， 然后对共享数据， 加强控制。

事实上这只是项目的一小部分， 还有两个小伙伴负责上位机部分， 硬件部分负责底层的采集与控制。 数据将传输到上位机， 进行计算， 定制策略， 达到自动控制。 而用户界面将使用web浏览器来接入， 来达到对蘑菇房的整体监控。

## 编译，安装

## 两种方法：

方法一, 老的Makefile.bak暂时还是可以编译的， 里面可以修改编译器CC， 具体查看Makefile.bak

```
	make -f Makefile.bak
```

方法二, 使用autoconf和automake， 系统必须要安装有相应的工具

```
	./autogen.sh
	./configure (交叉编译: ./configure )
	make (安装： make install  清除: make clean 删除configure设置: make distclean)
```

运行：

由于配置文件中日志在log目录下， 因此必须手动先创建log目录， 另外由于控制台命令是通过管道来传输的， 因此cmd.fifo也要创建一下， 这些目前并不是必须的， 会慢慢移除。

```
	mkfifo cmd.fifo
	mkdir -p log
	./mushroom
```

有一个控制台程序， 叫做mrconsole.c需要自己编译， 可以向mushroom程序发送命令。

```
gcc mrconsole.c -o mrconsole
./mrconsole
```
