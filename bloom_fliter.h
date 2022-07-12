#ifndef _BLOOM_FLITER_
#define _BLOOM_FLITER_
 
#include<iostream>
#include<vector>
#include<string>
#include<string.h>
 
/*
*一个数据32位，40亿个整数，每个整数需用一位表示，40亿位就完事
*/
 
class BitMap
{
public:
	BitMap()
		:_size(0)
	{}
    
    // size代表多少位，32位用一个unsigned int表示，向上取整
	BitMap(size_t size)
		:_size(0)
	{
		_array.resize((size>>5)+1);  //多少个数据，一个数据占32位，加一是至少一个数据
	}
 
	bool Set(size_t num)
	{
		size_t index = num >> 5;     //计算在哪个数据上
		size_t n = num % 32;
 
		if (_array[index] & (1 << (31 - n)))   //移位问题
		{
			return false;
		}
		else
		{
			size_t a = 1 << (31 - n);
			_array[index] |= a;
			++_size;
			return true;
		}
	}
 
	bool Found(size_t num){
        size_t index = num >> 5;     //计算在哪个数据上
		size_t n = num % 32;
 
		if (_array[index] & (1 << (31 - n)))   //移位问题
		{
			return true;
		}

        return false;
    }
	std::vector<size_t> _array; //数组
	size_t _size;          //位图中数据个数
};
 


// 下面是哈希函数

template<class T>  //各类哈希函数
size_t BKDRHash(const char *str)
{
	register size_t hash = 0;
	while (size_t ch = (size_t)*str++)
	{
		hash = hash * 131 + ch;
	}
	return hash;
}
 
template<class T>
size_t SDBMHash(const char *str)
{
	register size_t hash = 0;
	while (size_t ch = (size_t)*str++)
	{
		hash = 65599 * hash + ch; 
	}
	return hash;
}
 
template<class T>
size_t RSHash(const char * str)
{
	size_t hash = 0;
	size_t magic = 63689;
	while (size_t ch = (size_t)*str++)
	{
		hash = hash * magic + ch;
		magic *= 378551;
	}
	return hash;
}
 
 
template<class T>
size_t APHash(const char *str)
{
	register size_t hash = 0;
	size_t ch;
	for (long i = 0; ch = (size_t)*str++; i++)
	{
		if ((i & 1) == 0)
		{
			hash ^= ((hash << 7) ^ ch ^ (hash >> 3));
		}
		else
		{
			hash ^= (~((hash << 11) ^ ch ^ (hash >> 5)));
		}
	}
	return hash;
}
 
 
template<class T>
size_t JSHash(const char* str)
{
	if (!*str)
	{
		return 0;
	} 
	size_t hash = 1315423911;
	while (size_t ch = (size_t)*str++)
	{
		hash ^= ((hash << 5) + ch + (hash >> 2));
	}
	return hash;
}


template<class T>
struct __HashFun1          //5种哈希函数对应的仿函数
{
	size_t operator()(const T& key) const 
	{
		return BKDRHash<T>(key.c_str());
	}
};
 
template<class T>
struct __HashFun2
{
	size_t operator()(const T& key) const
	{
		return SDBMHash<T>(key.c_str());
	}
};
 
template<class T>
struct __HashFun3
{
	size_t operator()(const T& key) const 
	{
		return RSHash<T>(key.c_str());
	}
};
 
template<class T>
struct __HashFun4
{
	size_t operator()(const T& key) const 
	{
		return APHash<T>(key.c_str());
	}
};
 
template<class T>
struct __HashFun5
{
	size_t operator()(const T& key) const
	{
		return JSHash<T>(key.c_str());
	}
};
 
 
template<class K = std::string,
class HashFun1 = __HashFun1<K>,
class HashFun2 = __HashFun2<K>,
class HashFun3 = __HashFun3<K>,
class HashFun4 = __HashFun4<K>,
class HashFun5 = __HashFun5<K>>
class Bloom_Filter
{
public:

	Bloom_Filter(size_t size = 500000)
		:_capacity(size)
	{
		_bitmap._array.resize((size >> 5) + 1);
	}
 
	void _Set(const K& key)
	{
		_bitmap.Set(HashFun1()(key) % _capacity);
		_bitmap.Set(HashFun2()(key) % _capacity);
		_bitmap.Set(HashFun3()(key) % _capacity);
		_bitmap.Set(HashFun4()(key) % _capacity);
		_bitmap.Set(HashFun5()(key) % _capacity);
	}
 
	bool _IsIn(const K& key)
	{
		if (!_bitmap.Found(HashFun1()(key) % _capacity))
			return false;
		if (!_bitmap.Found(HashFun1()(key) % _capacity))
			return false;
		if (!_bitmap.Found(HashFun1()(key) % _capacity))
			return false;
		if (!_bitmap.Found(HashFun1()(key) % _capacity))
			return false;
		if (!_bitmap.Found(HashFun1()(key) % _capacity))
			return false;
		return true;
	}
private:
	BitMap _bitmap;
	size_t _capacity;
};

#endif
