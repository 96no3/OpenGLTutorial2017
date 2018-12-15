/****************************************************************************
 *
 * Copyright (c) 2016 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class DisplayText : MonoBehaviour {

    private Text display = null;

	void Awake () {
        display = this.GetComponent<Text>();
	}
	
	// Update is called once per frame
	void Update () {
        string text = "";
        for (int i = 0; i < SampleIO.Labels.Length; i++) {
            text += SampleIO.Labels[i] + "\n";
        }
        display.text = text;
    }
}
