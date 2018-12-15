/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : Interactive Sound with AISAC
 *         : インタラクティブサウンド再生サンプル
 * File    : AtomAisacScene.cpp
 *
 ****************************************************************************/

/*
 このサンプルは、Cocos2Dxによって、サウンドをキューID指定再生します。
 画面上のUIボタンは以下のような機能を持っています。
 　(1) 左側の再生ボタン x8	:	それぞれ対応するIDのキューを再生
　(2) 右上の停止ボタン x2	:	PlaybackId単位の停止，Player単位の停止
　(3) 右下のラベル x2	:	再生状態表示、再生時間表示。 情報は毎フレーム更新
 */

#include "AtomAisacScene.h"

USING_NS_CC;

//---------------------------------------------
//---- cocos2d-x ----
//---------------------------------------------

/* 画面に配置するUIボタンに割り当てるコールバック。音を止める */
void AtomAisacScene::menuStop(Ref* pSender)
{
	criadx2le::stop(currentPlaybackId);
}

/* 画面に配置するUIボタンに割り当てるコールバック。音をプレイヤーごと止める */
void AtomAisacScene::menuAllStop(Ref* pSender)
{
	criadx2le::stopAll();
}

bool AtomAisacScene::init()
{
	CCLOG("%s", "AtomAisacScene::init");
	if ( !Layer::init() )
    {
        return false;
    }
	/* このシーンで使うキューシート(ACB)と波形(AWB)をロードする */
	criadx2le::loadAcbFile("InteractiveSound.acb", "InteractiveSound.awb");

	cocos2d::Size size = cocos2d::Director::getInstance()->getWinSize();

	/* GUIの設定と生成。そして配置。ボタンとラベル */
	{
		auto size = Director::getInstance()->getWinSize();

		// Add Text Labe
		{
			auto label = cocos2d::ui::Text::create();
			label->setString("Hello AISAC!");
			label->setFontName("Marker Felt");
			label->setFontSize(30);
			label->setColor(Color3B(159, 168, 176));
			label->setPosition(cocos2d::Point(size.width / 2, size.height *0.6f));

			addChild(label);
		}

		// Add Slider
		{
			auto slider = cocos2d::ui::Slider::create();
			slider->loadBarTexture("Slider_base.png");
			slider->loadSlidBallTextures("Slider_button.png", "Slider_button.png", "");
			slider->setScale9Enabled(true);
			slider->setCapInsets(cocos2d::Rect(0, 0, 0, 0));
			slider->setContentSize(cocos2d::Size(250.0f, 19));
			slider->setPosition(Vec2(size.width / 2, size.height *0.5f));
			slider->addEventListener(CC_CALLBACK_2(AtomAisacScene::slideEvent, this));
			slider->setPercent(100);
			addChild(slider);
		}

		/* Scene Select Menu Button*/
		{
			auto *item = MenuItemImage::create(
												   "SELECT_SCENE_Normal.png",
												   "SELECT_SCENE_Push.png",
												   CC_CALLBACK_1(AtomAisacScene::menuSceneTransCallback, this));    
			item->setPosition( cocos2d::Point(size.width / 2 , item->getContentSize().height/2) );
			auto menu = Menu::create(item, NULL);
			menu->setPosition(cocos2d::Point::ZERO);
			this->addChild(menu, 1);
		}

	}

	/* キューIDの0番を指定 */
	criadx2le::playCueById(0);

	scheduleUpdate();
	
    return true;
}

void AtomAisacScene::update(float dt)
{
	/* ADX2 Atomライブラリのアップデート処理 */
	criadx2le::update();

}


cocos2d::Scene* AtomAisacScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = cocos2d::Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = AtomAisacScene::create();


    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

AtomAisacScene::AtomAisacScene() :
	vnumLabel(nullptr),
	stateLabel(nullptr),
	timeLabel(nullptr)
{

}

AtomAisacScene::~AtomAisacScene()
{
	/* このシーンで使ったACBを解放しなければならない */
	criadx2le::releaseAcbFile();

	CC_SAFE_RELEASE(vnumLabel);
	CC_SAFE_RELEASE(stateLabel);
	CC_SAFE_RELEASE(timeLabel);
}

void AtomAisacScene::slideEvent(Ref* pSender, cocos2d::ui::Slider::EventType type) {
	auto slider = dynamic_cast<cocos2d::ui::Slider*>(pSender);
	criadx2le::updateAisacParam(static_cast<float>(slider->getPercent()) / 100.0f);
}

void AtomAisacScene::menuSceneTransCallback(Ref* pSender) {
	criadx2le::stopAll();
	auto director = Director::getInstance();
	director->replaceScene(MenuScene::createScene());
	this->removeAllChildren();
}

