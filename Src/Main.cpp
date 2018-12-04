/**
* @file Main.cpp
*/
#include "GameEngine.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>


/// エンティティの衝突グループID.
enum EntityGroupId
{
	EntityGroupId_Player,
	EntityGroupId_PlayerShot,
	EntityGroupId_Enemy,
	EntityGroupId_EnemyShot,
	EntityGroupId_Others,
};

/// 衝突形状リスト.
static const Entity::CollisionData collisionDataList[] = {
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
  { glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
  { glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
};

//int time = 0;
//float scale = 1;

/**
* 敵の円盤の状態を更新する.
*/
struct UpdateToroid
{
	//void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
	void operator()(Entity::Entity& entity, double delta)
	{
		// 範囲外に出たら削除する.
		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
			GameEngine::Instance().RemoveEntity(&entity);
			return;
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

		//// 頂点シェーダーのパラメータをUBOにコピーする.
		//InterfaceBlock::VertexData data;
		//data.matModel = entity.CalcModelMatrix();
		//data.matNormal = glm::mat4_cast(entity.Rotation());
		//data.matMVP = matProj * matView * data.matModel;
		//data.color = entity.Color();
		//memcpy(ubo, &data, sizeof(InterfaceBlock::VertexData));
	}
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
	//void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
	void operator()(Entity::Entity& entity, double delta)
	{
		GameEngine& game = GameEngine::Instance();
		const GamePad gamepad = game.GetGamePad();

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
		pos = glm::min(glm::vec3(11, 100, 20), glm::max(pos, glm::vec3(-11, -100, 1)));
		entity.Position(pos);

		if (gamepad.buttons & GamePad::A) {
			shotInterval -= delta;
			if (shotInterval <= 0) {
				glm::vec3 pos = entity.Position();
				pos.x -= 0.3f; // 自機の中心から左に0.3ずらした位置が1つめの発射点.
				for (int i = 0; i < 2; ++i) {
					if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Player.bmp", UpdatePlayerShot())) {
						p->Velocity(glm::vec3(0, 0, 80));
						p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
					}
					pos.x += 0.6f; // 中心からに右に0.3ずらした位置が2つめの発射点.
				}
				shotInterval += 0.25; // 秒間4連射.
			}
		}
		else {
			shotInterval = 0;
		}

		/*InterfaceBlock::VertexData data;
		data.matModel = entity.CalcModelMatrix();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView * data.matModel;
		data.color = entity.Color();
		memcpy(ubo, &data, sizeof(InterfaceBlock::VertexData));*/
	}

private:
	double shotInterval = 0;
};

/**
* ゲーム状態の更新.
*/
class Update
{
public:
	void operator()(double delta)
	{
		GameEngine& game = GameEngine::Instance();

		if (!isInitialized) {
			isInitialized = true;
			game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
			game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
			game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) });

			//pPlayer = game.AddEntity(glm::vec3(0, 0, 2), "Aircraft", "Res/Player.bmp", UpdatePlayer());
			pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collisionDataList[EntityGroupId_Player]);
		}

		std::uniform_int_distribution<> posXRange(-15, 15);
		std::uniform_int_distribution<> posZRange(38, 40);
		interval -= delta;
		if (interval <= 0) {
			std::uniform_int_distribution<> rndAddingCount(1, 5);

			for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
				const glm::vec3 pos(posXRange(game.Rand()), 0, posZRange(game.Rand()));

				//if (Entity::Entity* p = game.AddEntity(pos, "Toroid", "Res/Toroid.bmp", UpdateToroid())) {
				if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Toroid.bmp", UpdateToroid())) {
					p->Velocity({ pos.x < 0 ? 3.0f : -3.0f, 0, -12.0f });
					p->Collision(collisionDataList[EntityGroupId_Enemy]);
				}
			}
			std::normal_distribution<> intervalRange(2.0, 0.5);
			interval += glm::clamp(intervalRange(game.Rand()), 0.5, 3.0);
		}
	}

private:
	bool isInitialized = false;
	double interval = 0;


	Entity::Entity* pPlayer = nullptr;
};

/**
* 自機の弾と敵の衝突処理.
*/
void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs)
{
	GameEngine& game = GameEngine::Instance();
	if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(), "Blast", "Res/Toroid.bmp", UpdateBlast())) {
		const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
		p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
	}
	lhs.Destroy();
	rhs.Destroy();
}

/// エントリーポイント.
//int main()
//{
//	GLFWEW::Window& window = GLFWEW::Window::Instance();
//	if (!window.Init(800, 600, "OpenGL Tutorial")) {
//		return 1;
//	}
//
//	const GLuint vbo = CreateVBO(sizeof(vertices), vertices);
//	const GLuint ibo = CreateIBO(sizeof(indices), indices);
//	const GLuint vao = CreateVAO(vbo, ibo);
//	const UniformBufferPtr uboVertex = UniformBuffer::Create(sizeof(VertexData), BINDINGPOINT_VERTEXDATA, "VertexData");
//	const UniformBufferPtr uboLight = UniformBuffer::Create(sizeof(LightData), BINDINGPOINT_LIGHTDATA, "LightData");
//	const UniformBufferPtr uboPostEffect = UniformBuffer::Create(sizeof(PostEffectData), BINDINGPOINT_POSTEFFECTDATA, "PostEffectData");
//		
//	const Shader::ProgramPtr progTutorial = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
//	const Shader::ProgramPtr progColorFilter = Shader::Program::Create("Res/ColorFilter.vert", "Res/ColorFilter.frag");
//	const Shader::ProgramPtr progPosterization = Shader::Program::Create("Res/Posterization.vert", "Res/Posterization.frag");
//
//	if (!vbo || !ibo || !vao || !uboVertex || !uboLight || !uboPostEffect || !progTutorial || !progColorFilter || !progPosterization) {
//		return 1;
//	}
//	
//	progTutorial->UniformBlockBinding(*uboVertex);
//	progTutorial->UniformBlockBinding(*uboLight);
//	progColorFilter->UniformBlockBinding(*uboPostEffect);
//
//	/// テクスチャデータ.
//	/*	static const uint32_t textureData[] = {
//		0xffffffff, 0xffcccccc, 0xffffffff, 0xffcccccc, 0xffffffff,
//		0xff888888, 0xffffffff, 0xff888888, 0xffffffff, 0xff888888,
//		0xffffffff, 0xff444444, 0xffffffff, 0xff444444, 0xffffffff,
//		0xff000000, 0xffffffff, 0xff000000, 0xffffffff, 0xff000000,
//		0xffffffff, 0xff000000, 0xffffffff, 0xff000000, 0xffffffff,
//		};
//	TexturePtr tex = Texture::Create(5, 5, GL_RGBA8, GL_RGBA, textureData);*/
//	//TexturePtr tex = Texture::LoadFromFile("Res/Sample.bmp");
//	//TexturePtr texToroid = Texture::LoadFromFile("Res/Toroid.bmp");
//	/*if (!tex || !texToroid ) {
//		return 1;
//	}*/
//	const int texNum = 3;
//	TexturePtr texToroid[texNum];
//	texToroid[0] = Texture::LoadFromFile("Res/twinte.bmp");
//	texToroid[1] = Texture::LoadFromFile("Res/twinte2.bmp");
//	texToroid[2] = Texture::LoadFromFile("Res/twinte3.bmp");
//
//	for (int i = 0; i < texNum; ++i) {
//		if (!texToroid[i]) {
//			return 1;
//		}
//	}
//	
//	Mesh::BufferPtr meshBuffer = Mesh::Buffer::Create(50000, 50000);
//	meshBuffer->LoadMeshFromFile("Res/ao_twinte_chan.fbx");	
//	//meshBuffer->LoadMeshFromFile("Res/Toroid.fbx");
//
//	Entity::BufferPtr entityBuffer = Entity::Buffer::Create(1024, sizeof(VertexData), 0, "VertexData");
//	if (!entityBuffer) {
//		return 1;
//	}
//
//	const OffscreenBufferPtr offscreen = OffscreenBuffer::Create(800, 600);
//
//	glEnable(GL_DEPTH_TEST);
//
//	// メインループ.
//	while (!window.ShouldClose()) {
//		static std::mt19937 rand(std::random_device{}());
//		const std::uniform_int_distribution<> noRange(0, 2);
//		Update(entityBuffer, meshBuffer, texToroid[noRange(rand)], progTutorial);
//
//		glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
//		glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//		//// 視点を回転移動させる.
//		//static float degree = 0.0f;
//		//degree += 0.01f;
//		//if (degree >= 360.0f) { degree -= 360.0f; }
//		//const glm::vec3 viewPos = glm::rotate(glm::mat4(1), glm::radians(degree), glm::vec3(0, 1, 0)) * glm::vec4(10, 5, 10, 1);
//		const glm::vec3 viewPos = glm::vec4(20, 20, -20, 1);
//	
//		progTutorial->UseProgram();
//
//		// 座標変換行列を作成してシェーダに転送する.		
//		const glm::mat4x4 matProj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
//		const glm::mat4x4 matView = glm::lookAt(viewPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
//
//		/*VertexData vertexData;
//		vertexData.matMVP = matProj * matView;
//		vertexData.matModel = glm::mat4(1);
//		vertexData.matNormal = glm::mat3x4(1);
//		vertexData.color = glm::vec4(1);
//		
//		uboVertex->BufferSubData(&vertexData);*/
//
//		LightData lightData;
//		/*lightData.ambientColor = glm::vec4(0.05f, 0.1f, 0.2f, 1);
//		lightData.light[0].position = glm::vec4(1, 1, 1, 1);
//		lightData.light[0].color = glm::vec4(2, 2, 2, 1);
//		lightData.light[1].position = glm::vec4(-0.2f, 0, 0.6f, 1);
//		lightData.light[1].color = glm::vec4(0.125f, 0.125f, 0.05f, 1);*/
//		lightData.light[0].color = glm::vec4(1, 1, 1, 1);
//		lightData.light[0].position = glm::vec4(40, 100, 20, 1);
//
//		uboLight->BufferSubData(&lightData);
//
//		//progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, tex->Id());
//
//		glBindVertexArray(vao);		
//		//glDrawElements(GL_TRIANGLES, renderingParts[0].size, GL_UNSIGNED_INT, renderingParts[0].offset);
//		/*progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, texToroid->Id());
//		meshBuffer->BindVAO();
//		meshBuffer->GetMesh("Cube")->Draw(meshBuffer);*/
//				
//		entityBuffer->Update(1.0 / 60.0, matView, matProj);
//		entityBuffer->Draw(meshBuffer);
//
//		glBindVertexArray(vao);
//
//		glBindFramebuffer(GL_FRAMEBUFFER, 0);
//		glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//		//glDisable(GL_DEPTH_TEST);
//		
//		// ライティングの利用
//		progTutorial->UseProgram();
//		progTutorial->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
//		
//		/*vertexData = {};
//		vertexData.matMVP = glm::mat4(1);
//
//		vertexData.matModel = glm::mat4(1);
//		vertexData.matNormal = glm::mat3x4(1);
//		vertexData.color = glm::vec4(1);
//		
//		uboVertex->BufferSubData(&vertexData);
//		entityBuffer->UniformBuffer()->BufferSubData(&vertexData);*/
//		
//		lightData = {};
//		lightData.ambientColor = glm::vec4(1);
//		uboLight->BufferSubData(&lightData);
//
//		// カラーフィルターシェーダの利用
//		progColorFilter->UseProgram();
//		progColorFilter->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
//
//		PostEffectData postEffect;
//		// 初期
//		postEffect.matColor = glm::mat4(1);
//		////// セピア調
//		////postEffect.matColor[0] = glm::vec4(0.393f, 0.349f, 0.272f, 0);
//		////postEffect.matColor[1] = glm::vec4(0.769f, 0.686f, 0.534f, 0);
//		////postEffect.matColor[2] = glm::vec4(0.189f, 0.168f, 0.131f, 0);
//		////postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
//		////// モノトーン調
//		////postEffect.matColor[0] = glm::vec4(0.299f, 0.299f, 0.299f, 0);
//		////postEffect.matColor[1] = glm::vec4(0.587f, 0.587f, 0.587f, 0);
//		////postEffect.matColor[2] = glm::vec4(0.114f, 0.114f, 0.114f, 0);
//		////postEffect.matColor[3] = glm::vec4(0, 0, 0, 1);
//		//// ネガポジ反転
//		//postEffect.matColor[0] = glm::vec4(-1, 0, 0, 0);
//		//postEffect.matColor[1] = glm::vec4(0, -1, 0, 0);
//		//postEffect.matColor[2] = glm::vec4(0, 0, -1, 0);
//		//postEffect.matColor[3] = glm::vec4(1, 1, 1, 1);
//
//		uboPostEffect->BufferSubData(&postEffect);
//
//		//// ポスター化シェーダーの利用
//		//progPosterization->UseProgram();
//		//progPosterization->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
//
//		glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);
//		
//		window.SwapBuffers();
//	}
//	//glDeleteVertexArrays(1, &vao);
//
//	return 0;
//}

int main()
{
	GameEngine& game = GameEngine::Instance();
	if (!game.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}

	game.LoadTextureFromFile("Res/Toroid.bmp");
	game.LoadTextureFromFile("Res/Player.bmp");
	game.LoadMeshFromFile("Res/Toroid.fbx");
	game.LoadMeshFromFile("Res/Player.fbx");
	game.LoadMeshFromFile("Res/Blast.fbx");

	game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &PlayerShotAndEnemyCollisionHandler);

	game.UpdateFunc(Update());
	game.Run();
	return 0;
}