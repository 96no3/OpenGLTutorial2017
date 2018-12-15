/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*JP
 * 本サンプルは、AISACのコントロールを行います。
 *
 * データ上のAISACにはピッチのカーブが設定されており、 スライダーを動かして
 * AISACコントロール値を変化させることでピッチが変化します。
 */
/*EN
 * This sample performs AISAC controls.
 *
 * An AISAC that has a pitch curve is set up on the data.
 * By changing the AISAC control value, the pitch changes.
 */
using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Scene_03_AISAC : MonoBehaviour
{
	#region Variables
	/* AISAC control value */
	private float[] aisacValueList = new float[2];
	/* CriAtomSource */
	private CriAtomSource atomSource = null;
	private string[] aisacControlList = new string[] {"rpm", "load"};
	#endregion

	IEnumerator Start()
	{
		// Wait for Loading ACB...
		while (CriAtom.CueSheetsAreLoading) {
			yield return null;
		}

		/* Get the AtomSource. */
		atomSource = gameObject.GetComponent<CriAtomSource>();
        for (int i = 0; i < aisacControlList.Length; i++) {
            atomSource.SetAisac(aisacControlList[i], 0.0f);
        }
        atomSource.Play();
    }
	
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
		
		GUILayout.BeginArea(new Rect(12, 70, 380, 390), "", Scene_00_SampleList.TextStyle);
		for (int i = 0; i < aisacControlList.Length; i++){
			GUILayout.Label( "AISAC Control Value " 
				+ this.aisacValueList[i].ToString("0.00")
				+ " " +  aisacControlList[i]);
			this.aisacValueList[i] = Scene_00_GUI.HorizontalSliderButton(this.aisacValueList[i], 0.0f, 1.0f, 0.2f, GUILayout.Height(100));
			/* Configure the AISAC. */
			atomSource.SetAisac(aisacControlList[i], this.aisacValueList[i]);
		}
        if (Scene_00_GUI.Button("Reset & Play", GUILayout.Height(100))) {   
            atomSource.Stop();
            for (int i = 0; i < aisacControlList.Length; i++) {
                aisacValueList[i] = 0.0f;
                atomSource.SetAisac(aisacControlList[i], aisacValueList[i]);
            }
            atomSource.Play();
        }
        GUILayout.EndArea();
		
		Scene_00_GUI.EndGui();
	}
	#endregion
}
/* end of file */
