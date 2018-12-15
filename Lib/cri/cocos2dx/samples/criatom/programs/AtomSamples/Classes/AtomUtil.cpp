/****************************************************************************
 *
 * CRI ADX2 LE SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 LE sample program
 * Module  : ADX2 LE simple wrapper for sample project
 *         : サンプル用簡易ADX2 LEラッパ
 * File    : AtomUtil.cpp
 *
 ****************************************************************************/
 #include "AtomUtil.h"

/* インゲームプレビューを有効にする場合、定義する */
#define USE_INGAME_PREVIEW

namespace {

	/*---- 変数定義 ----*/
	/* 全体設定ファイル名 */
	const CriChar8* acf_file_name = "ADX2_samples.acf";

	/* DSPバス設定名（ACF内で定義されています。これはデフォルト値） */
	const CriChar8* dsp_bus_setting_name = "DspBusSetting_0";

	/* HCA-MXコーデックのサンプリングレート */
	const CriSint32 sampling_rate_hcamx = 32000;

	/* ボイスプールへの設定 */
	const CriSint32	max_voice			= 8;		/* デフォルト設定でも8	*/
	const CriSint32	max_sampling_rate	= 48000*2;	/* ピッチ変更も加味		*/

	CriAtomExAcbHn			g_acb = nullptr;			/* 波形データバンクのハンドル	*/
	CriAtomExVoicePoolHn	g_standard_voice_pool;		/* ボイスプールハンドル(ADX/HCAコーデック用) 	*/
	CriAtomExVoicePoolHn	g_hcamx_voice_pool;			/* ボイスプールハンドル(HCA-MXコーデック用) 	*/
	CriAtomExPlayerHn		g_player;		/* 音声プレーヤのハンドル		*/
	CriAtomDbasId			g_dbas_id;		/* D-BASのID				*/

	/* 多重初期化を防ぐためのカウンタ  */
	CriUint32				g_initialize_count = 0;

	/* プレイヤーの状態 */
	const std::string g_playback_status_description[] = {
		"--",
		"Prepare",
		"Playing",
		"Removed",
	};
}

/* ADX2 関連 */
namespace criadx2le {
	void initialize()
	{
		/* 初期化カウントアップ */
		g_initialize_count++;
		if (g_initialize_count > 1) {
			/* 多重初期化は許さず */
			return;
		}

	#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        audioSessionInitialize();
	#endif
		criErr_SetCallback(&errorCallback);

		criAtomEx_SetUserAllocator(&allocCallback, &freeCallback, NULL);

		/* ADX2 初期化 */
		/* MEMO: HCA-MXでもストリーミング再生できるようにファイル読み込み最大数を増やす */
		CriFsConfig fs_config;
		criFs_SetDefaultConfig(&fs_config);
		fs_config.num_loaders = 32;		/* ざっくり多め。ストリーミング最大数＋ACB等の読み込みが使用 */
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		/* PC用 */
		CriAtomExConfig_WASAPI atom_wasapi_config;
		criAtomEx_SetDefaultConfig_WASAPI(&atom_wasapi_config);
		atom_wasapi_config.atom_ex.fs_config = &fs_config;
		atom_wasapi_config.hca_mx.output_sampling_rate = sampling_rate_hcamx;
		criAtomEx_Initialize_WASAPI(&atom_wasapi_config, NULL, 0);

	#elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
		/* Mac用 */
		CriAtomExConfig_MACOSX atom_macosx_config;
		criAtomEx_SetDefaultConfig_MACOSX(&atom_macosx_config);
		atom_macosx_config.atom_ex.fs_config = &fs_config;
		atom_macosx_config.hca_mx.output_sampling_rate = sampling_rate_hcamx;
		criAtomEx_Initialize_MACOSX(&atom_macosx_config, NULL, 0);

	#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
		/* iOS用 */
		CriAtomExConfig_IOS atom_ios_config;
		criAtomEx_SetDefaultConfig_IOS(&atom_ios_config);
		atom_ios_config.atom_ex.fs_config = &fs_config;
		atom_ios_config.hca_mx.output_sampling_rate = sampling_rate_hcamx;
		criAtomEx_Initialize_IOS(&atom_ios_config, NULL, 0);

	#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
		/* Android用 */
		CriAtomExConfig_ANDROID atom_android_config;
		criAtomEx_SetDefaultConfig_ANDROID(&atom_android_config);
		atom_android_config.atom_ex.fs_config = &fs_config;
		atom_android_config.hca_mx.output_sampling_rate = sampling_rate_hcamx;
		criAtomEx_Initialize_ANDROID(&atom_android_config, NULL, 0);
	#endif

	#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
		/* ANDROIDの場合で必要な一手間。assetsフォルダへのアクセスを可能にする */
		/* まずはJniHelperでActivityのContextを取得 */
		cocos2d::JniMethodInfo methodInfo;
		cocos2d::JniHelper::getStaticMethodInfo(methodInfo,
												"org/cocos2dx/lib/Cocos2dxActivity",
												"getContext",
												"()Landroid/content/Context;");
		 auto android_context_object = (jobject)methodInfo.env->CallStaticObjectMethod( methodInfo.classID, methodInfo.methodID );
		 /* 有効化。assetsフォルダはCocosプロジェクトのResource相当なので、ほぼ必須と言って良い手順 */
		 criFs_EnableAssetsAccess_ANDROID(cocos2d::JniHelper::getJavaVM(), android_context_object);
	#endif

		/* 全体設定(ACF)の登録 */
		CriChar8 acf_path[256];
		getResourcePath(acf_path, acf_file_name);
		criAtomEx_RegisterAcfFile(NULL, acf_path, NULL, 0);

		/* DSPバス設定の登録 */
		/* MEMO: これをやらないとDSPバスエフェクト（リバーブ等）が効きません。 */
		criAtomEx_AttachDspBusSetting(dsp_bus_setting_name, NULL, 0);

		/* ボイスプール作成 */
		CriAtomExStandardVoicePoolConfig vp_config;
		/* ボイスプールの設定。まずはデフォルト設定にして、その上で必要な値へ書き換えていく */
		criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&vp_config);
		vp_config.num_voices						= max_voice;
		vp_config.player_config.streaming_flag		= CRI_TRUE;
		vp_config.player_config.max_sampling_rate	= max_sampling_rate;
		/* 上で作った設定オブジェクトを渡して、ボイスプールを作成 */
		g_standard_voice_pool = criAtomExVoicePool_AllocateStandardVoicePool(&vp_config, NULL, 0);

		/* HCA-MX再生用のボイスプール作成 */
		CriAtomExHcaMxVoicePoolConfig hcamx_config;
		criAtomExVoicePool_SetDefaultConfigForHcaMxVoicePool(&hcamx_config);
		hcamx_config.num_voices							= max_voice;
		/* 一応ストリーミング再生可能で初期化しておく。メモリ再生のみなら不要。 */
		hcamx_config.player_config.streaming_flag		= CRI_TRUE;
		hcamx_config.player_config.max_sampling_rate	= sampling_rate_hcamx;
		g_hcamx_voice_pool = criAtomExVoicePool_AllocateHcaMxVoicePool(&hcamx_config, NULL, 0);

		/* DBASの作成（ストリーミング再生に必要） */
		g_dbas_id = criAtomDbas_Create(NULL, NULL, 0);

	#if defined(USE_INGAME_PREVIEW)
		CriAtomExAsrBusAnalyzerConfig analyze_config;
		CriSint32 bus_no;
		/* インゲームプレビュー用のモニタライブラリを初期化 */
		criAtomExMonitor_Initialize(NULL, NULL, 0);
		/* レベル測定機能を追加 */
		criAtomExAsr_SetDefaultConfigForBusAnalyzer(&analyze_config);
		for ( bus_no=0;bus_no<8;bus_no++) {
			criAtomExAsr_AttachBusAnalyzer(bus_no, &analyze_config);
		}
	#endif

		/* Player作成にも設定は必要 */
		CriAtomExPlayerConfig pf_config;
		criAtomExPlayer_SetDefaultConfig(&pf_config);
		pf_config.max_path_strings	= 1;
		pf_config.max_path			= 256;
		g_player					= criAtomExPlayer_Create(&pf_config, NULL, 0);
	}

	bool loadAcbFile(const CriChar8* acb_name, const CriChar8* awb_name) {
		if (g_acb != nullptr) {
			/* 既にロード済みのACBがある場合は何もしない */
			/* 複数のACBをロードすることもできるが、その分メモリ消費量も増える。本サンプルでは行わない */
			return false;
		}

		CriChar8 acb_path[256], awb_path[256];
		getResourcePath(acb_path, acb_name);
		getResourcePath(awb_path, awb_name);

		CriAtomExAcbHn acb_handle = criAtomExAcb_LoadAcbFile(NULL, acb_path, NULL, awb_path, NULL, 0);
		if (acb_handle == nullptr) {
			return false;
		}
		g_acb = acb_handle;
		return true;
	}

	void releaseAcbFile(){
		/* ロード済みACBのハンドルを解放 */
		criAtomExAcb_Release(g_acb);
		g_acb = nullptr;
	}

	void finalize()
	{
		/* 早すぎる終了処理をしないように。Atomを使っている人がまだいるかもしれない */
		g_initialize_count--;
		if (g_initialize_count > 0) {
			return;
		}

		criAtomExPlayer_Destroy(g_player);

	#if defined(USE_INGAME_PREVIEW)
		/* インゲームプレビュー関連機能の終了処理 */
		CriSint32 bus_no;
		for ( bus_no=0;bus_no<8;bus_no++) {
			criAtomExAsr_DetachBusAnalyzer(bus_no);
		}
		criAtomExMonitor_Finalize();
	#endif

		criAtomExVoicePool_Free(g_hcamx_voice_pool);
		criAtomExVoicePool_Free(g_standard_voice_pool);
		criAtomDbas_Destroy(g_dbas_id);
		criAtomEx_DetachDspBusSetting();
		criAtomEx_UnregisterAcf();

	#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        audioSessionFinalize();
		criAtomEx_Finalize_IOS();
	#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
		criAtomEx_Finalize_ANDROID();
	#elif (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
		criAtomEx_Finalize_MACOSX();
	#elif (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		criAtomEx_Finalize_WASAPI();
	#endif
	}

	/* AtomBasicSceneレイヤのupdateで呼ぶ */
	void update()
	{
		/* ADX2の定期処理 */
		criAtomEx_ExecuteMain();

	#if defined(USE_INGAME_PREVIEW)
		/* バス解析情報の取得 */
		CriAtomExAsrBusAnalyzerInfo analyze_info;
		CriSint32 bus_no;
		for ( bus_no=0;bus_no<8;bus_no++) {
			criAtomExAsr_GetBusAnalyzerInfo(bus_no, &analyze_info);
		}
	#endif

		return;
	}

	/* キューID指定で音を鳴らす処理。鳴らした音のプレイバックIDが返る (キューIDとプレイバックIDは別物) */
	CriAtomExPlaybackId playCueById(CriAtomExCueId cue_id)
	{
		/* MEMO: 特定の音だけピッチを変えて再生したい場合。      */
		/* (1) プレーヤにピッチを設定。                          */
		/* (2) 再生開始。                                        */
		/* (3) プレーヤのピッチを戻す。                          */
		/* {                                                     */
		/*   criAtomExPlayer_SetPitch(g_player, pitch);          */
		/*   criAtomExPlayer_SetCueId(g_player, g_acb, cue_id);  */
		/*   criAtomExPlayer_Start(g_player, pitch);             */
		/*   criAtomExPlayer_SetPitch(g_player, 0.0f);           */
		/* {                                                     */
		/* 補足: HCA-MXコーデックの場合はピッチ変更は無効。      */

		criAtomExPlayer_SetCueId(g_player, g_acb, cue_id);
		CriAtomExPlaybackId playback_id = criAtomExPlayer_Start(g_player);
		return playback_id;
	}

	/* プレイバックID指定で音を止める */
	void stop(CriAtomExPlaybackId playback_id)
	{
		criAtomExPlayback_Stop(playback_id);
	}
	
	/* プレイヤー自体を止める */
	void stopAll()
	{
		criAtomExPlayer_Stop(g_player);
	}
	
	/* Resourcesフォルダへのアクセスパス */
	void getResourcePath(CriChar8 *full_path, const CriChar8 *loc_path)
	{
	#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
		sprintf(full_path, "%s", cocos2d::FileUtils::getInstance()->fullPathForFilename(loc_path).c_str());
	#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		/* assetsへのアクセスパスは、必ずassets以下からの相対パスでファイルを指定する
		 * パスの先頭に'/'文字が入ってはダメ */
		sprintf(full_path, "%s", loc_path);
	#endif
	}
	
	void* allocCallback(void *obj, CriUint32 size)
	{
		return malloc(size);
	}
	
	void freeCallback(void *obj, void *ptr)
	{
		free(ptr);
	}
	
	void errorCallback(const CriChar8 *errid, CriUint32 p1, CriUint32 p2, CriUint32 *parray)
	{
		const CriChar8 *errmsg;
		errmsg = criErr_ConvertIdToMessage(errid, p1, p2);
		CCLOG("%s\n", errmsg);
	}

	const char* getCueName(CriAtomExCueId cue_id)
	{
		return criAtomExAcb_GetCueNameById(g_acb, cue_id);
	}

	/* 現在使用中のボイス数を(文字列表現で)取得する */
	std::string getVoiceNum()
	{
		CriSint32 standard_voice_num;
		CriSint32 hcamx_voice_num;
		criAtomExVoicePool_GetNumUsedVoices(g_standard_voice_pool, &standard_voice_num, NULL);
		criAtomExVoicePool_GetNumUsedVoices(g_hcamx_voice_pool, &hcamx_voice_num, NULL);

		/* 整数 --> 文字列 に変換 */
		std::stringstream time_ss;
		time_ss << "[StandardVoice: " << standard_voice_num << "/" << max_voice << "]" << 
                 "\n[HcaMxVoice   : " << hcamx_voice_num << "/" << max_voice << "]";
		return time_ss.str();
	}

	/* 現在プレイバック中の音声の再生時間を(文字列表現で)取得する */
	std::string getTime(CriAtomExCueId cue_id)
	{
		/* 整数 --> 文字列 に変換 */
		std::stringstream time_ss;
		time_ss << "[Time(msec): " << criAtomExPlayback_GetTime(cue_id) << "]";
		return time_ss.str();
	}

	CriSint32 getNumCueItems()
	{
		return criAtomExAcb_GetNumCues(g_acb);
	}

	std::string getPlaybackStatusDescription(CriAtomExPlaybackStatus status)
	{
		return g_playback_status_description[status];
	}

	void updateAisacParam(CriFloat32 aisac_value){
		if (aisac_value < 0.0f) {

			aisac_value = 0.0f;

		}

		criAtomExPlayer_SetAisacById(g_player, 0, aisac_value);

		criAtomExPlayer_UpdateAll(g_player);
	}
}
