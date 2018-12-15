/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : Simple Playback using Cocos2Dx
 *         : Cocos2Dxでの単純再生サンプル
 * File    : AtomBasicScene.h
 *
 ****************************************************************************/
#ifndef ATOM_BASIC_SCENE_H
#define ATOM_BASIC_SCENE_H

#include "cocos2d.h"
#include "../AtomUtil.h"
#include "../Scenes/MenuScene.h"

class AtomBasicScene : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // implement the "static create()" method manually
   CREATE_FUNC(AtomBasicScene);

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
	virtual bool init();
	virtual void update(float dt);

private:
    AtomBasicScene();
    ~AtomBasicScene();

    // a selector callback
	void menuStop(cocos2d::Ref* pSender);
	void menuAllStop(cocos2d::Ref* pSender);

	CC_SYNTHESIZE_RETAIN(cocos2d::Label*, vnumLabel,	VnumLabel);
	CC_SYNTHESIZE_RETAIN(cocos2d::Label*, stateLabel,	StateLabel);
	CC_SYNTHESIZE_RETAIN(cocos2d::Label*, timeLabel,	TimeLabel);
    CriAtomExPlaybackId			currentPlaybackId;
	void menuSceneTransCallback(Ref* pSender);
};

#endif // ATOM_BASIC_SCENE_H
