#include <windows.h>
#include <vector>
#include <bcrypt.h>
#include <stdexcept>
#include <array>
#include <random>
#pragma comment(lib, "bcrypt.lib")


// Windows CNG BCrypto API
uint32_t GenerateRandom256()
{
	uint32_t value = 0;
	NTSTATUS status = BCryptGenRandom(
										nullptr,
										(UCHAR*)&value,
										sizeof(value),
										BCRYPT_USE_SYSTEM_PREFERRED_RNG); // RNG是 Random Number Generation的缩写,表示使用系统的首选的随机数生成器.
	if (!BCRYPT_SUCCESS(status))
		throw std::runtime_error("BCryptGenRandom Failed");

	return value;
}


// 生成 1024 bit大小的随机数
std::array<BYTE, 128> GenerateRandom1024()
{
	std::array<BYTE, 128> value{};

	NTSTATUS status = BCryptGenRandom(
										nullptr,
										(UCHAR*)value.data(),
										value.size(),
										BCRYPT_USE_SYSTEM_PREFERRED_RNG);

	if (!BCRYPT_SUCCESS(status))
		throw std::runtime_error("BCryptGenRandom Failed");

	return value;
}


// 
int main()
{
	
}