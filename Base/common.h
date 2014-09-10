#ifndef __Common_h__
#define __Common_h__
#include "base.h"
#include "shm.h"
#pragma once
namespace Common {
	
	//基本公共操作类
	class base{
	public:
	CSharedMem *CreateShareMem(unsigned int nShmKey, size_t iSize, bool bInitMode);

	};



	//字符串工具类
	class Str
	{
	private:
		int age;
		void get_nextval(const char *T, int next[]);
		//公共方法
	public:
		int  KMP(const char *Text,const char* Pattern);
		Str();
		Str(int x);
		int getAge();
	};

}
#endif