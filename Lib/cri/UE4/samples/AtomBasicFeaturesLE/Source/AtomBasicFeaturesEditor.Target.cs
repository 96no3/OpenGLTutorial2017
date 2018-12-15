// Copyright (c) 2018 CRI Middleware Co., Ltd.

using UnrealBuildTool;
using System.Collections.Generic;

public class AtomBasicFeaturesEditorTarget : TargetRules
{
	public AtomBasicFeaturesEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "AtomBasicFeatures" } );
	}
}
