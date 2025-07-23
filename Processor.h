#ifndef _Processor_H_
#define _Processor_H_

//
#include <util/tc_singleton.h>
#include "DataProxyProto.h"
#include "global.pb.h"
#include "GlobalProto.h"
#include "UserInfoProto.h"

//
using namespace tars;
using namespace dbagent;
/**
 *请求处理类
 *
 */
class Processor
{
public:
    /**
     *
    */
    Processor();

    /**
     *
    */
    ~Processor();

public:
    int readDataFromDBEx(long uid, const string& table_name, const std::vector<string>& col_name, const std::vector<vector<string>>& whlist, const string& order_col, dbagent::TDBReadRsp &dataRsp);

    int readDataFromDB(long uid, const string& table_name, const std::vector<string>& col_name, const map<string, string>& whlist, const string& order_col, int limit_num,  dbagent::TDBReadRsp &dataRsp);

    int writeDataFromDB(dbagent::Eum_Query_Type dBOPType, long uid, const string& table_name, const std::map<string, string>& col_info, const map<string, string>& whlist);

    int delDataFromDB(long uid, const string& table_name, const map<string, string>& whlist);

public:
    int onSendQuest(const long lUid, const GlobalProto::SendQuestReq &req);
    int onQueryMessage(const long lUid, const GlobalProto::QueryMessageReq &req, GlobalProto::QueryMessageResp &resp);

public:
    int InsertMessage(const global::CreateMessageReq &req);
    int UpdateQuest(const global::ProcessQuestReq &req);

};

//singleton
typedef TC_Singleton<Processor, CreateStatic, DefaultLifetime> ProcessorSingleton;

#endif

