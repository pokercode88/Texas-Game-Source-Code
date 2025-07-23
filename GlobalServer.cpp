#include "GlobalServer.h"
#include "GlobalServantImp.h"
#include "LogComm.h"

//
using namespace std;

//
GlobalServer g_app;

/////////////////////////////////////////////////////////////////
void GlobalServer::initialize()
{
    //注册服务接口
    addServant<GlobalServantImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".GlobalServantObj");

    //初始化外部接口对象
    initOuterFactory();

    //注册动态加载命令
    TARS_ADD_ADMIN_CMD_NORMAL("reload", GlobalServer::reloadSvrConfig);

    //加载DB配置
    TARS_ADD_ADMIN_CMD_NORMAL("reloaddb", GlobalServer::reloadDBConfig);
}

/////////////////////////////////////////////////////////////////
void GlobalServer::destroyApp()
{
    //destroy application here:
    //...
}

/*
* 配置变更，重新加载配置
*/
bool GlobalServer::reloadSvrConfig(const string &command, const string &params, string &result)
{
    try
    {
        getOuterFactoryPtr()->readAllConfig();
        result = "reload server config success.";
        LOG_DEBUG << "reloadSvrConfig: " << result << endl;
        return true;
    }
    catch (TC_Exception const &e)
    {
        result = string("catch tc exception: ") + e.what();
    }
    catch (std::exception const &e)
    {
        result = string("catch std exception: ") + e.what();
    }
    catch (...)
    {
        result = "catch unknown exception.";
    }

    result += "\n fail, please check it.";
    LOG_DEBUG << "reloadSvrConfig: " << result << endl;
    return true;
}

/**
 * 加载DB配置数据
*/
bool GlobalServer::reloadDBConfig(const string &command, const string &params, string &result)
{
    try
    {
        loadDBConfig();

        result = "reload db config success.";
        LOG_DEBUG << "reloadDBConfig: " << result << endl;
        return true;
    }
    catch (TC_Exception const &e)
    {
        result = string("catch tc exception: ") + e.what();
    }
    catch (std::exception const &e)
    {
        result = string("catch std exception: ") + e.what();
    }
    catch (...)
    {
        result = "catch unknown exception.";
    }

    result += "\n fail, please check it.";
    LOG_DEBUG << "reloadDBConfig: " << result << endl;
    return true;
}

/**
* 初始化外部接口对象
**/
int GlobalServer::initOuterFactory()
{
    _pOuter = new OuterFactoryImp();
    return 0;
}

void GlobalServer::loadDBConfig()
{

}

/////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    try
    {
        g_app.main(argc, argv);
        g_app.waitForShutdown();
    }
    catch (std::exception &e)
    {
        cerr << "std::exception : " << e.what() << std::endl;
    }
    catch (...)
    {
        cerr << "unknown exception." << std::endl;
    }

    return -1;
}
/////////////////////////////////////////////////////////////////
