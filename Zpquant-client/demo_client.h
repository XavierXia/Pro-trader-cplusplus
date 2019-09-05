
#ifndef _DEMO_CLIENT_H
#define _DEMO_CLIENT_H


#include "ZpquantMdApi.h"
#include "ZpquantTraderApi.h"

class   CDemoClient: 
	public Zpquant::CZpquantMdSpi,
	public Zpquant::CZpquantTradeSpi
{
public:
	///深度tick行情通知
	virtual void OnTickRtnDepthMarketData(ZpquantMdsL2OnTickT *pDepthMarketData);

	///深度trade行情通知
	virtual void OnTradeRtnDepthMarketData(ZpquantMdsL2OnTradeT *pDepthMarketData);

	///深度order行情通知
	virtual void OnOrderRtnDepthMarketData(ZpquantMdsL2OnOrderT *pDepthMarketData);

public:
	/* 委托拒绝回报回调 */
	virtual void        OnBusinessReject(int32 errorCode, const ZpquantOrdReject *pOrderReject);
	/* 委托已收回报回调 */
	virtual void        OnOrderInsert(const ZpquantOrdCnfm *pOrderInsert);
	/* 委托确认回报回调 */
	virtual void        OnOrderReport(int32 errorCode, const ZpquantOrdCnfm *pOrderReport);
	/* 成交确认回报回调 */
	virtual void        OnTradeReport(const ZpquantTrdCnfm *pTradeReport);
	/* 资金变动回报回调 */
	virtual void        OnCashAssetVariation(const ZpquantCashAssetItem *pCashAssetItem);
	/* 持仓变动回报回调 */
	virtual void        OnStockHoldingVariation(const ZpquantStkHoldingItem *pStkHoldingItem);
	/* 出入金委托拒绝回报回调,暂不提供 */
	virtual void        OnFundTrsfReject(int32 errorCode, const ZpquantFundTrsfReject *pFundTrsfReject);
	/* 出入金委托执行回报回调,暂不提供 */
	virtual void        OnFundTrsfReport(int32 errorCode, const ZpquantFundTrsfReport *pFundTrsfReport);
	/* 市场状态信息回报回调 */
	virtual void        OnMarketState(const ZpquantMarketStateInfo *pMarketStateItem);

	/* 查询委托信息回调 */
	virtual void        OnQueryOrder(const ZpquantOrdItem *pOrder, const ZpquantQryCursor *pCursor, int32 requestId);
	/* 查询成交信息回调 */
	virtual void        OnQueryTrade(const ZpquantTrdItem *pTrade, const ZpquantQryCursor *pCursor, int32 requestId);
	/* 查询资金信息回调 */
	virtual void        OnQueryCashAsset(const ZpquantCashAssetItem *pCashAsset, const ZpquantQryCursor *pCursor, int32 requestId);
	/* 查询持仓信息回调 */
	virtual void        OnQueryStkHolding(const ZpquantStkHoldingItem *pStkHolding, const ZpquantQryCursor *pCursor, int32 requestId);
	/* 查询客户信息回调 */
	virtual void        OnQueryCustInfo(const ZpquantCustItemT *pCust, const ZpquantQryCursor *pCursor, int32 requestId);
	/* 查询股东账户信息回调 */
	virtual void        OnQueryInvAcct(const ZpquantInvAcctItem *pInvAcct, const ZpquantQryCursor *pCursor, int32 requestId);
	/* 查询出入金流水信息回调,暂不提供 */
	virtual void        OnQueryFundTransferSerial(const ZpquantFundTransferSerialItemT *pFundTrsf, const ZpquantQryCursor *pCursor, int32 requestId);
	/* 查询证券信息回调 */
	virtual void        OnQueryStock(const ZpquantStockBaseInfo *pStock, const ZpquantQryCursor *pCursor, int32 requestId);
	/* 查询市场状态信息回调 */
	virtual void        OnQueryMarketState(const ZpquantMarketStateInfo *pMarketState, const ZpquantQryCursor *pCursor, int32 requestId);

	virtual void        OnGetTradingDay(const ZpquantTradingDayInfo *pTradingDayInfo);
	virtual void		OnGetOrderResultFromRiskModel(const char* riskType, const char* newOrder, const char* oldOrder, bool isRejected);

public:
	void Start();
	void test_trade();

public:
	CDemoClient(int32 something = 0);
	virtual ~CDemoClient();
	char sendJsonDataStr[4096];

	Zpquant::CZpquantTradeApi  *pZpquantTradeApi;
	Zpquant::CZpquantMdApi  *pZpquantMdApi;

private:
	int32               something;

};


#endif /* _DEMO_MD_CLIENT_SPI_H */
