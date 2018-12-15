/****************************************************************************
 *
 * CRI ADX2 SDK
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 * Title   : CRI ADX2 sample program
 * File    : AtomUtilIOS.mm
 *
 ****************************************************************************/
/*
 * 本ソースでは、iOS固有の処理として以下のことを行っています。
 *   - 音声割り込み処理
 *   - メディアサーバのリセットへの対処
 *
 * 本サンプルはiOS6以降のみ対応しています。
 * iOS6より前のiOSに対応する場合は、AVAudioSesssionではなく、
 * AudioSessionのAPIを使用して同等の処理を実装する必要があります。
 */

#import <AVFoundation/AVAudioSession.h>
#import <AudioToolbox/AudioServices.h>

#import <cri_adx2le.h>


namespace {
    BOOL audioSessionSetActive(BOOL sw)
    {
        AVAudioSession* audio_session = [AVAudioSession sharedInstance];
        /* setActive try loop */
        for (int try_count = 0; try_count < 20; ++try_count) {
            NSError* ns_error = nil;
            if ([audio_session setActive: sw error: &ns_error]) {
                return YES;
            }
            [NSThread sleepForTimeInterval:0.2];
        }
        return NO;
    }
    
    BOOL audioSessionSetup()
    {
        BOOL result;
        
        result = audioSessionSetActive(NO);
        if (!result) {
            return NO;
        }
        
        /* AVAudioSession 各種設定が必要であれば、ここで行ってください */
        {
            AVAudioSession* audio_session = [AVAudioSession sharedInstance];
            NSError* ns_error = nil;
            /* カテゴリ設定 */
            result = [audio_session setCategory:AVAudioSessionCategoryAmbient error:&ns_error];
            if (!result) {
                return NO;
            }
        }
        
        result = audioSessionSetActive(YES);
        return result;
    }
}


@interface AtomUtilIOS_AudioSessionNotificationObserver : NSObject
@end

@implementation AtomUtilIOS_AudioSessionNotificationObserver
/* 音声割り込み処理 */
- (void)interruptionHandler: (NSNotification*)notification
{
    int value = [[notification.userInfo valueForKey: AVAudioSessionInterruptionTypeKey] intValue];
    switch (value) {
        case AVAudioSessionInterruptionTypeBegan:
			/* 音声割り込み開始時の処理 */
			/*   ADX2の音声出力を停止 */
            criAtomEx_StopSound_IOS();
			/*   AVAudioSessionをディアクティベート */
            audioSessionSetActive(NO);
            break;
        case AVAudioSessionInterruptionTypeEnded:
        default:
			/* 音声割り込み終了時の処理 */
			/*   AVAudioSessionをアクティベート */
            audioSessionSetActive(YES);
			/*   ADX2の音声出力を開始（再開） */
            criAtomEx_StartSound_IOS();
            break;
    }
}

/* メディアサーバがリセットされた際の処理 */
- (void)mediaServicesWereResetHandler: (NSNotification*)notification
{
    if (audioSessionSetup()) {
		/*
		 * メディアサーバがリセットされるとADX2内部のボイスが無効になるため、
		 * criAtomEx_RecoverSound_IOS を呼び出してボイスの再構築を行う。
		 */
        criAtomEx_RecoverSound_IOS();
    }
}
@end


namespace criadx2le {
    namespace {
        AtomUtilIOS_AudioSessionNotificationObserver* audiosession_notification_observer = nil;
    }
    
    bool audioSessionInitialize()
    {
        if ([[[UIDevice currentDevice] systemVersion] compare:@"6.0" options:NSNumericSearch] == NSOrderedAscending) {
            return false;
        }
        if (audiosession_notification_observer == nil) {
            audiosession_notification_observer = [AtomUtilIOS_AudioSessionNotificationObserver new];
            if (audiosession_notification_observer == nil) {
                return false;
            }
            /* NSNotificationCenter にオブザーバを登録 */
            NSNotificationCenter* notification_center = [NSNotificationCenter defaultCenter];
            AVAudioSession*       audio_session       = [AVAudioSession sharedInstance];
			/* AVAudioSessionInterruptionNotification に対するオブザーバとして */
			/* interruptionHandler: メソッドを登録する                         */
            [notification_center
             addObserver: audiosession_notification_observer
             selector: @selector(interruptionHandler:)
             name: AVAudioSessionInterruptionNotification
             object: audio_session
             ];
			/* AVAudioSessionInterruptionNotification に対するオブザーバとして */
			/* mediaServicesWereResetHandler: メソッドを登録する               */
            [notification_center
             addObserver: audiosession_notification_observer
             selector: @selector(mediaServicesWereResetHandler:)
             name: AVAudioSessionMediaServicesWereResetNotification
             object: audio_session
             ];
        }
        return audioSessionSetup() == YES;
    }
    
    void audioSessionFinalize()
    {
        if ([[[UIDevice currentDevice] systemVersion] compare:@"6.0" options:NSNumericSearch] == NSOrderedAscending) {
            return;
        }
        if (audiosession_notification_observer != nil) {
            /* NSNotificationCenter から Observer を削除 */
            [[NSNotificationCenter defaultCenter] removeObserver: audiosession_notification_observer];
            [audiosession_notification_observer release];
        }
        audiosession_notification_observer = nil;
    }
}
