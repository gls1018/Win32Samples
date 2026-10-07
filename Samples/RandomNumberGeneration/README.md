### 1. BCryptGenRandom
#### 1.1介绍
BCryptGenRandom主要用于下面生成随机数
- 密码学
- 密钥
- Nonce
- Salt
- Token
- IV
- 安全随机数


函数原形:
```cpp
NTSTATUS BCryptGenRandom(
    BCRYPT_ALG_HANDLE hAlgorithm,
    PUCHAR            pbBuffer,
    ULONG             cbBuffer,
    ULONG             dwFlags
);
```
其作用就是向一个Buffer里填充随机字节. hAlgorithm一般传入nullptr, dwFlags传入BCRYPT_USE_SYSTEM_PREFERRED_RNG, 两者配套使用. 

#### 1.2 生成均匀分布随机数

假设生成一个 [0~100)之间的随机数, 要求等概率. 下面代码是有问题的. 
```cpp
int main()
{
	uint16_t value = 0;
	NTSTATUS status = BCryptGenRandom(
										nullptr,
										(UCHAR*)&value,
										sizeof(value),
										BCRYPT_USE_SYSTEM_PREFERRED_RNG);
	if (!BCRYPT_SUCCESS(status))
		throw std::runtime_error("BCryptGenRandom Failed");

	value = value % 100;

	return value;
}
```
会产生 模偏差(Modulo bias), 可以通过拒绝采样来消除模偏差(Rejection sampling)


```cpp
int main()
{
	uint16_t value = 0;
	do{
	
		NTSTATUS status = BCryptGenRandom(
											nullptr,
											(UCHAR*)&value,
											sizeof(value),
											BCRYPT_USE_SYSTEM_PREFERRED_RNG);
	}while(value >= (0xFFFF+1)/100 *100)
	if (!BCRYPT_SUCCESS(status))
		throw std::runtime_error("BCryptGenRandom Failed");

	value = value % 100;

	return value;
}
```


### 2. C++ \<random>

C++ \<random>主要用于下面:
- 模拟
- 游戏
- 科学计算
- 测试
- 随机算法
- 统计
