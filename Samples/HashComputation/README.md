
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
