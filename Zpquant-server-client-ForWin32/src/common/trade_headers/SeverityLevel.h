#ifndef _COMMONFILES_COMPRETRADESYSTEMHEADERS_SEVERITYLEVEL_H_
#define _COMMONFILES_COMPRETRADESYSTEMHEADERS_SEVERITYLEVEL_H_

#include <vector>

using namespace std;
enum severity_levels
{
	normal,
	warning,
	error
};

typedef struct _clientListMdInfoT {
	char                clientName[32];//将该项当做key
	char                password[32];
	bool                bTradepermission;
	bool                bLoggedin;
	char				nngIpcMdAddress[64]; //行情中间件连接字符串
	char				subscription[1024];
} ClientListMdInfoT;

typedef struct _clientListTdInfoT {
	char                clientName[32];//将该项当做key
	char                password[32];
	int					nRequestId; //每一个client，唯一对应一个nRequestId，主要用于区分回报
	bool                bTradepermission;
	bool                bLoggedin;
	char				nngIpcTdAddress[64];//交易中间件连接字符串
	char				nngIpcTdResAddress[64];//交易回报中间件连接字符串
	char				subscription[1024];
} ClientListTdInfoT;

typedef struct _clientListTdResInfoT {
	char                clientName[32];//将该项当做key
	int					nRequestId; //每一个client，唯一对应一个nRequestId，主要用于区分回报
	bool                bLoggedin;
	char				nngIpcTdResAddress[64];//交易回报中间件连接字符串
	char				subscription[1024];
	vector<string>			orderlistv; //“当日委托”列表 115
	vector<string>			transactedlistv; //“当日成交”列表 116
	vector<string>			cancelorderlistv; //“已撤销委托” 列表
	vector<string>			untransactedlistv; //“未成交委托”列表 115与116的交集
	vector<string>			rejectedlistv; //“被拒绝委托”列表 6
} ClientListTdResInfoT;

#endif