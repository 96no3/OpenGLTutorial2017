/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : ADX2 LE simple wrapper for sample project
 *         : サンプル用簡易ADX2 LEラッパ
 * File    : AtomUtil.h
 *
 ****************************************************************************/
 #ifndef  CRI_INCL_ATOM_UTIL_H
#define  CRI_INCL_ATOM_UTIL_H

#include <string>
#include "cocos2d.h"
#include <cri_adx2le.h>

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include <AudioToolbox/AudioSession.h>
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#endif

namespace criadx2le {

	/*---- 関数宣言 ----*/

	void initialize();
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    bool audioSessionInitialize();
    void audioSessionFinalize();
	#endif
	bool loadAcbFile(const CriChar8* acb_name, const CriChar8* awb_name);
	void releaseAcbFile();
	void finalize();
	void update();
	CriAtomExPlaybackId playCueById(CriAtomExCueId cue_id);
	void stop(CriAtomExPlaybackId playback_id);
	void stopAll();
	void pauseAll(CriBool sw);
	void getResourcePath(CriChar8 *full_path, const CriChar8 *loc_path);
	void *allocCallback(void *obj, CriUint32 size);
	void freeCallback(void *obj, void *ptr);
	void errorCallback(const CriChar8 *errid, CriUint32 p1, CriUint32 p2, CriUint32 *parray);
	const char* getCueName(CriAtomExCueId cue_id);
	std::string getVoiceNum();
	std::string getTime(CriAtomExCueId cue_id);
	CriSint32 getNumCueItems();
	std::string getPlaybackStatusDescription(CriAtomExPlaybackStatus status);
	void updateAisacParam(CriFloat32 aisac_value);
}

#endif // CRI_INCL_ATOM_UTIL_H