/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : Interactive Sound with AISAC
 *         : インタラクティブサウンド再生サンプル
 * File    : AtomAisacScene.h
 *
 ****************************************************************************/
 #ifndef ATOM_AISAC_SCENE_H
#define ATOM_AISAC_SCENE_H

#include <cocos2d.h>
#include <ui/CocosGUI.h>
#include "../AtomUtil.h"
#include "../Scenes/MenuScene.h"

class AtomAisacScene : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // implement the "static create()" method manually
   CREATE_FUNC(AtomAisacScene);

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void update(float dt);

private:
    AtomAisacScene();
    ~AtomAisacScene();

    // a selector callback
	void menuStop(cocos2d::Ref* pSender);
	void menuAllStop(cocos2d::Ref* pSender);
	void menuPlayCueById(CriAtomExCueId cue_id);

	CC_SYNTHESIZE_RETAIN(cocos2d::Label*, vnumLabel,	VnumLabel);
	CC_SYNTHESIZE_RETAIN(cocos2d::Label*, stateLabel,	StateLabel);
	CC_SYNTHESIZE_RETAIN(cocos2d::Label*, timeLabel,	TimeLabel);
    CriAtomExPlaybackId			currentPlaybackId;
	void slideEvent(Ref* pSender, cocos2d::ui::Slider::EventType type);
	void menuSceneTransCallback(Ref* pSender);
};

#endif // ATOM_AISAC_SCENE_H
