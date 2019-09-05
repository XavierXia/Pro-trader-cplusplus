#include <assert.h>
#include <winsock2.h>
#include <string.h>
#include <tuple>
#include <iostream>
#include <sstream>
#include <string.h>
#include "rediscpp.h"

using namespace zpquant;
using namespace std;

#define SETSTRING(key, value) \
    stringstream ss;\
    ss << "SET " << key << " " << value;\
    string s;\
    getline(ss, s);\
    return Set(s);

Redis::Redis(string ip, int port) :
	ip(ip),
	port(port),
	redisCon(nullptr), 
	reply(nullptr)
{

}

Redis::~Redis()
{
	freeReply();
	if (nullptr == redisCon)
	{
		redisFree(redisCon);
		redisCon = nullptr;
	}
}

bool Redis::Connect()
{
	if (nullptr != redisCon) {
		return false;
	}

	redisCon = redisConnect(ip.c_str(), port);
	if (redisCon->err)
	{
		std::cerr << "error code : " << redisCon->err << ". " << redisCon->errstr << std::endl;
		return false;
	}

	return true;
}

void Redis::disConnect()
{
     ::redisFree(redisCon);
     cout << "redis disConnect success" << endl;
}

bool Redis::isError()
{
	if (nullptr == reply)
	{
		freeReply();
		disConnect();
		Connect();
		return true;
	}
	return false;
}

void Redis::freeReply()
{
	if (nullptr != reply)
	{
		::freeReplyObject(reply);
		reply = nullptr;
	}
}

void Redis::Get(const string & key)
{
	freeReply();
	reply = (redisReply*)::redisCommand(redisCon, "GET %s", key.c_str());
}

void Redis::Get(const string & key, string & value)
{
	Get(key);
	if (!isError() && reply->type == REDIS_REPLY_STRING)
	{
		value = reply->str;
	}
}

void Redis::Get(const string & key, int & value)
{
	Get(key);
	if (!isError() && reply->type == REDIS_REPLY_STRING)
	{
		value = ::atoi(reply->str);
	}
}

void Redis::Get(const string & key, float & value)
{
	Get(key);
	if (!isError() && reply->type == REDIS_REPLY_STRING)
	{
		value = ::atof(reply->str);
	}
}

bool Redis::Set(const string & key, const string & value)
{
	SETSTRING(key,value);
}

bool Redis::Set(const string & key, const int & value)
{
	SETSTRING(key, value);
}

bool Redis::Set(const string & key, const float & value)
{
	SETSTRING(key, value);
}

bool Redis::Set(const string & data)
{
	bool bret = false;
	freeReply();
	reply = (redisReply*)::redisCommand(redisCon, data.c_str());

	if (!(reply->type == REDIS_REPLY_STATUS && _stricmp(reply->str, "OK") == 0))
	{
		std::cout << reply->str << std::endl;
		std::cout << "Failed to execute " << __FUNCTION__ << std::endl;
		return bret;
	}
	bret = true;
	return bret;
}

bool Redis::Del(const string& key)
{
	freeReply();
	reply = (redisReply*)::redisCommand(redisCon, "DEL %s", key.c_str());

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 1L) return true;
	}

	return false;
}

bool Redis::ExistsKey(const string& key)
{
	freeReply();
	reply = (redisReply*)::redisCommand(redisCon, "exists %s", key.c_str());

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 1L) return true;
	}

	return false;
}

void Redis::Scan(const string& key, vector<string> &allKeyv)
{
	int Cursor = 0;
	bool done = false;

	while (!done)
	{
		freeReply();
		reply = (redisReply*)::redisCommand(redisCon, "SCAN %d MATCH %s COUNT 100", Cursor, key.c_str());
		int type = reply ? (reply->type) : 0;

		if (reply->type == REDIS_REPLY_ARRAY)
		{
			if (reply->elements == 0)
			{
				done = true;
				cout << "get 0 msg from " << key.c_str() << endl;
			}
			else
			{
				//reply->element[0] contains cursor
				Cursor = atoi(reply->element[0]->str);
				//reply->element[1] contains elements array:key1,key2,...
				redisReply ** siteCounters = reply->element[1]->element;

				for (size_t i = 0; i < reply->element[1]->elements; i++)
				{
					string elem = siteCounters[i++]->str;
					allKeyv.push_back(elem);
				}
				if (Cursor == 0)  //scan over
				{
					done = true;
				}
			}
		}
		else
		{
			done = true;
		}
	}
}

//创建list类型数据
bool Redis::Lpush(const string& key, string value)
{
	freeReply();
	reply = (redisReply*)::redisCommand(redisCon, "LPUSH %s %s", key.c_str(),value.c_str());

	if (reply->type == REDIS_REPLY_INTEGER)
	{
		if (reply->integer == 1L) return true;
	}

	return false;
}

//获取list的长度
int Redis::LLen(const string& key)
{
	freeReply();
	reply = (redisReply*)::redisCommand(redisCon, "LLEN %s", key.c_str());
	if (reply->type == REDIS_REPLY_INTEGER)
	{
		return reply->integer;
	}
}

//获取list中某个位置对应的value
void Redis::Lindex(const string& key, int index, string& value)
{
	freeReply();
	reply = (redisReply*)::redisCommand(redisCon, "LINDEX %s %d", key.c_str(),index);
	if (!isError() && reply->type == REDIS_REPLY_STRING)
	{
		value = reply->str;
	}
}

bool Redis::Flushdb()
{
	bool bret = false;
	freeReply();
	reply = (redisReply*)::redisCommand(redisCon, "FLUSHDB");

	if (!(reply->type == REDIS_REPLY_STATUS && _stricmp(reply->str, "OK") == 0))
	{
		return bret;
	}
	bret = true;
	return bret;
}



