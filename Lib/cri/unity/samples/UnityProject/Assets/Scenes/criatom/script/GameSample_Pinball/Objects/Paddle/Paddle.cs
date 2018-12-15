/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;

public class Paddle : MonoBehaviour {
	
	public bool UseBumpAnimation = true;
	public AnimationClip paddleAnimationClip;
	SoundManager sm;
	
	// Use this for initialization
	void Start () {
		
		sm = GameObject.FindGameObjectWithTag("SoundManager").GetComponent<SoundManager>();

		Animation animation = GetComponent<Animation>();

		if(paddleAnimationClip != null && animation != null){
			animation.AddClip(paddleAnimationClip,"PaddleAnimation");
		}
	}
	
	// Update is called once per frame
	void Update ()
	{
	
	
	}
	
	public bool paddleEnableFlag = true;

	public bool PaddleEnableFlag {
		set {
			paddleEnableFlag = value;

			Collider col = GetComponent<Collider>();

			if(col != null){
				if(paddleEnableFlag)
				{
					col.isTrigger = true;	
				} else {
					col.isTrigger = false;
				}
			}
		}
	}	
	void OnTriggerEnter (Collider other)
	{	
		Rigidbody rb = other.GetComponent<Rigidbody>();
		Animation animation = GetComponent<Animation>();

		if(other.tag == "Ball" && paddleEnableFlag){

			if(rb != null){
				rb.velocity = new Vector3 (
					rb.velocity.x + (Random.Range (-0.5f, 0.5f)), 
					Random.Range (1.1f, 1.2f) * 20.0f, 
					0);
			}
			
			if(animation.isPlaying == false && animation != null){
				sm.PlaybackCue(5);
				animation.Play ("PaddleAnimation");
			}
		}
	}
	
	public void PlayPaddleAnimation()
	{
		Animation animation = GetComponent<Animation>();

		if(paddleAnimationClip != null && animation != null){
			sm.PlaybackCue(5);
			animation.Stop();
			animation.Play ("PaddleAnimation");
		}
	}
	
}
