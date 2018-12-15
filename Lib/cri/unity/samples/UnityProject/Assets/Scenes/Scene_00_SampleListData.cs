/****************************************************************************
 *
 * Copyright (c) 2014 CRI Middleware Co., Ltd.
 *
 ****************************************************************************/

using System.Collections.Generic;

public static class Scene_00_SampleListData {
	public const string Title = "< CRI Samples >";
	public static readonly Dictionary<string, string[,]> SceneDict = new Dictionary<string, string[,]>(){
		{"Atom_Basic",
			new string[,] {
				{"Scene_01_SimplePlayback",
					"A simple sample that plays a sound\nby clicking the cube."
				},
				{"Scene_02_3DPosition",
					"This sample demonstrates distance attenuation\nby 3D positioning for the helicopter."
				},
				{"Scene_03_AISAC",
					"This sample demonstrates parameter control\nby AISAC.\nMoving the slider changes the pitch."
				},
				{"Scene_04_ControlParameter",
					"This sample demonstrates parameter control\nby SoundSource.\nMoving the slider changes the tone."
				},
				{"Scene_05_Category",
					"This sample demonstrates parameter control\nby Category.\nMoving the slider changes the volume."
				},
				{"Scene_06_BlockPlayback",
					"This sample demonstrates block playback\nBy changing the block during playback,\nsound status changes."
				},
				{"Scene_07_Ingame_Pinball",
					"A pinball game-like sample.\nWhile sounds are played by script,\nthe In-game preview connection\nfrom CRI Atom Craft is possible."
				},
			}
		},
		{"Atom_Script",
			new string[,] {
				{"ScriptSample01_ClickToPlay",
					"This sample plays a sound\nwhen an object is clicked."
				},
				{"ScriptSample02_SoundTest",
					"This sample displays a button\nfor each Cue in the CueSheet file(ACB file)."
				},
				{"ScriptSample03_TitleScene",
					"This sample cross-fades music\nwhen scenes are switched."
				},
				{"ScriptSample04_TitleScene",
					"This sample can start from any scene\nwhile controlling sounds across scenes."
				},
				{"ScriptSample05_LevelMeter",
					"This sample gets the volume in real time\nand visualizes it by the cube size."
				},
				{"GameSample_Pinball",
					"Pinball game sample project\nfor CRI Atom Craft"
				},
			}
		},
	};
}
