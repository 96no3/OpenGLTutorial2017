/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*JP
 * ボールが壁に当たった時に音を再生するスクリプトです。
 */
/*EN
 * This script plays a sound when a ball hits the wall.
 */
using UnityEngine;
using System.Collections;

public class Scene_07_Ingame_Pinball_PlayBounce : MonoBehaviour
{
	#region Variables
	private Scene_07_Ingame_Pinball_SoundManager soundManager = null;
	#endregion
	
	#region Functions
	void Start()
	{
		/* Get the SoundManager. */
		soundManager = FindObjectOfType(typeof(Scene_07_Ingame_Pinball_SoundManager)) as Scene_07_Ingame_Pinball_SoundManager;		
	}
	
	/* Play at collision. */
	void OnTriggerEnter(Collider other)
	{
		/* Play at collision hit. */
		soundManager.PlayBounce();
		
		/* If BGM is stopped, restore it. */
		soundManager.ResumeBGM();
	}
	#endregion
}
/* end of file */
