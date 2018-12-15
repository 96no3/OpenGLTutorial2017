/****************************************************************************
 *
 * CRI Middleware SDK
 *
 * Copyright (c) 2013-2017 CRI Middleware Co., Ltd.
 *
 * Library  : CRIWARE plugin for Unreal Engine 4
 * Module   : Build Settings of CriWare Module
 * File     : CriWare.Build.cs
 *
 ****************************************************************************/
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class CriWare : ModuleRules
	{
		public CriWare(ReadOnlyTargetRules Target) : base(Target)
        {
			Type = ModuleType.External;
	
	        string CriWarePath = Path.Combine(ModuleDirectory, "cri/");
	
	        if (Target.Platform == UnrealTargetPlatform.Win64) {

				PublicIncludePaths.Add(CriWarePath + "pc/include");
                PublicAdditionalLibraries.AddRange(
                    new string[] {
                            CriWarePath + "pc/libs/x64/cri_ware_pcx64_le_import.lib"
                    }
                );
                PublicDelayLoadDLLs.Add("cri_ware_pcx64_le.dll");
                PrivateDependencyModuleNames.AddRange(new string[] { "CriWareRuntime" });
                RuntimeDependencies.Add(new RuntimeDependency(CriWarePath + "pc/libs/x64/cri_ware_pcx64_le.dll"));
            }
            else if (Target.Platform == UnrealTargetPlatform.Win32) {

				PublicIncludePaths.Add(CriWarePath + "pc/include");
                RuntimeDependencies.Add(new RuntimeDependency(CriWarePath + "pc/libs/x86/cri_ware_pcx86_le.dll"));

            } else if (Target.Platform == UnrealTargetPlatform.Mac) {
				PublicFrameworks.AddRange(new string[] { "CoreAudio", "AudioToolbox", "AudioUnit" });
				PublicIncludePaths.Add(CriWarePath + "macosx/include");
				PublicLibraryPaths.Add(CriWarePath + "macosx/libs");
				string LibraryPath = CriWarePath + "macosx/libs/";
				if (Target.Configuration == UnrealTargetConfiguration.Shipping) {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							LibraryPath + "libcri_base_macosx.a",
							LibraryPath + "libcri_file_system_macosx.a",
							LibraryPath + "libcri_atom_macosx.a"
						}
					);
				} else if (Target.Configuration == UnrealTargetConfiguration.Debug) {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							LibraryPath + "libcri_base_macosxD.a",
							LibraryPath + "libcri_file_system_macosxD.a",
							LibraryPath + "libcri_atom_monitor_macosxD.a"
						}
					);
				} else {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							LibraryPath + "libcri_base_macosx.a",
							LibraryPath + "libcri_file_system_macosx.a",
							LibraryPath + "libcri_atom_monitor_macosx.a"
						}
					);
				}
			} else if (Target.Platform == UnrealTargetPlatform.IOS) {
				PublicFrameworks.AddRange(new string[] { "CoreAudio", "AudioToolbox" });
				PublicIncludePaths.Add(CriWarePath + "ios/include");
				PublicLibraryPaths.Add(CriWarePath + "ios/libs");
				string LibraryPath = CriWarePath + "ios/libs/";
				if (Target.Configuration == UnrealTargetConfiguration.Shipping) {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							LibraryPath + "libcri_base_ios.a",
							LibraryPath + "libcri_file_system_ios.a",
							LibraryPath + "libcri_atom_ios.a"
						}
					);
				} else if (Target.Configuration == UnrealTargetConfiguration.Debug) {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							LibraryPath + "libcri_base_iosD.a",
							LibraryPath + "libcri_file_system_iosD.a",
							LibraryPath + "libcri_atom_monitor_iosD.a"
						}
					);
				} else {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							LibraryPath + "libcri_base_ios.a",
							LibraryPath + "libcri_file_system_ios.a",
							LibraryPath + "libcri_atom_monitor_ios.a"
						}
					);
				}
			} else if (Target.Platform == UnrealTargetPlatform.Linux) {
	            PublicIncludePaths.Add(CriWarePath + "linux/include");
	            string LibraryPath = CriWarePath + "linux/libs/x86_64-unknown-linux-gnu/";
	            if (Target.Configuration == UnrealTargetConfiguration.Shipping) {
	                PublicAdditionalLibraries.AddRange(
	                    new string[] {
	                        LibraryPath + "libcri_base.a",
	                        LibraryPath + "libcri_file_system.a",
	                        LibraryPath + "libcri_atom.a",
	                        "asound",
	                        "stdc++",
	                        "m",
	                        "gcc_s",
	                        "gcc",
	                        "c",
	                    }
	                );
	            } else if (Target.Configuration == UnrealTargetConfiguration.Debug) {
	                PublicAdditionalLibraries.AddRange(
	                    new string[] {
	                        LibraryPath + "libcri_baseD.a",
	                        LibraryPath + "libcri_file_systemD.a",
	                        LibraryPath + "libcri_atomD.a",
	                        "asound",
	                        "stdc++",
	                        "m",
	                        "gcc_s",
	                        "gcc",
	                        "c",
	                    }
	                );
	            } else {
	                PublicAdditionalLibraries.AddRange(
	                    new string[] {
	                        LibraryPath + "libcri_base.a",
	                        LibraryPath + "libcri_file_system.a",
	                        LibraryPath + "libcri_atom.a",
	                        "asound",
	                        "stdc++",
	                        "m",
	                        "gcc_s",
	                        "gcc",
	                        "c",
	                    }
	                );
	            }
	        } else if (Target.Platform == UnrealTargetPlatform.Android) {
				PublicIncludePaths.Add(CriWarePath + "android/include");
				string LibraryPath = CriWarePath + "android/libs/";
				PublicLibraryPaths.Add(LibraryPath + "armeabi-v7a");
				PublicLibraryPaths.Add(LibraryPath + "x86");
				if (Target.Configuration == UnrealTargetConfiguration.Shipping) {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							"cri_base_android",
							"cri_file_system_android",
							"cri_atom_android"
						}
					);
				} else if (Target.Configuration == UnrealTargetConfiguration.Debug) {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							"cri_base_androidD",
							"cri_file_system_androidD",
							"cri_atom_monitor_androidD"
						}
					);
				} else {
					PublicAdditionalLibraries.AddRange(
						new string[] {
							"cri_base_android",
							"cri_file_system_android",
							"cri_atom_monitor_android"
						}
					);
				}
			} else {
                UnrealTargetPlatform TargetPlatformValue;

                /* MICROSOFT_UWP_UNREAL - UWP support */
                if (System.Enum.TryParse("UWP32", out TargetPlatformValue) && Target.Platform == TargetPlatformValue)
                {
                    PublicIncludePaths.Add(CriWarePath + "uwp/include");
                    PublicLibraryPaths.Add(CriWarePath + "uwp/libs/x86");
                    if (Target.Configuration == UnrealTargetConfiguration.Shipping)
                    {
                        PublicAdditionalLibraries.AddRange(
                            new string[] {
                        "cri_base_winrt_x86.lib",
                        "cri_file_system_winrt_x86.lib",
                        "cri_atom_winrt_x86.lib"
                            }
                        );
                    }
                    else if (Target.Configuration == UnrealTargetConfiguration.Debug)
                    {
                        PublicAdditionalLibraries.AddRange(
                            new string[] {
                        "cri_base_winrt_x86D.lib",
                        "cri_file_system_winrt_x86D.lib",
                        "cri_atom_monitor_winrt_x86D.lib"
                            }
                        );
                    }
                    else
                    {
                        PublicAdditionalLibraries.AddRange(
                            new string[] {
                        "cri_base_winrt_x86.lib",
                        "cri_file_system_winrt_x86.lib",
                        "cri_atom_monitor_winrt_x86.lib"
                            }
                        );
                    }
                } else if (System.Enum.TryParse("UWP64", out TargetPlatformValue) && Target.Platform == TargetPlatformValue) {
                    PublicIncludePaths.Add(CriWarePath + "uwp/include");
                    PublicLibraryPaths.Add(CriWarePath + "uwp/libs/x64");
                    if (Target.Configuration == UnrealTargetConfiguration.Shipping)
                    {
                        PublicAdditionalLibraries.AddRange(
                            new string[] {
                        "cri_base_winrt_x64.lib",
                        "cri_file_system_winrt_x64.lib",
                        "cri_atom_winrt_x64.lib"
                            }
                        );
                    }
                    else if (Target.Configuration == UnrealTargetConfiguration.Debug)
                    {
                        PublicAdditionalLibraries.AddRange(
                            new string[] {
                        "cri_base_winrt_x64D.lib",
                        "cri_file_system_winrt_x64D.lib",
                        "cri_atom_monitor_winrt_x64D.lib"
                            }
                        );
                    }
                    else
                    {
                        PublicAdditionalLibraries.AddRange(
                            new string[] {
                        "cri_base_winrt_x64.lib",
                        "cri_file_system_winrt_x64.lib",
                        "cri_atom_monitor_winrt_x64.lib"
                            }
                        );
                    }
                }
            }
		}
	}
}