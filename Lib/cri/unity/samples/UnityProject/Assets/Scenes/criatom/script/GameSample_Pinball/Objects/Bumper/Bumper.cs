/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;

public class Bumper : MonoBehaviour {
	
	public bool UseBumpAnimation = true;
	SoundManager sm;
	
	// Use this for initialization
	void Start () {
		
		sm = GameObject.FindGameObjectWithTag("SoundManager").GetComponent<SoundManager>();
	}
	
	// Update is called once per frame
	void Update ()
	{
	
	
	}
	
	public bool BumperEnableFlag = true;
	
	float lastPlaybackBumperTime = 0;
	
	void OnTriggerEnter (Collider other)
	{
		Rigidbody rb = other.GetComponent<Rigidbody>();
		Animation animation = GetComponent<Animation>();

		if(rb != null){
			//Debug.Log ("On!");	
			
			//GameObject go = GameObject.FindGameObjectWithTag ("Ball");
			//Debug.Log ("On! " + go.rigidbody.velocity.y.ToString ());
			//go.rigidbody.AddRelativeForce (0, 20, 0);
			
			if(other.tag == "Ball" && BumperEnableFlag){
				
				rb.velocity = new Vector3 (
					rb.velocity.x + (Random.Range (-0.5f, 0.5f)), 
					Random.Range (1.1f, 1.2f) * 20.0f, 
					0);
				
				if(lastPlaybackBumperTime+0.25 < Time.timeSinceLevelLoad){
					sm.PlaybackBumper(2);
					lastPlaybackBumperTime = Time.timeSinceLevelLoad;
				}
				if(UseBumpAnimation && animation != null){
					animation.Play ("BigAnimation");
				}
			}
		}
	}
	
	
}
