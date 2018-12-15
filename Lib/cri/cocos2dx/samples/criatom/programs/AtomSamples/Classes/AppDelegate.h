/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : Application delegate for ADX2 LE samples
 *         : ADX2 LEサンプルのアプリケーションデリゲート
 * File    : AppDelegate.h
 *
 ****************************************************************************/
 #ifndef  CRI_INCL_APP_DELEGATE_H
#define  CRI_INCL_APP_DELEGATE_H

#include "cocos2d.h"
#include "AtomUtil.h"

/**
@brief    The cocos2d Application.

The reason for implement as private inheritance is to hide some interface call by Director.
*/
class  AppDelegate : private cocos2d::Application
{
public:
    AppDelegate();
    virtual ~AppDelegate();

    /**
    @brief    Implement Director and Scene init code here.
    @return true    Initialize success, app continue.
    @return false   Initialize failed, app terminate.
    */
	bool applicationDidFinishLaunching() override;

    /**
    @brief  The function be called when the application enter background
    @param  the pointer of the application
    */
    void applicationDidEnterBackground() override;

    /**
    @brief  The function be called when the application enter foreground
    @param  the pointer of the application
    */
    void applicationWillEnterForeground() override;

	void initializeCriLibraries();
	void finalizeCriLibraries();
};

#endif // CRI_INCL_APP_DELEGATE_H

