/**
* @file MainGameState.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/Tutorial/TutorialCueSheet.h"
#include <algorithm>

namespace GameState {

	/// 衝突形状リスト.
	static const Entity::CollisionData collisionDataList[] = {
	  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
	  { glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
	  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
	  { glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
	};

	/**
	* 敵の弾の更新.
	*/
	struct UpdateEnemyShot
	{
		void operator()(Entity::Entity& entity, double delta)
		{
			// 範囲外に出たら削除する.
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40) {
				entity.Destroy();
				return;
			}
		}
	};

	/**
	* 敵の円盤の状態を更新する.
	*/
	struct UpdateToroid
	{
		explicit UpdateToroid(const Entity::Entity* t) : target(t) {}

		void operator()(Entity::Entity& entity, double delta)
		{
			GameEngine& game = GameEngine::Instance();

			// 範囲外に出たら削除する.
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
				GameEngine::Instance().RemoveEntity(&entity);
				return;
			}

			// 一定時間ごとに弾を発射.
			const float shotInterval = 2.0f;
			shotTimer += delta;
			if (shotTimer > shotInterval) {

				glm::vec3 shotPos = entity.Position();
				shotPos.x -= 0.4f; // 敵機の中心から左に0.4ずらした位置が1つめの発射点.
				for (int i = 0; i < 2; ++i) {

					// 画面外にいるときは発射しない.
					if (std::abs(pos.x) > 13 || pos.z > 41 || pos.z < -1) {
						continue;
					}

					// 自機の方向を計算.
					const glm::vec3 targetPos = target->Position();
					const glm::vec3 distance = targetPos - pos;
					const float radian = std::atan2(distance.x, distance.z);
					const float c = std::cos(radian);
					const float s = std::sin(radian);

					if (Entity::Entity* p = game.AddEntity(EntityGroupId_EnemyShot, shotPos, "Spario", "Res/Model/Toroid.bmp", UpdateEnemyShot()))					
					{
						p->Velocity(glm::vec3(20 * s, 0, 20 * c));
						p->Collision(collisionDataList[EntityGroupId_EnemyShot]);
						p->Color(glm::vec4(1.0f,1.0f, 1.0f, 1.0f) * 2.5f);
					}
					shotPos.x += 0.8f; // 中心からに右に0.4ずらした位置が2つめの発射点.
					game.PlayAudio(AudioPlayerId_Shot, CRI_TUTORIALCUESHEET_WEAPON_ENEMY);
				}
				shotTimer -= shotInterval;
			}

			// 円盤を回転させる.
			float rot = glm::angle(entity.Rotation());
			rot += glm::radians(60.0f) * static_cast<float>(delta);
			if (rot > glm::pi<float>() * 2.0f) {
				rot -= glm::pi<float>() * 2.0f;
			}
			entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));

			//// スケールを変化させる.
			//static std::mt19937 rand(std::random_device{}());

			//const int waitTime = 3600;
			//time = (time + 1) % waitTime;
			//if (time == 0) {
			//	const std::uniform_real_distribution<float> scaleRange(0.5, 2.0);
			//	scale = scaleRange(rand);
			//}
			//entity.Scale(glm::vec3(scale));
		}
	private:
		const Entity::Entity* target;
		double shotTimer = 0;
		//int time = 0;
		//float scale = 1;
	};

	/**
	* 自機の弾の更新.
	*/
	struct UpdatePlayerShot
	{
		void operator()(Entity::Entity& entity, double delta)
		{
			// 範囲外に出たら削除する.
			const glm::vec3 pos = entity.Position();
			if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40) {
				entity.Destroy();
				return;
			}
		}
	};

	/**
	* 爆発の更新.
	*/
	struct UpdateBlast
	{
		void operator()(Entity::Entity& entity, double delta) {
			timer += delta;
			if (timer >= 0.5) {
				entity.Destroy();
				return;
			}
			const float variation = static_cast<float>(timer * 4); // 変化量.
			entity.Scale(glm::vec3(static_cast<float>(1 + variation))); // 徐々に拡大する.
			// 時間経過で色と透明度を変化させる.
			static const glm::vec4 color[] = {
			  glm::vec4(1.0f, 1.0f, 0.75f, 1),
			  glm::vec4(1.0f, 0.5f, 0.1f, 1),
			  glm::vec4(0.25f, 0.1f, 0.1f, 0),
			};
			const glm::vec4 col0 = color[static_cast<int>(variation)];
			const glm::vec4 col1 = color[static_cast<int>(variation) + 1];
			const glm::vec4 newColor = glm::mix(col0, col1, std::fmod(variation, 1));
			entity.Color(newColor);
			// Y軸回転させる.
			glm::vec3 euler = glm::eulerAngles(entity.Rotation());
			euler.y += glm::radians(60.0f) * static_cast<float>(delta);
			entity.Rotation(glm::quat(euler));
		}

		double timer = 0;
	};


	/**
	* 自機の更新.
	*/
	struct UpdatePlayer
	{
		void operator()(Entity::Entity& entity, double delta)
		{
			GameEngine& game = GameEngine::Instance();
			const GamePad gamepad = game.GetGamePad();

			if (entity.invincible) {
				// 自機を点滅させる
				float Alpha = 0.5f + 0.5f * (float)sin(count * 0.7f);
				count++;
				invincibleTimer -= delta;
				if (invincibleTimer <= 0) {
					entity.invincible = false;
					invincibleTimer = 5;
					Alpha = 1.0f;
					count = 0;
				}
				entity.Color(glm::vec4(1.0f, 1.0f, 1.0f, Alpha));
			}

			if (!entity.GetIsActive()) {
				glm::vec3 vec = glm::vec3(0, 0, 0);
				glm::vec3 pos = entity.Position();
				if (restartTimer >= 2.5) {
					vec.z -= 2;
					vec.y -= 4;
				}
				else if (restartTimer >= 2) {
					pos = glm::vec3(0, 0, -2);
				}
				else {
					vec.z += 2;
				}
				restartTimer -= delta;
				if (restartTimer <= 0) {
					entity.SetIsActive(true);
					restartTimer = 3;
				}
				entity.Velocity(vec);
				entity.Position(pos);
				entity.Rotation(glm::quat());
			}
			else {
				glm::vec3 vec = glm::vec3(0, 0, 0);

				float rotZ = 0;
				if (gamepad.buttons & GamePad::DPAD_LEFT) {
					vec.x = 1;
					rotZ = -glm::radians(30.0f);
				}
				else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
					vec.x = -1;
					rotZ = glm::radians(30.0f);
				}
				if (gamepad.buttons & GamePad::DPAD_UP) {
					vec.z = 1;
				}
				else if (gamepad.buttons & GamePad::DPAD_DOWN) {
					vec.z = -1;
				}
				if (vec.x || vec.z) {
					vec = glm::normalize(vec) * 15.0f;
				}
				entity.Velocity(vec);
				entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));
				glm::vec3 pos = entity.Position();
				pos = glm::min(glm::vec3(11, 100, 30), glm::max(pos, glm::vec3(-11, -100, 1)));
				entity.Position(pos);

				if (gamepad.buttons & GamePad::A) {
					shotInterval -= delta;
					if (shotInterval <= 0) {
						glm::vec3 pos = entity.Position();
						pos.x -= 0.9f;	// 自機の中心から左に0.9ずらした位置が1つめの発射点.
						for (int i = 0; i < 2; ++i) {
							if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Model/Player.bmp", UpdatePlayerShot())) {
								p->Velocity(glm::vec3(0, 0, 80));
								p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
								p->Color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * 2.5f);
							}
							pos.x += 1.8f; // 中心からに右に0.9ずらした位置が2つめの発射点.
						}
						shotInterval += 0.25; // 秒間4連射.
						game.PlayAudio(AudioPlayerId_Shot, CRI_TUTORIALCUESHEET_WEAPON_PLAYER);
					}
				}
				else {
					shotInterval = 0;
				}
			}
		}

	private:
		double shotInterval = 0;
		double restartTimer = 3;
		double invincibleTimer = 5;
		int count = 0;
	};


	/**
	* 自機の弾と敵の衝突処理.
	*/
	void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs)
	{
		GameEngine& game = GameEngine::Instance();
		if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(), "Blast", "Res/Model/Toroid.bmp", UpdateBlast())) {
			const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
			p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
			p->Color(glm::vec4(1.0f, 0.75f, 0.5f, 1.0f) * 5.0f);
			game.Variable("score") += 100;
			if (game.Variable("score") == 1000 * game.Variable("check")) {
				game.Variable("stage")++;
				game.Variable("check")++;
			}
		}
		game.PlayAudio(AudioPlayerId_Bomb, CRI_TUTORIALCUESHEET_EXPLOSION_ENEMY);
		lhs.Destroy();
		rhs.Destroy();
	}

	/**
	* 自機と敵の衝突処理.
	*/
	void PlayerAndEnemyCollisionHandler(Entity::Entity& player, Entity::Entity& enemy)
	{
		GameEngine& game = GameEngine::Instance();
		if (!player.invincible) {
			if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, enemy.Position(), "Blast", "Res/Model/Toroid.bmp", UpdateBlast())) {
				const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
				p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
				p->Color(glm::vec4(1.0f, 0.75f, 0.5f, 1.0f) * 5.0f);
			}
			enemy.Destroy();
			game.PlayAudio(AudioPlayerId_Bomb, CRI_TUTORIALCUESHEET_EXPLOSION_ENEMY);

			if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, player.Position(), "Blast", "Res/Model/Toroid.bmp", UpdateBlast())) {
				const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
				p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
				p->Color(glm::vec4(1.0f, 0.75f, 0.5f, 1.0f) * 5.0f);
			}
			game.PlayAudio(AudioPlayerId_Bomb, CRI_TUTORIALCUESHEET_EXPLOSION_PLAYER);
			game.Variable("life")--;
			if (game.Variable("life") < 1) {
				player.Destroy();
			}
			else {
				player.SetIsActive(false);
				player.invincible = true;
			}
		}
	}

	/**
	* 自機と敵の弾の衝突処理.
	*/
	void PlayerAndEnemyShotCollisionHandler(Entity::Entity& player, Entity::Entity& enemyshot)
	{
		GameEngine& game = GameEngine::Instance();

		if (!player.invincible) {
			if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, player.Position(), "Blast", "Res/Model/Toroid.bmp", UpdateBlast())) {
				const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
				p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
				p->Color(glm::vec4(1.0f, 0.75f, 0.5f, 1.0f) * 5.0f);
			}
			game.PlayAudio(AudioPlayerId_Bomb, CRI_TUTORIALCUESHEET_EXPLOSION_PLAYER);
			game.Variable("life")--;
			if (game.Variable("life") < 1) {
				player.Destroy();
			}
			else {
				player.SetIsActive(false);
				player.invincible = true;
			}
			enemyshot.Destroy();
		}
	}


	/**
	* メインゲーム画面のコンストラクタ.
	*/
	MainGame::MainGame(Entity::Entity* p) : pSpaceSphere(p)
	{
		GameEngine& game = GameEngine::Instance();
		game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &PlayerShotAndEnemyCollisionHandler);
		game.CollisionHandler(EntityGroupId_Player, EntityGroupId_Enemy, &PlayerAndEnemyCollisionHandler);
		game.CollisionHandler(EntityGroupId_Player, EntityGroupId_EnemyShot, &PlayerAndEnemyShotCollisionHandler);
		game.Variable("score") = 0;
		game.Variable("stage") = 1;
		game.Variable("life") = 3;
		game.Variable("check") = 1;
	}

	/**
	* メインゲーム画面の更新.
	*/
	void MainGame::operator()(double delta)
	{
		GameEngine& game = GameEngine::Instance();

		if (!isInitialized) {
			isInitialized = true;
			game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
			game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
			game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) });

			pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Model/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collisionDataList[EntityGroupId_Player]);

			game.PlayAudio(AudioPlayerId_BGM, CRI_TUTORIALCUESHEET_BATTLE);
		}

		if (game.Variable("life") < 1) {
			game.StopAudio(AudioPlayerId_BGM);
			game.UpdateFunc(GameOver(pSpaceSphere));
		}

		std::uniform_int_distribution<> posXRange(-15, 15);
		std::uniform_int_distribution<> posZRange(38, 40);
		interval -= delta;
		if (interval <= 0) {
			std::uniform_int_distribution<> rndAddingCount(1, 5);

			for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
				const glm::vec3 pos(posXRange(game.Rand()), 0, posZRange(game.Rand()));
				
				if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Model/Toroid.bmp", "Res/Model/Toroid.Normal.bmp", UpdateToroid(pPlayer)))
				{
					p->Velocity({ pos.x < 0 ? 3.0f : -3.0f, 0, -12.0f });
					p->Collision(collisionDataList[EntityGroupId_Enemy]);
				}
			}
			std::normal_distribution<> intervalRange(2.0, 0.5);
			interval += glm::clamp(intervalRange(game.Rand()), 0.5, 3.0);
		}
		char str[32];
		snprintf(str, sizeof(str), "SCORE:%08.0f", game.Variable("score"));
		game.FontScale(glm::vec2(1.5f, 1.5f));
		game.FontColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		game.AddString(glm::vec2(420.0f, 8.0f), str);
		snprintf(str, sizeof(str), "STAGE:%02.0f", game.Variable("stage"));
		game.FontScale(glm::vec2(1.0f, 1.0f));
		game.FontColor(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
		game.AddString(glm::vec2(0.0f, 8.0f), str);
		snprintf(str, sizeof(str), "LIFE:%02.0f", game.Variable("life"));
		game.FontScale(glm::vec2(1.0f, 1.0f));
		if (game.Variable("life") <= 1) {
			game.FontColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
		else {
			game.FontColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		}
		game.AddString(glm::vec2(150.0f, 8.0f), str);
	}
}