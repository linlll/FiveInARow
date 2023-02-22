# Five In A Row

一个由C++开发的简易五子棋程序，能够实现多人在线下棋。

## 介绍

这是在自学了网络编程以及Qt之后个人所作的一个课程评测作业。

Qt程序中的逻辑简单清晰，较易复现。服务器端采用epoll多路IO复用，可实现多人在线同时下棋。

**此代码只耗时两天，十分粗糙，如果您真有问题，也十分欢迎一起交流。**

## 环境

Client将在Windows10上运行，Server将在Linux上运行。Qt版本5.14.1。

## Client

打开FiveInARow文件夹，打开其中的.pro文件，修改mainwindow.h文件中的ip地址为你服务器的ip地址。

```c++
const QString addr = "xxx.xxx.xxx.xxx";
```

然后编译，将会生成debug/release文件夹，我们将其中的FiveInARow.exe可执行文件复制出来，使用windeployqt进行部署，这样我们就可以有很多个五子棋客户端了。

## Server

打开server.cpp文件，默认端口号为8888，如被占用，请修改端口号。

```bash
cd FiveInARowServer
mkdir build && cd build
cmake ..
make
```

这样我们就可以运行

```bash
./server
```

## 效果

Server

![](C:/Users/LIN/Documents/GitHub/FiveInARow/images/1.png)

Client

![](C:/Users/LIN/Documents/GitHub/FiveInARow/images/1.gif)

## TODO

- [ ] 账号登录功能
- [ ] 好友功能
- [ ] 对话功能