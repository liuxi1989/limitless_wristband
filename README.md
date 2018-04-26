# limitless_wristband
无极限手环，是一个蓝牙智能手环开源项目。由一群爱好蓝牙技术的朋友共同开发维护，我们将开源所有设计资料，您可用于交流学习，但不可用于商业目的。<br><br>

### 一、 手环功能
无极限手环是一款个人健康数据的收集者，她会帮您记录全天的运动情况，计算行走距离以及消耗的热量，让您轻松掌控每天的运动量，您还可以轻松的将这些运动数据上传至微信运动，与上百好友PK。同时，她集成了高精度的心率传感器，可实时监测您的心率和血氧信息，在进行有氧运动时，可更好掌控运动节奏。<br>夜幕来临，当您进入梦乡时，她会自动监测您的睡眠质量，解析出深度睡眠和浅度睡眠时间。<br>通过蓝牙4.0技术，它还可以将您手机上收到任何提醒自动推至OLED屏进行显示，同时利用自身的振动马达进行提醒，重要的电话和信息不再错过。<br>通过特有的算法，让您的拍照更加有乐趣，您只需要挥动手腕，即可控制手机完成拍照。与手机APP结合后，会有更多新奇的玩法。<br>
### 二、开发环境文件
<1> nrf52832 SDK     ： [nRF5_SDK_12.3.0_d7731ad](http://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v12.x.x/nRF5_SDK_12.3.0_d7731ad.zip)<br>
<2> PC 端 IDE        ： [KEIL 5.2.4](https://pan.baidu.com/s/1dFnHzGl?errno=0&errmsg=Auth%20Login%20Sucess&&bduss=&ssnerror=0&traceid=)<br>
<3> [nRF_DeviceFamilyPack.8.11.1.pack](http://developer.nordicsemi.com/nRF5_SDK/pieces/nRF_DeviceFamilyPack/NordicSemiconductor.nRF_DeviceFamilyPack.8.11.1.pack)<br>
<4> [ARM.CMSIS.4.5.0.pack](https://pan.baidu.com/s/1BWcbPdipPgTnwZpKOq-AtQ)<br>
<5> 固件下载工具32位 ：[nRFgo Studio-Win32](http://www.nordicsemi.com/eng/nordic/download_resource/22286/65/59520294/30244)<br>
<6> 固件下载工具64位 ：[nRFgo Studio-Win64](http://www.nordicsemi.com/eng/nordic/download_resource/14964/69/47337491/2447)<br>
<7> [git 32位 ](https://github.com/git-for-windows/git/releases/download/v2.16.2.windows.1/Git-2.16.2-32-bit.exe)<br>
<8> [git 64位 ](https://github.com/git-for-windows/git/releases/download/v2.16.2.windows.1/Git-2.16.2-64-bit.exe)<br>
### 三、 开发环境搭建
1. 下载KEIL 5.2.4并安装，一路next，无需多言，安装成功后会弹出pack安装界面，如下图:<br>![pack安装界面](http://s6.sinaimg.cn/middle/002jmXUpzy7jUUiQTJjd5&690)<br>等待所有pack安装完成后再关闭界面 。
2. 输入正版密钥激活KEIL。
3. 首次打开本项目时，会提示缺少[nRF_DeviceFamilyPack.8.11.1.pack]()和[ARM.CMSIS.4.5.0.pack]()两个文件，直接点“是”进行安装，如果网速太慢，也可通过上边的连接下载后，双击安装。<br>![pack安装](http://s2.sinaimg.cn/middle/002jmXUpzy7jUUiQBSFd1&690)
4. 下载nRFgo Studio 软件，在安装前，为了避免jlink驱动的冲突，最好将PC上已有的jlink驱动卸载。之后一路next安装，当遇到安装jlink驱动时，选择同意即可。
5. 如果仅仅是在本地编译代码，以上的软件已经足够。如果有意向参与到项目中来，则要下载git ,之后一路next安装即可。