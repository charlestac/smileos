### 1. 安装Java SE ###
网址：http://www.oracle.com/technetwork/java/javase/downloads/jre-7u3-download-1501631.html

下载这个文件：jre-7u3-windows-i586.exe

### 2. 安装Eclipse IDE for C/C++ Developers ###
网址：http://www.eclipse.org/downloads/packages/eclipse-ide-cc-developers/heliossr2

下载这个文件：eclipse-cpp-helios-SR2-win32.zip
下载后解压到相应目录，不需安装，发送个快捷方式到桌面即可。

### 3. 安装Eclipse下开发ARM的插件 ###
网址：http://sourceforge.net/projects/gnuarmeclipse/files/Current%20Releases/0.5.3/

下载这个文件：org.eclipse.cdt.cross.arm.gnu\_0.5.3.201007311800.zip

下载后解压，把plugins/org.eclipse.cdt.cross.arm.gnu\_0.5.3.201007311800.jar文件放入Eclipse安装目录下的plugins目录里。

### 4. 安装arm-none-eabi-gcc编译器 ###
网址：http://www.yagarto.de/download/yagarto/yagarto-bu-2.21_gcc-4.6.2-c-c++_nl-1.19.0_gdb-7.3.1_eabi_20111119.html

请不要安装在路径有空格的目录！

下载并安装这个文件：yagarto-bu-2.21\_gcc-4.6.2-c-c%2B%2B\_nl-1.19.0\_gdb-7.3.1\_eabi\_20111119.exe

网址：http://www.yagarto.de/download/yagarto/yagarto-tools-20100703-setup.exe

下载并安装这个文件：yagarto-tools-20100703-setup.exe

### 5. 打开SmileOS工程 ###
打开Eclipse，并选择合适的目录作你的工作空间，把SmileOS.zip工程解压后放入到工作空间的目录内，Import SmileOS工程到工作空间。
SmileOS.zip：http://smileos.googlecode.com/files/SmileOS.zip

### 6. 更新SmileOS工程 ###
使用SVN工具，update到最新的源代码

### 7. 编译 ###
ctrl+B

### 8. 下载运行 ###
使用dnw.exe下载Debug/SmileOS.hex到0x30000000地址处运行

### 9. 导入Eclipse风格文件 ###
为了在Eclipse下更好地阅读及修改代码，强烈建议导入我配制的Eclipse风格文件
http://smileos.googlecode.com/files/SmileOS.epf