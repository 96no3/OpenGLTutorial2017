/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;

/*JP クリックしたオブジェクトにCriAtomSourceがあればその音を鳴らす。多重再生OK。 */
/*EN If the object clicked has the CriAtomSource, the sound is played. Multiple playbacks are allowed. */
public class PlaySoundOnClick : MonoBehaviour {

	private bool trigger = false;

	void Start () {
	}

	void Update () {
		if (trigger) {
			CriAtomSource atomSrc = gameObject.GetComponent<CriAtomSource>();
			if (atomSrc != null) {
				atomSrc.Play();
			}
			trigger = false;
		}
	}
	
	void OnGUI()
	{
		if (Scene_00_SampleList.ShowList == true) {
			return;
		}

		if (Camera.main == null) {
			return;
		}

		Scene_00_GUI.BeginGui("01/SampleMain1");

		/* Set UI skin. */
		GUI.skin = Scene_00_SampleList.uiSkin;

		var pos = Camera.main.WorldToScreenPoint(transform.position);
		pos.y   = Screen.height - pos.y;
		if (Scene_00_GUI.Button(new Rect(pos.x, pos.y, 150, 150), "Play")) {
			trigger = true;
		}

		Scene_00_GUI.EndGui();
	}
}
