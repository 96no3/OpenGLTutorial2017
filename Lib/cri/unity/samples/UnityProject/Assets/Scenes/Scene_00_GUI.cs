/****************************************************************************
 *
 * Copyright (c) 2015 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using System.Collections;
using System.Collections.Generic;


public class Scene_00_GUI : MonoBehaviour {
#if UNITY_EDITOR || UNITY_STANDALONE_WIN  || UNITY_STANDALONE_OSX
	const string  yAxisName   = "Vertical";
	const string  xAxisName   = "Horizontal";
	const KeyCode fireKeyCode = KeyCode.Return;
    const KeyCode jumpKeyCode = KeyCode.J;
#elif UNITY_TVOS 
	const string  yAxisName   = "Vertical";
	const string  xAxisName   = "Horizontal";
	const KeyCode fireKeyCode = KeyCode.Joystick1Button14;
#else
	const string  yAxisName   = "Vertical";
	const string  xAxisName   = "Horizontal";
	const KeyCode fireKeyCode = KeyCode.Joystick1Button1;
    const KeyCode jumpKeyCode = KeyCode.Joystick1Button0;
#endif

#if UNITY_EDITOR || UNITY_STANDALONE_WIN  || UNITY_STANDALONE_OSX
    public static string jumpButtonName = "J Key";
#elif UNITY_XBOXONE
    public static string jumpButtonName = "A Button";
#elif UNITY_PS4
    public static string jumpButtonName = "X Button";
#elif UNITY_IOS || UNITY_ANDROID
    public static string jumpButtonName = "Toggle Button";
#else 
    public static string jumpButtonName = "Joystick1Button0";
#endif

    private static Scene_00_GUI _instance;
	private static Scene_00_GUI instance {
		get {
			if (_instance == null) {
				var go = new GameObject("Scene_00_GUI");
				return go.AddComponent<Scene_00_GUI>();
			}
			return _instance;
		}
	}

	List<string> controlNames = new List<string>();
	string controlPrefix      = null;
	int    controlIndex       = 0;
	int    forcusIndex        = 0;
	string focusedControlName = "";
	int    sliderDisplacement = 0;
	bool   fired              = false;
    bool   jumped             = false;
	bool   yAxisDowned        = false;
	bool   xAxisDowned        = false;
	bool   virtualPadExist	  = false;
	
	void Awake()
	{
		if (_instance != null) {
			Destroy (this);
			return;
		}
		_instance = this;


		if (GameObject.Find ("SampleIO") != null) {
			virtualPadExist = true;
		}
	}
	
	void OnDestroy ()
	{
		if (_instance != this) {
			return;
		}
		_instance = null;
	}

	void Update ()
	{
		// default values
		float yAxis = 0.0f;
		float xAxis = 0.0f;
		fired = false;

		// virtual pad event mapping
		if (virtualPadExist) {
			SampleIO.Key virtualPadKey = SampleIO.GetPushedKey ();
			switch (virtualPadKey) {
			case SampleIO.Key.A:
				fired = true;
				break;
			case SampleIO.Key.B:
				jumped = true;
				break;
			case SampleIO.Key.Up:
				yAxis = 1.0f;
				break;
			case SampleIO.Key.Down:
				yAxis = -1.0f;
				break;
			case SampleIO.Key.Left:
				xAxis = -1.0f;
				break;
			case SampleIO.Key.Right:
				xAxis = 1.0f;
				break;
			}
		}

		// Fire (click focused button)
		fired |= Input.GetKeyDown(fireKeyCode);
        jumped|= Input.GetKeyDown(jumpKeyCode);

        // Up/Down (change focus)
        {
			yAxis += Input.GetAxisRaw(yAxisName);

			if (Mathf.Abs(yAxis) > 0.1f) {
				if (!yAxisDowned) {
					if (yAxis > 0.0f) {
						--forcusIndex;
					} else if (yAxis < -0.1f) {
						++forcusIndex;
					}
				}
				forcusIndex = Mathf.Max(0, Mathf.Min(controlNames.Count - 1, forcusIndex));
				focusedControlName = (controlNames.Count > 0) ? controlNames[forcusIndex] : "";
				yAxisDowned = true;
			} else {
				yAxisDowned = false;
			}
		}

		// Left/Right (move focused slider)
		{
			xAxis += Input.GetAxisRaw(xAxisName);

			if (Mathf.Abs(xAxis) > 0.1f) {
				if (!xAxisDowned) {
					sliderDisplacement = (xAxis > 0.0f) ? 1 : -1;
				} else {
					sliderDisplacement = 0;
				}
				xAxisDowned = true;
			} else {
				sliderDisplacement = 0;
				xAxisDowned = false;
			}
		}

		// Set default focus
		if (focusedControlName.Length == 0) {
			if (controlNames.Count == 1) {
				focusedControlName = controlNames[0];
			} else {
				focusedControlName = (controlNames.Count >= 2) ? controlNames[forcusIndex] : "";
			}
		}
		controlNames.Clear();
	}
	
	private string GenCurrentControlName()
	{
		return string.Format("{0}/{1:0000}", controlPrefix, controlIndex);
	}

	private void AddControlName(string controlName)
	{
		int index = controlNames.BinarySearch(controlName);
		if (index < 0) {
			controlNames.Insert(~index, controlName);
		}
	}

	static public void BeginGui(string controlPrefix)
	{
		instance.controlPrefix = controlPrefix;
		instance.controlIndex  = 0;
		GUI.FocusControl(instance.focusedControlName);
	}

	static public void EndGui()
	{
		instance.controlPrefix = null;
		instance.controlIndex  = 0;
	}
	
	static public bool Toggle(bool value, string text, params GUILayoutOption[] options)
	{
		if (GUI.enabled) {
			string controlName = instance.GenCurrentControlName();
			GUI.SetNextControlName(controlName);
			bool result = value;
			if ((instance.fired && (GUI.GetNameOfFocusedControl() == controlName))||(instance.jumped)) {
				result = !result;
				instance.fired  = false;
                instance.jumped = false;
			}
			result = GUILayout.Toggle(result, text, options);
			++(instance.controlIndex);
			instance.AddControlName(controlName);
			return result;
		} else {
			GUILayout.Toggle(value, text, options);
			return false;
		}
	}

    static public bool Button(GUIContent content, GUIStyle style, params GUILayoutOption[] options)
	{
		if (GUI.enabled) {
			string controlName = instance.GenCurrentControlName();
			GUI.SetNextControlName(controlName);
			bool result = false;
			if (GUI.GetNameOfFocusedControl() == controlName) {
				result = instance.fired;
				instance.fired = false;
			}
			result |= GUILayout.Button(content, style, options);
			++(instance.controlIndex);
			instance.AddControlName(controlName);
			return result;
		} else {
			GUILayout.Button(content, style, options);
			return false;
		}
	}
	
	static public bool Button(string text, params GUILayoutOption[] options)
	{
		if (GUI.enabled) {
			string controlName = instance.GenCurrentControlName();
			GUI.SetNextControlName(controlName);
			bool result = false;
			if (GUI.GetNameOfFocusedControl() == controlName) {
				result = instance.fired;
				instance.fired = false;
			}
			result |= GUILayout.Button(text, options);
			++(instance.controlIndex);
			instance.AddControlName(controlName);
			return result;
		} else {
			GUILayout.Button(text, options);
			return false;
		}
	}
	
	static public bool Button(Rect position, string text)
	{
		if (GUI.enabled) {
			string controlName = instance.GenCurrentControlName();
			GUI.SetNextControlName(controlName);
			bool result = false;
			if (GUI.GetNameOfFocusedControl() == controlName) {
				result = instance.fired;
				instance.fired = false;
			}
			result |= GUI.Button(position, text);
			++(instance.controlIndex);
			instance.AddControlName(controlName);
			return result;
		} else {
			GUI.Button(position, text);
			return false;
		}
	}

	static public float HorizontalSlider(float value, float leftValue, float rightValue, params GUILayoutOption[] options)
	{
		float result;
		if (GUI.enabled) {
			string controlName = instance.GenCurrentControlName();
			GUI.SetNextControlName(controlName);
			if (GUI.GetNameOfFocusedControl() == controlName) {
				if (instance.sliderDisplacement != 0) {
				    value += instance.sliderDisplacement * (rightValue - leftValue) / 20.0f;
				    instance.sliderDisplacement = 0;
				    GUI.changed = true;
                }
			}
			result = GUILayout.HorizontalSlider(value, leftValue, rightValue, options);
			++(instance.controlIndex);
			instance.AddControlName(controlName);
		} else {
			result = GUILayout.HorizontalSlider(value, leftValue, rightValue, options);
		}
		return result;
	}

	static public float HorizontalSlider(Rect position, float value, float leftValue, float rightValue)
	{
		float result;
		if (GUI.enabled) {
			string controlName = instance.GenCurrentControlName();
			GUI.SetNextControlName(controlName);
			if (GUI.GetNameOfFocusedControl() == controlName) {
				if (instance.sliderDisplacement != 0) {
				    value += instance.sliderDisplacement * (rightValue - leftValue) / 20.0f;
				    instance.sliderDisplacement = 0;
				    GUI.changed = true;
                }
			}
			result = GUI.HorizontalSlider(position, value, leftValue, rightValue);
			++(instance.controlIndex);
			instance.AddControlName(controlName);
		} else {
			result = GUI.HorizontalSlider(position, value, leftValue, rightValue);
		}
		return result;
	}

    static public float HorizontalSliderButton(float value, float leftValue, float rightValue, float delta, params GUILayoutOption[] options)
    {
        GUILayout.BeginHorizontal();
        if (Scene_00_GUI.Button("-", options))
        {
            value = Mathf.Max(value - delta, leftValue);
        }
        if (Scene_00_GUI.Button("+", options))
        {
            value = Mathf.Min(value + delta, rightValue);
        }
        GUILayout.EndHorizontal();
        return value;
    }

    static public float HorizontalSliderButton(Rect position, float value, float leftValue, float rightValue, float delta)
    {
        GUILayout.BeginHorizontal();
        if (Scene_00_GUI.Button(new Rect(position.x, position.y, position.width/2, position.height), "-"))
        {
            value = Mathf.Max(value - delta, leftValue);
        }
        if (Scene_00_GUI.Button(new Rect(position.x + position.width / 2, position.y, position.width / 2, position.height), "+"))
        {
            value = Mathf.Min(value + delta, rightValue);
        }
        GUILayout.EndHorizontal();
        return value;
    }
}
