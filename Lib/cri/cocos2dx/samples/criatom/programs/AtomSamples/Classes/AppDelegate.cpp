/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : Application delegate for ADX2 LE samples
 *         : ADX2 LEサンプルのアプリケーションデリゲート
 * File    : AppDelegate.cpp
 *
 ****************************************************************************/
 
#include "AppDelegate.h"
#include "./Scenes/MenuScene.h"

USING_NS_CC;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
	criadx2le::finalize();
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = GLViewImpl::create("CRI Sample");
        director->setOpenGLView(glview);
    }

    glview->setDesignResolutionSize(640, 960, ResolutionPolicy::SHOW_ALL);

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);


	/* CRIライブラリの初期化 */
	criadx2le::initialize();

    // create a scene. it's an autorelease object
    auto scene = MenuScene::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    /* サスペンド時は音声を一時停止 */
    criAtomEx_StopSound_ANDROID();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    /* サスペンド時は音声を一時停止 */
    criAtomEx_StopSound_IOS();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();
    
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    /* リジューム時は音声を再開 */
    criAtomEx_StartSound_ANDROID();
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    /* サスペンド時は音声を一時停止 */
    criAtomEx_StopSound_IOS();
#endif
}
