感谢大家一直以来对SmileOS项目的关注和支持，很遗憾地告诉大家，本人（jiaojinxing1987@gmail.com）不再升级SmileOS的功能，SmileOS作为本人学习操作系统的综合项目，水平有限，bug难免，但用非常精简的代码实现了很多的功能（尽管都不完善），纵观国内，开源面向arm的多进程操作系统几乎没有，这个小东西还是挺有意思的！有意接手主持这个项目的朋友可以联系我！

在这里隆重向大家推荐国内优秀嵌入式操作系统的典范：SylixOS(www.sylixos.com），望大家多多支持！

# SmileOS #
## 缘起： ##
出于学习和尝试的目的，利用周末和晚上时间码了一个ARM9操作系统，取名SmileOS，微笑（微小）操作系统。

目前只写了mini2440的BSP，mini2440上可以跑。

创建了一个Q群，群号220836807，欢迎加入共同学习探讨。

## 特性： ##
### 一、多进程 ###
支持多进程，并且利用MMU的快速上下文切换功能，进程切换时不用切换页表，避免了回写并无效Cache和排干WriteBuffer，无效TLB，重建页表、Cache和TLB等耗时的操作，从而极大地提高了系统的性能，限制就是最多只能创建127个进程，每个进程只能使用0~32MB的虚拟地址空间，支持进程保护。

### 二、虚拟内存管理 ###
每个进程使用0~32MB的虚拟地址空间，使用缺页中断和二级页表以支持虚拟内存管理，进程退出时会回收进程运行时占用的物理页面。

独特的内存堆管理算法有利于减少缺页中断的次数和进程运行时占用的物理页面及帮助程序员快速定位内存使用错误。

### 三、IPC ###
支持基本的进程间通信机制，如信号量，互斥量，消息队列，本地回环socket，管道等。

### 四、多线程 ###
支持内核多线程用于移植lwIP网络协议栈和实现中断底半部处理程序及提供一些内核服务，支持内核线程堆栈溢出检查。

内核线程的调度算法基于优先级，进程的调度算法基于时间片，调度时先调度内核线程再调度进程。

应用程序可以使用GNU/libpth库（符合pthread接口）支持用户空间的非抢占式多线程。

### 五、C++ ###
支持用C++语言编写的应用程序。

### 六、内核模块 ###
支持内核模块的动态加载和卸载，可用于安装设备驱动程序、文件系统或运行一个运行在内核里的“应用程序”。内核模块可以使用内核、C库、数学库、GCC库提供的全部符号。

### 七、网络 ###
目前lwIP网络协议栈也移植好了，简单的telnetd用于控制台，简单的ftpd用于下载进程二进制文件到文件系统。

telnet与控制台：

![http://smileos.googlecode.com/files/telnetd.png](http://smileos.googlecode.com/files/telnetd.png)

ftp服务器：

![http://smileos.googlecode.com/files/ftpd.png](http://smileos.googlecode.com/files/ftpd.png)

### 八、文件系统 ###
虚拟文件系统用于支持多种常用的文件系统，如yaffs2、fatfs、devfs、rootfs等。

提供符合posix标准的文件与目录操作接口。

简单的驱动程序框架，易于编写各种设备的驱动程序。

支持select和poll。

支持yaffs2、fatfs文件系统。

MTD驱动。

### 九、成熟的C库 ###
成熟的嵌入式C库newlib。

### 十、常用工具与组件 ###

vi编辑器：

![http://smileos.googlecode.com/files/vi.png](http://smileos.googlecode.com/files/vi.png)

运行在进程空间里的goahead服务器：

![http://smileos.googlecode.com/files/goahead.png](http://smileos.googlecode.com/files/goahead.png)

运行在进程空间里的基于著名的libffmpeg的视频播放器：

![http://smileos.googlecode.com/files/ffmpeg2.png](http://smileos.googlecode.com/files/ffmpeg2.png)

![http://smileos.googlecode.com/files/ffmpeg1.png](http://smileos.googlecode.com/files/ffmpeg1.png)

###  ###
## 开始SmileOS学习或开发： ##
请看http://code.google.com/p/smileos/wiki/HOWTO

## 致谢： ##
韩辉（sylixos@gmail.com），SylixOS（一个写得非常棒的嵌入式操作系统，www.sylixos.com）的作者，我的前部门经理，在他两年的带领下，教会了我非常多嵌入式软件知识（没有这些知识的积累，SmileOS也不可能写成）和人生的道理，给了我非常自由的成长空间，你是一位难得的良师益友。

陈健生，我的大学同学，帮我写了一编图文并茂的step by step文档，虽然你编码的水平我不恭维，但你是个不折不扣的好男人:-)

我的老婆小玉，在你怀孕期间，仍“不辞劳苦”地承担部分家务和每天给我煮可口的饭菜，在我对着电脑编码忽略你的时候也毫无怨言并不时给我倒水，你是这世上对我最好最体贴的女人了，没有之一。周末陪你逛公园陪你去买菜一起做饭你便知足不已，让我感受到编码不是生活的全部，还有很多东西值得我去追求和享受。