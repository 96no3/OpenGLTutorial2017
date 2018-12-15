/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*JP
 * 本サンプルは、音声パラメータのコントロールを行います。
 *
 * ボリュームとピッチ、８つのバスセンドレベルをスライダーにより変化することができ、
 * リアルタイムに音色が変化します。
 * 
 * 各バスには以下のバスエフェクトが設定されています。
 * 
 * BUS0:エフェクト無し（メインバス）
 * BUS1:リバーブ
 * BUS2:ピッチシフタ
 * BUS3:エコー＋サラウンダ
 * BUS4:コンプレッサ
 * BUS5:コーラス
 * BUS6:バイクアッド（ローパス）
 * BUS7:ディストーション
 * 
 */
/*EN
 * This sample performs the control of sound parameters.
 *
 * Volume, pitch, and eight bus send levels can be changed by the slider,
 * and the sound tone changes in real-time.
 * 
 * The following bus effect is set up on each bus:
 * 
 * BUS0: no effect (main bus)
 * BUS1: reverb
 * BUS2: pitch shifter
 * BUS3: echo + surrounder
 * BUS4: compressor
 * BUS5: chorus
 * BUS6: biquad (low-pass)
 * BUS7: distortion
 * 
 */
using UnityEngine;
using System.Collections;

public class Scene_04_ControlParameter : MonoBehaviour
{
	#region Variables
	/* Object that the component is assinged */
	public GameObject atomCompObject;
	/* Maximum number of bussend */
	private const int maxBus = 8;
	/* Volume */
	private float volumeValue = 1.0f;
	/* Pitch */
	private float pitchValue = 0.0f;
	/* Bus send 0-7 */
	private float[] busSendlevelValue = new float[maxBus];
	/* Description of bus effect */
	private string[] busEffectType = new string[maxBus] {
		" No Effect",
		" Reverb",
		" Pitch Shifter",
		" Echo+Surrounder",
		" Compressor",
		" Chorus",
		" Biquad Low pass",
		" Distortion"
	};
	#endregion

	#region Functions
	void OnEnable()
	{
		/* Initialize parameters */
		ResetAtomSourceParameter();
		
		/* Get volume and pitch that are specified on the Inspector. */
		CriAtomSource atom_source = gameObject.GetComponent<CriAtomSource>();
		this.volumeValue = atom_source.volume;
		this.pitchValue = atom_source.pitch;
	}
	
	void ResetAtomSourceParameter()
	{
		this.volumeValue = 1.0f;
		this.pitchValue = 0.0f;
		/* Bus 0, the main bus, outputs 1.0f by default */
		this.busSendlevelValue[0] = 1.0f;
		
		/* Bus 1-7, which is mostly used for bus effect, outputs 0.0f by Default. */
		for (int i = 1; i < 8; i++) {
			this.busSendlevelValue[i] = 0.0f;
		}
	}

	void Update()
	{
		Ray ray = Camera.main.ScreenPointToRay(Input.mousePosition);
		if (Input.GetButtonDown("Fire1") == true) {
			/* By clicking on the Cube on the screen, a sound is played. */
			RaycastHit hit;
			if (Physics.Raycast(ray, out hit, 100)) {
				GameObject selectedGameObject = hit.collider.gameObject;
				if (selectedGameObject != null) {
					/* Playback with the Cue name that is assigned to AtomSource component. */
					selectedGameObject.GetComponent<CriAtomSource>().Play();
				}
			}
		}
	}
	
	void OnGUI()
	{
#if UNITY_WEBGL
		if (CriAtom.CueSheetsAreLoading) {
			return;
		}
#endif
		
		if (Scene_00_SampleList.ShowList == true) {
			return;
		}
		
		Scene_00_GUI.BeginGui("01/SampleMain");

		/* Set UI skin. */
		GUI.skin = Scene_00_SampleList.uiSkin;
		
		bool on_reset;
		GUILayout.BeginArea(new Rect(12, 70, 820, 380), "", Scene_00_SampleList.TextStyle);
		GUILayout.BeginHorizontal();		
		{
			GUILayout.Label("Atom Source Parameters");
			if (Scene_00_GUI.Button("Play")) {
				/* Playback with the Cue name that is assigned to AtomSource component. */
				this.atomCompObject.GetComponent<CriAtomSource>().Play();
			}
			if (Scene_00_GUI.Button("Stop")) {
				/* Stop the playback. */
				this.atomCompObject.GetComponent<CriAtomSource>().Stop();
			}
			on_reset = Scene_00_GUI.Button("Reset Paremeters");
			if (on_reset== true) {
				/* Initialize parameters by pressing the GUI reset button. */
				ResetAtomSourceParameter();
			}
		}
		GUILayout.EndHorizontal();		
		GUILayout.BeginHorizontal();		
		{
			GUILayout.BeginVertical();
			/* Volume control GUI */
			GUILayout.Label ("Volume " + this.volumeValue.ToString("0.00"));
            this.volumeValue = Scene_00_GUI.HorizontalSliderButton(this.volumeValue, 0.0f, 1.0f, 0.2f);
            GUILayout.EndVertical();
	
			GUILayout.BeginVertical();
			/* Pitch control GUI */
			GUILayout.Label("Pitch " + this.pitchValue.ToString("0.00"));
            this.pitchValue = Scene_00_GUI.HorizontalSliderButton(this.pitchValue, -1200.0f, 1200.0f, 100.0f);
            GUILayout.EndVertical();
		}
		GUILayout.EndHorizontal();
		GUILayout.Label ("-- Bus Send Level --");

		/* Bus Send Level control GUI */
		GUILayout.BeginHorizontal();
		{
			GUILayout.BeginVertical();
			for (int i = 0; i < maxBus / 2; i++) {
				GUILayout.Label ("[" + i.ToString() + "] " + this.busSendlevelValue[i].ToString("0.00") + busEffectType[i]);
                this.busSendlevelValue[i] = Scene_00_GUI.HorizontalSliderButton(busSendlevelValue[i], 0.0f, 1.0f, 0.2f, GUILayout.Height(720/maxBus*0.18f));
            }
            GUILayout.EndVertical();
			GUILayout.BeginVertical();
			GUILayout.Space(12);
			GUILayout.EndVertical();
			GUILayout.BeginVertical();
			for (int i =  maxBus / 2; i < maxBus; i++) {
				GUILayout.Label ("[" + i.ToString() + "] " + this.busSendlevelValue[i].ToString("0.00") + busEffectType[i]);
                this.busSendlevelValue[i] = Scene_00_GUI.HorizontalSliderButton(busSendlevelValue[i], 0.0f, 1.0f, 0.2f, GUILayout.Height(720 / maxBus * 0.18f));
            }
            GUILayout.EndVertical();
		}
		GUILayout.EndHorizontal();
		GUILayout.Space(12);

		
		GUILayout.EndArea();


		/* When the GUI slider changes, the playback parameter on the AtomSource also changes. */
		/* Get the AtomSource component that is attached to the game object. */
		CriAtomSource atom_source = gameObject.GetComponent<CriAtomSource>();

		/* Update the volume and the pitch. */
		/* If a pramater exists on the Inspector side, control it on the Inspector side. */
		atom_source.volume = this.volumeValue;
		atom_source.pitch = this.pitchValue;

		/* Update the bus send level. */
		/* A parameter that does not exist on the Inspector side, control it by functions. */
		/* Bus 0 is 1.0f by default, and multiplication is applied. */
		atom_source.SetBusSendLevel(0, this.busSendlevelValue[0]);
		for (int i = 1; i < maxBus; i++) {
			/* Bus 1-7 is 0.0f by default, and addition is applied.(specifying offset) */
			atom_source.SetBusSendLevelOffset(i, this.busSendlevelValue[i]);
		}

		Scene_00_GUI.EndGui();
	}
	#endregion
}

/* end of file */
