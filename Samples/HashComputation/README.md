### Hash算法

- MD2  16字节
- MD4  16字节
- MD5  16字节

- SHA-1
    - SHA-1 20字节


- SHA-2
    - SHA-224  28字节
    - SHA-256  32字节
    - SHA-384  48字节
    - SHA-512  64字节


- SHA-3
    - SHA3-224  28字节
    - SHA3-256  32字节
    - SHA3-384  48字节
    - SHA3-512  64字节



### 使用BCrypt系列API计算Hash

如何使用Win32API计算 MD5/SHA1/SHA256/SHA384/SHA512等等HASH值

首先需要包含 "bcrypt.h"头文件. 并且链接bcrypt.lib
```cpp
#include <bcrypt.h>
#pragma comment (lib, "bcrypt.lib")
```

依次调用下面API
- BCryptOpenAlgorithmProvider
- BCryptGetProperty
- BCryptCreateHash
- BCryptHashData
- BCryptFinishHash
- BCryptDestroyHash
- BCryptCloseAlgorithmProvider


分别对应下面功能
- 打开算法提供程序
- 获取Hash对象的大小
- 创建Hash对象
- 向Hash对象中输入数据
- 根据数据计算Hash值
- 销毁Hash对象
- 关闭算法提供程序



BCryptHashData 可以调用多次. 

也就是
```cpp
BCryptHashData("hel");
BCryptHashData("lo");
```
和
```cpp
BCryptHashData("hello");
```
等价.
因为BCryptHashData的作用是往Hash对象中喂数据, 一次性喂完和两次喂效果一样. 
这样的好处是, 假如有一个10GB的文件, 在计算它的Hash时,不用一次性把10GB文件全部读入内存.可以分成10次读入,调用10次BCryptHashData.
