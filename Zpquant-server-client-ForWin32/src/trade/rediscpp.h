#ifndef REDIS_CPP_INCLUDED
#define REDIS_CPP_INCLUDED

//#pragma comment(lib, "hiredis.lib")
//#pragma comment(lib, "Win32_Interop.lib")

#include <assert.h>
#include <winsock2.h>
#include <string.h>
#include <tuple>
#include <string.h>
#include "hiredis/hiredis.h"

using namespace std;

namespace zpquant {

	class Redis
	{
	public:
		Redis(string ip = "127.0.0.1", int port = 6379);
		~Redis();
		bool Connect();
		void freeReply();

		//暂时不使用模板
		void Get(const string & key, string & value);
		void Get(const string & key, int & value);
		void Get(const string & key, float & value);

		bool Set(const string & key, const string & value);
		bool Set(const string & key, const int & value);
		bool Set(const string & key, const float & value);

		//删除
		bool Del(const string& key);
		//检查某个key是否存在
		bool ExistsKey(const string& key);
	private:


		void Get(const string & key);
		bool Set(const string & data);

	private:
		bool isError();
		void disConnect();

	private:
		std::string ip;
		int port;
		redisContext* redisCon;
		redisReply * reply;
	};

}

#endif


/*
int main()
{
	Redis r("127.0.0.1", 6379);
	bool b = r.Connect;
	if (!b)
		return -1;

	r.Set("testtimes", 1);
	r.Set("float:pi", 3.14159265);
	r.Set("string", "test");


	r.HashSet("hset", "myhash", "field1", 123.2342343);
	r.HashSet("hmset", "myhash", "field1", 1111, "field2", "f2");
	r.HashSet("hset", "myhash", "field1", 123.2342343);
	r.HashSet("hmset", "myhash", "field1", 1111, "field2", "f2");

	//wrong command
	r.HashSet("hset", "myhash", "field1", 1, 123.2342343);
	r.HashSet("hmset", "myhash", "field1", 1, 1111, "field2", "f2");

	return 0;
}
*/