/****************************************************************************
 *
 * Copyright (c) 2016 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using System.Linq;

public class VirtualPad : MonoBehaviour {
	private Button recBtn = null;
	private Button playBtn = null;
	private Text recTxt = null;
	private Text playTxt = null;
	private Text toggleTxt = null;
	private bool recording = false;
	private bool playing = false;
	private GameObject virtualPad = null;

	struct HightlightButtonInfo {
		public int frame;
		public Image image;
	}

	private List<HightlightButtonInfo> hilitBtns = new List<HightlightButtonInfo>();

	public void Awake() {
		recBtn = GameObject.Find("Record").GetComponent<Button>();
		playBtn = GameObject.Find("Replay").GetComponent<Button>();
		recTxt = GameObject.Find("Record").GetComponentInChildren<Text>();
		playTxt = GameObject.Find("Replay").GetComponentInChildren<Text>();
		toggleTxt = GameObject.Find("Toggle").GetComponentInChildren<Text> ();
		virtualPad = GameObject.Find("VirtualPad").GetComponent<CanvasRenderer>().gameObject;
	}

	public void Start() {
		if (SampleIO.ShowRecordingControls == false) {
			recBtn.interactable = false;
			playBtn.interactable = false;
			recBtn.GetComponent<Image>().color = Color.clear;
			recBtn.GetComponentInChildren<Text>().color = Color.clear;
			playBtn.GetComponent<Image>().color = Color.clear;
			playBtn.GetComponentInChildren<Text>().color = Color.clear;
		}
		if (SampleIO.ShowVirtualPad == false) {
			virtualPad.SetActive(false);
			toggleTxt.text = "Show";
		}
		if (SampleIO.BeginRecordWithScene) {
			recTxt.text = "Stop";
			playBtn.interactable = false;
			recording = true;
		}
		if (SampleIO.BeginReplayWithScene) {
			playTxt.text = "Stop";
			recBtn.interactable = false;
			playing = true;
		}
	}

	private void HighlightKey(SampleIO.Key key, Color color)
	{
		if (key != SampleIO.Key.Noinput && key != SampleIO.Key.Invalid) {
			Transform t = virtualPad.transform.FindChild(key.ToString());
			if (t != null) {
				Image img = t.gameObject.GetComponent<Image>();
				if (img != null) {
					hilitBtns.Add(new HightlightButtonInfo() {frame = Time.frameCount, image = img});
					img.color = color;
				}
			}
		}
	}

	public void Update() {
		if (playing) {
			HighlightKey(SampleIO.GetPushedKey(), Color.blue);
		}
		if (SampleIO.IsReplaying == false && playing) {
			playTxt.text = "Play";
			recBtn.interactable = true;
			playing = false;
		}

		// Button highlight removed after 10 frames.
		if (hilitBtns.Count > 0) {
			hilitBtns.RemoveAll(i => { 
				if (i.frame + 10 <= Time.frameCount) {
					i.image.color = Color.white; 
					return true;
				} 
				return false; 
			});
		}
	}
	
	// SampleIO‚ð‰î‚µ‚½“ü—Í
    public void PushKey(int inputKey) {
		// Button Highlight System
		if (recording) {
			HighlightKey(((SampleIO.Key)inputKey), Color.red);
		}

        SampleIO.PushKey(inputKey);
    }
	// ReplayButton wrapper to SampleIO
	public void PushRecordButton() {
		if (!recording) {
			recTxt.text = "Stop";
			playBtn.interactable = false;
			recording = true;
		} else {
			recTxt.text = "Rec";
			playBtn.interactable = true;
			recording = false;
		}

		SampleIO.OnPushRecordButton(recording);
	}
	// RecordButton wrapper to SampleIO
	public void PushReplayButton() {
		if (!playing) {
			playTxt.text = "Stop";
			recBtn.interactable = false;
			playing = true;
		} else {
			playTxt.text = "Play";
			recBtn.interactable = true;
			playing = false;
		}

		SampleIO.OnPushReplayButton(playing);
	}
	// Show/Hide VirtualPad
	public void PushToggleButton() {
		virtualPad.SetActive(!virtualPad.activeSelf);
		toggleTxt.text = virtualPad.activeSelf ? "Hide" : "Show";
	}
}
