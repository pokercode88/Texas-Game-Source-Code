#include "Processor.h"
#include "globe.h"
#include "LogComm.h"
#include "DataProxyProto.h"
#include "ServiceDefine.h"
#include "util/tc_hash_fun.h"
#include "uuid.h"
#include "CommonStruct.pb.h"
#include "CommonCode.pb.h"
#include "GlobalServer.h"
#include "Java2RoomProto.h"
#include "RoomServant.h"
#include "third.pb.h"
#include "ServiceUtil.h"


//
using namespace std;
using namespace dataproxy;
using namespace dbagent;

//
#define GIFT_EXPIRED_TIME (24*60*60)

/**
 *
*/
Processor::Processor()
{

}

/**
 *
*/
Processor::~Processor()
{

}

int Processor::readDataFromDBEx(long uid, const string& table_name, const std::vector<string>& col_name, const std::vector<vector<string>>& whlist, const string& order_col, dbagent::TDBReadRsp &dataRsp)
{
    int iRet = 0;
    dbagent::TDBReadReq rDataReq;
    rDataReq.keyIndex = 0;
    rDataReq.queryType = dbagent::E_SELECT;
    rDataReq.tableName = table_name;

    vector<dbagent::TField> fields;
    dbagent::TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : col_name)
    {
        tfield.colName = item;
        fields.push_back(tfield);
    }
    rDataReq.fields = fields;

    //where条件组
    if(!whlist.empty())
    {
        vector<dbagent::ConditionGroup> conditionGroups;
        dbagent::ConditionGroup conditionGroup;
        conditionGroup.relation = dbagent::AND;
        vector<dbagent::Condition> conditions;
        for(auto item : whlist)
        {
            if(item.size() != 3)
            {
                continue;
            }
            dbagent::Condition condition;
            condition.condtion =  dbagent::Eum_Condition(S2I(item[1]));
            condition.colType = dbagent::STRING;
            condition.colName = item[0];
            condition.colValues = item[2];
            conditions.push_back(condition);
        }
        conditionGroup.condition = conditions;
        conditionGroups.push_back(conditionGroup);
        rDataReq.conditions = conditionGroups;
    }

    //order by字段
    if(!order_col.empty())
    {
        vector<dbagent::OrderBy> orderBys;
        dbagent::OrderBy orderBy;
        orderBy.sort = dbagent::DESC;
        orderBy.colName = order_col;
        orderBys.push_back(orderBy);
        rDataReq.orderbyCol = orderBys;
    }
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->read(rDataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "read data from dbagent failed, rDataReq:" << printTars(rDataReq) << ",dataRsp: " << printTars(dataRsp) << endl;
        return -1;
    }
    return 0;
}

int Processor::readDataFromDB(long uid, const string& table_name, const std::vector<string>& col_name, const map<string, string>& whlist, const string& order_col, int limit_num,  dbagent::TDBReadRsp &dataRsp)
{
    int iRet = 0;
    dbagent::TDBReadReq rDataReq;
    rDataReq.keyIndex = 0;
    rDataReq.queryType = dbagent::E_SELECT;
    rDataReq.tableName = table_name;

    vector<dbagent::TField> fields;
    dbagent::TField tfield;
    tfield.colArithType = E_NONE;
    for(auto item : col_name)
    {
        tfield.colName = item;
        fields.push_back(tfield);
    }
    rDataReq.fields = fields;

    //where条件组
    if(!whlist.empty())
    {
        vector<dbagent::ConditionGroup> conditionGroups;
        dbagent::ConditionGroup conditionGroup;
        conditionGroup.relation = dbagent::AND;
        vector<dbagent::Condition> conditions;
        for(auto item : whlist)
        {
            dbagent::Condition condition;
            condition.condtion = dbagent::E_EQ;
            condition.colType = dbagent::STRING;
            condition.colName = item.first;
            condition.colValues = item.second;
            conditions.push_back(condition);
        }
        conditionGroup.condition = conditions;
        conditionGroups.push_back(conditionGroup);
        rDataReq.conditions = conditionGroups;
    }

    //order by字段
    if(!order_col.empty())
    {
        vector<dbagent::OrderBy> orderBys;
        dbagent::OrderBy orderBy;
        orderBy.sort = dbagent::DESC;
        orderBy.colName = order_col;
        orderBys.push_back(orderBy);
        rDataReq.orderbyCol = orderBys;
    }

    if(limit_num > 0)
    {
        //指定返回的行数的最大值
        rDataReq.limit = limit_num;
        //指定返回的第一行的偏移量
        rDataReq.limit_from = 0;
    }

    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->read(rDataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "read data from dbagent failed, rDataReq:" << printTars(rDataReq) << ",dataRsp: " << printTars(dataRsp) << endl;
        return -1;
    }
    return 0;
}

int Processor::writeDataFromDB(dbagent::Eum_Query_Type dBOPType, long uid, const string& table_name, const std::map<string, string>& col_info, const map<string, string>& whlist)
{
    int iRet = 0;
    //更新上个赛季状态
    dbagent::TDBWriteReq uDataReq;
    uDataReq.keyIndex = 0;
    uDataReq.queryType = dBOPType;
    uDataReq.tableName = table_name;

    vector<dbagent::TField> fields;
    dbagent::TField tfield;

    for(auto item : col_info)
    {
        tfield.colArithType = E_NONE;
        tfield.colType = dbagent::STRING;
        tfield.colName = item.first;
        tfield.colValue = item.second;
        fields.push_back(tfield);
    }
    uDataReq.fields = fields;

    //where条件组
    vector<dbagent::ConditionGroup> conditionGroups;
    dbagent::ConditionGroup conditionGroup;
    conditionGroup.relation = dbagent::AND;
    vector<dbagent::Condition> conditions;
    dbagent::Condition condition;
    for(auto item : whlist)
    {
        dbagent::Condition condition;
        condition.condtion = dbagent::E_EQ;
        condition.colType = dbagent::STRING;
        condition.colName = item.first;
        condition.colValues = item.second;
        conditions.push_back(condition);
    }
    conditionGroup.condition = conditions;
    conditionGroups.push_back(conditionGroup);
    uDataReq.conditions = conditionGroups;

    dbagent::TDBWriteRsp uDataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->write(uDataReq, uDataRsp);
    if (iRet != 0 || uDataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "update data to dbagent failed, uDataReq:" << printTars(uDataReq) << ",uDataRsp: " << printTars(uDataRsp) << endl;
        return -1;
    }
    return 0;
}

int Processor::delDataFromDB(long uid, const string& table_name, const map<string, string>& whlist)
{
    int iRet = 0;
    dbagent::TDBWriteReq uDataReq;
    uDataReq.keyIndex = 0;
    uDataReq.queryType = dbagent::E_DELETE;
    uDataReq.tableName = table_name;

    vector<dbagent::TField> fields;
    dbagent::TField tfield;

    //where条件组
    vector<dbagent::ConditionGroup> conditionGroups;
    dbagent::ConditionGroup conditionGroup;
    conditionGroup.relation = dbagent::AND;
    vector<dbagent::Condition> conditions;
    dbagent::Condition condition;
    for(auto item : whlist)
    {
        dbagent::Condition condition;
        condition.condtion = dbagent::E_EQ;
        condition.colType = dbagent::STRING;
        condition.colName = item.first;
        condition.colValues = item.second;
        conditions.push_back(condition);
    }
    conditionGroup.condition = conditions;
    conditionGroups.push_back(conditionGroup);
    uDataReq.conditions = conditionGroups;

    dbagent::TDBWriteRsp uDataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->write(uDataReq, uDataRsp);
    if (iRet != 0 || uDataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "update data to dbagent failed, uDataReq:" << printTars(uDataReq) << ",uDataRsp: " << printTars(uDataRsp) << endl;
        return -1;
    }
    return 0;
}

// 申诉
int Processor::onSendQuest(const long lUid, const GlobalProto::SendQuestReq &req)
{
    int iRet = 0;
    FUNC_ENTRY("");

    long lTime = TNOW;

    Pb::GameQuestInfo info;
    info.set_lplayerid(lUid);
    info.set_itype(req.itype());
    info.set_scontent(req.scontent());
    info.set_ldate(req.ldate());
    info.set_ltime(lTime);
    for (auto item : req.slink())
    {
        info.add_slink(item);
    }

    string sInfo;
    info.SerializeToString(&sInfo);

    string table_name = "tb_quest";
    std::map<string, string> col_info = {{"uid", L2S(lUid)}, {"quest_type", I2S(req.itype())},
                {"quest_content", req.scontent()}, {"quest_date", L2S(req.ldate())}, {"log_time", L2S(lTime)}, {"info", sInfo}, {"status", "0"}};
                
    iRet = writeDataFromDB(dbagent::E_INSERT, lUid, table_name, col_info, {});
    if(iRet != 0)
    {
        LOG_ERROR << "write into db err. iRet: "<< iRet << endl;
        return iRet;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//
int Processor::onQueryMessage(const long lUid, const GlobalProto::QueryMessageReq &req, GlobalProto::QueryMessageResp &resp)
{
    FUNC_ENTRY("");

    int iRet = 0;

    string table_name = "tb_message";
    std::vector<string> col_name = {"id", "message_type", "message_date", "message_content"};
    std::vector<vector<string>> whlist = {
        {"message_type", "0", I2S(int(req.itype()))}
    };
    if( global::E_MESSAGE_TYPE(req.itype()) != global::E_MESSAGE_TYPE::E_MESSAGE_NOTICE)
    {
        whlist.push_back({"uid", "0", L2S(lUid)});
    }

    dbagent::TDBReadRsp dataRsp;
    iRet = readDataFromDBEx(lUid, table_name, col_name, whlist, "message_date", dataRsp);
    if(iRet != 0)
    {
        LOG_ERROR<<"query message err! uid: "<< lUid << endl;
        return iRet;
    }

    if(req.icurrentpage() <= 0)
    {
        LOG_ERROR << "param err. icurrentpage: "<< req.icurrentpage() << endl;
        return -1;
    }

    int pageCount = 10;
    int startIndex = (req.icurrentpage() -1) * pageCount;

    LOG_DEBUG << "startIndex: "<< startIndex << ", page: "<< req.icurrentpage() << ", size: "<< dataRsp.records.size() << endl;

    Pb::MessageDataInfo info;

    int i = 0;
    int count = 0;
    for (auto it = dataRsp.records.begin(); it != dataRsp.records.end(); ++it)
    {
        i++;
        if(i < startIndex)
        {
            continue;
        }
        if(count >= pageCount)
        {
            break;
        }

        long message_id = 0;
        int message_type = 0;
        long message_date = 0;
        string message_content;
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            if (itfield->colName == "id")
            {
                message_id = S2L(itfield->colValue);
            }
            else if (itfield->colName == "message_type")
            {
                message_type = S2I(itfield->colValue);
            }
            else if (itfield->colName == "message_date")
            {
                message_date = ServiceUtil::GetTimeStamp(itfield->colValue);
            }
            if (itfield->colName == "message_content")
            {
                message_content = itfield->colValue;
            }
        }

        auto data = resp.add_vlist();
        data->set_itype(message_type);
        data->set_lmessageid(message_id);
        data->set_lmessagedate(message_date);

        info.ParseFromString(message_content);
        auto pInfo = data->mutable_dinfo();
        pInfo->set_stitle(info.stitle());
        pInfo->set_scontent(info.scontent());

        count++;
    }
    int pages = dataRsp.records.size() % pageCount != 0 && dataRsp.records.size() > 0 ? dataRsp.records.size() / pageCount + 1 : dataRsp.records.size() / pageCount;

    resp.set_itype(req.itype());
    resp.set_icurrentpage(req.icurrentpage());
    resp.set_itotoalpage(pages);

    FUNC_EXIT("", iRet);
    return iRet;
}

int Processor::InsertMessage(const global::CreateMessageReq &req)
{
    int iRet = 0;
    FUNC_ENTRY("");

    Pb::MessageDataInfo info;
    info.set_stitle(req.sMessageTitle);
    info.set_scontent(req.sMessageContent);

    string s_info;
    info.SerializeToString(&s_info);

    string table_name = "tb_message";
    std::map<string, string> col_info = {{"uid", L2S(req.lMessageTargetID)}, {"message_type", I2S(int(req.iMessageType))},
                {"message_content", s_info}, {"message_date", g_app.getOuterFactoryPtr()->GetTimeFormat()},
    };
    iRet = writeDataFromDB(dbagent::E_INSERT, req.lMessageTargetID, table_name, col_info, {});
    if(iRet != 0)
    {
        LOG_ERROR << "write into db err. iRet: "<< iRet << endl;
        return iRet;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

int Processor::UpdateQuest(const global::ProcessQuestReq &req)
{
    int iRet = 0;
    FUNC_ENTRY("");

     string table_name = "tb_quest";
    map<string, string> whList = {{"id", L2S(req.lId)}};
    map<string, string> dataList = {{"process_content", req.sProcessContent}, {"process_name", req.sProcessName}, {"process_date", g_app.getOuterFactoryPtr()->GetTimeFormat()}, {"status", L2S(1)}};

    iRet = writeDataFromDB(dbagent::E_UPDATE, req.lId, table_name, dataList, whList);

    FUNC_EXIT("", iRet);
    return iRet;
}
