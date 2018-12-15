/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Initializer
 * File     : CriWareInitializer.cpp
 *
 ****************************************************************************/

/* プリプロセッサ定義の初期化 */
#if !defined(CRIWARE_USE_XAUDIO2)
#define CRIWARE_USE_XAUDIO2 0
#endif
#if !defined(CRIWARE_USE_PCM_OUTPUT)
#define CRIWARE_USE_PCM_OUTPUT 0
#endif
#if !defined(CRIWARE_WITH_UE4_SOUND)
#define CRIWARE_WITH_UE4_SOUND 1
#endif
#if !defined(CRIWARE_USE_INTEL_MEDIA)
#define CRIWARE_USE_INTEL_MEDIA 0
#endif

/***************************************************************************
 *      インクルードファイル
 *      Include files
 ***************************************************************************/
/* モジュールヘッダ */
#include "CriWareInitializer.h"

/* CRIWAREプラグインヘッダ */
#include "CriWareRuntimePrivatePCH.h"
#include "CriWareFileIo.h"
#include "SoundAtomConfig.h"
#include "AtomListener.h"

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#else	/* </cri_delete_if_LE> */

#if defined(XPT_TGT_PC)
#include "AllowWindowsPlatformTypes.h"
#include <cri_le_atom_wasapi.h>
#include "HideWindowsPlatformTypes.h"
#elif defined(XPT_TGT_WINRT)
#include <cri_atom_winrt.h>
#include "AllowWindowsPlatformTypes.h"
#include <cri_atom_wasapi.h>
#include "HideWindowsPlatformTypes.h"
#elif defined(XPT_TGT_MACOSX)
#include <cri_atom_macosx.h>
#elif defined(XPT_TGT_IOS)
#include <cri_atom_ios.h>
#elif defined(XPT_TGT_LINUX)
#include <cri_atom_linux.h>
#elif defined(XPT_TGT_ANDROID)
#include <cri_atom_android.h>
#endif

#endif

/* ANSI Cヘッダ */
#include <stdio.h>

/* Unreal Engine 4関連ヘッダ */
#include "Misc/CoreDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/MessageDialog.h"
#include "HAL/FileManager.h"

/***************************************************************************
 *      定数マクロ
 *      Macro Constants
 ***************************************************************************/
#define LOCTEXT_NAMESPACE "CriWareInitializer"

/* 最大入出力ch数をプラットフォームごとに切り分け */
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#else	/* </cri_delete_if_LE> */

#if defined(XPT_TGT_PC) || defined(XPT_TGT_MACOSX) \
	|| defined(XPT_TGT_WINRT) || defined(XPT_TGT_LINUX)
#define CRIWARE_UE4_MAX_CHANNELS 8
#elif defined(XPT_TGT_IOS) || defined(XPT_TGT_ANDROID)
#define CRIWARE_UE4_MAX_CHANNELS 2
#else
#error /* CRIWARE UE4 Pluginでは未対応のプラットフォームです。 */
#endif

#endif

/* 最大ボイスリミットグループ数 */
#define CRIWARE_UE4_MAX_VOICE_LIMIT_GROUPS 128

/* 最大カテゴリ数 */
#define CRIWARE_UE4_MAX_CATEGORIES 128

/***************************************************************************
 *      処理マクロ
 *      Macro Functions
 ***************************************************************************/
/* 初期化／終了処理の共通化 */
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#else	/* </cri_delete_if_LE> */

#if CRIWARE_USE_PCM_OUTPUT
#include "AtomAudioDevice.h"
#define CriAtomExConfig_UE4				CriAtomExConfigForUserPcmOutput
#define criAtomEx_SetDefaultConfig_UE4	criAtomEx_SetDefaultConfigForUserPcmOutput
#define criAtomEx_Initialize_UE4		criAtomEx_InitializeForUserPcmOutput
#define criAtomEx_Finalize_UE4			criAtomEx_FinalizeForUserPcmOutput
#elif defined(XPT_TGT_PC)
#define CriAtomExConfig_UE4				CriAtomExConfig_WASAPI
#define criAtomEx_SetDefaultConfig_UE4	criAtomEx_SetDefaultConfig_WASAPI
#define criAtomEx_Initialize_UE4		criAtomEx_Initialize_WASAPI
#define criAtomEx_Finalize_UE4			criAtomEx_Finalize_WASAPI
#elif defined(XPT_TGT_WINRT)
#define CriAtomExConfig_UE4				CriAtomExConfig_WASAPI
#define criAtomEx_SetDefaultConfig_UE4	criAtomEx_SetDefaultConfig_WASAPI
#define criAtomEx_Initialize_UE4		criAtomEx_Initialize_WASAPI
#define criAtomEx_Finalize_UE4			criAtomEx_Finalize_WASAPI
#elif defined(XPT_TGT_MACOSX)
#define CriAtomExConfig_UE4				CriAtomExConfig_MACOSX
#define criAtomEx_SetDefaultConfig_UE4	criAtomEx_SetDefaultConfig_MACOSX
#define criAtomEx_Initialize_UE4		criAtomEx_Initialize_MACOSX
#define criAtomEx_Finalize_UE4			criAtomEx_Finalize_MACOSX
#elif defined(XPT_TGT_IOS)
#define CriAtomExConfig_UE4				CriAtomExConfig_IOS
#define criAtomEx_SetDefaultConfig_UE4	criAtomEx_SetDefaultConfig_IOS
#define criAtomEx_Initialize_UE4		criAtomEx_Initialize_IOS
#define criAtomEx_Finalize_UE4			criAtomEx_Finalize_IOS
#elif defined(XPT_TGT_LINUX)
#define CriAtomExConfig_UE4			CriAtomExConfig_LINUX
#define criAtomEx_SetDefaultConfig_UE4		criAtomEx_SetDefaultConfig_LINUX
#define criAtomEx_Initialize_UE4		criAtomEx_Initialize_ALSA
#define criAtomEx_Finalize_UE4			criAtomEx_Finalize_ALSA
#elif defined(XPT_TGT_ANDROID)
#define CriAtomExConfig_UE4				CriAtomExConfig_ANDROID
#define criAtomEx_SetDefaultConfig_UE4	criAtomEx_SetDefaultConfig_ANDROID
#define criAtomEx_Initialize_UE4		criAtomEx_Initialize_ANDROID
#define criAtomEx_Finalize_UE4			criAtomEx_Finalize_ANDROID
#endif

#define isExistSection_FileSystem_or_isExistSection_Atom_or_isExistSection_Mana (isExistSection_FileSystem || isExistSection_Atom)

#endif

/* Intel Media SDK */
#if CRIWARE_USE_INTEL_MEDIA
extern "C" extern void criMvPly_AttachAsvd2();
#endif

/* 未使用引数警告回避マクロ */
#define UNUSED(arg)						{ if ((arg) == (arg)) {} }

/* vsprintf関数の呼び分け */
#if defined(XPT_TGT_PC) || defined(XPT_TGT_XBOXONE)
#define CRIWARE_VSPRINTF(a, b, c, d)	vsprintf_s(a, b, c, d)
#else
#define CRIWARE_VSPRINTF(a, b, c, d)	vsprintf(a, c, d)
#endif

/***************************************************************************
 *      データ型宣言
 *      Data Type Declarations
 ***************************************************************************/
/* FCriWareStatics共有用 */
typedef TSharedPtr<class FCriWareStatics> FCriWareStaticsPtr;
typedef TSharedRef<class FCriWareStatics> FCriWareStaticsRef;

/***************************************************************************
 *      変数宣言
 *      Prototype Variables
 ***************************************************************************/
/* ログ出力用 */
DEFINE_LOG_CATEGORY(LogCriWareRuntime);

/* コールバックログ出力用 */
DECLARE_LOG_CATEGORY_EXTERN(LogCriWare, Verbose, All);
DEFINE_LOG_CATEGORY(LogCriWare);

/* ビルド文字列 */
volatile const char criware_ue4_plugin_build_string[] =
	"\nCRIWARE UE4 Plugin Ver." CRIWARE_UE4_PLUGIN_VERSION
	" Build:" __DATE__ " " __TIME__ "\n";

/***************************************************************************
 *      クラス宣言
 *      Prototype Classes
 ***************************************************************************/
class FCriWareStatics
{
public:
	/* コンストラクタ */
	FCriWareStatics();

	/* デストラクタ */
	~FCriWareStatics();

	/* CriWareStaticsの取得 */
	static FCriWareStaticsRef GetCriWareStatics();

	/* I/Oインターフェース */
	static CriFsIoInterfacePtr DefaultIoInterface;
	static CriFsIoInterface CustomIoInterface;

	/* 初期化処理 */
	void Initialize();

	/* 終了処理 */
	void Finalize();

	/* サスペンド時の処理 */
	void HandleApplicationWillDeactivate();

	/* レジューム時の処理 */
	void HandleApplicationHasReactivated();

	/* モニタの無効化 */
	void DisableMonitor();

	/* コンテンツディレクトリ */
	FString FsContentDir;

	/* メモリ再生ボイスプール */
	CriAtomExVoicePoolHn StandardMemoryVoicePool;

	/* ストリーム再生ボイスプール */
	CriAtomExVoicePoolHn StandardStreamingVoicePool;

	/* HCA-MXメモリ再生ボイスプール */
	CriAtomExVoicePoolHn HcaMxMemoryVoicePool;

	/* HCA-MXストリーム再生ボイスプール */
	CriAtomExVoicePoolHn HcaMxStreamingVoicePool;

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* リスナ */
	CriAtomEx3dListenerHn Listener;

	/* 距離係数 */
	float AtomDistanceFactor;

	/* AtomListener */
	FAtomListener* AtomListener;

	/* ACFデータ */
	FSoftObjectPath AcfAssetReference;
	uint8* AcfData;

#if CRIWARE_USE_PCM_OUTPUT
	/* ユーザPCM出力用 */
	FAtomAudioDevice* AudioDevice;
#endif

	/* ASRラック */
	TArray<CriAtomExAsrRackId> AsrRackId;

private:
	/* 共有されるCriWareStatics */
	static FCriWareStaticsPtr CriWareStaticsSingleton;

	/* 初期化カウンタ */
	int32 InitializationCount;

	/* iniファイル名 */
	FString CriWareIni;

	/* バインダ情報 */
	int32 FsNumBinders;
	int32 FsMaxBinds;

	/* ローダ情報 */
	int32 FsNumLoaders;
	int32 FsMaxFiles;

	/* ログ出力を行うかどうか */
	bool FsOutputsLog;

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* 最大バーチャルボイス数 */
	int32 AtomMaxVirtualVoices;

	/* インゲームプレビューを行うかどうか */
	bool AtomUsesInGamePreview;

	/* ログ出力を行うかどうか */
	bool AtomOutputsLog;

	/* メモリ再生ボイス情報 */
	int32 AtomNumStandardMemoryVoices;
	int32 AtomStandardMemoryVoiceNumChannels;
	int32 AtomStandardMemoryVoiceSamplingRate;

	/* ストリーム再生ボイス情報 */
	int32 AtomNumStandardStreamingVoices;
	int32 AtomStandardStreamingVoiceNumChannels;
	int32 AtomStandardStreamingVoiceSamplingRate;

	/* ACFファイル名 */
	FString AtomAcfFileName;
	FString AtomAcfString;

	/* サウンドレンダラタイプ */
	CriAtomSoundRendererType AtomSoundRendererType;

	/* ASRラック設定 */
	TArray<FString> AtomAsrRackConfig;

	/* HCA-MX再生用情報 */
	int32 AtomHcaMxVoiceSamplingRate;
	int32 AtomNumHcaMxMemoryVoices;
	int32 AtomHcaMxMemoryVoiceNumChannels;
	int32 AtomNumHcaMxStreamingVoices;
	int32 AtomHcaMxStreamingVoiceNumChannels;

	/* WASAPI: 排他モード用の設定 */
	bool AtomIsExclusive_WASAPI;
	int32 AtomBitsPerSample_WASAPI;
	int32 AtomSamplingRate_WASAPI;
	int32 AtomNumChannels_WASAPI;

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* D-BAS ID */
	int32 DbasId;

	/* iniファイルのロード */
	void LoadIniFile();

	/* ライブラリの初期化 */
	void InitializeLibrary();

	/* ライブラリの終了 */
	void FinalizeLibrary();

#if WITH_EDITOR
// ADX2 LE v2.06 では使用しない
#if !defined(CRIWARE_UE4_LE)
	// ACFリロード処理用
	void ReloadAcf(const TArray<FString>& Args);
#endif
	IConsoleCommand *Command;
#endif
};

/***************************************************************************
 *      関数宣言
 *      Prototype Functions
 ***************************************************************************/
static void criware_error_callback_func(
	const CriChar8 *errid, CriUint32 p1, CriUint32 p2, CriUint32 *parray);
static void *criware_alloc_func(void *obj, CriUint32 size);
static void criware_free_func(void *obj, void *ptr);
static void criware_fs_logging_func(void *obj, const char* format, ...);
static void criware_atom_logging_func(void* obj, const CriChar8 *log_string);

// ADX2 LE v2.06 では使用しない
#if !defined(CRIWARE_UE4_LE)
#if WITH_EDITOR
// ACFがリロード可能かチェックするための関数
extern "C" CriAtomExAcbHn CRIAPI criAtomExAcb_GetCurrentAcb(void);
#endif
#endif

/***************************************************************************
 *      変数定義
 *      Variable Definition
 ***************************************************************************/
FCriWareStaticsPtr FCriWareStatics::CriWareStaticsSingleton;

#if defined(CRIWARE_UE4_LE)
void* UCriWareInitializer::CriWareDllHandle = nullptr;
uint32 UCriWareInitializer::NumInstances = 0;
#endif

/***************************************************************************
 *      クラス定義
 *      Class Definition
 ***************************************************************************/
UCriWareInitializer::UCriWareInitializer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	/* Localization of unreal properties metadata with LOCTEXT markups and reflection */
	CRI_LOCCLASS(GetClass());
#endif

	/* バージョン文字列の埋め込み */
	UNUSED(criware_ue4_plugin_build_string);

	IConsoleManager::Get().RegisterConsoleVariable(
		TEXT("cri.ShowSoundLocation"),
		0,
		TEXT("Show sound location by speaker icon when Play In Editor mode.\n")
		TEXT("0: Do not show. (default)")
		TEXT("1: Show sound location."),
		ECVF_Cheat);
}

/* コンテンツパスの取得 */
FString UCriWareInitializer::GetContentDir()
{
	/* コンテンツディレクトリのパスを返す */
	return FCriWareStatics::GetCriWareStatics()->FsContentDir;
}

FString UCriWareInitializer::ConvertToAbsolutePathForExternalAppForRead(const TCHAR* Filename)
{
	/* ファイルサイズの取得 */
	/* 備考）ファイルサーバ使用時に、この処理により	*/
	/* 　　　ファイルがローカルにコピーされる。		*/
	const int64 FileSize = IFileManager::Get().FileSize(Filename);
	if (FileSize < 0) {
		UE_LOG(LogCriWareRuntime, Warning, TEXT("Could not find file %s."), Filename);
	}

	/* フルパスへ変換 */
	FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(Filename);

#if defined(XPT_TGT_PS4)
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
#elif defined(XPT_TGT_IOS)
    AbsolutePath.ReplaceInline(TEXT("../"), TEXT(""));
    AbsolutePath.ReplaceInline(TEXT(".."), TEXT(""));
    AbsolutePath.ReplaceInline(FPlatformProcess::BaseDir(), TEXT(""));

    // if filehostip exists in the command line, cook on the fly read path should be used
    FString Value;

    // Cache this value as the command line doesn't change...
    static bool bHasHostIP = FParse::Value(FCommandLine::Get(), TEXT("filehostip"), Value) || FParse::Value(FCommandLine::Get(), TEXT("streaminghostip"), Value);
    if (bHasHostIP)
    {
        static FString ReadPathBase = FString([NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0]) + TEXT("/");
        AbsolutePath = ReadPathBase + AbsolutePath;
    }
    else
    {
        static FString ReadPathBase = FString([[NSBundle mainBundle] bundlePath]) + TEXT("/cookeddata/");
        AbsolutePath = ReadPathBase + AbsolutePath;
    }
#endif

	return AbsolutePath;
}

/* メモリ再生ボイスプールの取得 */
CriAtomExVoicePoolHn UCriWareInitializer::GetMemoryVoicePool()
{
	return FCriWareStatics::GetCriWareStatics()->StandardMemoryVoicePool;
}

/* ストリーム再生ボイスプールの取得 */
CriAtomExVoicePoolHn UCriWareInitializer::GetStreamingVoicePool()
{
	return FCriWareStatics::GetCriWareStatics()->StandardStreamingVoicePool;
}

FSoftObjectPath UCriWareInitializer::GetAtomConfigAssetReference()
{
	return FCriWareStatics::GetCriWareStatics()->AcfAssetReference;
}

/* リスナの取得 */
CriAtomEx3dListenerHn UCriWareInitializer::GetListener()
{
	return FCriWareStatics::GetCriWareStatics()->Listener;
}

/* 距離係数の取得 */
float UCriWareInitializer::GetDistanceFactor()
{
	return FCriWareStatics::GetCriWareStatics()->AtomDistanceFactor;
}

/* リスナの有効化／無効化 */
void UCriWareInitializer::SetListenerAutoUpdateEnabled(bool bEnabled)
{
	FAtomListener* AtomListener = FCriWareStatics::GetCriWareStatics()->AtomListener;
	if (AtomListener != nullptr) {
		AtomListener->SetAutoUpdateEnabled(bEnabled);
	}
}

/* リスナ位置の指定 */
void UCriWareInitializer::SetListenerLocation(FVector Location)
{
	FAtomListener* AtomListener = FCriWareStatics::GetCriWareStatics()->AtomListener;
	if (AtomListener != nullptr) {
		AtomListener->SetListenerLocation(Location);
	}
}

/* リスナの向きの指定 */
void UCriWareInitializer::SetListenerRotation(FRotator Rotation)
{
	FAtomListener* AtomListener = FCriWareStatics::GetCriWareStatics()->AtomListener;
	if (AtomListener != nullptr) {
		AtomListener->SetListenerRotation(Rotation);
	}
}

/* リスナ位置の取得 */
FVector UCriWareInitializer::GetListenerLocation()
{
	FAtomListener* AtomListener = FCriWareStatics::GetCriWareStatics()->AtomListener;
	if (AtomListener != nullptr) {
		return AtomListener->GetListenerLocation();
	} else {
		return FVector::ZeroVector;
	}
}

/* リスニングポイントの取得 */
FVector UCriWareInitializer::GetListeningPoint()
{
	FAtomListener* AtomListener = FCriWareStatics::GetCriWareStatics()->AtomListener;
	if (AtomListener != nullptr) {
		return AtomListener->GetListeningPoint();
	} else {
		return FVector::ZeroVector;
	}
}

/* ACFがロード済みかどうかの判定 */
bool UCriWareInitializer::IsAcfLoaded()
{
	if (FCriWareStatics::GetCriWareStatics()->AcfData != nullptr) {
		return true;
	} else {
		return false;
	}
}

void UCriWareInitializer::DisableMonitor()
{
	FCriWareStatics::GetCriWareStatics()->DisableMonitor();
}

void UCriWareInitializer::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	/* エディタを使用する場合は標準の音声出力も有効にしておく。                     */
	/* →Unreal Editor上でクリック音等を再生するために必要。                        */
	/* ※XAudio2は多重初期化を許可しているので、Windows上でエディタを使用する場合、 */
	/* 　Unreal Engine標準のAudioDeviceが別途初期化されても大丈夫なはず。           */
#elif CRIWARE_WITH_UE4_SOUND || CRIWARE_USE_PCM_OUTPUT
	/* CRIWARE_WITH_UE4_SOUNDが定義されている場合や、PCM出力を行う場合には、        */
	/* Unreal Engine標準のサウンド機能を併用する。                                  */
	/* 注意）プラットフォームのサウンドライブラリが多重初期化に対応していない場合、 */
	/* 　　　定義を有効にすることでエラー等の問題が発生する可能性あり。             */
#else
	/* 上記以外のケースについては、UE4標準の音声出力を無効化。                      */
	FCommandLine::Append(TEXT(" -nosound "));
#endif

	/* 初期化処理の実行 */
	FCriWareStatics::GetCriWareStatics()->Initialize();
}

void UCriWareInitializer::BeginDestroy()
{
	Super::BeginDestroy();

	/* 終了処理の実行 */
	FCriWareStatics::GetCriWareStatics()->Finalize();
}

/* コンストラクタ */
FCriWareStatics::FCriWareStatics()
{
	Listener = NULL;
	InitializationCount = 0;
	FsNumBinders = FS_NUM_BINDERS;
	FsMaxBinds = FS_MAX_BINDS;
	FsNumLoaders = FS_NUM_LOADERS;
	FsMaxFiles = FS_MAX_FILES;
	FsOutputsLog = FS_OUTPUT_LOG;
	AtomMaxVirtualVoices = ATOM_MAX_VIRTUAL_VOICES;
	AtomUsesInGamePreview = ATOM_USES_INGAME_PREVIEW;
	AtomOutputsLog = ATOM_OUTPUT_LOG;
	AtomNumStandardMemoryVoices = ATOM_NUM_STANDARD_MEMORY_VOICES;
	AtomStandardMemoryVoiceNumChannels = ATOM_STANDARD_MEMORY_VOICE_NUM_CHANNELS;
	AtomStandardMemoryVoiceSamplingRate = ATOM_STANDARD_MEMORY_VOICE_SAMPLING_RATE;
	AtomNumStandardStreamingVoices = ATOM_NUM_STANDARD_STREAMING_VOICES;
	AtomStandardStreamingVoiceNumChannels = ATOM_STANDARD_STREAMING_VOICE_NUM_CHANNELS;
	AtomStandardStreamingVoiceSamplingRate = ATOM_STANDARD_STREAMING_VOICE_SAMPLING_RATE;
	AtomDistanceFactor = ATOM_DISTANCE_FACTOR;
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
	DbasId = -1;
	StandardMemoryVoicePool = NULL;
	StandardStreamingVoicePool = NULL;
	HcaMxMemoryVoicePool = NULL;
	HcaMxStreamingVoicePool = NULL;
	AtomListener = nullptr;
	AcfAssetReference.Reset();
	AcfData = nullptr;

#if CRIWARE_USE_PCM_OUTPUT
	AudioDevice = nullptr;
#endif

	AsrRackId.Reset();

#if WITH_EDITOR
	Command = nullptr;
#endif

	/* サウンドレンダラタイプ */
	AtomSoundRendererType = CRIATOM_SOUND_RENDERER_NATIVE;

	/* ASRラック設定 */
	AtomAsrRackConfig.Reset();

	/* HCA-MX用の設定 */
	AtomHcaMxVoiceSamplingRate = CRIATOM_DEFAULT_OUTPUT_SAMPLING_RATE;
	AtomNumHcaMxMemoryVoices = 0;
	AtomHcaMxMemoryVoiceNumChannels = ATOM_STANDARD_MEMORY_VOICE_NUM_CHANNELS;
	AtomNumHcaMxStreamingVoices = 0;
	AtomHcaMxStreamingVoiceNumChannels = ATOM_STANDARD_STREAMING_VOICE_NUM_CHANNELS;

	/* WASAPI用の設定 */
	AtomIsExclusive_WASAPI = false;
	AtomBitsPerSample_WASAPI = 24;
	AtomSamplingRate_WASAPI = CRIATOM_DEFAULT_OUTPUT_SAMPLING_RATE;
	AtomNumChannels_WASAPI = CRIATOM_DEFAULT_OUTPUT_CHANNELS;

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
}

/* デストラクタ */
FCriWareStatics::~FCriWareStatics()
{
}

/* FCriWareStaticsの取得 */
FCriWareStaticsRef FCriWareStatics::GetCriWareStatics()
{
	/* 実体の有無をチェック */
	if (!FCriWareStatics::CriWareStaticsSingleton.IsValid()) {
		/* 実体がなければ確保 */
		FCriWareStatics::CriWareStaticsSingleton = MakeShareable(new FCriWareStatics);
	}

	/* 参照ポインタを返す */
	return FCriWareStatics::CriWareStaticsSingleton.ToSharedRef();
}

/* 初期化処理 */
void FCriWareStatics::Initialize()
{
	/* 初期化カウンタの更新 */
	InitializationCount++;
	if (InitializationCount != 1) {
		return;
	}

	/* iniファイルの読み込み */
	LoadIniFile();

	/* ライブラリの初期化 */
	InitializeLibrary();

	/* サスペンド／レジューム時の処理を登録 */
	FCoreDelegates::ApplicationWillDeactivateDelegate.AddRaw(this, &FCriWareStatics::HandleApplicationWillDeactivate);
	FCoreDelegates::ApplicationHasReactivatedDelegate.AddRaw(this, &FCriWareStatics::HandleApplicationHasReactivated);

// ADX2 LE v2.06 では使用しない
#if !defined(CRIWARE_UE4_LE)
#if WITH_EDITOR
	// ACFリロードコマンドの登録 
	Command = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("cri.ReloadAcf"),
		TEXT("Change the loaded ACF to the specified asset."),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FCriWareStatics::ReloadAcf),
		ECVF_Default
	);
#endif
#endif
}

/* iniファイルのロード */
void FCriWareStatics::LoadIniFile()
{
	/* 設定値の読み込み */
	FText ErrorMessage;
	FString EngineIniFilePath;

	/* CriWare.iniが存在する場合は設定値の読み込みを行う */
	/* [Atom]、[FileSystem]、[Mana]のカテゴリの設定値もGConfigに格納しておく                                   */
	GConfig->LoadGlobalIniFile(CriWareIni, TEXT("CriWare"));
	GConfig->LoadGlobalIniFile(EngineIniFilePath, TEXT("Engine"));
	FString SectionName_FileSystem = "FileSystem";
	FString SectionName_Atom = "Atom";
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
	FString SectionName_UClassOld = "/Script/CriWareEditor.CriWarePluginSettings";
	FString SectionName_UClass = "/Script/CriWareRuntime.CriWarePluginSettings";
	TArray<FString> Section_FileSystem;
	TArray<FString> Section_Atom;
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
	TArray<FString> Section_UClass;

	/* CriWare.iniを作成していた場合への対応 */
	/* [FileSystem]、[Atom]、[Mana]カテゴリが存在した場合は該当するパラメータをPluginパラメータとして使用 */
	/* 上記カテゴリが存在しない場合はEngine.iniに記載しているパラメータを使用する */
	bool isExistSection_FileSystem = GConfig->GetSection(*SectionName_FileSystem, Section_FileSystem, CriWareIni);
	bool isExistSection_Atom = GConfig->GetSection(*SectionName_Atom, Section_Atom, CriWareIni);
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
	bool isExistSection_UClass = GConfig->GetSection(*SectionName_UClass, Section_UClass, EngineIniFilePath);
	if (!isExistSection_UClass) {
		SectionName_UClass = SectionName_UClassOld;
		isExistSection_UClass = GConfig->GetSection(*SectionName_UClass, Section_UClass, EngineIniFilePath);
	}

	if ((isExistSection_FileSystem_or_isExistSection_Atom_or_isExistSection_Mana) && !isExistSection_UClass){
		/* 手動でiniファイルを記述していた場合はカテゴリ名を[FileSystem]、[Atom]、[Mana]としてCriWareを初期化 */
		SectionName_FileSystem = "FileSystem";
		SectionName_Atom = "Atom";
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
	}
	else if(!(isExistSection_FileSystem_or_isExistSection_Atom_or_isExistSection_Mana) && isExistSection_UClass){
		GConfig->LoadGlobalIniFile(CriWareIni, TEXT("Engine"));

		/* iniファイルの自動生成に移行済みの場合はカテゴリ名を[/Script/CriWareRuntime.CriWarePluginSettings]としてCriWareを初期化 */
		SectionName_FileSystem = SectionName_UClass;
		SectionName_Atom = SectionName_UClass;
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
	} else if ((isExistSection_FileSystem_or_isExistSection_Atom_or_isExistSection_Mana) && isExistSection_UClass){
		ErrorMessage = LOCTEXT("FailedToMarkForAddConfigFileError", "CRIWARE Plugin Error:\nBoth new and old format settings was found in configuration files.\nPlease follow this instructions to initialize CRIWARE plugin correctly.\n\nIn order to use the new format:\n1. Delete old configuration files (*CriWare.ini).\n\nIn order to use the old format:\n1. Open new configuration file (*Engine.ini) by text editor.\n2. Remove section [/Script/CriWareRuntime.CriWarePluginSettings].\n\n[Appendix]\nConfiguration files will be in the following directories:\n- Engine/Config/\n- Engine/Platform/\n- [ProjectDir]/Config/\n- [ProjectDir]/Config/[Platform]/");
	}

	// show errors, if any
	if (!ErrorMessage.IsEmpty()) {
		const CriChar8 *errmsg;

		/* エラー文字列の表示 */
		/* Display an error message */
		errmsg = "CRIWARE Plugin Error:\nBoth new and old format settings was found in configuration files.\nPlease follow this instructions to initialize CRIWARE plugin correctly.\n\nIn order to use the new format:\n1. Delete old configuration files (*CriWare.ini).\n\nIn order to use the old format:\n1. Open new configuration file (*Engine.ini) by text editor.\n2. Remove section [/Script/CriWareRuntime.CriWarePluginSettings].\n\n[Appendix]\nConfiguration files will be in the following directories:\n- Engine/Config/\n- Engine/Platform/\n- [ProjectDir]/Config/\n- [ProjectDir]/Config/[Platform]/";

		/* ログ出力 */
		UE_LOG(LogCriWareRuntime, Error, TEXT("%s"), UTF8_TO_TCHAR(errmsg));
		FMessageDialog::Open(EAppMsgType::Ok, ErrorMessage);
	}

	/* === FileSystem関連パラメータ === */

	/* コンテンツディレクトリパス指定の取得 */
	GConfig->GetString(*SectionName_FileSystem, TEXT("ContentDir"), FsContentDir, CriWareIni);

	/* フルパスかどうかチェック */
	if ((FsContentDir.StartsWith(TEXT("/")) == false)
		&& (FsContentDir.Contains(TEXT(":")) == false)) {
		/* フルパスでない場合はゲームディレクトリパスを付加 */
		FsContentDir = FPaths::ProjectContentDir() + FsContentDir;
	}

	/* パスの終端に「/」を付加 */
	if (FsContentDir.EndsWith(TEXT("/")) == false) {
		FsContentDir += "/";
	}

	/* バインダ情報の取得 */
	GConfig->GetInt(*SectionName_FileSystem, TEXT("NumBinders"), FsNumBinders, CriWareIni);
	GConfig->GetInt(*SectionName_FileSystem, TEXT("MaxBinds"), FsMaxBinds, CriWareIni);

	/* ローダ情報の取得 */
	GConfig->GetInt(*SectionName_FileSystem, TEXT("NumLoaders"), FsNumLoaders, CriWareIni);
	GConfig->GetInt(*SectionName_FileSystem, TEXT("MaxFiles"), FsMaxFiles, CriWareIni);

	/* ログ出力を行うかどうか */
	if (!GConfig->GetBool(*SectionName_FileSystem, TEXT("OutputsLog"), FsOutputsLog, CriWareIni)){
		GConfig->GetBool(*SectionName_FileSystem, TEXT("OutputsLogFileSystem"), FsOutputsLog, CriWareIni);
	}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* === Atom関連パラメータ === */

	/* 最大バーチャルボイス数の取得 */
	GConfig->GetInt(*SectionName_Atom, TEXT("MaxVirtualVoices"), AtomMaxVirtualVoices, CriWareIni);

	/* インゲームプレビューを使用するかどうか */
	GConfig->GetBool(*SectionName_Atom, TEXT("UsesInGamePreview"), AtomUsesInGamePreview, CriWareIni);

	/* ログ出力を行うかどうか */
	if (!GConfig->GetBool(*SectionName_Atom, TEXT("OutputsLog"), AtomOutputsLog, CriWareIni)){
		GConfig->GetBool(*SectionName_Atom, TEXT("OutputsLogAtom"), AtomOutputsLog, CriWareIni);
	}

	/* メモリ再生用ボイス情報の取得 */
	GConfig->GetInt(*SectionName_Atom, TEXT("NumStandardMemoryVoices"), AtomNumStandardMemoryVoices, CriWareIni);
	GConfig->GetInt(*SectionName_Atom, TEXT("StandardMemoryVoiceSamplingRate"), AtomStandardMemoryVoiceSamplingRate, CriWareIni);
	GConfig->GetInt(*SectionName_Atom, TEXT("StandardMemoryVoiceNumChannels"), AtomStandardMemoryVoiceNumChannels, CriWareIni);

	/* プラットフォームがサポートする最大ch数でクリップする */
	if (AtomStandardMemoryVoiceNumChannels > CRIWARE_UE4_MAX_CHANNELS) {
		AtomStandardMemoryVoiceNumChannels = CRIWARE_UE4_MAX_CHANNELS;
	}

	/* ストリーム再生用ボイス情報の取得 */
	GConfig->GetInt(*SectionName_Atom, TEXT("NumStandardStreamingVoices"), AtomNumStandardStreamingVoices, CriWareIni);
	GConfig->GetInt(*SectionName_Atom, TEXT("StandardStreamingVoiceSamplingRate"), AtomStandardStreamingVoiceSamplingRate, CriWareIni);
	GConfig->GetInt(*SectionName_Atom, TEXT("StandardStreamingVoiceNumChannels"), AtomStandardStreamingVoiceNumChannels, CriWareIni);

	/* プラットフォームがサポートする最大ch数でクリップする */
	if (AtomStandardStreamingVoiceNumChannels > CRIWARE_UE4_MAX_CHANNELS) {
		AtomStandardStreamingVoiceNumChannels = CRIWARE_UE4_MAX_CHANNELS;
	}

	/* サウンドレンダラタイプの取得 */
	int32 SoundRendererType;
	GConfig->GetInt(*SectionName_Atom, TEXT("SoundRendererType"), SoundRendererType, CriWareIni);
	AtomSoundRendererType = static_cast<CriAtomSoundRendererType>(SoundRendererType);
// ADX2 LE v2.06 では使用しない
#if !defined(CRIWARE_UE4_LE)
	/* 最終出力には必ずハードウェアボイスをアサインする */
	switch (AtomSoundRendererType) {
		case CRIATOM_SOUND_RENDERER_HW1:
		case CRIATOM_SOUND_RENDERER_HW2:
		case CRIATOM_SOUND_RENDERER_HW3:
		case CRIATOM_SOUND_RENDERER_HW4:
		break;
		
		default:
		AtomSoundRendererType = CRIATOM_SOUND_RENDERER_NATIVE;
		break;
	}
#else
	AtomSoundRendererType = CRIATOM_SOUND_RENDERER_NATIVE;
#endif
	/* ASRラック設定の取得 */
	GConfig->GetArray(*SectionName_Atom, TEXT("AsrRackConfig"), AtomAsrRackConfig, CriWareIni);

	/* HCA-MXサンプリングレートの取得 */
	GConfig->GetInt(*SectionName_Atom, TEXT("HcaMxVoiceSamplingRate"), AtomHcaMxVoiceSamplingRate, CriWareIni);

	/* HCA-MXメモリ再生用ボイス情報の取得 */
	GConfig->GetInt(*SectionName_Atom, TEXT("NumHcaMxMemoryVoices"), AtomNumHcaMxMemoryVoices, CriWareIni);
	GConfig->GetInt(*SectionName_Atom, TEXT("HcaMxMemoryVoiceNumChannels"), AtomHcaMxMemoryVoiceNumChannels, CriWareIni);

	/* プラットフォームがサポートする最大ch数でクリップする */
	if (AtomHcaMxMemoryVoiceNumChannels > CRIWARE_UE4_MAX_CHANNELS) {
		AtomHcaMxMemoryVoiceNumChannels = CRIWARE_UE4_MAX_CHANNELS;
	}

	/* HCA-MXストリーム再生用ボイス情報の取得 */
	GConfig->GetInt(*SectionName_Atom, TEXT("NumHcaMxStreamingVoices"), AtomNumHcaMxStreamingVoices, CriWareIni);
	GConfig->GetInt(*SectionName_Atom, TEXT("HcaMxStreamingVoiceNumChannels"), AtomHcaMxStreamingVoiceNumChannels, CriWareIni);

	/* プラットフォームがサポートする最大ch数でクリップする */
	if (AtomHcaMxStreamingVoiceNumChannels > CRIWARE_UE4_MAX_CHANNELS) {
		AtomHcaMxStreamingVoiceNumChannels = CRIWARE_UE4_MAX_CHANNELS;
	}

	/* ACFファイル名の取得 */
	/* 備考）互換性維持のための処理。 */
	GConfig->GetString(*SectionName_Atom, TEXT("AcfFileName"), AtomAcfFileName, CriWareIni);

	/* ACFアセット名の取得 */
	GConfig->GetString(*SectionName_Atom, TEXT("AtomConfig"), AtomAcfString, CriWareIni);

	/* 距離係数の取得 */
	GConfig->GetFloat(*SectionName_Atom, TEXT("DistanceFactor"), AtomDistanceFactor, CriWareIni);
	if (AtomDistanceFactor <= 0.0f) {
		UE_LOG(LogCriWareRuntime, Error, TEXT("Invalid distance factor."));
		AtomDistanceFactor = 1.0f;
	}

	/* WASAPI: 排他モード設定の取得 */
	GConfig->GetBool(*SectionName_FileSystem, TEXT("WASAPI_IsExclusive"), AtomIsExclusive_WASAPI, CriWareIni);
	GConfig->GetInt(*SectionName_FileSystem, TEXT("WASAPI_BitsPerSample"), AtomBitsPerSample_WASAPI, CriWareIni);
	GConfig->GetInt(*SectionName_FileSystem, TEXT("WASAPI_SamplingRate"), AtomSamplingRate_WASAPI, CriWareIni);
	GConfig->GetInt(*SectionName_FileSystem, TEXT("WASAPI_NumChannels"), AtomNumChannels_WASAPI, CriWareIni);
	if (AtomIsExclusive_WASAPI != false) {
		/* 備考）現状、PCMフォーマットは16bit or 24bitしか使用できない。 */
		AtomBitsPerSample_WASAPI = ((AtomBitsPerSample_WASAPI <= 16) ? 16 : 24);
	}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* === パラメータの補正（初期化に失敗するケースを排除） === */

	/* バーチャルボイス数の補正 */
	int32 RequiredVoices = (AtomNumStandardStreamingVoices + AtomNumStandardMemoryVoices
		+ AtomNumHcaMxStreamingVoices + AtomNumHcaMxMemoryVoices) * 2;
	if (AtomMaxVirtualVoices < RequiredVoices) {
		AtomMaxVirtualVoices = RequiredVoices;
	}

	/* ファイル数の補正 */
	int32 RequiredFiles = AtomNumStandardStreamingVoices + AtomNumHcaMxStreamingVoices + 1;
	if (FsNumBinders < RequiredFiles) {
		FsNumBinders = RequiredFiles;
	}
	if (FsMaxBinds < RequiredFiles) {
		FsMaxBinds = RequiredFiles;
	}
	if (FsNumLoaders < RequiredFiles) {
		FsNumLoaders = RequiredFiles;
	}
	if (FsMaxFiles < RequiredFiles) {
		FsMaxFiles = RequiredFiles;
	}
}

/* ライブラリの初期化 */
void FCriWareStatics::InitializeLibrary()
{
#if defined(CRIWARE_UE4_LE)

#if PLATFORM_WINDOWS
#if PLATFORM_64BITS
	FString WinDir = TEXT("x64");
	FString DLLName = TEXT("cri_ware_pcx64_le.dll");
#else
	FString WinDir = TEXT("x86");
	FString DLLName = TEXT("cri_ware_pcx86_le.dll");
#endif

	if (UCriWareInitializer::NumInstances == 0) {
		if (!UCriWareInitializer::CriWareDllHandle)
		{
			FString PluginPath = FString::Printf(TEXT("Plugins/Runtime/CriWare/CriWare"));
			FString CorePath = FPaths::Combine(*(FPaths::ProjectDir()), PluginPath);
			if (!FPaths::DirectoryExists(CorePath)) {
				CorePath = FPaths::Combine(*(FPaths::EngineDir()), PluginPath);
			}
			FString RootDllPath = CorePath / TEXT("Source/ThirdParty/CriWare/cri/pc/libs") / WinDir;
			FPlatformProcess::PushDllDirectory(*RootDllPath);
			UCriWareInitializer::CriWareDllHandle = FPlatformProcess::GetDllHandle(*(RootDllPath / DLLName));
			FPlatformProcess::PopDllDirectory(*RootDllPath);
			if (!UCriWareInitializer::CriWareDllHandle) {
				UE_LOG(LogCriWareRuntime, Error, TEXT("Failed to load CriWare Plugin DLL."));
				return;
			}
		}
	}
	UCriWareInitializer::NumInstances++;

#endif

#endif

	/* エラーコールバック関数の登録 */
	criErr_SetCallback(criware_error_callback_func);

	/* アロケータの登録 */
	criFs_SetUserAllocator(criware_alloc_func, criware_free_func, NULL);
	criAtomEx_SetUserAllocator(criware_alloc_func, criware_free_func, NULL);

	/* ライブラリの初期化 */
	CriAtomExConfig_UE4 atom_config;
	criAtomEx_SetDefaultConfig_UE4(&atom_config);
	atom_config.atom_ex.max_virtual_voices = AtomMaxVirtualVoices;
	atom_config.atom_ex.max_parameter_blocks = AtomMaxVirtualVoices * 16;
	atom_config.atom_ex.max_voice_limit_groups = AtomMaxVirtualVoices;
	atom_config.atom_ex.max_categories = AtomMaxVirtualVoices;
	atom_config.atom_ex.max_sequences = AtomMaxVirtualVoices;
	atom_config.atom_ex.max_tracks = AtomMaxVirtualVoices * 2;
	atom_config.atom_ex.max_track_items = AtomMaxVirtualVoices * 2;
	atom_config.atom_ex.max_voice_limit_groups = CRIWARE_UE4_MAX_VOICE_LIMIT_GROUPS;
	atom_config.atom_ex.max_categories = CRIWARE_UE4_MAX_CATEGORIES;
	atom_config.atom_ex.categories_per_playback = CRIATOMEXCATEGORY_MAX_CATEGORIES_PER_PLAYBACK;
	atom_config.asr.output_channels = CRIWARE_UE4_MAX_CHANNELS;
	atom_config.asr.num_buses = CRIATOMEXASR_MAX_BASES;
	atom_config.asr.sound_renderer_type = AtomSoundRendererType;
	atom_config.hca_mx.output_channels = CRIWARE_UE4_MAX_CHANNELS;
	atom_config.hca_mx.max_sampling_rate = AtomHcaMxVoiceSamplingRate;
	atom_config.hca_mx.max_voices = AtomNumHcaMxMemoryVoices + AtomNumHcaMxStreamingVoices;
	atom_config.hca_mx.num_mixers = ((atom_config.hca_mx.max_voices > 0) ? 1 : 0);
	CriFsConfig fs_config;
	criFs_SetDefaultConfig(&fs_config);
	fs_config.num_binders = FsNumBinders;
	fs_config.max_binds = FsMaxBinds;
	fs_config.num_loaders = FsNumLoaders;
	fs_config.max_files = FsMaxFiles;
	atom_config.atom_ex.fs_config = &fs_config;

#if defined(XPT_TGT_PC) && !CRIWARE_USE_XAUDIO2
	if (AtomIsExclusive_WASAPI != false) {
		/* WASAPIの出力フォーマットを指定 */
		WAVEFORMATEXTENSIBLE ExclusiveFormat;
		memset(&ExclusiveFormat, 0, sizeof(ExclusiveFormat));
		WAVEFORMATEX *MixerFormat = (WAVEFORMATEX *)&ExclusiveFormat;
		MixerFormat->wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		MixerFormat->nChannels = (uint16)AtomNumChannels_WASAPI;
		MixerFormat->nSamplesPerSec = (uint32)AtomSamplingRate_WASAPI;
		MixerFormat->wBitsPerSample = ((AtomBitsPerSample_WASAPI > 16) ? 32 : 16);
		MixerFormat->nBlockAlign = MixerFormat->wBitsPerSample / 8 * MixerFormat->nChannels;
		MixerFormat->nAvgBytesPerSec = MixerFormat->nSamplesPerSec * MixerFormat->nBlockAlign;
		MixerFormat->cbSize = 22;
		ExclusiveFormat.Samples.wValidBitsPerSample = AtomBitsPerSample_WASAPI;
		ExclusiveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		criAtom_SetAudioClientFormat_WASAPI(MixerFormat);

		/* 排他モードを有効化 */
		criAtom_SetAudioClientShareMode_WASAPI(AUDCLNT_SHAREMODE_EXCLUSIVE);
	}
#endif

	/* ライブラリの初期化 */
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#else	/* </cri_delete_if_LE> */
	criAtomEx_Initialize_UE4(&atom_config, NULL, 0);
#endif

	/* ASRラックの作成 */
	for (const FString& ItemString : AtomAsrRackConfig) {
		/* ラック設定の初期化 */
		CriAtomExAsrRackConfig AsrRackConfig;
		criAtomExAsrRack_SetDefaultConfig(&AsrRackConfig);
		AsrRackConfig.output_channels = CRIWARE_UE4_MAX_CHANNELS;
		AsrRackConfig.num_buses = CRIATOMEXASR_MAX_BASES;
		AsrRackConfig.sound_renderer_type = CRIATOM_SOUND_RENDERER_ASR;

#if defined(XPT_TGT_PS4)
		/* PS4用の追加設定 */
		CriAtomExAsrRackConfig_PS4 AsrRackConfigPs4;
		criAtomExAsrRack_SetDefaultConfig_PS4(&AsrRackConfigPs4);
#endif

		/* サウンドレンダラタイプ指定の有無をチェック */
		int32 Pos = ItemString.Find(TEXT("SoundRendererType="));
		if (Pos >= 0) {
			FString Value = ItemString.Mid(Pos + 18);
			AsrRackConfig.sound_renderer_type = static_cast<CriAtomSoundRendererType>(FCString::Atoi(*Value));
		}

		/* パッドスピーカー対応のための特別処理 */
		if (AsrRackConfig.sound_renderer_type == 255) {
#if defined(XPT_TGT_PS4)
			/* ユーザーID値の取得 */
			SceUserServiceUserId UserId;
			sceUserServiceGetInitialUser(&UserId);

			/* パッドスピーカー出力用に情報を追加 */
			AsrRackConfigPs4.port_user_id = UserId;
			AsrRackConfigPs4.port_type = SCE_AUDIO_OUT_PORT_TYPE_PADSPK;
			AsrRackConfig.output_channels = 1;
			AsrRackConfig.context = &AsrRackConfigPs4;
			AsrRackConfig.sound_renderer_type = CRIATOM_SOUND_RENDERER_NATIVE;
#else
			/* 他機種ではマスターにミックス */
			AsrRackConfig.sound_renderer_type = CRIATOM_SOUND_RENDERER_ASR;
#endif
		}

// ADX2 LE v2.06 では使用しない
#if !defined(CRIWARE_UE4_LE)
		/* ANYの場合はASRを割り当て */
		if (AsrRackConfig.sound_renderer_type == CRIATOM_SOUND_RENDERER_ANY) {
			AsrRackConfig.sound_renderer_type = CRIATOM_SOUND_RENDERER_ASR;
		}
#endif

		/* ASRラックの作成 */
		CriAtomExAsrRackId NewRackId = criAtomExAsrRack_Create(&AsrRackConfig, NULL, 0);
		AsrRackId.Add(NewRackId);
	}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

#if CRIWARE_USE_PCM_OUTPUT
	/* オーディオ出力の作成 */
	AudioDevice = new FAtomAudioDevice();
#endif

	/* I/Oインターフェースの差し替え */
	criFs_SetSelectIoCallback(CriWareFileIo::SelectIo);

#if CRIWARE_USE_ATOM_MONITOR
	/* ファイルアクセスログ出力の開始 */
	if (FsOutputsLog != false) {
		criFs_SetUserLogOutputFunction(criware_fs_logging_func, NULL);
		criFs_AttachLogOutput(CRIFS_LOGOUTPUT_MODE_DEFAULT, NULL);
	}

	/* インゲームプレビューの開始 */
	if ((AtomUsesInGamePreview != false) || (AtomOutputsLog != false)) {
		criAtomExMonitor_Initialize(NULL, NULL, 0);

		/* プロファイラ向けにラウドネスメーターをアタッチ */
		criAtomMeter_AttachLoudnessMeter(NULL, NULL, 0);
	}

	/* ログ出力の開始 */
	if (AtomOutputsLog != false) {
		criAtomExMonitor_SetLogCallback(&criware_atom_logging_func, NULL);
	}
#endif

	/* D-BASの作成 */
	/* 備考）現状D-BASの破棄は明示的に行う必要があるため、IDを保持しておく必要がある。 */
	if ((AtomNumStandardStreamingVoices + AtomNumHcaMxStreamingVoices) > 0) {
		CriAtomExDbasConfig dbas_config;
		criAtomExDbas_SetDefaultConfig(&dbas_config);
		dbas_config.max_bps = criAtomEx_CalculateAdxBitrate(
			AtomStandardStreamingVoiceNumChannels, AtomStandardStreamingVoiceSamplingRate) * AtomNumStandardStreamingVoices;
		dbas_config.max_bps += criAtomEx_CalculateAdxBitrate(
			AtomHcaMxStreamingVoiceNumChannels, AtomHcaMxVoiceSamplingRate) * AtomNumHcaMxStreamingVoices;
		dbas_config.max_streams = AtomNumStandardStreamingVoices + AtomNumHcaMxStreamingVoices;
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
		DbasId = criAtomExDbas_Create(&dbas_config, NULL, 0);
	}

	/* メモリ再生用ボイスの確保 */
	if (AtomNumStandardMemoryVoices > 0) {
		CriAtomExStandardVoicePoolConfig pool_config;
		criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&pool_config);
		pool_config.num_voices = AtomNumStandardMemoryVoices;
		pool_config.player_config.max_channels = AtomStandardMemoryVoiceNumChannels;
		pool_config.player_config.max_sampling_rate = AtomStandardMemoryVoiceSamplingRate;
		StandardMemoryVoicePool = criAtomExVoicePool_AllocateStandardVoicePool(&pool_config, NULL, 0);
	}

	/* ストリーム再生用ボイスの確保 */
	if (AtomNumStandardStreamingVoices > 0) {
		CriAtomExStandardVoicePoolConfig pool_config;
		criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&pool_config);
		pool_config.num_voices = AtomNumStandardStreamingVoices;
		pool_config.player_config.max_channels = AtomStandardStreamingVoiceNumChannels;
		pool_config.player_config.max_sampling_rate = AtomStandardStreamingVoiceSamplingRate;
		pool_config.player_config.streaming_flag = CRI_TRUE;
		StandardStreamingVoicePool = criAtomExVoicePool_AllocateStandardVoicePool(&pool_config, NULL, 0);
	}

	/* HCA-MXメモリ再生用ボイスの確保 */
	if (AtomNumHcaMxMemoryVoices > 0) {
		CriAtomExHcaMxVoicePoolConfig pool_config;
		criAtomExVoicePool_SetDefaultConfigForHcaMxVoicePool(&pool_config);
		pool_config.num_voices = AtomNumHcaMxMemoryVoices;
		pool_config.player_config.max_channels = AtomHcaMxMemoryVoiceNumChannels;
		pool_config.player_config.max_sampling_rate = AtomHcaMxVoiceSamplingRate;
		HcaMxMemoryVoicePool = criAtomExVoicePool_AllocateHcaMxVoicePool(&pool_config, NULL, 0);
	}

	/* HCA-MXストリーム再生用ボイスの確保 */
	if (AtomNumHcaMxStreamingVoices > 0) {
		CriAtomExHcaMxVoicePoolConfig pool_config;
		criAtomExVoicePool_SetDefaultConfigForHcaMxVoicePool(&pool_config);
		pool_config.num_voices = AtomNumHcaMxStreamingVoices;
		pool_config.player_config.max_channels = AtomHcaMxStreamingVoiceNumChannels;
		pool_config.player_config.max_sampling_rate = AtomHcaMxVoiceSamplingRate;
		pool_config.player_config.streaming_flag = CRI_TRUE;
		HcaMxStreamingVoicePool = criAtomExVoicePool_AllocateHcaMxVoicePool(&pool_config, NULL, 0);
	}

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	if (AtomAcfString.Len() > 0) {
		/* アセットのロード */
		AcfAssetReference = FSoftObjectPath(AtomAcfString);
		USoundAtomConfig* AcfObject = Cast<USoundAtomConfig>(AcfAssetReference.TryLoad());
		if (AcfObject != nullptr) {
			/* ACFデータの取得 */
			int32 AcfDataSize = AcfObject->RawData.GetBulkDataSize();
			AcfObject->RawData.GetCopy((void**)&AcfData, false);
			if (AcfData == nullptr) {
				UE_LOG(LogCriWareRuntime, Error, TEXT("Failed to load '%s'."), *AtomAcfFileName);
			} else {
				/* ACFデータのロード */
				criAtomEx_RegisterAcfData(AcfData, AcfDataSize, NULL, 0);
			}
		}
	} else if (AtomAcfFileName.Len() > 0) {
		/* 拡張子の取得 */
		FString FileExt = FPaths::GetExtension(AtomAcfFileName);

		/* 拡張子の有無でACFファイル or ACFアセットの判別を行う。 */
		/* 備考）拡張子なし or *.uasset時はアセットと判定。 */
		if ((FileExt.Len() > 0) && (FileExt != "uasset")) {
			/* ACFファイルパスの作成 */
			/* 備考）ファイル名指定時はファイルの格納先はFsContentDir。 */
			FString AcfFilePath = FsContentDir + AtomAcfFileName;

			/* ACFファイルのロード */
			criAtomEx_RegisterAcfFile(NULL, TCHAR_TO_UTF8(*AcfFilePath), NULL, 0);
		} else {
			/* ACFアセットのロード */
			/* 備考）アセット名指定時はFsContentDirを考慮しない。 */
			USoundAtomConfig* AcfObject = USoundAtomConfig::LoadAcfAsset(AtomAcfFileName);
			if (AcfObject != nullptr) {
				/* ACFデータの取得 */
				int32 AcfDataSize = AcfObject->RawData.GetBulkDataSize();
				AcfObject->RawData.GetCopy((void**)&AcfData, false);
				if (AcfData == nullptr) {
					UE_LOG(LogCriWareRuntime, Error, TEXT("Failed to load '%s'."), *AtomAcfFileName);
				} else {
					/* ACFデータのロード */
					criAtomEx_RegisterAcfData(AcfData, AcfDataSize, NULL, 0);
				}
			}
		}
	}

	/* リスナの作成 */
	Listener = criAtomEx3dListener_Create(NULL, NULL, 0);

	/* 距離係数の設定 */
	criAtomEx3dListener_SetDistanceFactor(Listener, AtomDistanceFactor);

	/* AtomListenerの作成 */
	AtomListener = new FAtomListener(Listener, AtomDistanceFactor);

#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */
}

/* 終了処理 */
void FCriWareStatics::Finalize()
{
	/* 初期化カウンタの更新 */
	InitializationCount--;
	if (InitializationCount != 0) {
		return;
	}

#if WITH_EDITOR
	/* ACFリロードコマンドの登録解除 */
	if (Command != nullptr) {
		IConsoleManager::Get().UnregisterConsoleObject(Command);
		Command = nullptr;
	}
#endif

	/* サスペンド／レジューム時の処理を登録解除 */
	FCoreDelegates::ApplicationWillDeactivateDelegate.RemoveAll(this);
	FCoreDelegates::ApplicationHasReactivatedDelegate.RemoveAll(this);

	/* ライブラリの終了 */
	FinalizeLibrary();
}

/* ライブラリの終了 */
void FCriWareStatics::FinalizeLibrary()
{
#if !defined(CRIWARE_UE4_LE)	/* <cri_delete_if_LE> */
#endif	/* </cri_delete_if_LE> */

	/* 再生の停止を要求 */
	criAtomExPlayer_StopAllPlayersWithoutReleaseTime();

	/* ボイスプールの破棄 */
	criAtomExVoicePool_FreeAll();

	/* AtomListenerの破棄 */
	if (AtomListener != nullptr) {
		delete AtomListener;
		AtomListener = nullptr;
	}

	/* リスナの破棄 */
	if (Listener != NULL) {
		criAtomEx3dListener_Destroy(Listener);
		Listener = NULL;
	}

	/* D-BASの破棄 */
	if (DbasId != -1) {
		criAtomExDbas_Destroy(DbasId);
		DbasId = -1;
	}

	/* ACBのリリース */
	criAtomExAcb_ReleaseAll();

#if CRIWARE_USE_ATOM_MONITOR
	/* ログ出力の停止 */
	if (AtomOutputsLog != false) {
		criAtomExMonitor_SetLogCallback(NULL, NULL);
	}

	/* インゲームプレビューの終了 */
	if ((AtomUsesInGamePreview != false) || (AtomOutputsLog != false)) {
		criAtomMeter_DetachLoudnessMeter();
		criAtomExMonitor_Finalize();
	}

	/* ファイルアクセスログ出力の停止 */
	if (FsOutputsLog != false) {
		criFs_DetachLogOutput();
	}
#endif

	/* ASRラックの破棄 */
	for (const CriAtomExAsrRackId& ItemRackId : AsrRackId) {
		criAtomExAsrRack_Destroy(ItemRackId);
	}
	AsrRackId.Reset();

	/* ACFデータの解放 */
	if (AcfData != nullptr) {
		criAtomEx_UnregisterAcf();
		FMemory::Free(AcfData);
		AcfData = nullptr;
	}

	/* ライブラリの終了 */
	criAtomEx_Finalize_UE4();

#if CRIWARE_USE_PCM_OUTPUT
	/* デバイスのクローズ */
	if (AudioDevice != nullptr) {
		delete AudioDevice;
		AudioDevice = nullptr;
	}
#endif

	/* アロケータの登録解除 */
	criAtomEx_SetUserAllocator(NULL, NULL, NULL);
	criFs_SetUserAllocator(NULL, NULL, NULL);

	/* エラーコールバックの登録解除 */
	criErr_SetCallback(NULL);

#if defined(CRIWARE_UE4_LE)
	if (UCriWareInitializer::NumInstances == 0)
	{
		return;
	}

	UCriWareInitializer::NumInstances--;

	if (UCriWareInitializer::NumInstances == 0 && UCriWareInitializer::CriWareDllHandle)
	{
		FPlatformProcess::FreeDllHandle(UCriWareInitializer::CriWareDllHandle);
		UCriWareInitializer::CriWareDllHandle = nullptr;
	}
#endif
}

/* サスペンド時の処理 */
void FCriWareStatics::HandleApplicationWillDeactivate()
{
#if defined(XPT_TGT_IOS)
	criAtomEx_StopSound_IOS();
#elif defined(XPT_TGT_ANDROID)
	criAtomEx_StopSound_ANDROID();
#endif
}

/* レジューム時の処理 */
void FCriWareStatics::HandleApplicationHasReactivated()
{
#if defined(XPT_TGT_IOS)
	criAtomEx_StartSound_IOS();
#elif defined(XPT_TGT_ANDROID)
	criAtomEx_StartSound_ANDROID();
#endif
}

/* モニタの無効化 */
void FCriWareStatics::DisableMonitor()
{
#if CRIWARE_USE_ATOM_MONITOR
	/* モニタライブラリの終了 */
	if ((AtomUsesInGamePreview != false) || (AtomOutputsLog != false)) {
		criAtomMeter_DetachLoudnessMeter();
		criAtomExMonitor_Finalize();
	}
#endif

	AtomUsesInGamePreview = false;
	AtomOutputsLog = false;
}

// ADX2 LE v2.06 では使用しない
#if !defined(CRIWARE_UE4_LE)
#if WITH_EDITOR
// ACFのリロード
void FCriWareStatics::ReloadAcf(const TArray<FString>& Args)
{
	// ACBロード中かどうかチェック
	CriAtomExAcbHn AcbHn = criAtomExAcb_GetCurrentAcb();
	if (AcbHn != nullptr) {
		UE_LOG(LogCriWareRuntime, Error, TEXT("Cannot reload Atom Config while previewing."));
		return;
	}

	// ACFデータの解放 
	if (AcfData != nullptr) {
		UE_LOG(LogCriWareRuntime, Log, TEXT("Atom Config is unloaded."));
		criAtomEx_UnregisterAcf();
		FMemory::Free(AcfData);
		AcfData = nullptr;
	}

	// ACFアセットの参照をクリア
	AcfAssetReference.Reset();

	// 引数のチェック
	if (Args.Num() <= 0) {
		// 引数未指定時はリロードしない
		return;
	}

	// ACFアセットのロード
	// 備考）アセット名指定時はFsContentDirを考慮しない。
	USoundAtomConfig* AcfObject = USoundAtomConfig::LoadAcfAsset(Args[0]);
	if (AcfObject == nullptr) {
		// 備考）エラーはUSoundAtomConfig::LoadAcfAsset関数内で表示済み。
		return;
	}

	// ACFデータの取得
	int32 AcfDataSize = AcfObject->RawData.GetBulkDataSize();
	AcfObject->RawData.GetCopy((void**)&AcfData, false);
	if (AcfData == nullptr) {
		UE_LOG(LogCriWareRuntime, Error, TEXT("Failed to load '%s'."), *AcfObject->GetName());
		return;
	}

	// ACFデータのロード
	criAtomEx_RegisterAcfData(AcfData, AcfDataSize, NULL, 0);

	// ACFアセットの参照の更新
	AcfAssetReference = FSoftObjectPath(AcfObject);

	// 結果の表示
	UE_LOG(LogCriWareRuntime, Log, TEXT("Atom Config '%s' is loaded."), *AcfObject->GetName());
}
#endif
#endif

/***************************************************************************
 *      関数定義
 *      Function Definition
 ***************************************************************************/
/* エラーコールバック関数 */
static void criware_error_callback_func(
	const CriChar8 *errid, CriUint32 p1, CriUint32 p2, CriUint32 *parray)
{
	const CriChar8 *errmsg;

	UNUSED(parray);

	/* エラー文字列の表示 */
	/* Display an error message */
	errmsg = criErr_ConvertIdToMessage(errid, p1, p2);

	/* ログ出力 */
	switch (errmsg[0]) {
		case 'W':
		UE_LOG(LogCriWare, Warning, TEXT("%s"), UTF8_TO_TCHAR(errmsg));
		break;

		default:
		UE_LOG(LogCriWare, Error, TEXT("%s"), UTF8_TO_TCHAR(errmsg));
		break;
	}
}

static void *criware_alloc_func(void *obj, CriUint32 size)
{
	void *ptr;

	UNUSED(obj);

	/* メモリの確保 */
	ptr = FMemory::Malloc(size);

	return (ptr);
}

static void criware_free_func(void *obj, void *ptr)
{
	UNUSED(obj);

	/* メモリの解放 */
	FMemory::Free(ptr);

	return;
}

/* File Systemログ出力 */
static void criware_fs_logging_func(void *obj, const char* format, ...)
{
	CriChar8 str[1024];
	va_list	args;

	va_start(args, format);
	CRIWARE_VSPRINTF(str, sizeof(str), format, args);
	va_end(args);

	UE_LOG(LogCriWare, Log, TEXT("%s"), UTF8_TO_TCHAR(str));
}

/* Atomログ出力 */
static void criware_atom_logging_func(void* obj, const CriChar8 *log_string)
{
	UNUSED(obj);

	/* ログの表示 */
	/* Display log */
	UE_LOG(LogCriWare, Log, TEXT("%s"), UTF8_TO_TCHAR(log_string));

	return;
}

#undef LOCTEXT_NAMESPACE

/* --- end of file --- */
