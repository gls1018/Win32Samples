#include <windows.h>
#include <bcrypt.h>
#include <print>
#include <iostream>
#include <vector>
#include <array>
#pragma comment (lib, "bcrypt.lib")




int main()
{
	// 1. 打开 SHA-256 算法提供程序
	BCRYPT_ALG_HANDLE hAlg = nullptr;
	NTSTATUS status = BCryptOpenAlgorithmProvider(
													&hAlg, 
													BCRYPT_SHA256_ALGORITHM, 
													nullptr, 
													0);
	if (!BCRYPT_SUCCESS(status))
		throw std::runtime_error("BCryptOpenAlgorithmProvider Failed");

	// 2. 获取 Hash Object 大小.
	DWORD dwObjSize;
	DWORD dwResSize;

	status = BCryptGetProperty(
								hAlg,
								BCRYPT_OBJECT_LENGTH,
								(UCHAR*)&dwObjSize,
								sizeof(dwObjSize),
								&dwResSize,
								0);
	if (!BCRYPT_SUCCESS(status))
	{
		BCryptCloseAlgorithmProvider(hAlg, 0);
		throw std::runtime_error("BCryptGetProperty Failed");
	}

	// 3. 分配 Hash Object 
	BCRYPT_HASH_HANDLE hHash;
	std::vector<BYTE> hashObject(dwObjSize);
	status = BCryptCreateHash(
								hAlg,
								&hHash,
								hashObject.data(),
								hashObject.size(),
								nullptr,
								0,
								0);

	if (!BCRYPT_SUCCESS(status))
	{
		BCryptCloseAlgorithmProvider(hAlg, 0);
		throw std::runtime_error("BCryptCreateHash Failed");
	}

	// 4. 输入数据
	std::string str = "hello";

	status = BCryptHashData(
							hHash, 
							(UCHAR*)str.data(),
							str.size(),
							0);

	if (!BCRYPT_SUCCESS(status))
	{
		BCryptDestroyHash(hHash);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		throw std::runtime_error("BCryptHashData Failed");
	}
	
	// 5. 获取最终 Hash
	std::array<BYTE, 32> hashValue{};
	status = BCryptFinishHash(
								hHash, 
								hashValue.data(), 
								hashValue.size(), 
								0);
	if (!BCRYPT_SUCCESS(status))
	{
		BCryptDestroyHash(hHash);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		throw std::runtime_error("BCryptFinishHash Failed");
	}

	// 6. 输出
	for (BYTE byte : hashValue)
	{
		std::print("{:02x}", byte);      //输出 2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824
	}
	std::cout << "\n";

	// 7. 清理
	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hAlg, 0);
	return 0;
}