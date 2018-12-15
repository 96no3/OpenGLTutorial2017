/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class SoundManager : MonoBehaviour {
	
	/* CueSheet name */
	private string cueSheetName = "PinballMain";
	
	CriAtomSource atomSourceSe;
	CriAtomSource atomSourceBall;
	CriAtomSource atomSourceBgm;
	CriAtomSource atomSourceBumper;
	
	
	void Awake ()
	{
		/* Create a CriAtomSource for SE. */
		atomSourceSe = gameObject.AddComponent<CriAtomSource> ();
		atomSourceSe.cueSheet = cueSheetName;
		
		/* Create a CriAtomSource for Ball. */
		atomSourceBall = gameObject.AddComponent<CriAtomSource> ();
		atomSourceBall.cueSheet = cueSheetName;
		
		/* Create a CriAtomSource for Bumper. */
		atomSourceBumper = gameObject.AddComponent<CriAtomSource> ();
		atomSourceBumper.cueSheet = cueSheetName;
		
		/* Create a CriAtomSource for BGM. */
		atomSourceBgm = gameObject.AddComponent<CriAtomSource> ();
		atomSourceBgm.cueSheet = cueSheetName;
	}
	// Use this for initialization.
	void Start () {
		/* The DSP bus setting is specified on the CRIWARE Object side. */
		//CriAtom.AttachDspBusSetting("DspBusSetting_0");
		
		/* Add the level monitor feature for in-game previewing. */
		CriAtom.SetBusAnalyzer(true);
	}
	
	float lastResumeBgmTime = 0;
	// Update is called once per frame.
	void Update () {
		
		if(lastResumeBgmTime + 2 < Time.timeSinceLevelLoad){
			ResumeBGM();
			lastResumeBgmTime = Time.timeSinceLevelLoad;	
		}
	}
	
	public void PlaybackCue(int index)
	{
		atomSourceSe.Play(index);
	}
	
	public void PlayGameOver()
	{
		//atomSourceSe.Play(4); // by Cue ID
		atomSourceSe.Play("GameOver"); // by Cue Name
	}
		
	
	float lastPlaybackBallTime = 0;
	public float lastVelocity = 0.0f;
	/// <summary>
	/// Ball hitting sound
	/// </summary>
	public void PlaybackBall(int index,float velocity)
	{
		if(lastPlaybackBallTime+0.25 < Time.timeSinceLevelLoad){
			velocity = Mathf.Min(velocity,1.0f);
			atomSourceBall.SetAisac(0,velocity);
			atomSourceBall.Play(index);
			lastPlaybackBallTime = Time.timeSinceLevelLoad;
			
			lastVelocity = velocity;
		}
	}
	
	float lastPlaybackBumperTime = 0;
	public void PlaybackBumper(int index)
	{
		if(lastPlaybackBumperTime+0.25 < Time.timeSinceLevelLoad){
			atomSourceBumper.Play(index);
			lastPlaybackBumperTime = Time.timeSinceLevelLoad;
		}
	}
	
	public void ResumeBGM()
	{
		/* Play if the status is in the PlayEnd or the Stop. (automatically restart when ACB is updated) */
		CriAtomSource.Status status = atomSourceBgm.status;
		if ((status == CriAtomSource.Status.Stop) || (status == CriAtomSource.Status.PlayEnd)) {
			/* Play */
			PlayBGM();
			

			
		}
	}
	private CriAtomExPlayback playbackBGM;
	CriAtomEx.CueInfo cueInfo;
	public void PlayBGM()
	{
		bool startFlag = false;
		CriAtomSource.Status status = atomSourceBgm.status;
		if ((status == CriAtomSource.Status.Stop) || (status == CriAtomSource.Status.PlayEnd)) {
			this.playbackBGM = atomSourceBgm.Play(100);
			startFlag = true;
		}
		
		/*	Move to the next block except for the first playback. */
		if(startFlag == false){
			int cur = this.playbackBGM.GetCurrentBlockIndex();
			CriAtomExAcb acb = CriAtom.GetAcb("PinballMain");
			if(acb != null){
				acb.GetCueInfo("BGM",out this.cueInfo);
				
				cur++;
				if(this.cueInfo.numBlocks > 0){
					this.playbackBGM.SetNextBlockIndex(cur % this.cueInfo.numBlocks);
				}
				
			}
		}
	}
	
	public void Pause()
	{
		atomSourceBgm.Pause(true);
	}
	public void Resume()
	{
		atomSourceBgm.Pause(false);
	}
	
	
}
