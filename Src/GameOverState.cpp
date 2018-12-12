/**
* @file GameOver.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/Tutorial/TutorialCueSheet.h"

namespace GameState {

	/**
	* ゲームオーバー画面のコンストラクタ.
	*/
	GameOver::GameOver(Entity::Entity* p) : pSpaceSphere(p) {
		GameEngine& game = GameEngine::Instance();
		game.PlayAudio(AudioPlayerId_BGM, CRI_TUTORIALCUESHEET_GAMEOVER);
	}

	/// ゲームオーバー画面の更新.
	void GameOver::operator()(double delta)
	{
		GameEngine& game = GameEngine::Instance();
		game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
		
		const float offset = timer == 0 ? 0 : (2.0f - timer) * (2.0f - timer) * 2.0f * 400.0f;
		game.FontScale(glm::vec2(2.0f, 2.0f));
		game.FontColor(glm::vec4(1.0f, 0, 0, 1.0f));
		game.AddString(glm::vec2(300.0f + offset, 260.0f), "Game Over");
		game.FontScale(glm::vec2(0.5f, 0.5f));
		game.FontColor(glm::vec4(0.75f, 0.75f, 0.75f, 1.0f));
		game.AddString(glm::vec2(480.0f + offset, 328.0f), "Press Enter To Title");
		if (timer > 0) {
			timer -= static_cast<float>(delta);
			if (timer <= 0) {
				game.StopAudio(AudioPlayerId_BGM);
				game.UpdateFunc(Title(pSpaceSphere));
			}
		}
		else if (game.GetGamePad().buttonDown & GamePad::START) {
			game.PlayAudio(AudioPlayerId_UI, CRI_TUTORIALCUESHEET_START);
			timer = 2;
		}
	}

}
