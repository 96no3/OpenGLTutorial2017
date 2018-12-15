/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;
using System.Collections.Generic; // for an array

public class SoundTest : MonoBehaviour {
	private List<string> cueNameList = new List<string> ();
	private string cueSheetName = "PinballMain"; // CueSheet name
	CriAtomSource atomSourceSe;

	void Awake ()
	{
	}

	CriAtomEx.CueInfo[] cueInfoList;

	void Start ()
	{
		atomSourceSe = gameObject.AddComponent<CriAtomSource> ();
		atomSourceSe.cueSheet = cueSheetName;
		CriAtomExAcb acb = CriAtom.GetAcb (cueSheetName);
		cueInfoList = acb.GetCueInfoList ();
		foreach (CriAtomEx.CueInfo cueInfo in cueInfoList) {
			cueNameList.Add (cueInfo.name);
		}
		CriAtomEx.AttachDspBusSetting("DspBusSetting_0");
		CriAtom.SetBusAnalyzer(true); // Enable the level meter
	}

	public bool soundDebug = true;

	void OnGUI()
	{
		if (Scene_00_SampleList.ShowList == true) {
			return;
		}

		Scene_00_GUI.BeginGui("01/SampleMain");
		
		/* Set UI skin. */
		GUI.skin = Scene_00_SampleList.uiSkin;

		if(soundDebug){
			GUILayout.BeginArea(new Rect(Screen.width - 150, 0, 150, 300));
			foreach (CriAtomEx.CueInfo cueInfo in cueInfoList) {
				if(Scene_00_GUI.Button(cueInfo.name)){
					atomSourceSe.Stop();
					atomSourceSe.Play(cueInfo.name);
				}
			}
			GUILayout.EndArea();
		}
		
		Scene_00_GUI.EndGui();
	}

}
