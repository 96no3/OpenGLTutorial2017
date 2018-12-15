/****************************************************************************
 *
 * Copyright (c) 2011 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

/*
 * 本プログラムはサンプルシーンの切り替えを行います。
 * This program switches a sample scene to another scene.
 */

 /*
  * Script Defines
  */
// Use SceneManager from unity 5.3.
#if !(UNITY_4_0 || UNITY_4_0_1 || UNITY_4_1 || UNITY_4_2 || UNITY_4_3 || UNITY_4_5 || UNITY_4_6 || UNITY_4_7 || UNITY_5_0 || UNITY_5_1 || UNITY_5_2)
#define SAMPLELIST_USE_SCENEMANAGER
#endif


using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class Scene_00_SampleList : MonoBehaviour
{
    #region Variables
	public static GUISkin uiSkin;
    public static bool ShowList = false;
    public static bool ShowSampleCategoryList = false;
    public static bool ShowSampleSceneList = false;
    public static GUIStyle TextStyle = null;
	
	Texture2D bgTexture = null;
	bool      enableGui = true;
    string currentSampleCategory = "";

    public static GUIStyle ButtonStyle
    {
        get
        {
            GUIStyle style = new GUIStyle("button");
            style.normal.textColor = Color.white;
            style.margin.left = 16;
            style.alignment = TextAnchor.MiddleLeft;
            return style;
        }
    }

    public static GUIStyle BoxStyle
    {
        get
        {
            GUIStyle style = new GUIStyle("box");
            style.normal.textColor = Color.white;
            style.margin.left = 16;
            style.alignment = TextAnchor.MiddleCenter;
            style.fixedHeight = 120;
			style.fixedWidth = 600;
            return style;
        }
    }
	#endregion

    #region Functions
    /// <summary>
    /// Create a background texture and style.
    /// </summary>
    void InitializeBgTexture()
    {
        if (this.bgTexture == null) {
            this.bgTexture = new Texture2D(128, 128);
            TextStyle = new GUIStyle();
            Color col = new Color(0, 0, 0, 0.7f);
            for (int y = 0; y < this.bgTexture.height; ++y) {
                for (int x = 0; x < this.bgTexture.width; ++x) {
                    this.bgTexture.SetPixel(x, y, col);
                }
            }
            this.bgTexture.Apply();

            TextStyle.normal.textColor = Color.white;
            TextStyle.normal.background = this.bgTexture;
            TextStyle.alignment = TextAnchor.MiddleCenter;
        }
    }
    /// <summary>
    /// Awake this instance.
    /// </summary>
    void Awake()
    {
        ShowSampleCategoryList = false;
        ShowSampleSceneList = false;

		/* Load a custom skin. */
		uiSkin = (GUISkin)Resources.Load("SU3DJPFont/SU3DJPFontSkinMid", typeof(GUISkin));

        /* Create a background texture and style. */
        InitializeBgTexture();
    }

    /// <summary>
    /// Raises the GUI event.
    /// </summary>
    void OnGUI()
	{
		if (!enableGui) {
			return;
		}

		Scene_00_GUI.BeginGui("00/SceneMenu");

		/* Set UI skin. */
		GUI.skin = Scene_00_SampleList.uiSkin;

	#if SAMPLELIST_USE_SCENEMANAGER
		string current_scene_name = UnityEngine.SceneManagement.SceneManager.GetActiveScene().name;
#else
		string current_scene_name = Application.loadedLevelName;
#endif
        GUILayout.Label(Scene_00_SampleListData.Title + " \"" + current_scene_name + "\"");
        ShowSampleCategoryList = Scene_00_GUI.Toggle(ShowSampleCategoryList, "Change to Other Sample: Push " + Scene_00_GUI.jumpButtonName);
        if (ShowSampleCategoryList)
        {
            if (ShowSampleSceneList)
            {
                ShowSampleSceneList = false;
            }
            foreach (string key in Scene_00_SampleListData.SceneDict.Keys)
            {
                if (Scene_00_GUI.Button(new GUIContent(key), ButtonStyle))
                {
                    {
                        currentSampleCategory = key;
                        ShowSampleSceneList = true;
                        ShowSampleCategoryList = false;
                        break;
                    }
                }
            }
        }

        if (ShowSampleSceneList) {
            var sceneList = Scene_00_SampleListData.SceneDict[currentSampleCategory];
            for (int i = 0; i < sceneList.GetLength(0); i++) {
                string sceneName = sceneList[i, 0];
                string sceneDescription = sceneList[i, 1];
                if (Scene_00_GUI.Button(new GUIContent(sceneName, sceneDescription), ButtonStyle)) {
                    // Destroy game objects in the current scene and load a scene.
                    StartCoroutine(DestroyAllGameObjectsAndLoadLevel(sceneName));
                    enableGui = false;
                }
            }
            // Display description of the sample scene.
            GUILayout.Space(8);
            GUILayout.Box(GUI.tooltip, BoxStyle);
        }
        Scene_00_GUI.EndGui();
    }

    void Update() {
        ShowList = ShowSampleCategoryList | ShowSampleSceneList;    
    }

	public IEnumerator DestroyAllGameObjectsAndLoadLevel(string sceneName)
	{
		/* Destroy game objects in the current scene. */
		foreach (GameObject go in GameObject.FindObjectsOfType(typeof(GameObject))) {
			if (go != gameObject) {
				Destroy(go);
			}
		}
		yield return true;
		Destroy(gameObject);
		/* Load a scene. */
	#if SAMPLELIST_USE_SCENEMANAGER
		UnityEngine.SceneManagement.SceneManager.LoadScene(sceneName);
	#else
		Application.LoadLevel(sceneName);
	#endif
	}
    #endregion
}

/* end of file */
