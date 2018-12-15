/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;

public class Respawn : MonoBehaviour {
	
	public GameObject ball;
	GameMain gm;	
	//SoundManager sm;
	
	void Start () {
		ball = Resources.Load("Ball",typeof(GameObject)) as GameObject;
		
		gm = GameObject.FindGameObjectWithTag ("GameMain").GetComponent<GameMain>();
		//sm = GameObject.FindGameObjectWithTag("SoundManager").GetComponent<SoundManager>();

	}
	
	// Update is called once per frame
	void Update () {
		if(gm.level == GameMain.EnumGameState.Main){
		
			GameObject go = GameObject.FindGameObjectWithTag("Ball");
			
			if(go == null){
				GameObject.Instantiate(ball,this.transform.position,this.transform.rotation);		
			}
		}
	}
	
	
}
