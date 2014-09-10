#include "Bctrl.h"
int main()
{
	Bctrl *obj=new Bctrl;
	int i=0;
	char buff[1024];
	while(1)
	{
		sprintf_s(buff,"Send:%d",i++);
		obj->AppendCode((unsigned char*)buff,strlen(buff));
		Sleep(900);
	}

}

