
#ifndef _DEMO_CLIENT_H
#define _DEMO_CLIENT_H


#include "ZpquantMdApi.h"
#include "ZpquantTraderApi.h"

class   CDemoClient: 
	public Zpquant::CZpquantMdSpi,
	public Zpquant::CZpquantTradeSpi
{
public:
	///���tick����֪ͨ
	virtual void OnTickRtnDepthMarketData(ZpquantMdsL2OnTickT *pDepthMarketData);

	///���trade����֪ͨ
	virtual void OnTradeRtnDepthMarketData(ZpquantMdsL2OnTradeT *pDepthMarketData);

	///���order����֪ͨ
	virtual void OnOrderRtnDepthMarketData(ZpquantMdsL2OnOrderT *pDepthMarketData);

public:
	/* ί�оܾ��ر��ص� */
	virtual void        OnBusinessReject(int32 errorCode, const ZpquantOrdReject *pOrderReject);
	/* ί�����ջر��ص� */
	virtual void        OnOrderInsert(const ZpquantOrdCnfm *pOrderInsert);
	/* ί��ȷ�ϻر��ص� */
	virtual void        OnOrderReport(int32 errorCode, const ZpquantOrdCnfm *pOrderReport);
	/* �ɽ�ȷ�ϻر��ص� */
	virtual void        OnTradeReport(const ZpquantTrdCnfm *pTradeReport);
	/* �ʽ�䶯�ر��ص� */
	virtual void        OnCashAssetVariation(const ZpquantCashAssetItem *pCashAssetItem);
	/* �ֱֲ䶯�ر��ص� */
	virtual void        OnStockHoldingVariation(const ZpquantStkHoldingItem *pStkHoldingItem);
	/* �����ί�оܾ��ر��ص�,�ݲ��ṩ */
	virtual void        OnFundTrsfReject(int32 errorCode, const ZpquantFundTrsfReject *pFundTrsfReject);
	/* �����ί��ִ�лر��ص�,�ݲ��ṩ */
	virtual void        OnFundTrsfReport(int32 errorCode, const ZpquantFundTrsfReport *pFundTrsfReport);
	/* �г�״̬��Ϣ�ر��ص� */
	virtual void        OnMarketState(const ZpquantMarketStateInfo *pMarketStateItem);

	/* ��ѯί����Ϣ�ص� */
	virtual void        OnQueryOrder(const ZpquantOrdItem *pOrder, const ZpquantQryCursor *pCursor, int32 requestId);
	/* ��ѯ�ɽ���Ϣ�ص� */
	virtual void        OnQueryTrade(const ZpquantTrdItem *pTrade, const ZpquantQryCursor *pCursor, int32 requestId);
	/* ��ѯ�ʽ���Ϣ�ص� */
	virtual void        OnQueryCashAsset(const ZpquantCashAssetItem *pCashAsset, const ZpquantQryCursor *pCursor, int32 requestId);
	/* ��ѯ�ֲ���Ϣ�ص� */
	virtual void        OnQueryStkHolding(const ZpquantStkHoldingItem *pStkHolding, const ZpquantQryCursor *pCursor, int32 requestId);
	/* ��ѯ�ͻ���Ϣ�ص� */
	virtual void        OnQueryCustInfo(const ZpquantCustItemT *pCust, const ZpquantQryCursor *pCursor, int32 requestId);
	/* ��ѯ�ɶ��˻���Ϣ�ص� */
	virtual void        OnQueryInvAcct(const ZpquantInvAcctItem *pInvAcct, const ZpquantQryCursor *pCursor, int32 requestId);
	/* ��ѯ�������ˮ��Ϣ�ص�,�ݲ��ṩ */
	virtual void        OnQueryFundTransferSerial(const ZpquantFundTransferSerialItemT *pFundTrsf, const ZpquantQryCursor *pCursor, int32 requestId);
	/* ��ѯ֤ȯ��Ϣ�ص� */
	virtual void        OnQueryStock(const ZpquantStockBaseInfo *pStock, const ZpquantQryCursor *pCursor, int32 requestId);
	/* ��ѯ�г�״̬��Ϣ�ص� */
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
