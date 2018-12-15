/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*JP
 * 本サンプルは、ピンボール風のサンプルプログラムです。
 * スクリプトにより音声再生を行っており、さらにインゲームプレビューを行う事
 * ができます。
 * 
 * [インゲームプレビューの手順]
 * 1. CRI Atom Craftでexample/pinball/Pinball.atmcprojを開きます。
 * 2. ツール上でF10キーを押してインゲームプレビューを開始します。
 * 
 * 注意）
 * あらかじめメニューの[ツール/プロパティ.../ターゲット/]の項目のIPアドレス
 * をこのアプリの起動しているIPアドレスに変更しておく必要があります。
 *
 * また、インゲームプレビュー開始時にエラーメッセージが出ることがありますが、
 * これはACBファイルが一時的にアンロードされているのにもかかわらず、音声再
 * 生をリクエストしているためです。
 * 
 * また、このスクリプトでは以下のサンプルも兼ねています。
 * - CriAtomSourceをスクリプトで生成して再生（サウンドの集中管理）
 * - キュー情報の取得
 */
/*EN
 * This sample is a pin ball game-like sample program.
 * Sound playback is controlled by script, and the in-game preview is available.
 * 
 * [Procedure for in-game preview]
 * 1. Open example/pinball/Pinball.atmcproj in CRI Atom Craft.
 * 2. Press the [F10] key on the tool to start the in-game preview.
 * 
 * [Note]
 * 1 Select the [Tool] menu to display the [Properties] window, and change 
 *   the [IP address] item on the [Target] pane to the IP address for PC 
 *   on which this tool runs beforehand.
 * 2 An error message may be displayed when starting the in-game preview.
 *   This is because a sound playback is requested even though the ACB file 
 *   is temporarily unloaded.
 * 3 This script has the following roles as a sample;
 *   - Create CriAtomSource by script and play (central management of sounds)
 *   - Retrieve the Cue information
 */
using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Scene_07_Ingame_Pinball_SoundManager : MonoBehaviour
{
	#region Variables
	/* volume of the category [main] */
	private float mainVolume = 1.0f;
	
	/* AtomSource for bouncing sound */
	private CriAtomSource atomSourceBounce = null;	
	/* SE number */
	private int seNo = 2;	
	/* AtomSource for BGM */
	private CriAtomSource atomSourceBGM = null;
	/* BGM number */
	private int bgmNo = 0;	
	
	/* CueSheet name */
	private string cueSheetName = "PinballMain";
	
	/* list of Cue name */
	private List<string> cueNameList = new List<string>();
	#endregion
	
	#region Enumerators
	/* enum list of Cues */
	enum enumCueNameList
	{
		wood,
		glass,
		synth,
		bgm0,
		bgm1
	};
	#endregion

	#region Functions
	void Awake()
	{
		/* Get the Cue info by specifying the ACB file name (CueSheet name). */
		CriAtomExAcb acb = CriAtom.GetAcb(cueSheetName);	

		/* Create a list of Cue names. */
		CriAtomEx.CueInfo[] cueInfoList = acb.GetCueInfoList();
		foreach(CriAtomEx.CueInfo cueInfo in cueInfoList){
			cueNameList.Add(cueInfo.name);	
		}
		
		/* Create the CriAtomSource for bouncing sound. */
		atomSourceBounce = gameObject.AddComponent<CriAtomSource>();
		atomSourceBounce.cueSheet = cueSheetName;
		atomSourceBounce.cueName = cueNameList[(int)enumCueNameList.wood + seNo];
		
		/* Create the CriAtomSource for BGM. */
		atomSourceBGM = gameObject.AddComponent<CriAtomSource>();
		atomSourceBGM.cueSheet = cueSheetName;
		atomSourceBGM.cueName = cueNameList[(int)enumCueNameList.bgm0 + bgmNo];
		
		
	}
	
	/* Bouncing sound */
	public void PlayBounce()
	{
		/* Specify a Cue name. */
		atomSourceBounce.cueName = cueNameList[(int)enumCueNameList.wood + seNo];
		/* Play */
		atomSourceBounce.Play();	
	}
	
	public void ResumeBGM()
	{
		/* Play when in the PLAYEND or in the STOP. */
		/* (Automatically restore at the time of ACB update) */
		CriAtomSource.Status status = atomSourceBGM.status;
		if ((status == CriAtomSource.Status.Stop) || (status == CriAtomSource.Status.PlayEnd)) {
			/* Play */
			PlayBGM();	/* Change the volume of category [main]. */
			CriAtom.SetCategoryVolume("main", mainVolume);

			/* Add the feature of level monitor for in-game preview. */
			CriAtom.SetBusAnalyzer(true);
		}
	}
	
	/* Play BGM. */
	public void PlayBGM()
	{
		/* Stop BGM. */
		atomSourceBGM.Stop();
		/* Specify a Cue name. */
		atomSourceBGM.cueName = cueNameList[(int)enumCueNameList.bgm0 + bgmNo];
		/* Play BGM. */
		atomSourceBGM.Play();		
	}
	
	void OnGUI()
	{
		if (Scene_00_SampleList.ShowList == true) {
			return;
		}
		
		Scene_00_GUI.BeginGui("01/SampleMain");

		/* Set UI skin. */
		GUI.skin = Scene_00_SampleList.uiSkin;

		GUILayout.BeginArea(new Rect(Screen.width - 336, 70+32, 320, 140), "", Scene_00_SampleList.TextStyle);
		
		GUILayout.Label("Main Volume " + mainVolume.ToString("0.00"));
		this.mainVolume = Scene_00_GUI.HorizontalSliderButton(mainVolume, 0.0f, 1.0f, 0.2f);
		
		/* Change the volume of the category [main]. */
		CriAtom.SetCategoryVolume("main", mainVolume);
		
		if (Scene_00_GUI.Button("BGM No." + bgmNo.ToString("0"))) {				
			bgmNo++;
			if (bgmNo > 1)
				bgmNo = 0;
			PlayBGM();
		}
		if (Scene_00_GUI.Button("SE No." + seNo.ToString("0") + " " + cueNameList[(int)enumCueNameList.wood + seNo])) {				
			seNo++;
			if (seNo > 2)
				seNo = 0;
		}
		GUILayout.EndArea();
		
		Scene_00_GUI.EndGui();
	}
	#endregion
}

/* end of file */
