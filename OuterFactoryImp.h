#ifndef _OUTER_FACTORY_IMP_H_
#define _OUTER_FACTORY_IMP_H_

#include <string>
#include "globe.h"
#include "OuterFactory.h"
#include "ConfigDefine.h"
#include "ConfigServant.h"
#include "DBAgentServant.h"
#include "DataProxyProto.h"
#include "HallServant.h"
#include "PushServant.h"
#include "RoomServant.h"
#include <wbl/regex_util.h>

//
using namespace tars;
using namespace config;
using namespace dataproxy;
using namespace dbagent;
using namespace push;
using namespace JFGame;

/**
 * 外部工具接口对象工厂
 */
class OuterFactoryImp : public OuterFactory
{
public:
    /**
     *
    */
    OuterFactoryImp();

    /**
     *
    */
    virtual ~OuterFactoryImp();

private:
    //
    void createAllObject();
    //
    void deleteAllObject();

public:
    //加载配置
    void load();

public:
    //框架中用到的outer接口(不能修改):
    const OuterProxyFactoryPtr &getProxyFactory() const;
    //获取服务配置
    const tars::TC_Config &getConfig() const;
    //游戏配置服务代理
    const ConfigServantPrx getConfigServantPrx();
    //数据库代理服务代理
    const DBAgentServantPrx getDBAgentServantPrx(const long uid);
    //数据库代理服务代理
    const DBAgentServantPrx getDBAgentServantPrx(const string key);
    //广场服务代理
    const hall::HallServantPrx getHallServantPrx(const int64_t uid);
    //获取PushServer代理
    const PushServantPrx getPushServantPrx(const long uid);
    //获取RoomServer代理
    const RoomServantPrx getRoomServantPrx(const long uid, const string &sRoomID);

public:
    // 读取所有配置
    void readAllConfig();

    void readPrxConfig();

    void readListAllRoomAddress();

    //打印代理配置
    void printPrxConfig();

public:
    //拆分字符串成整形
    int splitInt(string szSrc, vector<int> &vecInt);
    //格式化时间
    string timeFormat(const string &sFormat, time_t timeCluster);

    string GetTimeFormat();

private:
    //框架用到的共享对象(不能修改):
    tars::TC_Config *_pFileConf;
    //
    OuterProxyFactoryPtr _pProxyFactory;

private:
    //游戏配置服务
    std::string _ConfigServantObj;
    ConfigServantPrx _ConfigServerPrx;

    //数据库代理服务
    std::string _DBAgentServantObj;
    DBAgentServantPrx _DBAgentServerPrx;

    //大厅服务
    std::string _HallServantObj;
    hall::HallServantPrx _HallServerPrx;

    //PUSH服务
    std::string _PushServantObj;
    PushServantPrx _PushServerPrx;

    map<string, string> _mapRoomServerFromRemote;
};

//ptr
typedef TC_AutoPtr<OuterFactoryImp> OuterFactoryImpPtr;

#endif


