/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*JP
 * 本サンプルは、カテゴリを利用しています。
 *
 * それぞれのカテゴリのボリュームをスライダーで変化させることで、再生中
 * の音声のボリュームをカテゴリ別に一括コントロールすることができます。
 * また、「Play Special Voice」ボタンをクリックすることでセリフボイスが
 * 再生されますが、このとき、データ側に指定されているカテゴリとREACTに
 * よる作用で、BGMのダッキングが自動的に行われます。
 *
 */
/*EN
 * This sample uses the category feature.
 *
 * By controlling individual category's volume with the slider,
 * the volumes of the sounds being played can be controlled at once
 * by the category type.
 * Also, by clicking on the "Play Special Voice" button, a dialog voice is
 * is played, and the ducking to BGM is automatically applied by the interaction
 * of the category set up on the data and the REACT.
 */
using UnityEngine;
using System.Collections;

public class Scene_05_Category : MonoBehaviour
{
	#region Variables
	/* Volume of BGM category */
	private float bgmVolume = 1.0f;
	/* Volume of SE category */
	private float seVolume = 1.0f;
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

		/* Get the CriAtomSource component. */
		CriAtomSource atomSource = this.GetComponent<CriAtomSource>();

		/* Display GUI, and control the playback. */
		GUILayoutOption[] option = new GUILayoutOption[]{ GUILayout.Height(28) };
		GUILayout.BeginArea(new Rect(42, 96, 480, 300), Scene_00_SampleList.TextStyle);
		{
			/*  When a button is clicked, the corresponding sound is played. */
			if (Scene_00_GUI.Button("Play SE1", option)) {
				/* Play a Cue of the SE category. */
				atomSource.Play("gun1_High");
			}
			if (Scene_00_GUI.Button("Play SE2", option)) {
				/* Play a Cue of the SE category. */
				atomSource.Play("bomb2");
			}
			if (Scene_00_GUI.Button("Play Special Voice\n(BGM Volume goes down by REACT)", GUILayout.Height(56))) {
				/* Play a Cue that belongs to the SPECIAL category. */
				/* The "REACT" is configured on the data side, which turns down */
				/* the volume of BGM category when playing a SPECIAL category's sound. */
				/* So, this playback turns down the BGM volume. */
				atomSource.Play("cri_middleware_sdk_Special");
			}
			GUILayout.Space(28);

			/* Display the slider for controlling the Category volume. */
			GUILayout.Label("BGM Category Volume : " + CriAtom.GetCategoryVolume("BGM").ToString("0.00"));
			
			GUILayout.BeginHorizontal();
			this.bgmVolume = Scene_00_GUI.HorizontalSliderButton(bgmVolume, 0.0f, 1.0f, 0.2f);
			GUILayout.Label(this.bgmVolume.ToString("0.00"));
			GUILayout.EndHorizontal();
			
			GUILayout.Label("SE Category Volume : " + CriAtom.GetCategoryVolume("SE").ToString("0.00"));
			GUILayout.BeginHorizontal();
			this.seVolume = Scene_00_GUI.HorizontalSliderButton(seVolume, 0.0f, 1.0f, 0.2f);
			GUILayout.Label(this.seVolume.ToString("0.00"));
			GUILayout.EndHorizontal();
		}
		GUILayout.EndArea();
		
		/* Specify category volumes. */
		CriAtom.SetCategoryVolume("BGM", bgmVolume);
		CriAtom.SetCategoryVolume("SE", seVolume);
		
		Scene_00_GUI.EndGui();
	}

	void OnDisable()
	{
		/* Restore the category volumes before moving to another scene. */
		CriAtom.SetCategoryVolume("BGM", 1.0f);
		CriAtom.SetCategoryVolume("SE", 1.0f);
	}
	#endregion
}

/* end of file */
