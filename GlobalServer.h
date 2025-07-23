#ifndef _GlobalServer_H_
#define _GlobalServer_H_

#include <iostream>
#include "servant/Application.h"
#include "OuterFactoryImp.h"

//
using namespace tars;

/**
 *
 **/
class GlobalServer : public Application
{
public:
    /**
     *
     */
    GlobalServer() {}

    /**
     *
     **/
    virtual ~GlobalServer() {};

    /**
     *
     **/
    virtual void initialize();

    /**
     *
     **/
    virtual void destroyApp();

public:
    /*
    * 配置变更，重新加载配置
    */
    bool reloadSvrConfig(const string &command, const string &params, string &result);

    /**
     * 加载DB配置数据
    */
    bool reloadDBConfig(const string &command, const string &params, string &result);

public:
    /**
    * 初始化外部接口对象
    **/
    int initOuterFactory();

    /**
    * 取外部接口对象
    **/
    OuterFactoryImpPtr getOuterFactoryPtr()
    {
        return _pOuter;
    }

public:
    /**
     * 加载DB数据
    */
    void loadDBConfig();

private:
    //外部接口对象
    OuterFactoryImpPtr _pOuter;
};

////
extern GlobalServer g_app;

////////////////////////////////////////////
#endif
