/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;

public class DeathTrigger : MonoBehaviour {
	
	GameObject gm;	
	SoundManager sm;
	
	// Use this for initialization
	void Start () {
		gm = GameObject.FindGameObjectWithTag ("GameMain");		
		sm = GameObject.FindGameObjectWithTag("SoundManager").GetComponent<SoundManager>();
	}
	
	// Update is called once per frame
	void Update () {
	
	}
	
	void OnTriggerEnter(Collider other)
	{
		if(other.gameObject.tag == "Ball"){
			Debug.Log ("Death ! ");	
			GameObject.Destroy(other.gameObject);
			
			gm.GetComponent<GameMain> ().AddDeathCount ();
			sm.PlaybackCue(1);
		}
	}
}
