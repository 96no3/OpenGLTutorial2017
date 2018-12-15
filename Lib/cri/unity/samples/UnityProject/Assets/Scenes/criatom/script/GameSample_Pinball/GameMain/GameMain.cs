/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;

public class GameMain : MonoBehaviour {
	
	int Score = 0;
	int BallCount = 3;
	
	SoundManager sm;
	
	public Paddle paddleL;
	public Paddle paddleR;
	
	
	public enum EnumGameState{
		Title,
		Main,
		Gameover		
	};
	
	public EnumGameState level = EnumGameState.Title;
	
	
	// Use this for initialization
	void Start () {
		sm = GameObject.FindGameObjectWithTag("SoundManager").GetComponent<SoundManager>();
	
	}
	
	// Update is called once per frame
	void Update ()
	{
		Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
		/* When a button is clicked */
		if (Input.GetButtonDown("Fire1") == true) {
			RaycastHit hit;
			if (Physics.Raycast(ray, out hit, 100)) {
				/*	Get the game object	*/
				GameObject selectedGameObject = hit.collider.gameObject;
				if (selectedGameObject != null &&
					selectedGameObject.tag == "Cabinet") {
					
					GameObject[] balls = GameObject.FindGameObjectsWithTag("Ball");
					foreach(GameObject ball in balls){
						Rigidbody rb = ball.GetComponent<Rigidbody>();
						if(rb != null){
							rb.velocity = new Vector3(0,0,0);
							rb.position = new Vector3(hit.point.x,hit.point.y,0);
						}
					}
				}
			}
		}
		if(Input.GetKeyDown(KeyCode.LeftArrow)){
			if(paddleL != null)
			paddleL.PlayPaddleAnimation();
		}
		if(Input.GetKeyDown(KeyCode.RightArrow)){
			if(paddleR != null)
			paddleR.PlayPaddleAnimation();
		}
		
		switch(this.level)
		{
			case EnumGameState.Gameover:
			
			if(lastLevelChangeTime+10 < Time.timeSinceLevelLoad){
				ChangeLevel(EnumGameState.Title);	
			}
				
			break;
			case EnumGameState.Title:
			
			if(lastLevelChangeTime+30 < Time.timeSinceLevelLoad){
				ChangeLevel(EnumGameState.Main);	
			}
			
			break;			
		}
	}
		
	bool stopperFlag= false;
	bool gamePauseFlag = false;
	
	//float gameSpeed = 2.0f;
	
	void OnGUI ()
	{
		Scene_00_GUI.BeginGui("01/SampleMain");

		switch(level)
		{
			case EnumGameState.Title:
			{
			if(Scene_00_GUI.Button(new Rect(Screen.width/2-100,Screen.height/4-50,200,100),"Click to Start!")){
					ChangeLevel(EnumGameState.Main);
					
				}
			}
				break;
			case EnumGameState.Main:
			{
			
				//GUI.backgroundColor = Color.black;
			
				GUILayout.BeginArea(new Rect(Screen.width - 100,0,Screen.width/3,Screen.height));
				if(level == EnumGameState.Main){		
					GUILayout.BeginVertical();
					GUILayout.Label ("Ball " + BallCount.ToString());	
					GUILayout.Label (" "); 
					GUILayout.Label ("Score " + Score.ToString ());	
					
					bool tmpStopperflag = stopperFlag;
					stopperFlag = Scene_00_GUI.Toggle(stopperFlag,"Stopper");
					if(tmpStopperflag != stopperFlag)
					{	
						Debug.Log("StopperFlag = " + stopperFlag.ToString());
						GameObject[] stoppers = GameObject.FindGameObjectsWithTag("Stopper");
						foreach(GameObject stopper in stoppers){
							Renderer sr = stopper.GetComponent<Renderer>();
							if(sr != null){
								sr.enabled = stopperFlag;
							}
							Collider sc = stopper.GetComponent<Collider>();
							if(sc != null){
								sc.enabled = stopperFlag;
							}
						}
					}
					GUILayout.EndVertical();
				
					if(stopperFlag){
					GUILayout.BeginHorizontal();
					if(gamePauseFlag)
					{
						if(Scene_00_GUI.Button("Resume",GUILayout.ExpandWidth(false))){
							GameObject[] balls = GameObject.FindGameObjectsWithTag("Ball");
							foreach(GameObject ball in balls){
								Rigidbody rb = ball.GetComponent<Rigidbody>();
								if(rb != null){
									rb.velocity = new Vector3(0,0,0);
									rb.useGravity = true;
								}
							}
							gamePauseFlag = !gamePauseFlag;
							sm.Resume();
						}
					} else {
						if(Scene_00_GUI.Button("Pause",GUILayout.ExpandWidth(false))){
							GameObject[] balls = GameObject.FindGameObjectsWithTag("Ball");
							foreach(GameObject ball in balls){
								Rigidbody rb = ball.GetComponent<Rigidbody>();
								if(rb != null){
									rb.velocity = new Vector3(0,0,0);
									rb.useGravity = false;
								}
							}						
							gamePauseFlag = !gamePauseFlag;
							sm.Pause();
						}
					}
					GUILayout.Label (" "); 
					GUILayout.EndHorizontal();
				
					if(Scene_00_GUI.Button("Multi Ball",GUILayout.ExpandWidth(false))){
						GameObject ball = GameObject.FindGameObjectWithTag("Ball");	
						GameObject respawn = GameObject.FindGameObjectWithTag("Respawn");	
						if(ball != null && respawn != null){
							GameObject.Instantiate(ball,respawn.transform.position,respawn.transform.rotation);		
						}
					}
					
					/* Adjust the game speed */
					//gameSpeed = GUILayout.HorizontalSlider(gameSpeed,0.5f,2.0f);
					//if(GUI.changed){
					//	Time.timeScale = gameSpeed;
					//}
					
					//GUI.contentColor = Color.blue;
					GUILayout.Label("AISAC \n"+sm.lastVelocity.ToString("F02"));
				}
				}
				GUILayout.EndArea();
			} break;
			case EnumGameState.Gameover:
			{		
				if(Scene_00_GUI.Button(new Rect(Screen.width/2-100,Screen.height/4-50,200,100),"Game Over!")){
					ChangeLevel(EnumGameState.Title);
				}
			}
			break;
		}

		Scene_00_GUI.EndGui();
	}
	
	public void AddScore (int s)
	{
		Score += s;	
	}
	
	public void ChangeLevel(EnumGameState newLevel)
	{
		level = newLevel;
		lastLevelChangeTime = Time.timeSinceLevelLoad;
		
		switch(level)
		{
			case EnumGameState.Main:
	
			BallCount = 3;
		
			//	all bumper start
			GameObject[] bumpers = GameObject.FindGameObjectsWithTag("Bumper");
			foreach(GameObject bumperGo in bumpers){
				Bumper bumper = bumperGo.GetComponent<Bumper>();
				if(bumper != null){
					bumper.BumperEnableFlag = true;
				} else {
					Paddle paddle = bumperGo.GetComponent<Paddle>();
					paddle.PaddleEnableFlag = true;
				}
			}
			break;
			
			
		}
		
	}
	
	float lastLevelChangeTime = 0.0f;
	public void AddDeathCount()
	{
		BallCount -=1;	
		if(BallCount < 0){
			ChangeLevel(EnumGameState.Gameover);
			sm.PlayGameOver();	
			
			//	all bumper stop
			GameObject[] bumpers = GameObject.FindGameObjectsWithTag("Bumper");
			foreach(GameObject bumperGo in bumpers){
				Bumper bumper = bumperGo.GetComponent<Bumper>();
				if(bumper != null){
					bumper.BumperEnableFlag = false;
				} else {
					Paddle paddle = bumperGo.GetComponent<Paddle>();
					paddle.PaddleEnableFlag = false;
				}
			}
		}
		
		sm.PlayBGM();
	}
}
