## 数据可靠传输设计实验

* 开发平台VS2022，使用VS2022打开源码的.sln项目文件，C++20标准。
* 首先需要修改每一个main.cpp中的input和output文件的路径

```
	pns->setInputFile("E:\\desktop\\计算机网络\\实验\\second\\input.txt");
	pns->setOutputFile("E:\\desktop\\计算机网络\\实验\\second\\output.txt");
```

以及修改`stdafx.h`文件中的路径

```
#pragma comment (lib,"E:\\desktop\\计算机网络\\实验\\second\\rdt\\netsimlib.lib")
```

* 修改完以后编译得到exe文件，运行exe文件就可以正常运行