/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Build Settings of CriWareRuntime Module
 * File     : CriWareRuntime.Build.cs
 *
 ****************************************************************************/
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class CriWareRuntime : ModuleRules
	{
		public CriWareRuntime(ReadOnlyTargetRules Target) : base(Target)
        {
			/* 4.15 - remove PCH includes */
			PrivatePCHHeaderFile = "Private/CriWareRuntimePrivatePCH.h";
	
			PrivateIncludePaths.AddRange(new string[]
			{
				"CriWareRuntime/Private",
				"CriWareRuntime/Private/Sections",
				"CriWareRuntime/Private/Tracks",
			});
	
			PublicDependencyModuleNames.AddRange(
				new string[] {
					"Core",
					"CoreUObject",
					"Engine",
					"CriWare",
					"RHI",
					"RenderCore",
					"ShaderCore",
					"SlateCore",
					"MovieScene",
					"HeadMountedDisplay",
				}
			);
	
			PublicIncludePaths.AddRange(
				new string[] {
					"Runtime/CriWareRuntime/Public",
					"Runtime/CriWareRuntime/Classes",
				}
			);
	
			string engine_path = System.IO.Path.GetFullPath(BuildConfiguration.RelativeEnginePath);
	
			PrivateIncludePaths.AddRange(
				new string[] {
					engine_path + "Source/Runtime/Engine/Private",
				}
			);
	
			if ((Target.Platform == UnrealTargetPlatform.Win32)
				|| (Target.Platform == UnrealTargetPlatform.Win64)) {
				if (WindowsPlatform.Compiler == WindowsCompiler.VisualStudio2015) {
					PublicAdditionalLibraries.Add("legacy_stdio_definitions.lib");
				}
			}
	
	        	        /* Intel Media SDK */
	        if (Target.Platform == UnrealTargetPlatform.Win64 ||
                Target.Platform == UnrealTargetPlatform.Win32) {
                string IntelDir = "/../../Binaries/ThirdParty/Intel/";
                string LibName = Target.Platform == UnrealTargetPlatform.Win64 ? "libmfxsw64.dll" : "libmfxsw32.dll";
                string LibPath = IntelDir + Target.Platform.ToString() + "/" + LibName;
                if (File.Exists(ModuleDirectory + LibPath)) {
                    RuntimeDependencies.Add(new RuntimeDependency(ModuleDirectory + LibPath));
					Definitions.Add("CRIWARE_USE_INTEL_MEDIA=1");
                }
            }
	
	        if (UEBuildConfiguration.bBuildEditor == true) {
				PublicDependencyModuleNames.Add("UnrealEd");
				PublicDependencyModuleNames.Add("Matinee");
			}

            UnrealTargetPlatform TargetPlatformValue;
            bool bSupported = ((Target.Platform == UnrealTargetPlatform.Win32)
				|| (Target.Platform == UnrealTargetPlatform.Win64)
				|| (Target.Platform == UnrealTargetPlatform.Mac)
				|| (Target.Platform == UnrealTargetPlatform.IOS)
				|| (Target.Platform == UnrealTargetPlatform.Linux)
				|| (Target.Platform == UnrealTargetPlatform.Android)
                /* MICROSOFT_UWP_UNREAL - UWP support */
                || (System.Enum.TryParse("UWP32", out TargetPlatformValue) && Target.Platform == TargetPlatformValue)
                || (System.Enum.TryParse("UWP64", out TargetPlatformValue) && Target.Platform == TargetPlatformValue));
			Definitions.Add(string.Format("WITH_CRIWARE={0}", bSupported ? 1 : 0));
            Definitions.Add(string.Format("CRIWARE_UE4_LE=1"));
        }
    }
}