// Copyright (c) 2018 CRI Middleware Co., Ltd.

using UnrealBuildTool;
using System.Collections.Generic;

public class AtomBasicFeaturesTarget : TargetRules
{
	public AtomBasicFeaturesTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "AtomBasicFeatures" } );
	}
}
