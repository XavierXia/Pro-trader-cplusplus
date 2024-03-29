#include    <Windows.h>
#include    <iostream>
#include    "unistd.h"
#include    <string.h>
#include    "ZpquantTraderApi.h"
#include    "demo_trader_client_spi.h"
#include    "ZpquantMdApi.h"
#include    "demo_md_client_spi.h"

int
main(void) {

    //连接后端server TOD
    fprintf(stdout, "...zpquant_client_main_demo start\n");

    //交易
    Zpquant::CZpquantTradeApi  *pZpquantTradeApi = new Zpquant::CZpquantTradeApi();
    Zpquant::CZpquantTradeSpi  *pZpquantTradeSpi = new demoTraderClientSpi();
    //行情
    Zpquant::CZpquantMdApi  *pZpquantMdApi = new Zpquant::CZpquantMdApi();
    Zpquant::CZpquantMdSpi  *pZpquantMdSpi = new demoMdClientSpi();

    if (!pZpquantTradeApi || !pZpquantTradeSpi) {
        fprintf(stderr, "pZpquantTradeApi/pZpquantTradeSpi内存不足!\n");
        return ENOMEM;
    }

    if (!pZpquantMdApi || !pZpquantMdSpi) {
        fprintf(stderr, "pZpquantMdSpi/pZpquantMdSpi 内存不足!\n");
        return ENOMEM;
    }

        /*
    *************************************
    行情类接口实现
    */
    /* 打印API版本信息 */
	//cout << "ZpquantClientApi 版本: \n" << Zpquant::CZpquantMdApi::GetVersion() << endl;

    /* 注册spi回调接口 */
    pZpquantMdApi->RegisterSpi(pZpquantMdSpi);

    ZpquantUserLoginField userLoginT;
	//测试用户名正常登陆
    strncpy(userLoginT.UserID, "md_client1",sizeof(userLoginT.UserID) - 1);
	//测试用户名为空
	//strncpy(userLoginT.UserID, "", sizeof(userLoginT.UserID) - 1);
	//测试Can not find <username>
	//strncpy(userLoginT.UserID, "client1", sizeof(userLoginT.UserID) - 1);

	//测试密码正常登陆
    strncpy(userLoginT.UserPassword, "pass123",sizeof(userLoginT.UserPassword) - 1);
	//测试密码为空
	//strncpy(userLoginT.UserPassword, "", sizeof(userLoginT.UserPassword) - 1);
	//测试密码错误
	//strncpy(userLoginT.UserPassword, "123456", sizeof(userLoginT.UserPassword) - 1);

    //strncpy(userLoginT.strIP, "47.105.111.100",sizeof(userLoginT.strIP) - 1);
	strncpy(userLoginT.strIP, "127.0.0.1", sizeof(userLoginT.strIP) - 1);
    userLoginT.uPort = 8800;

	cout << "...InitMdSource before!!!" << endl;
    if(!pZpquantMdApi->InitMdSource(&userLoginT)){
         fprintf(stderr, "InitMdSource失败!\n");
         return EINVAL;
    }

    //sleep(100);

    if (! pZpquantMdApi->Start()) {
        fprintf(stderr, "启动API失败!\n");
        return EINVAL;
    }
	Sleep(1000);

	int32 dataType = MDS_SUB_DATA_TYPE_L2_SNAPSHOT | MDS_SUB_DATA_TYPE_L2_ORDER | MDS_SUB_DATA_TYPE_L2_TRADE;
	//行情订阅测试
	if (1)
	{
		string codelist1("600000.SH,000011.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist1.c_str(), dataType, MDS_SUB_MODE_SET);
	}

	//追加行情订阅测试
	if (1)
	{
		string codelist2("600519.SH,000516.SZ,600547.SH,300413.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist2.c_str(), dataType, MDS_SUB_MODE_APPEND);
	}

	//删除行情订阅测试
	if (1)
	{
		string codelist3("300413.SZ,000011.SZ");
		pZpquantMdApi->SubscribeMarketData(codelist3.c_str(), dataType, MDS_SUB_MODE_DELETE);
	}

	if (0)
	{
		string codelist1("600000.SH");
		pZpquantMdApi->SubscribeMarketData(codelist1.c_str(), dataType, MDS_SUB_MODE_SET);
	}
	
    /*
    *************************************
    交易类接口实现
    */

	/* 打印API版本信息 */
	//cout << "pZpquantTradeApi 版本: \n" << Zpquant::CZpquantTradeApi::GetVersion() << endl;

    /* 注册spi回调接口 */
    pZpquantTradeApi->RegisterSpi(pZpquantTradeSpi);

    ZpquantUserLoginField userLoginTradeT;
    strncpy(userLoginTradeT.UserID, "td_client1",sizeof(userLoginTradeT.UserID) - 1);
    strncpy(userLoginTradeT.UserPassword, "pass123",sizeof(userLoginTradeT.UserPassword) - 1);
	strncpy(userLoginTradeT.strIP, "127.0.0.1", sizeof(userLoginTradeT.strIP) - 1);
    userLoginTradeT.uPort = 8800;

	cout << "...InitTraderSource before!!!" << endl;
    if(!pZpquantTradeApi->InitTraderSource(&userLoginTradeT)){
         fprintf(stderr, "...InitTraderSource!\n");
         return EINVAL;
    }


    /*
     * 设置登录OES时使用的用户名和密码
     * @note 如通过API接口设置，则可以不在配置文件中配置;
     *          支持通过前缀指定密码类型, 如 md5:PASSWORD, txt:PASSWORD
     */
    // pOesApi->SetThreadUsername("customer1");
    // pOesApi->SetThreadPassword("txt:123456");
    // pOesApi->SetThreadPassword("md5:e10adc3949ba59abbe56e057f20f883e");

    /* 启动 */
    if (! pZpquantTradeApi->Start()) {
        fprintf(stderr, "启动API失败!\n");
        return EINVAL;
    }

    /* 打印当前交易日 */
    fprintf(stdout, "服务端交易日: %08d\n", pZpquantTradeApi->GetTradingDay());

            /*
    *************************************
    交易下单接口实例
    */
    /*
    {"msgId":"13", "seqNo":1,"isEnd":"Y", invAcctId":"A188800368","securityId":"600000","mktId":1,
    "originalHld":1000000, totalBuyHld":0,"totalSellHld":0,"sellFrzHld":0, totalTrsfInHld":0,"totalTrsfOutHld":0,
    "trsfOutFrzHld":0,"lockHld":0, lockFrzHld":0,"unlockFrzHld":0,"coveredFrzHld":0,"coveredHld":0, 
    coveredAvlHld":0,"sumHld":1000000,"sellAvlHld":1000000,"trsfOutAvlHld":1000000,lockAvlHld":1000000}
    */
	Sleep(1000);

    //测试 查询特定某只股票持仓，上海，若不测试该项，请使用if(0)
	if (0)
	{
		string codeString = "600000";
		pZpquantTradeApi->QueryStkHolding(codeString.c_str(),1, 0);
	}


	//测试查询特定某只股票持仓，深圳
	if (1)
	{
		string codeString = "000001";
		pZpquantTradeApi->QueryStkHolding(codeString.c_str(), 2, 0);
	}

	//查询账号中两市场的持仓信息
	if (0)
	{
		string codeString1 = "allStk";
		uint8 sclb = 1; //1或2都可以使用
		pZpquantTradeApi->QueryStkHolding(codeString1.c_str(), sclb, 0);
	}

	Sleep(1000);

    //下单买入 上海 异常价格
	if (0)
	{
		string codeString2 = "600519";
		pZpquantTradeApi->SendOrder(codeString2.c_str(),1,1,52000,100);

		//测试 将全部拒绝转发的委托添加至“被拒绝委托”列表。
		for (int i = 0; i < 15; i++)
		{
			pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 1, 52000, 100);
			Sleep(2000);
		}
	}

	//下单买入 上海
	if (0)
	{
		/*
		ZpquantOrdReqT zOrdReqT;
		string codeString2 = "600519";
		memset(zOrdReqT.pSecurityId, 0, 7 * sizeof(char));
		memcpy(zOrdReqT.pSecurityId, codeString2.c_str(), 7 * sizeof(char));

		zOrdReqT.mktId = 1;
		zOrdReqT.ordType = 0;
		zOrdReqT.bsType = 1; //买入
		zOrdReqT.ordQty = 200;
		zOrdReqT.ordPrice = 9800000;
		pZpquantTradeApi->SendOrder(&zOrdReqT);
		*/

		string codeString2 = "600519";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 1, 9800000, 200);
	}

	//下单买入 深圳
	if (0)
	{
		string codeString2 = "000516";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 2, 1, 42000, 1000);
	}

	//下单买入 深圳
	if (0)
	{
		string codeString2 = "000001";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 2, 1, 145000, 100);

		//测试 将全部已发送订单添加至“当日委托”列表。
		for (int i = 0; i < 15; i++)
		{
			pZpquantTradeApi->SendOrder(codeString2.c_str(), 2, 1, 145000, 100);
			Sleep(2000);
		}
	}

	//下单买入 上海
	if (1)
	{
		string codeString2 = "600000";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 1, 122000, 100);

		//测试 将全部已成交订单添加至“当日成交”列表。
		for (int i = 0; i < 15; i++)
		{
			pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 1, 122000, 100);
			Sleep(2000);
		}
	}


	//下单卖出 深圳
	if (0)
	{
		string codeString2 = "000001";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 2, 2, 140000, 300);
	}

	//下单卖出 上海
	if (0)
	{
		string codeString2 = "600519";
		pZpquantTradeApi->SendOrder(codeString2.c_str(), 1, 2, 9600000, 100);
	}

	//委托之后撤单，必须在下单回报中进行测试，
	if (0) 
	{
		//通过待撤委托的 origClOrdId 进行撤单
		int64 origClOrdId = 111;//待撤委托的origClOrdId需要通过回报消息获取
		pZpquantTradeApi->SendCancelOrder(1,0,0, origClOrdId);
	}

	if (0)
	{
		//通过待撤委托的 origClSeqNo 进行撤单
		//origClEnvId     被撤委托的客户端环境号 (小于等于0, 则使用当前会话的 clEnvId)
		int32 origClSeqNo = 1;//待撤委托的origClSeqNo需要通过回报消息获取
		int8 origClEnvId = 11;//待撤委托的origClEnvId需要通过回报消息获取
		int64 origClOrdId = 0; //填0，表示不用该项撤单,第四项
		pZpquantTradeApi->SendCancelOrder(2, origClSeqNo, origClEnvId, 0);
	}

    /* 等待回报消息接收完成 */
    while(1)
    {
		;
    }

    // /* 停止 */
    // pZpquantTradeApi->Stop();

    // delete pZpquantTradeApi;
    // delete pZpquantTradeSpi;
    // delete pZpquantMdApi;
    // delete pZpquantMdSpi;

    return 0;
}