/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : Menu scene for ADX2 LE samples
 *         : ADX2 LEサンプルのメニューシーン
 * File    : MenuScene.h
 *
 ****************************************************************************/
#ifndef CRI_INCL_MENU_SCENE_H
#define CRI_INCL_MENU_SCENE_H

#include "cocos2d.h"
#include "../Scenes/AtomBasicScene.h"
#include "../Scenes/AtomAisacScene.h"

class MenuScene : public cocos2d::Layer
{
public:
	~MenuScene();

    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();  

	// implement the "static create()" method manually
    CREATE_FUNC(MenuScene);
};

#endif // CRI_INCL_MENU_SCENE_H
