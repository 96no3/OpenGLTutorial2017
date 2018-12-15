/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : Menu scene for ADX2 LE samples
 *         : ADX2 LEサンプルのメニューシーン
 * File    : MenuScene.cpp
 *
 ****************************************************************************/
#include "MenuScene.h"

USING_NS_CC;

MenuScene::~MenuScene(){
	//CCLOG("MenuScene is terminated.");
}

Scene* MenuScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = MenuScene::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

namespace
{
	template <typename Layer>
	void replaceScene(Ref*) {
		Director::getInstance()->replaceScene(Layer::createScene());			
	}
}

// on "init" you need to initialize your instance
bool MenuScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
	/* アイテム配置用定数 */
    cocos2d::Size windowSize  = Director::getInstance()->getWinSize();
    float descriptionPosX  = windowSize.width  * 0.50f;
    float descriptionPosY  = windowSize.height * 0.90f;
    float selectButtonPosX = windowSize.width  * 0.15f;

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.
	Menu* menu;
	{
        auto addItem = [&](MenuItem* item, const float& posx, const float& posy){
            item->setPosition(cocos2d::Point(posx, posy));
            menu = Menu::create(item, NULL);
            menu->setPosition(Vec2::ZERO);
            this->addChild(menu, 1);
        };

        auto addLabel = [&](const std::string& text, const float& posx, const float& posy, const int& size){
            auto label = Label::createWithSystemFont(text, "Arial", size, cocos2d::Size(500, size*2),
                                                     TextHAlignment::LEFT, TextVAlignment::CENTER);
            label->setPosition(cocos2d::Point(posx, posy));
            this->addChild(label, 1);
        };
        
        addLabel("[ CRI ADX2: Select Sample ]",descriptionPosX, descriptionPosY, 30);
        
        // for AtomBasic Scene
		{
            // SelectButton
            auto selectItem = MenuItemImage::create("PlayNormal.png", "PlayPush.png", replaceScene<AtomBasicScene>);
            addItem(selectItem, selectButtonPosX, windowSize.height / 2 + selectItem->getContentSize().height * 2);
			addLabel("AtomBasic", selectItem->getPosition().x + selectItem->getContentSize().width * 3, selectItem->getPosition().y, 24);
		}
        // for AtomAisac Scene
		{
            // SelectButton
            auto selectItem = MenuItemImage::create("PlayNormal.png", "PlayPush.png", replaceScene<AtomAisacScene>);
            addItem(selectItem, selectButtonPosX, windowSize.height / 2);
			addLabel("AtomAisac", selectItem->getPosition().x + selectItem->getContentSize().width * 3, selectItem->getPosition().y, 24);
		}
	}

    return true;
}
