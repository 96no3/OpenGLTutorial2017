/****************************************************************************
 *
 * Copyright (c) 2015 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using UnityEngine;
using UnityEditor;
using UnityEditor.Callbacks;
using System;
using System.IO;
#if (UNITY_5 || UNITY_2017_1_OR_NEWER) && (UNITY_IOS || UNITY_TVOS)
using UnityEditor.iOS.Xcode;
#endif

public class CriWareBuildPostprocessor : ScriptableObject
{
	private static string prefsFilePath = "Assets/Editor/CriWare/CriWareBuildPostprocessorPrefs.asset";
	public bool iosAddDependencyFrameworks	= true;

	private const string SofdecWorkerFilename = "sofdec2.worker.js";
	private const string SrcDirWebGL = "Assets/Plugins/WebGL/";
	private const string DstDirStreamingAssets = "/StreamingAssets/";

	[MenuItem("GameObject/CRIWARE/Create CriWareBuildPostprocessorPrefs.asset")]
	public static void Create()
	{
		CriWareBuildPostprocessor instance = (CriWareBuildPostprocessor)AssetDatabase.LoadAssetAtPath(prefsFilePath, typeof(CriWareBuildPostprocessor));
		if (instance) {
			Debug.LogError("[CRIWARE] Preferences file of CriWareBuildPostprocessor already exists. '(" + prefsFilePath + ")");
			Selection.activeObject = instance;
			return;
		}
		
		var scobj = ScriptableObject.CreateInstance<CriWareBuildPostprocessor>();
		if (scobj == null) {
			Debug.Log("[CRIWARE] Failed to create CriWareBuildPostprocessor");
			return;
		}
		
		Directory.CreateDirectory(Path.GetDirectoryName(prefsFilePath));
		AssetDatabase.CreateAsset(scobj, prefsFilePath);
		
		AssetDatabase.SaveAssets();
		AssetDatabase.Refresh();
		
		Debug.Log("[CRIWARE] Created the preferences file of CriWareBuildPostprocessor. (" + prefsFilePath + ")");

		Selection.activeObject = scobj;
	}
	
	
	[PostProcessScene]
	public static void OnPostProcessScene() {
		CheckGraphicsApiProcess ();
	}
	
	
	[PostProcessBuild]
	public static void OnPostprocessBuild(BuildTarget build_target, string path)
	{
		CriWareBuildPostprocessor instance = (CriWareBuildPostprocessor)AssetDatabase.LoadAssetAtPath(prefsFilePath, typeof(CriWareBuildPostprocessor));
		if (instance == null) {
			instance = new CriWareBuildPostprocessor();
			Debug.Log(
				"[CRIWARE] Run CriWareBuildPostprocessor.OnPostprocessBuild with default preferences.\n"
				+ "If you want to change the preferences, please create the preferences file by 'CRI/Create CriWareBuildPostprocessorPrefs.asset' menu."
				);
		} else {
			Debug.Log(
				"[CRIWARE] Run CriWareBuildPostprocessor.OnPostprocessBuild with default preferences.\n"
				+ "If you want to change the preferences, please edit the preferences file (" + prefsFilePath + ")"
				);
		}

		if (instance.iosAddDependencyFrameworks) {
			instance.IosAddDependencyFrameworksProcess(build_target, path);
		}
		
		switch (build_target) {
			case BuildTarget.WebGL:
				instance.PostprocessBuildWebGL(path);
				break;
		}
	}
	
	
	private void IosAddDependencyFrameworksProcess(BuildTarget build_target, string path)
	{
#if UNITY_IOS
#if UNITY_5 || UNITY_2017_1_OR_NEWER
		if (build_target != BuildTarget.iOS) {
			return;
		}
		string project_path = path + "/Unity-iPhone.xcodeproj/project.pbxproj";
		PBXProject project = new PBXProject();
		project.ReadFromString(File.ReadAllText(project_path));
		string target = project.TargetGuidByName("Unity-iPhone");
		
		Debug.Log("[CRIWARE][iOS] Add dependency frameworks (VideoToolbox.framework, Metal.framework)");
		project.AddFrameworkToProject(target, "VideoToolbox.framework", false);
		project.AddFrameworkToProject(target, "Metal.framework", false);
		
		File.WriteAllText(project_path, project.WriteToString());
#else
		if (build_target != BuildTarget.iPhone) {
			return;
		}
		Debug.LogWarning("[CRIWARE][iOS] Please add dependency frameworks (VideoToolbox.framework, Metal.framework) on Xcode.");
#endif
#endif
#if UNITY_TVOS
		if (build_target != BuildTarget.tvOS) {
			return;
		}
		string project_path = path + "/Unity-iPhone.xcodeproj/project.pbxproj";
		PBXProject project = new PBXProject();
		project.ReadFromString(File.ReadAllText(project_path));
		string target = project.TargetGuidByName("Unity-iPhone");

		Debug.Log("[CRIWARE][iOS] Add dependency framework (Metal.framework)");
		//project.AddFrameworkToProject(target, "VideoToolbox.framework", false);
		project.AddFrameworkToProject(target, "Metal.framework", false);

		File.WriteAllText(project_path, project.WriteToString());
#endif
	}
	
	
	private static bool CheckGraphicsApiProcess() {
		return true;
	}


	private void PostprocessBuildWebGL(string path)
	{
		string dstDirPath = path + DstDirStreamingAssets;
		string dstFilePath = dstDirPath + SofdecWorkerFilename;
		string srcFilePath = Path.ChangeExtension(SrcDirWebGL + SofdecWorkerFilename, ".bin");
		bool ret = true;
		try {
			Directory.CreateDirectory(dstDirPath);
			File.Copy(srcFilePath, dstFilePath, true);
		}
		catch (System.Exception) {
			Debug.Log("[CRIWARE][WebGL] copy to " + dstFilePath + " failed.");
			ret = false;
		}
		if (ret) {
			Debug.Log("[CRIWARE][WebGL] copy to " + dstFilePath + " succeeded.");
		}
	}
}

