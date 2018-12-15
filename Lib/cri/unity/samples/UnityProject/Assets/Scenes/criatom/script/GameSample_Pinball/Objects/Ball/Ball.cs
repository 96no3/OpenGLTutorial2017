/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;

public class Ball : MonoBehaviour {
	
	SoundManager sm;
	
	public GameObject ps;
	
	// Use this for initialization.
	void Start () {
		sm = GameObject.FindGameObjectWithTag("SoundManager").GetComponent<SoundManager>();
	}
	
	// Update is called once per frame.
	void Update () {
		if(this.transform.parent.position.y < -20.0f){
			GameObject.Destroy(this.transform.parent.gameObject);	
		}
	}
	
	void OnTriggerEnter(Collider other)
	{
		Rigidbody rb = this.transform.parent.gameObject.GetComponent<Rigidbody>();

		/* Calculate the strength. */
		float sqrLen = Mathf.Sqrt(
			rb.velocity.x
			*rb.velocity.x
			+
			rb.velocity.y
			*rb.velocity.y			
			);
		//Debug.Log("Mag : " + sqrLen.ToString());
		
		sm.PlaybackBall(0,(sqrLen/20.0f));
		
		if(ps != null){
			GameObject go = Instantiate(ps,this.transform.parent.transform.localPosition,this.transform.parent.transform.localRotation) as GameObject;
			//go.transform.parent = this.transform;
			GameObject.DestroyObject(go,0.75f);
		}
	}
}
