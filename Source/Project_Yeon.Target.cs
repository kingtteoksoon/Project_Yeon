// Copyright Kim seok-hyun, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class Project_YeonTarget : TargetRules
{
	public Project_YeonTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("Project_Yeon");
	}
}
