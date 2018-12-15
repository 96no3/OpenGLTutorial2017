/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*JP
 * 本サンプルは、シンプルな再生を行います。
 * クリックしたオブジェクトのCriAtomSourceを取得し、音声の再生を行います。
 *
 * CubeにCRI AtomSourceスクリプトが適用されており、銃声の音が設定されています。
 * また、PlayOnStartが有効になっているため、実行時に発音されます。
 */
/*EN
 * This sample performs the simple playback.
 * After retrieving the CriAtomSource of the object on which you clicked, 
 * it plays a sound.
 *
 * A CRI Atom Source script is assigned to the Cube, which specifies a gunshot.
 * As the PlayOnStart is enabled, the sound is played at the start-up.
 */
using UnityEngine;
using System.Collections;

public class Scene_01_SimplePlayback : MonoBehaviour
{
	#region Variables
	public CriAtomSource atomSource;
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

		if (Scene_00_GUI.Button(new Rect(100, 100, 160, 160), "Push to Play")) {
			atomSource.Play();
		}
		
		Scene_00_GUI.EndGui();
	}
	#endregion
}
/* end of file */
