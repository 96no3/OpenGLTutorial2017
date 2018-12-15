/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;

public class ScriptSample04_GameScene : MonoBehaviour {

	private int 	SceneMusicCueId = 2;
	private string 	nextSceneName   = "ScriptSample04_TitleScene";


	/* Called before the first Update(). */
	void Start () {
		/* Play BGM. */
		ScriptSample04_SoundManager.PlayCueId(SceneMusicCueId);
	}

	/* Show and control the scene-switching GUI. */
	void OnGUI(){
		if (Scene_00_SampleList.ShowList == true) {
			return;
		}
		
		/* Set UI skin. */
		GUI.skin = Scene_00_SampleList.uiSkin;

		Scene_00_GUI.BeginGui("01/SampleMain");
		if (Scene_00_GUI.Button(new Rect(Screen.width-250,200,150,150), "change\nscene")) {
			Application.LoadLevel(nextSceneName);				
		}
		Scene_00_GUI.EndGui();
	}
}

