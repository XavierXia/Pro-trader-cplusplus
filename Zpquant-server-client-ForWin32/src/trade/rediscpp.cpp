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
