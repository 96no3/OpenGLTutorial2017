/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*JP
 * 本サンプルでは、ブロック再生を行います。
 * 右側のボタンで音を再生し、数値のボタンでブロックを指定します。
 */
/*EN
 * This sample demonstrates a block playback.
 * Play a sound by the right side buttons.
 * Specify a block by the number buttons.
 */
using UnityEngine;
using System.Collections;

public class Scene_06_BlockPlayback : MonoBehaviour
{
	#region Variables
	private GameObject selectedGameObject;
	private CriAtomExPlayback playback;
	private CriAtomEx.CueInfo cueInfo;
	private int nextBlockIndex = 0;
	#endregion
	
	#region Functions
	void OnGUI()
	{
#if UNITY_WEBGL
		if (CriAtom.CueSheetsAreLoading) {
			return;
		}
#endif
		
		if (Scene_00_SampleList.ShowList == true) {
			return;
		}
		
		Scene_00_GUI.BeginGui("01/SampleMain");

		/* Set UI skin. */
		GUI.skin = Scene_00_SampleList.uiSkin;

		/* Get the AtomSource. */
		CriAtomSource audio = this.GetComponent<CriAtomSource>();
		/* Get the ACB. */
		CriAtomExAcb acb = CriAtom.GetAcb("DemoProj");

		GUILayoutOption[] option = new GUILayoutOption[]{GUILayout.Width(320), GUILayout.Height(28)};

		GUILayout.BeginArea(new Rect(Screen.width / 2, 70, 320, Screen.height - 60));
		if (Scene_00_GUI.Button("Start [Karinba]", option)) {
			/* Play after stopping the playback. */
			audio.Stop();
			this.playback = audio.Play("kalimbaScaleUp");
			acb.GetCueInfo("kalimbaScaleUp", out this.cueInfo);
			this.nextBlockIndex = 0;
		}
		if (Scene_00_GUI.Button("Start [beamUp]", option)) {
			/* Play after stopping the playback. */
			audio.Stop();
			this.playback = audio.Play("beamUp");
			acb.GetCueInfo("beamUp", out this.cueInfo);
			this.nextBlockIndex = 0;
		}
		if (Scene_00_GUI.Button("Start [Music]", option)) {
			/* Play after stopping the playback. */
			audio.Stop();
			this.playback = audio.Play("Music");
			acb.GetCueInfo("Music", out this.cueInfo);
			this.nextBlockIndex = 0;
		}
		if (Scene_00_GUI.Button("Stop", option)) {
			/* Stop */
			audio.Stop();
		}
		int cur = this.playback.GetCurrentBlockIndex();
		GUILayout.Space(10);
		GUILayout.Label("Current block index: " + cur);
		GUILayout.Space(10);
		GUILayout.Label("Next block index: " + this.nextBlockIndex);

		/*
		string[] list = new string[this.cueInfo.numBlocks];
		for (int i = 0; i < this.cueInfo.numBlocks; i++) {
			if (cur == i) {
				list[i] = "< " + i.ToString() + " > ";
			} else {
				list[i] = i.ToString();
			}
		}*/

		int index = -1;
		GUILayout.BeginHorizontal();
		for (int i = 0; i < this.cueInfo.numBlocks; i++) {
			string s;
			if (cur == i) {
				s = "< " + i.ToString() + " > ";
			} else {
				s = i.ToString();
			}
			if (Scene_00_GUI.Button(s)) {
				index = i;
			}
		}
		GUILayout.EndHorizontal();

		//int index = GUILayout.Toolbar(this.nextBlockIndex, list);
		if ((index > 0) && (index != this.nextBlockIndex)) {
			/* Specify a block. */
			this.playback.SetNextBlockIndex(index);
			this.nextBlockIndex = index;
		}
		GUILayout.EndArea();
		
		Scene_00_GUI.EndGui();
	}
	#endregion
}
/* end of file */
