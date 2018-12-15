/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : Simple Playback using Cocos2Dx
 *         : Cocos2Dxでの単純再生サンプル
 * File    : AtomBasicSceneScene.cpp
 *
 ****************************************************************************/

/*
 このサンプルは、Cocos2Dxによって、サウンドをキューID指定再生します。
 画面上のUIボタンは以下のような機能を持っています。
 　(1) 左側の再生ボタン x8	:	それぞれ対応するIDのキューを再生
　(2) 右上の停止ボタン x2	:	PlaybackId単位の停止，Player単位の停止
　(3) 右下のラベル x2	:	再生状態表示、再生時間表示。 情報は毎フレーム更新
 */

#include "AtomBasicScene.h"

USING_NS_CC;

//---------------------------------------------
//---- cocos2d-x ----
//---------------------------------------------

/* 画面に配置するUIボタンに割り当てるコールバック。音を止める */
void AtomBasicScene::menuStop(Ref* pSender)
{
	criadx2le::stop(currentPlaybackId);
}

/* 画面に配置するUIボタンに割り当てるコールバック。音をプレイヤーごと止める */
void AtomBasicScene::menuAllStop(Ref* pSender)
{
	criadx2le::stopAll();
}

bool AtomBasicScene::init()
{
	CCLOG("%s", "AtomBasicScene::init");
	if ( !Layer::init() )
    {
        return false;
    }
	/* このシーンで使うキューシート(ACB)と波形(AWB)をロードする */
	criadx2le::loadAcbFile("Basic.acb", "Basic.awb");

	cocos2d::Size size = cocos2d::Director::getInstance()->getWinSize();

	/* GUIの設定と生成。そして配置。ボタンとラベル */
	{
		/* MenuItem (ボタン) をためこむベクタ。あとで一気に一個のMenuとしてadd */
		cocos2d::Vector<cocos2d::MenuItem*> buttonItems;

		/* ボタン生成 & ベクタへのプッシュバックの処理 */
		auto pushButton = [this, &size, &buttonItems](float x, float y,
				const char* ui_normal_image, const char* ui_push_image, cocos2d::ccMenuCallback c){
			auto *item = cocos2d::MenuItemImage::create(ui_normal_image, ui_push_image, std::move(c));
			item->setPosition(cocos2d::Point(size.width  * x, size.height * y));
			buttonItems.pushBack(item);
		};

		/* ラベル生成 & UIとして追加の処理 */
		auto addLabel = [this, &size](cocos2d::Label *pLabel, float x, float y){
			pLabel->setPosition(cocos2d::Point(size.width  * x, size.height * y));
			addChild(pLabel);
		};

		/* 鳴らしたいキューの数だけボタンを作る */
		/* 注意：この処理はキューIDが０から始まって連続で存在することを前提としています。 */
		for (CriAtomExCueId id = 0; id < criadx2le::getNumCueItems(); id++) {
			/* キュー名の存在チェック */
			if (criadx2le::getCueName(id) == NULL) {
				CCLOG("%s", "SAMPLE: Cue ID is not sequential number. (0, 1, 2, .... , num_cue-1)");
				continue;
			}
			/* キュー名のボタンとラベル作成 */
			pushButton(0.10f, 0.95f - static_cast<float>(id)*0.10f, "PlayNormal.png", "PlayPush.png",
				[this, id](Ref*)->void{currentPlaybackId = criadx2le::playCueById(id);});
			addLabel(cocos2d::Label::createWithTTF(criadx2le::getCueName(id), "fonts/Marker Felt.ttf", 25),
					0.30f, 0.95f - static_cast<float>(id)*0.10f);
		}

		/* 音声停止ボタン (プレイバックID単位) */
		{
			pushButton(0.60f, 0.95f, "StopNormal.png", "StopPush.png",
					CC_CALLBACK_1(AtomBasicScene::menuStop, this));
			addLabel(cocos2d::Label::createWithTTF("Stop", "fonts/Marker Felt.ttf", 25),
					0.75f, 0.95f);
		}

		/* 音声停止ボタン (プレイヤー単位) */
		{
			pushButton(0.60f, 0.85f, "StopNormal.png", "StopPush.png",
					CC_CALLBACK_1(AtomBasicScene::menuAllStop, this));
			addLabel(cocos2d::Label::createWithTTF("stopAll", "fonts/Marker Felt.ttf", 25),
					0.75f, 0.85f);
		}

		/* ここまでに作った全てのMenuItem(ボタン)をGUIとして追加 */
		{
			cocos2d::Menu* menu = cocos2d::Menu::createWithArray(buttonItems);
			menu->setPosition(cocos2d::Point::ZERO);
			this->addChild(menu);
		}

		/* 現在のボイス数を示すラベルを追加 */
		{
			setVnumLabel(cocos2d::Label::createWithTTF(
					criadx2le::getVoiceNum(),
					"fonts/Marker Felt.ttf", 30));
			addLabel(vnumLabel, 0.75f, 0.17f);
		}

		/* 現在のプレイヤー状態を示すラベルを追加 */
		{
			setStateLabel(cocos2d::Label::createWithTTF(
					criadx2le::getPlaybackStatusDescription( criAtomExPlayback_GetStatus(currentPlaybackId) ),
					"fonts/Marker Felt.ttf", 30));
			addLabel(stateLabel, 0.75f, 0.10f);
		}

		/* 現在の再生時間を示すラベルを追加 */
		{
			setTimeLabel(cocos2d::Label::createWithTTF(
					criadx2le::getTime(currentPlaybackId),
					"fonts/Marker Felt.ttf", 30));
			addLabel(timeLabel, 0.75f, 0.05f);
		}

		/* Scene Select Menu Button*/
		{
			auto *item = MenuItemImage::create(
												   "SELECT_SCENE_Normal.png",
												   "SELECT_SCENE_Push.png",
												   CC_CALLBACK_1(AtomBasicScene::menuSceneTransCallback, this));    
			item->setPosition( cocos2d::Point(size.width / 2 , item->getContentSize().height/2) );
			auto menu = Menu::create(item, NULL);
			menu->setPosition(cocos2d::Point::ZERO);
			this->addChild(menu, 1);
		}
	}

	scheduleUpdate();
	
    return true;
}

void AtomBasicScene::update(float dt)
{
	/* ADX2 Atomライブラリのアップデート処理 */
	criadx2le::update();

	/* ラベル情報の更新 */
	vnumLabel->setString(criadx2le::getVoiceNum());
	stateLabel->setString(criadx2le::getPlaybackStatusDescription( criAtomExPlayback_GetStatus(currentPlaybackId) ));
	timeLabel->setString(criadx2le::getTime(currentPlaybackId));
}


cocos2d::Scene* AtomBasicScene::createScene()
{
    // 'scene' is an autorelease object
    auto scene = cocos2d::Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = AtomBasicScene::create();


    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

AtomBasicScene::AtomBasicScene() :
	vnumLabel(nullptr),
	stateLabel(nullptr),
	timeLabel(nullptr)
{

}

AtomBasicScene::~AtomBasicScene()
{
	/* このシーンで使ったACBを解放しなければならない */
	criadx2le::releaseAcbFile();

	CC_SAFE_RELEASE(vnumLabel);
	CC_SAFE_RELEASE(stateLabel);
	CC_SAFE_RELEASE(timeLabel);
}

void AtomBasicScene::menuSceneTransCallback(Ref* pSender) {
	criadx2le::stopAll();
	auto director = Director::getInstance();
	director->replaceScene(MenuScene::createScene());
	this->removeAllChildren();
}