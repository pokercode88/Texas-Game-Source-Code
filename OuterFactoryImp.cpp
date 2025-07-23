#include <sstream>
#include "OuterFactoryImp.h"
#include "LogComm.h"
#include "GlobalServer.h"
#include "util/tc_hash_fun.h"

//
using namespace wbl;

//
OuterFactoryImp::OuterFactoryImp() : _pFileConf(NULL)
{
    createAllObject();
}

//
OuterFactoryImp::~OuterFactoryImp()
{
    deleteAllObject();
}

//
void OuterFactoryImp::deleteAllObject()
{
    if (_pFileConf)
    {
        delete _pFileConf;
        _pFileConf = NULL;
    }
}

//代理管理器
const OuterProxyFactoryPtr &OuterFactoryImp::getProxyFactory() const
{
    return _pProxyFactory;
}

//获取服务配置
const tars::TC_Config &OuterFactoryImp::getConfig() const
{
    return *_pFileConf;
}

//
void OuterFactoryImp::createAllObject()
{
    __TRY__

    deleteAllObject();

    //tars代理Factory,访问其他tars接口时使用
    _pProxyFactory = new OuterProxyFactory();
    if (!_pProxyFactory)
    {
        ROLLLOG_ERROR << "create outer proxy factory fail, ptr null." << endl;
        terminate();
    }

    LOG_DEBUG << "init proxy factory succ." << endl;

    load();

    __CATCH__
}

void OuterFactoryImp::load()
{
    __TRY__

    //拉取远程配置
    g_app.addConfig(ServerConfig::ServerName + ".conf");

    _pFileConf = new tars::TC_Config();
    if (!_pFileConf)
    {
        ROLLLOG_ERROR << "create config parser fail, ptr null." << endl;
        terminate();
    }

    _pFileConf->parseFile(ServerConfig::BasePath + ServerConfig::ServerName + ".conf");
    LOG_DEBUG << "init config file succ, base path: " << ServerConfig::BasePath + ServerConfig::ServerName + ".conf" << endl;

    readAllConfig();

    __CATCH__
}

// 读取所有配置
void OuterFactoryImp::readAllConfig()
{
    __TRY__

    readPrxConfig();
    printPrxConfig();

    readListAllRoomAddress();

    __CATCH__
}

//拆分字符串成整形
int OuterFactoryImp::splitInt(string szSrc, vector<int> &vecInt)
{
    split_int(szSrc, "[ \t]*\\|[ \t]*", vecInt);
    return 0;
}

//格式化时间
string OuterFactoryImp::timeFormat(const string &sFormat, time_t timeCluster)
{
    //string sFormat("%Y-%m-%d %H:%M:%S");
    if (sFormat.empty())
        return "";

    time_t t = timeCluster;
    auto pTm = localtime(&t);
    if (!pTm)
        return "";

    char sTimeString[255] = "\0";
    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), pTm);
    return string(sTimeString);
}

//代理配置
void OuterFactoryImp::readPrxConfig()
{
    _ConfigServantObj = (*_pFileConf).get("/Main/Interface/ConfigServer<ProxyObj>", "");
    _DBAgentServantObj = (*_pFileConf).get("/Main/Interface/DBAgentServer<ProxyObj>", "");
    _HallServantObj = (*_pFileConf).get("/Main/Interface/HallServer<ProxyObj>", "");
    _PushServantObj = (*_pFileConf).get("/Main/Interface/PushServer<ProxyObj>", "");
}

//打印代理配置
void OuterFactoryImp::printPrxConfig()
{
    FDLOG_CONFIG_INFO << "_ConfigServantObj ProxyObj : " << _ConfigServantObj << endl;
    FDLOG_CONFIG_INFO << "_DBAgentServantObj ProxyObj : " << _DBAgentServantObj << endl;
    FDLOG_CONFIG_INFO << "_HallServantObj ProxyObj : " << _HallServantObj << endl;
    FDLOG_CONFIG_INFO << "_PushServantObj ProxyObj : " << _PushServantObj << endl;
}

void OuterFactoryImp::readListAllRoomAddress()
{
    getConfigServantPrx()->listAllRoomAddress(_mapRoomServerFromRemote);
    ROLLLOG_DEBUG << "readListAllRoomAddress  _mapRoomServerFromRemote:" << _mapRoomServerFromRemote.size() << endl;

    return ;
}

//游戏配置服务代理
const ConfigServantPrx OuterFactoryImp::getConfigServantPrx()
{
    if (!_ConfigServerPrx)
    {
        _ConfigServerPrx = Application::getCommunicator()->stringToProxy<ConfigServantPrx>(_ConfigServantObj);
        ROLLLOG_DEBUG << "Init _ConfigServantObj succ, _ConfigServantObj : " << _ConfigServantObj << endl;
    }

    return _ConfigServerPrx;
}

//数据库代理服务代理
const DBAgentServantPrx OuterFactoryImp::getDBAgentServantPrx(const long uid)
{
    if (!_DBAgentServerPrx)
    {
        _DBAgentServerPrx = Application::getCommunicator()->stringToProxy<dbagent::DBAgentServantPrx>(_DBAgentServantObj);
        ROLLLOG_DEBUG << "Init _DBAgentServantObj succ, _DBAgentServantObj : " << _DBAgentServantObj << endl;
    }

    if (_DBAgentServerPrx)
    {
        return _DBAgentServerPrx->tars_hash(uid);
    }

    return NULL;
}

//数据库代理服务代理
const DBAgentServantPrx OuterFactoryImp::getDBAgentServantPrx(const string key)
{
    if (!_DBAgentServerPrx)
    {
        _DBAgentServerPrx = Application::getCommunicator()->stringToProxy<dbagent::DBAgentServantPrx>(_DBAgentServantObj);
        ROLLLOG_DEBUG << "Init _DBAgentServantObj succ, _DBAgentServantObj : " << _DBAgentServantObj << endl;
    }

    if (_DBAgentServerPrx)
    {
        return _DBAgentServerPrx->tars_hash(tars::hash<string>()(key));
    }

    return NULL;
}

//
const hall::HallServantPrx OuterFactoryImp::getHallServantPrx(const int64_t uid)
{
    if (!_HallServerPrx)
    {
        _HallServerPrx = Application::getCommunicator()->stringToProxy<hall::HallServantPrx>(_HallServantObj);
        ROLLLOG_DEBUG << "Init _HallServantObj succ, _HallServantObj:" << _HallServantObj << endl;
    }

    if (_HallServerPrx)
    {
        return _HallServerPrx->tars_hash(uid);
    }

    return NULL;
}

//PushServer代理
const PushServantPrx OuterFactoryImp::getPushServantPrx(const long uid)
{
    if (!_PushServerPrx)
    {
        _PushServerPrx = Application::getCommunicator()->stringToProxy<push::PushServantPrx>(_PushServantObj);
        ROLLLOG_DEBUG << "Init _PushServantObj succ, _PushServantObj:" << _PushServantObj << endl;
    }

    if (_PushServerPrx)
    {
        return _PushServerPrx->tars_hash(uid);
    }

    return NULL;
}

//RoomServant代理
const RoomServantPrx OuterFactoryImp::getRoomServantPrx(const long uid, const string &sRoomID)
{
    auto itAddress = _mapRoomServerFromRemote.find(sRoomID);
    if (itAddress == _mapRoomServerFromRemote.end())
    {
        ROLLLOG_ERROR << "_mapRoomServerFromRemote size: " << _mapRoomServerFromRemote.size() << ", sRoomID: "<< sRoomID << endl;
        return NULL;
    }

    auto RoomServerPrx = Application::getCommunicator()->stringToProxy<JFGame::RoomServantPrx>(itAddress->second);
    return RoomServerPrx->tars_hash(uid);
}

//格式化时间
string OuterFactoryImp::GetTimeFormat()
{
    string sFormat("%Y-%m-%d %H:%M:%S");
    time_t t = TNOW;
    auto pTm = localtime(&t);
    if (pTm == NULL)
        return "";

    char sTimeString[255] = "\0";
    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), pTm);
    return string(sTimeString);
}
