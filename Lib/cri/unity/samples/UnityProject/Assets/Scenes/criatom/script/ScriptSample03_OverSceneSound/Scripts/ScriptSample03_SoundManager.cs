/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class ScriptSample03_SoundManager : MonoBehaviour {
	/* ACB file name (CueSheet name) */
	public string cueSheetName = "CueSheet_0";
	
	private CriAtomSource atomSourceMusic;
	static private ScriptSample03_SoundManager instance = null;
	
	void Awake ()
	{
		if (instance != null) {
			/*JP
			 * 多重生成を回避するため, 後から作られたSoundManagerは自分自身を破棄する.
			 * ただし, SoundManagerをコンポーネントにもつGameObject自体は破棄されない. 
			 * そのため, Unityエディタのヒエラルキー上には, 同じ名前のGameObjectが複数存在することになる
			 */
			/*EN
			 * To prevent multiple generation, the SoundManager created later destroys itself.
			 * However, the GameObject with the SoundManager as a component is not destroyed. 
			 * Therefore, in the hierarchy of Unity Editor, multiple GameObjects of the same name exist. 
			 */
			GameObject.Destroy(this);
			return;
		}
		
		/* Create the CriAtomSource for BGM. */
		atomSourceMusic = gameObject.AddComponent<CriAtomSource> ();
		atomSourceMusic.cueSheet = cueSheetName;	
		
		/* Do not destroy the SoundManger when scenes are switched. */
		GameObject.DontDestroyOnLoad(this.gameObject);
		instance = this;
	}
	
	static public void PlayCueId(int cueId){
		instance.atomSourceMusic.Play(cueId);
	}

	void OnDestroy(){
		if (instance == this) {
			instance = null;
		}
	}
	
}
