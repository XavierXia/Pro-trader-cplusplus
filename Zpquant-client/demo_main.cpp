#include    <Windows.h>
#include    <iostream>
#include    "unistd.h"
#include    <string.h>
#include    "demo_client.h"

int
main(void) 
{
	CDemoClient* demo_client = new CDemoClient();
	//建立和后台的连接
	demo_client->Start();
	demo_client->test_trade();

	while (1)
	{
		Sleep(2000);
	}

	return 0;
}