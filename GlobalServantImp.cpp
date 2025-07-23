#include "GlobalServantImp.h"
#include "servant/Application.h"
#include "ServiceDefine.h"
#include "globe.h"
#include "LogComm.h"
#include "Push.h"
#include "JFGameHttpProto.h"
#include "CommonStruct.h"
#include "CommonCode.h"
#include "XGameComm.pb.h"
#include "GlobalServer.h"
#include "Processor.h"


#define PAGE_COUNT 10
//////////////////////////////////////////////////////

GlobalServantImp::GlobalServantImp()
{

}

GlobalServantImp::~GlobalServantImp()
{

}

void GlobalServantImp::initialize()
{

}

//////////////////////////////////////////////////////
void GlobalServantImp::destroy()
{
    //destroy servant here:
    //...
}

void GlobalServantImp::initializeTimer()
{

}

//http请求处理接口
tars::Int32 GlobalServantImp::doRequest(const vector<tars::Char> &reqBuf, const map<std::string, std::string> &extraInfo, vector<tars::Char> &rspBuf, tars::TarsCurrentPtr current)
{
    FUNC_ENTRY("");

    int iRet = 0;

    __TRY__

    ROLLLOG_DEBUG << "recive request, reqBuf size : " << reqBuf.size() << endl;

    if (reqBuf.empty())
    {
        iRet = -1;
        return iRet;
    }

    THttpPackage thttpPack;
    if (!reqBuf.empty())
    {
        toObj(reqBuf, thttpPack);
    }

    if (thttpPack.vecData.empty())
    {
        iRet = -2;
        return iRet;
    }

    THttpPackage thttpPackRsp;
    thttpPackRsp.stUid.lUid = thttpPack.stUid.lUid;
    thttpPackRsp.stUid.sToken = thttpPack.stUid.sToken;
    thttpPackRsp.iVer = thttpPack.iVer;
    thttpPackRsp.iSeq = thttpPack.iSeq;
    tobuffer(thttpPackRsp, rspBuf);
    ROLLLOG_DEBUG << "response buff size: " << rspBuf.size() << endl;

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

//tcp请求处理接口
tars::Int32 GlobalServantImp::onRequest(tars::Int64 lUin, const std::string &sMsgPack, const std::string &sCurServrantAddr, const JFGame::TClientParam &stClientParam, const JFGame::UserBaseInfoExt &stUserBaseInfo, tars::TarsCurrentPtr current)
{
    int iRet = 0;

    __TRY__

    ROLLLOG_DEBUG << "recv msg, uid : " << lUin << ", addr : " << stClientParam.sAddr << endl;

    async_response_onRequest(current, 0);

    XGameComm::TPackage pkg;
    pbToObj(sMsgPack, pkg);

    if (pkg.vecmsghead_size() == 0)
    {
        ROLLLOG_DEBUG << "package empty." << endl;
        return -1;
    }

    ROLLLOG_DEBUG << "recv msg, uid : " << lUin << ", msg : " << logPb(pkg) << endl;

    for (int i = 0; i < pkg.vecmsghead_size(); ++i)
    {
        int64_t ms1 = TNOWMS;

        const auto &head = pkg.vecmsghead(i);
        switch(head.nmsgid())
        {
        // 申诉
        case XGameProto::ActionName::GLOBAL_SEND_QUEST:
        {
            GlobalProto::SendQuestReq req;
            pbToObj(pkg.vecmsgdata(i), req);
            iRet = onSendQuest(pkg, req, sCurServrantAddr);
            break;
        }
        // 消息列表
        case XGameProto::ActionName::GLOBAL_QUERY_MESSAGE:
        {
            GlobalProto::QueryMessageReq req;
            pbToObj(pkg.vecmsgdata(i), req);
            iRet = onQueryMessage(pkg, req, sCurServrantAddr);
            break;
        }
        default:
        {
            ROLLLOG_ERROR << "invalid msg id, uid: " << lUin << ", msg id: " << head.nmsgid() << endl;
            break;
        }
        }

        if (iRet != 0)
        {
            ROLLLOG_ERROR << "Process msg fail, uid: " << lUin << ", msg id: " << head.nmsgid()  << ", iRet: " << iRet << endl;
        }
        else
        {
            ROLLLOG_DEBUG << "Process msg succ, uid: " << lUin << ", msg id: " << head.nmsgid() << endl;
        }

        int64_t ms2 = TNOWMS;
        if ((ms2 - ms1) > COST_MS)
        {
            ROLLLOG_WARN << "@Performance, msgid:" << head.nmsgid() << ", costTime:" << (ms2 - ms1) << endl;
        }
    }

    __CATCH__
    return iRet;
}

tars::Int32 GlobalServantImp::onSendQuest(const XGameComm::TPackage &pkg, const GlobalProto::SendQuestReq &req, const std::string &sCurServrantAddr)
{
    FUNC_ENTRY("");

    int iRet = 0;

    __TRY__

    ROLLLOG_DEBUG << "onSendQuest.. req:" << logPb(req) << endl;

    long lUid = pkg.stuid().luid();

    iRet = ProcessorSingleton::getInstance()->onSendQuest(lUid, req);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "onSendQuest err. uid:"<< lUid << endl;
    }

    GlobalProto::SendQuestResp resp;
    resp.set_iresultcode(iRet);

    toClientPb(pkg, sCurServrantAddr, XGameProto::ActionName::GLOBAL_SEND_QUEST, resp);

    __CATCH__

    FUNC_EXIT("", iRet);
    return iRet;
}

// 查询消息
tars::Int32 GlobalServantImp::onQueryMessage(const XGameComm::TPackage &pkg, const GlobalProto::QueryMessageReq &req, const std::string &sCurServrantAddr)
{
    FUNC_ENTRY("");

    int iRet = 0;

    __TRY__

    ROLLLOG_DEBUG << "onQueryMessage... req:" << logPb(req) << endl;

    long lUid = pkg.stuid().luid();

    GlobalProto::QueryMessageResp resp;
    iRet = ProcessorSingleton::getInstance()->onQueryMessage(lUid, req, resp);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "onQueryMessage.. err! uid:" << lUid << endl;
    }

    resp.set_iresultcode(iRet);

    toClientPb(pkg, sCurServrantAddr, XGameProto::ActionName::GLOBAL_QUERY_MESSAGE, resp);

    __CATCH__

    FUNC_EXIT("", iRet);

    return iRet;
}

tars::Int32 GlobalServantImp::createMessage(const global::CreateMessageReq &req, global::CreateMessageResp &resp, tars::TarsCurrentPtr current)
{
    FUNC_ENTRY("");

    int iRet = 0;

    __TRY__

    iRet = ProcessorSingleton::getInstance()->InsertMessage(req);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "insert message err. "<< endl;
    }

    resp.iResultCode = iRet;

    __CATCH__

    FUNC_EXIT("", iRet);

    return iRet;
}

tars::Int32 GlobalServantImp::processQuest(const global::ProcessQuestReq &req, global::ProcessQuestResp &resp, tars::TarsCurrentPtr current)
{
    FUNC_ENTRY("");

    int iRet = 0;

    __TRY__

    iRet = ProcessorSingleton::getInstance()->UpdateQuest(req);
    if(iRet != 0)
    {
        ROLLLOG_ERROR << "proce quest err. "<< endl;
    }

    resp.iResultCode = iRet;

    __CATCH__

    FUNC_EXIT("", iRet);

    return iRet;
}

//发送消息到客户端
template<typename T>
int GlobalServantImp::toClientPb(const XGameComm::TPackage &tPackage, const std::string &sCurServrantAddr, XGameProto::ActionName actionName, const T &t)
{
    XGameComm::TPackage rsp;
    rsp.set_iversion(tPackage.iversion());

    auto ptuid = rsp.mutable_stuid();
    ptuid->set_luid(tPackage.stuid().luid());
    rsp.set_igameid(tPackage.igameid());
    rsp.set_sroomid(tPackage.sroomid());
    rsp.set_iroomserverid(tPackage.iroomserverid());
    rsp.set_isequence(tPackage.isequence());
    rsp.set_iflag(tPackage.iflag());

    auto mh = rsp.add_vecmsghead();
    mh->set_nmsgid(actionName);
    mh->set_nmsgtype(XGameComm::MSGTYPE::MSGTYPE_RESPONSE);
    mh->set_servicetype(XGameComm::SERVICE_TYPE::SERVICE_TYPE_GLOBAL);
    rsp.add_vecmsgdata(pbToString(t));

    auto pPushPrx = Application::getCommunicator()->stringToProxy<JFGame::PushPrx>(sCurServrantAddr);
    if (pPushPrx)
    {
        ROLLLOG_DEBUG << "toclient pb: " << logPb(rsp) << ", t: " << logPb(t) << endl;
        pPushPrx->tars_hash(tPackage.stuid().luid())->async_doPushBuf(NULL, tPackage.stuid().luid(), pbToString(rsp));
    }
    else
    {
        ROLLLOG_ERROR << "pPushPrx is null: " << logPb(rsp) << ", t: " << logPb(t) << endl;
    }

    return 0;
}

tars::Int32 GlobalServantImp::doCustomMessage(bool bExpectIdle)
{
    return 0;
}
