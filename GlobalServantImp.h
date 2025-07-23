#ifndef _GlobalServantImp_H_
#define _GlobalServantImp_H_

#include "servant/Application.h"
#include "GlobalServant.h"
#include "XGameComm.pb.h"
#include "CommonCode.pb.h"
#include "CommonStruct.pb.h"
#include "config.pb.h"
#include "globe.h"
#include "LogComm.h"
#include "JFGameHttpProto.h"
#include "OuterFactoryImp.h"
#include "GlobalProto.h"
#include "global.pb.h"

//
using namespace std;
using namespace JFGame;
using namespace JFGameHttpProto;
using namespace global;


/**
 *
 *服务接口
 */
class GlobalServantImp : public GlobalServant
{
public:
    /**
     *
     */
    GlobalServantImp();

    /**
     *
     */
    virtual ~GlobalServantImp();

    /**
     *
     */
    virtual void initialize();

    /**
     *
     */
    virtual void destroy();

    /**
     *
     */
    void initializeTimer();

public:
    //HTTP请求处理接口
    virtual tars::Int32 doRequest(const vector<tars::Char> &reqBuf, const map<std::string, std::string> &extraInfo, vector<tars::Char> &rspBuf, tars::TarsCurrentPtr current);
    //TCP请求处理接口
    virtual tars::Int32 onRequest(tars::Int64 lUin, const std::string &sMsgPack, const std::string &sCurServrantAddr, const JFGame::TClientParam &stClientParam, const JFGame::UserBaseInfoExt &stUserBaseInfo, tars::TarsCurrentPtr current);

    //创建信息
    virtual tars::Int32 createMessage(const global::CreateMessageReq &req, global::CreateMessageResp &resp, tars::TarsCurrentPtr current);

    //处理问题
    virtual tars::Int32 processQuest(const global::ProcessQuestReq &req, global::ProcessQuestResp &resp, tars::TarsCurrentPtr current);

public:
    // 申诉
    tars::Int32 onSendQuest(const XGameComm::TPackage &pkg, const GlobalProto::SendQuestReq &req, const std::string &sCurServrantAddr);
    // 消息列表
    tars::Int32 onQueryMessage(const XGameComm::TPackage &pkg, const GlobalProto::QueryMessageReq &req, const std::string &sCurServrantAddr);
public:
    //时钟周期回调
    virtual tars::Int32 doCustomMessage(bool bExpectIdle = false);

private:
    //发送消息到客户端
    template<typename T>
    int toClientPb(const XGameComm::TPackage &tPackage, const std::string &sCurServrantAddr, XGameProto::ActionName actionName, const T &t);
};

/////////////////////////////////////////////////////
#endif
