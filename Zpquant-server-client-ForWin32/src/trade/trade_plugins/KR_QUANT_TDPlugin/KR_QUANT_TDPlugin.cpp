#include "KR_QUANT_TDPlugin.h"
#include <stdarg.h>
#include <thread>
#include <iostream>

#define SOCKET_ADDRESS "tcp://47.105.111.100:8001"
#define ADDRESS1 "inproc://test"
#define ADDRESS2 "tcp://127.0.0.1:8000"
#define ADDRESS3 "ipc:///tmp/reqreptd.ipc"

const string CKR_QUANT_TDPlugin::s_strAccountKeyword = "username;password;";
extern char ProcessName[256];
#define NOTIFY_LOGIN_SUCCEED {m_boolIsOnline = true; std::unique_lock<std::mutex> lk(m_mtxLoginSignal);m_cvLoginSignalCV.notify_all();}
#define NOTIFY_LOGIN_FAILED  {m_boolIsOnline = false;std::unique_lock<std::mutex> lk(m_mtxLoginSignal);m_cvLoginSignalCV.notify_all();}
//const char THE_CONFIG_FILE_NAME[150]="E:\work\tools\zpquant-trader-forWin32\zpquantForWin32\zpquantForWin32\src\third\Kr360Quant\conf\oes_client.conf";
const char THE_CONFIG_FILE_NAME[150] = "oes_client.conf";

#define NAME ("kr_quant_td")

date CKR_QUANT_TDPlugin::GetTradeday(ptime _Current)
{
	if (_Current.time_of_day() < time_duration(15, 31, 0, 0))//
		return _Current.date();
	else
	{
		if (_Current.date().day_of_week().as_enum() == Friday)
			return _Current.date() + days(3);
		else
			return _Current.date() + days(1);
	}
}


CKR_QUANT_TDPlugin::CKR_QUANT_TDPlugin():m_StartAndStopCtrlTimer(m_IOservice)
{
    tdnnsocket.socket_set(AF_SP, NN_PAIR);
	Redis.Connect();
    //tdnnsocket.connect(ADDRESS3);
}

CKR_QUANT_TDPlugin::~CKR_QUANT_TDPlugin()
{

}

bool CKR_QUANT_TDPlugin::IsOnline()
{
	return true;
}


void CKR_QUANT_TDPlugin::CheckSymbolValidity(const unordered_map<string, string> & insConfig)
{
	
}

string CKR_QUANT_TDPlugin::GetCurrentKeyword()
{
	return NAME;
}

string CKR_QUANT_TDPlugin::GetProspectiveKeyword(const ptree & in)
{
	string retKey = NAME;
	return retKey;
}

void CKR_QUANT_TDPlugin::GetState(ptree & out)
{
	out.put("online", "true");
}

void CKR_QUANT_TDPlugin::ShowMessage(severity_levels lv, const char * fmt, ...)
{
	char buf[1024];
	va_list arg;
	va_start(arg, fmt);
	vsnprintf(buf, 1024, fmt, arg);
	va_end(arg);
	boost::log::sources::severity_logger< severity_levels > m_Logger;
	BOOST_LOG_SEV(m_Logger, lv) << "[" << to_iso_string(microsec_clock::universal_time() + hours(8)) << "] " << m_strUsername.c_str() << ": " << buf;
}

void CKR_QUANT_TDPlugin::TDAddClientUser(const ptree &in, ptree &out, MTradePluginContextInterface* pTradePluginContext, unsigned int AccountNumber, string & strConfigFile)
{
	cout << "...come in TDAddClientUser,TDInit" << endl;
	ptree g_Config;
	boost::property_tree::read_json(strConfigFile, g_Config);
}

/*
./thunder-trader|0: ...ReqAddSource, ptree, in: {
    "type": "reqaddmarketdatasource",
    "sourcetype": "kr_td_quant",
    "username": "1111",
    "password": "11"
}
*/
void CKR_QUANT_TDPlugin::TDInit(const ptree & in, ptree & out, MTradePluginContextInterface * pTradePluginContext, unsigned int AccountNumber, string & strConfigFile)
{
	cout << "...CKR_QUANT_TDPlugin,TDInit" << endl;
	out.put("clientType", "td");

    auto temp = in.find("username");
    if (temp != in.not_found())
    {
        m_strUsername = temp->second.data();
        if(m_strUsername.size()>32)
		{
			out.put("username", m_strUsername.c_str());
			out.put("errCode", "1");//若errCode为1，表示登录失败
			out.put("reason", "kr360:username is too long");
			ShowMessage(severity_levels::error, "login error,%s", "kr360:username is too long");
			return;
		}
        else if(m_strUsername.empty())
		{
			out.put("username", m_strUsername.c_str());
			out.put("errCode", "1");//若errCode为1，表示登录失败
			out.put("reason", "kr360:username is empty");
			ShowMessage(severity_levels::error, "login error,%s", "kr360:username is empty");
			return;
		}
    }
    else
	{
		out.put("username", "");
		out.put("errCode", "1");//若errCode为1，表示登录失败
		out.put("reason", "kr360:Can not find <username>");
		ShowMessage(severity_levels::error, "login error,%s", "kr360:Can not find <username>");
		return;
	}

    temp = in.find("password");
    if (temp != in.not_found())
    {
        m_strPassword = temp->second.data();
        if(m_strPassword.size()>32)
		{
			out.put("username", m_strUsername.c_str());
			out.put("errCode", "1");//若errCode为1，表示登录失败
			out.put("reason", "kr360:password is too long");
			ShowMessage(severity_levels::error, "login error,%s", "kr360:password is too long");
			return;
		}
        else if(m_strPassword.empty())
		{
			out.put("username", m_strUsername.c_str());
			out.put("errCode", "1");//若errCode为1，表示登录失败
			out.put("reason", "kr360:password is empty");
			ShowMessage(severity_levels::error, "login error,%s", "kr360:password is empty");
			return;
		}
    }
    else
	{
		out.put("username", m_strUsername.c_str());
		out.put("errCode", "1");//若errCode为1，表示登录失败
		out.put("reason", "kr360:Can not find <password>");
		ShowMessage(severity_levels::error, "login error,%s", "kr360:Can not find <password>");
		return;
	}

	//初始化登录账号
	ptree g_Config;
	boost::property_tree::read_json(strConfigFile, g_Config);

	if (g_Config.find("td_userlist") != g_Config.not_found())
	{
		ptree pos_array = g_Config.get_child("td_userlist");
		ptree::iterator pos = pos_array.begin();
		for (; pos != pos_array.end(); ++pos)
		{
			string username = pos->second.get<string>("username");
			string password = pos->second.get<string>("password");
			uint8 tradepermission = pos->second.get<uint8>("tradepermission");
			cout << "...TDInit: " << username << " | " << password << endl;
			ClientListTdInfoT clientT;
			memset(clientT.clientName, 0, 32 * sizeof(char));
			memcpy(clientT.clientName, username.c_str(), 32 * sizeof(char));
			memset(clientT.password, 0, 32 * sizeof(char));
			memcpy(clientT.password, password.c_str(), 32 * sizeof(char));
			clientT.bLoggedin = false;
			if (tradepermission == 1)
				clientT.bTradepermission = true;
			else
				clientT.bTradepermission = false;

			memset(clientT.nngIpcTdAddress, 0, 64 * sizeof(char));
			sprintf(clientT.nngIpcTdAddress, "ipc:///%s/td.ipc", username.c_str());
			memset(clientT.nngIpcTdResAddress, 0, 64 * sizeof(char));
			sprintf(clientT.nngIpcTdResAddress, "ipc:///%s/tdres.ipc", username.c_str());
			tclient.insert(pair<string, ClientListTdInfoT>(username, clientT));
		}
	}
	else
	{
		//没有指定用户，因此不支持任何一个用户登录
		out.put("username", m_strUsername.c_str());
		out.put("errCode", "1");//若errCode为1，表示登录失败
		//out.put("reason", "server端没有指定支持的用户列表");
		out.put("reason", "no userlist in server config");
		ShowMessage(severity_levels::error, "login error,%s", "kr360:no userlist in server config");
		return;
	}

	//进行校验
	map<string, ClientListTdInfoT>::iterator itr;
	string addrres;
	itr = tclient.find(m_strUsername);
	if (itr != tclient.end()) //发现
	{
		string pw = itr->second.password;
		string addr = itr->second.nngIpcTdAddress;
		addrres = itr->second.nngIpcTdResAddress;
		if (pw != m_strPassword)
		{
			out.put("username", m_strUsername.c_str());
			out.put("errCode", "1");//若errCode为1，表示登录失败
			//out.put("reason", "登录用户密码错误");
			out.put("reason", "password error");
			return; //拒绝
		}

		out.put("username", m_strUsername.c_str());
		out.put("nngIpcTdAddress", addr.c_str());
		out.put("nngIpcTdResAddress", addrres.c_str());
		out.put("errCode", "0");//若errCode为0，表示登录成功
		out.put("reason", "login succeed");
		ShowMessage(normal, "login success, username:%s", m_strUsername.c_str());

		itr->second.bLoggedin = true;
		//初始化nng连接字符串
		//nnsocket.socket_set(AF_SP, NN_PAIR);
		tdnnsocket.connect(addr.c_str());
		//nnsocket.connect(ADDRESS3);
		//string addrs = ADDRESS3;
		//nnsocket.connect(addrs.c_str());
	}
	else
	{
		out.put("username", m_strUsername.c_str());
		out.put("errCode", "1");//若errCode为1，表示登录失败
		out.put("reason", "username no exist");
		return; //拒绝登录
	}

    pOesApi = new Quant360::OesClientApi();
    pOesSpi = new OesClientMySpi();

	ClientListTdResInfoT TdResInfoT;
	memset(TdResInfoT.clientName, 0, 32 * sizeof(char));
	memcpy(TdResInfoT.clientName, m_strUsername.c_str(), 32 * sizeof(char));
	memset(TdResInfoT.nngIpcTdResAddress, 0, 64 * sizeof(char));
	sprintf(TdResInfoT.nngIpcTdResAddress, "%s", addrres.c_str());
	pOesSpi->initTdResclient(m_strUsername, TdResInfoT);

    if (!pOesApi || !pOesSpi) 
    {
        ShowMessage(severity_levels::error,"pOesApi or pOesSpi new error!");
        return;
    }

    /* 打印API版本信息 */
    //ShowMessage(severity_levels::normal, "OesClientApi 版本: %s\n",
            //Quant360::OesClientApi::GetVersion());

    /* 注册spi回调接口 */
    pOesApi->RegisterSpi(pOesSpi);

    /* 加载配置文件 */
    if (! pOesApi->LoadCfg(THE_CONFIG_FILE_NAME)) {
        ShowMessage(severity_levels::error,"加载配置文件失败!");
        return;
    }

    /*
     * 设置客户端本地的设备序列号
     * @note 为满足监管需求，需要设置客户端本机的硬盘序列号
     */
    pOesApi->SetCustomizedDriverId("C02TL13QGVC8");

    //Start();

	m_StartAndStopCtrlTimer.expires_from_now(boost::posix_time::seconds(3));
	m_StartAndStopCtrlTimer.async_wait(boost::bind(
		&CKR_QUANT_TDPlugin::TimerHandler,
		this,
		&m_StartAndStopCtrlTimer,
		boost::asio::placeholders::error));
	m_futTimerThreadFuture = std::async(std::launch::async, [this] {
		this->m_IOservice.run();
		return true;
	});
}

void CKR_QUANT_TDPlugin::TimerHandler(boost::asio::deadline_timer* timer, const boost::system::error_code& err)
{
	cout << "... CKR_QUANT_TDPlugin::TimerHandler!\n" << endl;

	if (err)
	{
		ShowMessage(normal, "%s: Timmer is canceled.%s", GetCurrentKeyword().c_str(), err.message().c_str());
	}
	else
	{
		ptime  time_now = second_clock::universal_time() + hours(8);
		time_duration tid = time_now.time_of_day();
		ptime nextActiveTime = not_a_date_time;
		if (tid >= time_duration(0, 0, 0, 0) && tid < time_duration(9, 0, 0, 0))
		{
			nextActiveTime = ptime(time_now.date(), time_duration(9, 0, 0, 0));
		}
		//为当日初始化阶段
		if (tid >= time_duration(9, 0, 0, 0) && tid < time_duration(9, 5, 0, 0))
		{
			nextActiveTime = ptime(time_now.date(), time_duration(9, 5, 0, 0));
			//若是早上九点到九点十五分，就开始变量初始化等每日启动操作
			cout << "... CKrQuantMDPluginImp::TimerHandler，开始变量初始化等每日启动操作!\n" << endl;
			/*
			0.检测宽睿是否连接
			1.检测是否交易日;
			2.将订单相关的list变量全部清空;
			3.交易日下单笔数记数重置;
			*/
		}
		if (tid >= time_duration(9, 5, 0, 0) && tid < time_duration(9, 15, 0, 0))
		{
			nextActiveTime = ptime(time_now.date(), time_duration(9, 15, 0, 0));
		}
		else if (tid >= time_duration(9, 15, 0, 0) && tid < time_duration(11, 30, 0, 0))
		{
			Start();
			nextActiveTime = ptime(time_now.date(), time_duration(11, 30, 0, 0));
		}
		else if (tid >= time_duration(11, 30, 0, 0) && tid < time_duration(13, 0, 0, 0))
		{
			//Stop();
			Start();
			nextActiveTime = ptime(time_now.date(), time_duration(13, 0, 0, 0));
		}
		else if (tid >= time_duration(13, 0, 0, 0) && tid < time_duration(14, 56, 0, 0))
		{
			//todo
			Start();
			nextActiveTime = ptime(time_now.date(), time_duration(14, 56, 0, 0));
		}
		else if (tid >= time_duration(14, 56, 0, 0) && tid < time_duration(14, 56, 40, 0))
		{
			nextActiveTime = ptime(time_now.date(), time_duration(15, 00, 0, 0));

			//每日撤销全部未成交订单
			char keyName[60];
			keyName[0] = '\0';
			sprintf(keyName, "wcj-%d-*", 123);
			vector<string> valueRV;
			Redis.Scan(keyName, valueRV);

			if (valueRV.size() > 0)
			{
				for (int iv = 0; iv < valueRV.size(); iv++)
				{
					string kv = valueRV[iv];

					ptree c_value;
					std::stringstream jmsg(kv.c_str());
					try {
						boost::property_tree::read_json(jmsg, c_value);
					}
					catch (std::exception & e) {
						fprintf(stdout, "cannot parse from string 'msg(c_value,redis value,14:56 cancelOrder)' \n");
						continue;
					}

					//每次撤单前都需要检查一下是否超过撤单总数
					bool isCancelOrderLimited = false;
					string listKey = "cancelOrderCheckList";
					bool isExist = Redis.ExistsKey(listKey);
					if (isExist) //这个list存在
					{
						if (Redis.LLen(listKey) >= 10000)
						{
							isCancelOrderLimited = true;
						}
						Redis.Lpush(listKey, "0");
					}
					else //这个list不存在，就创建并插入
					{
						Redis.Lpush(listKey, "0");
					}

					//未超过总的撤单数之后，进行尾盘撤单操作
					if (!isCancelOrderLimited)
					{
						uint8 mktId = c_value.get<uint8>("mktId");
						int64 clOrdId = c_value.get<int64>("clOrdId");

						/* 通过待撤委托的 clOrdId 进行撤单 */
						int ret = OesClientMain_CancelOrder(pOesApi, mktId, NULL, NULL, 0, 0, clOrdId);
						ShowMessage(normal, "ret:%d, sendOrder:%s", ret, kv.c_str());
						//单位为毫秒
						Sleep(1);
					}
					else //超过了撤单总数
					{
						ptree pt;
						pt.put("type", "cancelOrderCheck");
						pt.put("newOrder", kv);
						pt.put("isCancelOrderLimited", "1"); //1表示超过了撤单总数

						std::ostringstream buf2;
						write_json(buf2, pt, false);
						tdnnsocket.send(buf2.str().c_str(), strlen(buf2.str().c_str()) + 1, 0);

						//保存被server端拒绝的订单
						ptime epoch(date(1970, Jan, 1));
						time_duration time_from_epoch = microsec_clock::universal_time() - epoch;
						int64 currentStamp = time_from_epoch.total_microseconds();
						string cStampStr = boost::lexical_cast<string>(currentStamp);
						keyName[0] = '\0';
						sprintf(keyName, "jjsvr-%d-%s", 124, cStampStr.c_str());
						Redis.Set(keyName, kv);
					}
				}
			}
		}
		else if (tid >= time_duration(15, 0, 0, 0) && tid < time_duration(15, 30, 0, 0))
		{
			//todo
			Start();
			nextActiveTime = ptime(time_now.date(), time_duration(15, 30, 0, 0));
		}
		else if (tid >= time_duration(15, 30, 0, 0) && tid < time_duration(15, 35, 0, 0))
		{
			nextActiveTime = ptime(time_now.date(), time_duration(15, 35, 0, 0));
			//每日15点30到15点35分，存入当日委托列表、当日成交列表、当日未成交列表、当日被拒绝列表
			//TODO
		}
		else if (tid >= time_duration(15, 35, 0, 0) && tid < time_duration(23, 59, 59, 999))
		{
			//test时Start，正常情况应为Stop
			//Stop(); //若为Stop，系统不会开启各种线程接收交易数据
			Start();
			nextActiveTime = ptime(time_now.date() + days(1), time_duration(9, 0, 0, 0));
		}

		m_StartAndStopCtrlTimer.expires_at(nextActiveTime);
		m_StartAndStopCtrlTimer.async_wait(boost::bind(&CKR_QUANT_TDPlugin::TimerHandler, this,
			&m_StartAndStopCtrlTimer,boost::asio::placeholders::error));
		ShowMessage(normal, "%s: Next:%s\n", GetCurrentKeyword().c_str(), to_simple_string(nextActiveTime).c_str());
	}
}

void * CKR_QUANT_TDPlugin::tdThreadMain(void *pParams)
{
    CKR_QUANT_TDPlugin *tdimp = (CKR_QUANT_TDPlugin *) pParams;
    char buf[1024];
    while(1)
    {
        int rc = tdimp->tdnnsocket.recv(buf, sizeof(buf), 0);
        cout<<"...CKR_QUANT_TDPlugin,tdThreadMain recv: " << buf << endl;

        ptree c_Config;
		//bool isAbleParseJson = true;
        std::stringstream jmsg(buf);  
        try {
            boost::property_tree::read_json(jmsg, c_Config);
        }
        catch(std::exception & e){
            fprintf(stdout, "cannot parse from string 'msg'(CKR_QUANT_TDPlugin,tdThreadMain) \n");
            //return NULL;
			//isAbleParseJson = false;
			continue;
        }

        auto temp = c_Config.find("type");
        if (temp != c_Config.not_found())
        {
            string sType = temp->second.data();
            if(sType == "query")
            {
                auto cate = c_Config.find("category");
                string sCate;

                if (cate != c_Config.not_found()) sCate = cate->second.data();
                if(sCate == "clientOverview"){
                    /* 查询 客户端总览信息 */
                    tdimp->OesClientMain_QueryClientOverview(tdimp->pOesApi);

                }else if(sCate == "cashAsset"){
                    /* 查询 所有关联资金账户的资金信息 */
                    tdimp->OesClientMain_QueryCashAsset(tdimp->pOesApi, NULL);
                }else if(sCate == "stkInfo"){
                    /* 查询 指定上证 600000 的产品信息 */
                    //auto scode = c_Config.find("code");
                    //string code;
                    //if (scode != c_Config.not_found()) code = scode->second.data();

					string code = c_Config.get<string>("code");
					uint8 sclb = c_Config.get<uint8>("sclb");
					int32 requestId = c_Config.get<int32>("requestId");

                    tdimp->OesClientMain_QueryStock(tdimp->pOesApi, code.c_str(), sclb, OES_SECURITY_TYPE_STOCK,OES_SUB_SECURITY_TYPE_UNDEFINE);
                
				}else if(sCate == "stkHolding"){
                    auto scode = c_Config.find("code");
                    auto ssclb = c_Config.find("sclb");
                    string code;
                    string sclb;

                    if (scode != c_Config.not_found()) code = scode->second.data();
                    if (ssclb != c_Config.not_found()) sclb = ssclb->second.data();

                    if(code == "allStk"){
                        /* 查询 沪深两市 所有股票持仓 */
                        cout << "...allStk\n" << endl;
                        tdimp->OesClientMain_QueryStkHolding(tdimp->pOesApi, OES_MKT_ID_UNDEFINE, NULL);
                    }else{
                        if(sclb == "1"){ //上海
                            tdimp->OesClientMain_QueryStkHolding(tdimp->pOesApi, OES_MKT_ID_SH_A, code.c_str());
                        }else{ //深圳
                            tdimp->OesClientMain_QueryStkHolding(tdimp->pOesApi, OES_MKT_ID_SZ_A, code.c_str());
                        }
                    }
                }
            }
			else if(sType == "buy" || sType == "sell")
			{
                auto scode = c_Config.find("code");
                auto ssclb = c_Config.find("sclb");
                auto swtfs = c_Config.find("wtfs");
                auto samount = c_Config.find("amount");
                auto sprice = c_Config.find("price");
                string code;
                string sclb;
                string wtfs;
                string amount;
                string price;

                if (scode != c_Config.not_found()) code = scode->second.data();
                if (ssclb != c_Config.not_found()) sclb = ssclb->second.data();
                if (swtfs != c_Config.not_found()) wtfs = swtfs->second.data();
                if (samount != c_Config.not_found()) amount = samount->second.data();
                if (sprice != c_Config.not_found()) price = sprice->second.data();

                uint8 mmbz,mktId;
                if(sType == "buy"){
                    mmbz = 1; //OES_BS_TYPE_BUY
                }else{
                    mmbz = 2; //OES_BS_TYPE_SELL 
                }

                if(sclb == "1"){
                    mktId = 1; 
                }else{
                    mktId = 2;
                }

				char keyName[60];

				//检查该笔订单股票数量不超过9999手
				//**************************************************
				bool isWTSLLimited = false;
				if (atoi(amount.c_str()) >= 9999 * 100)
				{
					isWTSLLimited = true;
				}

				//委托数量超过限制
				if (isWTSLLimited)
				{
					ptree pt;
					pt.put("type", "stockAmountCheck");
					pt.put("newOrder", buf);
					pt.put("oldOrder", "");
					pt.put("isRejected", "1"); //1表示委托数量超过限制

					std::ostringstream buf2;
					write_json(buf2, pt, false);
					tdimp->tdnnsocket.send(buf2.str().c_str(), strlen(buf2.str().c_str()) + 1, 0);

					//保存被server端拒绝的订单
					ptime epoch(date(1970, Jan, 1));
					time_duration time_from_epoch = microsec_clock::universal_time() - epoch;
					int64 currentStamp = time_from_epoch.total_microseconds();
					string cStampStr = boost::lexical_cast<string>(currentStamp);
					keyName[0] = '\0';
					sprintf(keyName, "jjsvr-%d-%s", 124, cStampStr.c_str());
					tdimp->Redis.Set(keyName, buf);

					continue;
				}

				//是否会自成交标志
				//每笔下单前检查“未成交委托列表”，如何存在买卖方向相反，代码、市场类别均相同，
				//	价格存在重叠区间（买价大于等于卖价的）的未成交订单，则拒绝转发该笔委托。
				//**************************************************
				bool isZCJ = false;
				string ZCJString;

				//检查是否会出现自成交问题
				keyName[0] = '\0';
				sprintf(keyName, "wcj-%d-%s-%d-*", 123, code.c_str(), mktId);
				vector<string> valueRV;
				tdimp->Redis.Scan(keyName,valueRV);
				if (valueRV.size() > 0)
				{
					for (int iv = 0; iv < valueRV.size(); iv++)
					{
						string kkey = valueRV[iv];
						string kv;
						tdimp->Redis.Get(kkey,kv);
						//if (kv.length() <= 0) continue;

						ptree c_value;
						std::stringstream jmsg(kv.c_str());
						try {
							boost::property_tree::read_json(jmsg, c_value);
						}
						catch (std::exception & e) {
							fprintf(stdout, "cannot parse from string 'msg(c_value,redis value)' \n");
							//return false;
							continue;
						}

						string securityIdR = c_value.get<string>("securityId");
						int8 mktIdR = c_value.get<int8>("mktId");
						int8 bsTypeR = c_value.get<int8>("bsType");
						int ordPrice = c_value.get<int>("ordPrice");

						if(code == securityIdR && mktId == mktIdR)
						{
							if (mmbz == 1 && bsTypeR == 2) //下单委托为买，而未成交中为卖
							{
								if (atoi(price.c_str()) >= ordPrice)
								{
									isZCJ = true;
									ZCJString = kv;
									break;
								}
							}
							else if(mmbz == 2 && bsTypeR == 1) //下单委托为卖，而未成交中为买
							{
								if (ordPrice >= atoi(price.c_str()))
								{
									isZCJ = true;
									ZCJString = kv;
									break;
								}
							}
						}
					}
				}

				//出现了自成交
				if (isZCJ)
				{
					ptree pt;
					pt.put("type", "selfmake-transaction");
					pt.put("newOrder", buf);
					pt.put("oldOrder", ZCJString);
					pt.put("isRejected", "1"); //1表示这两个订单出现自成交

					std::ostringstream buf2;
					write_json(buf2, pt, false);
					tdimp->tdnnsocket.send(buf2.str().c_str(), strlen(buf2.str().c_str()) + 1, 0);

					//保存被server端拒绝的订单
					ptime epoch(date(1970, Jan, 1));
					time_duration time_from_epoch = microsec_clock::universal_time() - epoch;
					int64 currentStamp = time_from_epoch.total_microseconds();
					string cStampStr = boost::lexical_cast<string>(currentStamp);
					keyName[0] = '\0';
					sprintf(keyName, "jjsvr-%d-%s", 124, cStampStr.c_str());
					tdimp->Redis.Set(keyName, buf);

					continue;
				}

				//是否出现isTurnover,每笔下单前检查前推一秒至该笔下单之间的委托数量，
				//即本次下单记数减去一秒前下单记数。若数量大于300，则拒绝转发该笔委托
				//**************************************************
				bool isTurnover = false;
				string listKey = "turnoverCheckList";
				bool isExist = tdimp->Redis.ExistsKey(listKey);
				if (isExist) //这个list存在
				{
					//if (tdimp->Redis.LLen(listKey) >= 300)
					//测试时改成5
					//测试完毕需要改回300
					if (tdimp->Redis.LLen(listKey) >= 5)
					{
						ptime epoch(date(1970, Jan, 1));
						time_duration time_from_epoch = microsec_clock::universal_time() - epoch;
						int64 currentStamp = time_from_epoch.total_microseconds();

						string listStampStr;
						//tdimp->Redis.Lindex(listKey,299,listStampStr);
						//测试时改成4
						//测试完毕需要改回299
						tdimp->Redis.Lindex(listKey, 4, listStampStr);
						int64 listStampInt = boost::lexical_cast<int64>(listStampStr);

						//以微秒为单位
						if (currentStamp - listStampInt < 1000000) //小于1秒
						{
							isTurnover = true;
						}
						else
						{
							string cStampStr = boost::lexical_cast<string>(currentStamp);
							tdimp->Redis.Lpush(listKey, cStampStr);
						}
					}
					else
					{
						ptime epoch(date(1970, Jan, 1));
						time_duration time_from_epoch = microsec_clock::universal_time() - epoch;
						int64 currentStamp = time_from_epoch.total_microseconds();
						string cStampStr = boost::lexical_cast<string>(currentStamp);

						tdimp->Redis.Lpush(listKey, cStampStr);
					}
				}
				else //这个list不存在，就创建并插入,不需要
				{
					ptime epoch(date(1970, Jan, 1));
					time_duration time_from_epoch = microsec_clock::universal_time() - epoch;
					int64 currentStamp = time_from_epoch.total_microseconds();
					string cStampStr = boost::lexical_cast<string>(currentStamp);

					tdimp->Redis.Lpush(listKey,cStampStr);
				}

				//出现了Turnover
				if (isTurnover)
				{
					ptree pt;
					pt.put("type", "turnover");
					pt.put("newOrder", buf);
					pt.put("oldOrder", "");
					pt.put("isRejected", "1"); //1表示出现Turnover

					std::ostringstream buf2;
					write_json(buf2, pt, false);
					tdimp->tdnnsocket.send(buf2.str().c_str(), strlen(buf2.str().c_str()) + 1, 0);

					//保存被server端拒绝的订单
					ptime epoch(date(1970, Jan, 1));
					time_duration time_from_epoch = microsec_clock::universal_time() - epoch;
					int64 currentStamp = time_from_epoch.total_microseconds();
					string cStampStr = boost::lexical_cast<string>(currentStamp);
					keyName[0] = '\0';
					sprintf(keyName, "jjsvr-%d-%s", 124, cStampStr.c_str());
					tdimp->Redis.Set(keyName, buf);

					continue;
				}
				
				//没有出现自成交,没有出现turnover
				if (!isZCJ && !isTurnover) 
				{
					int32 ret;
					if (wtfs == "0") {//限价
						cout << "...下单操作：sType: " << sType << endl;
						ret = tdimp->OesClientMain_SendOrder(tdimp->pOesApi, mktId, code.c_str(), NULL,
							OES_ORD_TYPE_LMT, mmbz, atoi(amount.c_str()), atoi(price.c_str()));
					}
					else { //市价
						ret = tdimp->OesClientMain_SendOrder(tdimp->pOesApi, mktId, code.c_str(), NULL,
							OES_ORD_TYPE_SZ_MTL_BEST, mmbz, atoi(amount.c_str()), atoi(price.c_str()));
					}
					tdimp->ShowMessage(normal, "ret:%d, sendOrder:%s", ret, buf);
				}
            }
            else if(sType == "cancelOrder")
			{
				//撤单数检查:每笔撤单前检查总撤单数量，总数达到10000，则不再接受新的下单。
				bool isCancelOrderLimited = false;
				string listKey = "cancelOrderCheckList";
				bool isExist = tdimp->Redis.ExistsKey(listKey);
				if (isExist) //这个list存在
				{
					//测试时改成5或3
					//测试完毕需要改回10000
					if (tdimp->Redis.LLen(listKey) >= 3)
					{
						isCancelOrderLimited = true;
					}
					tdimp->Redis.Lpush(listKey, "0");
				}
				else //这个list不存在，就创建并插入
				{
					tdimp->Redis.Lpush(listKey, "0");
				}

				//未超过总的撤单数
				if (!isCancelOrderLimited)
				{
					uint8 mktId = c_Config.get<uint8>("mktId");
					int32 origClSeqNo = c_Config.get<int32>("origClSeqNo");
					int8 origClEnvId = c_Config.get<int8>("origClEnvId");
					int64 origClOrdId = c_Config.get<int64>("origClOrdId");

					int ret;
					if (origClOrdId != 0)
					{
						/* 通过待撤委托的 clOrdId 进行撤单 */
						ret = tdimp->OesClientMain_CancelOrder(tdimp->pOesApi, mktId, NULL, NULL, 0, 0, origClOrdId);
					}
					else
					{
						/*
						 * 通过待撤委托的 clSeqNo 进行撤单
						 * - 如果撤单时 origClEnvId 填0，则默认会使用当前客户端实例的 clEnvId 作为
						 *   待撤委托的 origClEnvId 进行撤单
						 */
						ret = tdimp->OesClientMain_CancelOrder(tdimp->pOesApi, mktId, NULL, NULL, origClSeqNo, origClEnvId, 0);
					}

					tdimp->ShowMessage(normal, "ret:%d, sendOrder:%s", ret, buf);
				}
				else //超过了撤单总数
				{
					ptree pt;
					pt.put("type", "cancelOrderCheck");
					pt.put("newOrder", buf);
					pt.put("oldOrder", "");
					pt.put("isRejected", "1"); //1表示超过了撤单总数

					std::ostringstream buf2;
					write_json(buf2, pt, false);
					tdimp->tdnnsocket.send(buf2.str().c_str(), strlen(buf2.str().c_str()) + 1, 0);

					//保存被server端拒绝的订单
					ptime epoch(date(1970, Jan, 1));
					time_duration time_from_epoch = microsec_clock::universal_time() - epoch;
					int64 currentStamp = time_from_epoch.total_microseconds();
					string cStampStr = boost::lexical_cast<string>(currentStamp);
					char keyName[60];
					keyName[0] = '\0';
					sprintf(keyName, "jjsvr-%d-%s", 124, cStampStr.c_str());
					tdimp->Redis.Set(keyName, buf);
				}
            }
			else if(sType == "GetTradingDay")
			{ //获取是否为交易日

				int32 tradeDay = tdimp->pOesApi->GetTradingDay();
				cout << "...sType:GetTradingDay: " << to_string(tradeDay) << endl;

				ptree pt;
				pt.put("type", "tradingDay");
				pt.put("value", tradeDay);
				pt.put("isTradingDay", "1"); //写个默认值，1表示今天为交易日

				std::ostringstream buf;
				write_json(buf, pt, false);
				tdimp->tdnnsocket.send(buf.str().c_str(), strlen(buf.str().c_str()) + 1, 0);
            }
        }
        else
            throw std::runtime_error("order2server:Can not find <type>");
    }
}

bool CKR_QUANT_TDPlugin::Start()
{
	/* 启动 */
    if (! pOesApi->Start()) 
    {
        ShowMessage(severity_levels::error,"CKR_QUANT_TDPlugin,启动API失败!");
        return false;
    }

	/*
    pthread_t       rptThreadId;
    int32           ret = 0;

    //创建回报接收线程 
    ret = pthread_create(&rptThreadId, NULL, tdThreadMain, (void *) this);
    if (ret != 0) {
        fprintf(stderr, "创建交易回调接收线程失败! error[%d - %s]\n",
            ret, strerror(ret));
        return false;
    }
	*/

	/* 创建回报接收线程 */
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tdThreadMain, (LPVOID)this, 0, NULL);

	return true;
}

bool CKR_QUANT_TDPlugin::Stop()
{
	/* 停止 */
    pOesApi->Stop();

    delete pOesApi;
    delete pOesSpi;

	return true;
}


/**
 * 发送委托请求
 *
 * @param   pOesApi         oes客户端
 * @param   mktId           市场代码 @see eOesMarketIdT
 * @param   pSecurityId     股票代码 (char[6]/char[8])
 * @param   pInvAcctId      股东账户代码 (char[10])，可 NULL
 * @param   ordType         委托类型 @see eOesOrdTypeT, eOesOrdTypeShT, eOesOrdTypeSzT
 * @param   bsType          买卖类型 @sse eOesBuySellTypeT
 * @param   ordQty          委托数量 (单位为股/张)
 * @param   ordPrice        委托价格 (单位精确到元后四位，即1元 = 10000)
 *
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
int32
CKR_QUANT_TDPlugin::OesClientMain_SendOrder(Quant360::OesClientApi *pOesApi,
        uint8 mktId, const char *pSecurityId, const char *pInvAcctId,
        uint8 ordType, uint8 bsType, int32 ordQty, int32 ordPrice) {
    OesOrdReqT          ordReq = {NULLOBJ_OES_ORD_REQ};

    ordReq.clSeqNo = (int32) ++ pOesApi->apiEnv.ordChannel.lastOutMsgSeq;
    ordReq.mktId = mktId;
    ordReq.ordType = ordType;
    ordReq.bsType = bsType;

    strncpy(ordReq.securityId, pSecurityId, sizeof(ordReq.securityId) - 1);
    if (pInvAcctId) {
        /* 股东账户可不填 */
        strncpy(ordReq.invAcctId, pInvAcctId, sizeof(ordReq.invAcctId) - 1);
    }

    ordReq.ordQty = ordQty;
    ordReq.ordPrice = ordPrice;

    return pOesApi->SendOrder(&ordReq);
}


/**
 * 发送撤单请求
 *
 * @param   pOesApi         oes客户端
 * @param   mktId           被撤委托的市场代码 @see eOesMarketIdT
 * @param   pSecurityId     被撤委托股票代码 (char[6]/char[8]), 可空
 * @param   pInvAcctId      被撤委托股东账户代码 (char[10])，可空
 * @param   origClSeqNo     被撤委托的流水号 (若使用 origClOrdId, 则不必填充该字段)
 * @param   origClEnvId     被撤委托的客户端环境号 (小于等于0, 则使用当前会话的 clEnvId)
 * @param   origClOrdId     被撤委托的客户订单编号 (若使用 origClSeqNo, 则不必填充该字段)
 *
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
int32
CKR_QUANT_TDPlugin::OesClientMain_CancelOrder(Quant360::OesClientApi *pOesApi,
        uint8 mktId, const char *pSecurityId, const char *pInvAcctId,
        int32 origClSeqNo, int8 origClEnvId, int64 origClOrdId) {
    OesOrdCancelReqT    cancelReq = {NULLOBJ_OES_ORD_CANCEL_REQ};

    cancelReq.clSeqNo = (int32) ++ pOesApi->apiEnv.ordChannel.lastOutMsgSeq;
    cancelReq.mktId = mktId;

    if (pSecurityId) {
        /* 撤单时被撤委托的股票代码可不填 */
        strncpy(cancelReq.securityId, pSecurityId, sizeof(cancelReq.securityId) - 1);
    }

    if (pInvAcctId) {
        /* 撤单时被撤委托的股东账户可不填 */
        strncpy(cancelReq.invAcctId, pInvAcctId, sizeof(cancelReq.invAcctId) - 1);
    }

    cancelReq.origClSeqNo = origClSeqNo;
    cancelReq.origClEnvId = origClEnvId;

    cancelReq.origClOrdId = origClOrdId;

    return pOesApi->SendCancelOrder(&cancelReq);
}

/**
 * 查询客户端总览信息
 *
 * @param   pSessionInfo    会话信息
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
int32
CKR_QUANT_TDPlugin::OesClientMain_QueryClientOverview(Quant360::OesClientApi *pOesApi) {
    OesClientOverviewT  clientOverview = {NULLOBJ_OES_CLIENT_OVERVIEW};
    int32               ret = 0;
    int32               i = 0;
    char sendJsonDataStr[4086];

    ret = pOesApi->GetClientOverview(&clientOverview);
    if (ret < 0) {
        return ret;
    }

    sprintf(sendJsonDataStr,">>> 客户端总览信息: {客户端编号[%d], 客户端类型[%hhu], " \
            "客户端状态[%hhu], 客户端名称[%s], 上海现货对应PBU[%d], " \
            "深圳现货对应PBU[%d], 委托通道流控阈值[%d], 查询通道流控阈值[%d], " \
            "关联的客户数量[%d]}\n",
            clientOverview.clientId, clientOverview.clientType,
            clientOverview.clientStatus, clientOverview.clientName,
            clientOverview.sseStkPbuId, clientOverview.szseStkPbuId,
            clientOverview.ordTrafficLimit, clientOverview.qryTrafficLimit,
            clientOverview.associatedCustCnt);
    //publisher.publish("oes_resp",sendJsonDataStr);

    // for (i = 0; i < clientOverview.associatedCustCnt; i++) {
    //     sprintf(sendJsonDataStr,"    >>> 客户总览信息: {客户代码[%s], 客户状态[%hhu], " \
    //             "风险评级[%hhu], 营业部代码[%d], 客户姓名[%s]}\n",
    //             clientOverview.custItems[i].custId,
    //             clientOverview.custItems[i].status,
    //             clientOverview.custItems[i].riskLevel,
    //             clientOverview.custItems[i].branchId,
    //             clientOverview.custItems[i].custName);
    //     publisher.publish("oes_resp",sendJsonDataStr);

    //     if (clientOverview.custItems[i].spotCashAcct.isValid) {
    //         sprintf(sendJsonDataStr,"        >>> 资金账户总览: {资金账户[%s], " \
    //                 "资金类型[%hhu], 账户状态[%hhu], 出入金是否禁止[%hhu]}\n",
    //                 clientOverview.custItems[i].spotCashAcct.cashAcctId,
    //                 clientOverview.custItems[i].spotCashAcct.cashType,
    //                 clientOverview.custItems[i].spotCashAcct.cashAcctStatus,
    //                 clientOverview.custItems[i].spotCashAcct.isFundTrsfDisabled);
    //     }

    //     if (clientOverview.custItems[i].shSpotInvAcct.isValid) {
    //         sprintf(sendJsonDataStr,"        >>> 股东账户总览: {股东账户代码[%s], " \
    //                 "市场代码[%hhu], 账户状态[%hhu], 是否禁止交易[%hhu], 席位号[%d], " \
    //                 "当日累计有效交易类委托笔数[%d], 当日累计有效非交易类委托笔数[%d], " \
    //                 "当日累计有效撤单笔数[%d], 当日累计被OES拒绝的委托笔数[%d], " \
    //                 "当日累计被交易所拒绝的委托笔数[%d], 当日累计成交笔数[%d]}\n",
    //                 clientOverview.custItems[i].shSpotInvAcct.invAcctId,
    //                 clientOverview.custItems[i].shSpotInvAcct.mktId,
    //                 clientOverview.custItems[i].shSpotInvAcct.status,
    //                 clientOverview.custItems[i].shSpotInvAcct.isTradeDisabled,
    //                 clientOverview.custItems[i].shSpotInvAcct.pbuId,
    //                 clientOverview.custItems[i].shSpotInvAcct.trdOrdCnt,
    //                 clientOverview.custItems[i].shSpotInvAcct.nonTrdOrdCnt,
    //                 clientOverview.custItems[i].shSpotInvAcct.cancelOrdCnt,
    //                 clientOverview.custItems[i].shSpotInvAcct.oesRejectOrdCnt,
    //                 clientOverview.custItems[i].shSpotInvAcct.exchRejectOrdCnt,
    //                 clientOverview.custItems[i].shSpotInvAcct.trdCnt);
    //     }

    //     if (clientOverview.custItems[i].szSpotInvAcct.isValid) {
    //         sprintf(sendJsonDataStr,"        >>> 股东账户总览: {股东账户代码[%s], " \
    //                 "市场代码[%hhu], 账户状态[%hhu], 是否禁止交易[%hhu], 席位号[%d], " \
    //                 "当日累计有效交易类委托笔数[%d], 当日累计有效非交易类委托笔数[%d], " \
    //                 "当日累计有效撤单笔数[%d], 当日累计被OES拒绝的委托笔数[%d], " \
    //                 "当日累计被交易所拒绝的委托笔数[%d], 当日累计成交笔数[%d]}\n",
    //                 clientOverview.custItems[i].szSpotInvAcct.invAcctId,
    //                 clientOverview.custItems[i].szSpotInvAcct.mktId,
    //                 clientOverview.custItems[i].szSpotInvAcct.status,
    //                 clientOverview.custItems[i].szSpotInvAcct.isTradeDisabled,
    //                 clientOverview.custItems[i].szSpotInvAcct.pbuId,
    //                 clientOverview.custItems[i].szSpotInvAcct.trdOrdCnt,
    //                 clientOverview.custItems[i].szSpotInvAcct.nonTrdOrdCnt,
    //                 clientOverview.custItems[i].szSpotInvAcct.cancelOrdCnt,
    //                 clientOverview.custItems[i].szSpotInvAcct.oesRejectOrdCnt,
    //                 clientOverview.custItems[i].szSpotInvAcct.exchRejectOrdCnt,
    //                 clientOverview.custItems[i].szSpotInvAcct.trdCnt);
    //     }
    // }

    return 0;
}


/**
 * 查询市场状态
 *
 * @param   pOesApi         oes客户端
 * @param   exchId          交易所代码 @see eOesExchangeIdT
 * @param   platformId      交易平台类型 @see eOesPlatformIdT
 *
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
int32
CKR_QUANT_TDPlugin::OesClientMain_QueryMarketStatus(Quant360::OesClientApi *pOesApi,
        uint8 exchId, uint8 platformId) {
    OesQryMarketStateFilterT    qryFilter = {NULLOBJ_OES_QRY_MARKET_STATE_FILTER};

    qryFilter.exchId = exchId;
    qryFilter.platformId = platformId;

    /* 也可直接使用 pOesApi->QueryMarketState(NULL, 0) 查询所有的市场状态 */
    return pOesApi->QueryMarketState(&qryFilter, 0);
}


/**
 * 查询资金
 *
 * @param   pOesApi         oes客户端
 * @param   pCashAcctId     资金账户代码
 *
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
int32
CKR_QUANT_TDPlugin::OesClientMain_QueryCashAsset(Quant360::OesClientApi *pOesApi,
        const char *pCashAcctId) {
    OesQryCashAssetFilterT  qryFilter = {NULLOBJ_OES_QRY_CASH_ASSET_FILTER};

    if (pCashAcctId) {
        strncpy(qryFilter.cashAcctId, pCashAcctId,
                sizeof(qryFilter.cashAcctId) - 1);
    }

    /* 也可直接使用 pOesApi->QueryCashAsset(NULL, 0) 查询客户所有资金账户 */
    return pOesApi->QueryCashAsset(&qryFilter, 0);
}


/**
 * 查询产品
 *
 * @param   pOesApi         oes客户端
 * @param   pSecurityId     产品代码
 * @param   mktId           市场代码
 * @param   securityType    证券类别
 * @param   subSecurityType 证券子类别
 *
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
int32
CKR_QUANT_TDPlugin::OesClientMain_QueryStock(Quant360::OesClientApi *pOesApi,
        const char *pSecurityId, uint8 mktId, uint8 securityType,
        uint8 subSecurityType) {
    OesQryStockFilterT  qryFilter = {NULLOBJ_OES_QRY_STOCK_FILTER};

    if (pSecurityId) {
        strncpy(qryFilter.securityId, pSecurityId,
                sizeof(qryFilter.securityId) - 1);
    }

    qryFilter.mktId = mktId;
    qryFilter.securityType = securityType;
    qryFilter.subSecurityType = subSecurityType;

    return pOesApi->QueryStock(&qryFilter, 0);
}


/**
 * 查询股票持仓
 *
 * @param   pOesApi         oes客户端
 * @param   mktId           市场代码 @see eOesMarketIdT
 * @param   pSecurityId     股票代码 (char[6]/char[8])
 *
 * @return  大于等于0，成功；小于0，失败（错误号）
 */
int32
CKR_QUANT_TDPlugin::OesClientMain_QueryStkHolding(Quant360::OesClientApi *pOesApi,
        uint8 mktId, const char *pSecurityId) {
    OesQryStkHoldingFilterT qryFilter = {NULLOBJ_OES_QRY_STK_HOLDING_FILTER};

    qryFilter.mktId = mktId;
    if (pSecurityId) {
        strncpy(qryFilter.securityId, pSecurityId,
                sizeof(qryFilter.securityId) - 1);
    }

    /* 也可直接使用 pOesApi->QueryStkHolding(NULL, 0) 查询客户所有持仓 */
    return pOesApi->QueryStkHolding(&qryFilter, 0);
}
