/**
* @file Engine.h
*/
#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED
#include "glad/glad.h"
#include "GameObject.h"
#include "Scene.h"
#include "Mesh.h"
#include "FramebufferObject.h"
#include <GLFW/glfw3.h>

#include <unordered_map>
#include <utility>
#include <fstream>
#include <filesystem>

// 先行宣言
using ProgramObjectPtr = std::shared_ptr<class ProgramObject>;

// SSBOのバインディングポイント番号
constexpr GLuint bpJointMatrices = 0; // 関節行列

// シェーダで使えるライトの数
constexpr size_t maxShaderLightCount = 16;

// シェーダのロケーション番号
constexpr GLint locTransformMatrix = 0;
constexpr GLint locNormalMatrix = 1;
constexpr GLint locViewProjectionMatrix = 2;
constexpr GLint locShadowTextureMatrix = 10;
constexpr GLint locShadowNormalOffset = 11;
constexpr GLint locColor = 100;
constexpr GLint locAlphaCutoff = 102;
constexpr GLint locAmbientLight = 107;
constexpr GLint locDirectionalLightColor = 108;
constexpr GLint locDirectionalLightDirection = 109;
constexpr GLint locLightCount = 110;
constexpr GLint locLightColorAndFalloffAngle = 111;
constexpr GLint locLightPositionAndRadius = locLightColorAndFalloffAngle + maxShaderLightCount;
constexpr GLint locLightDirectionAndConeAngle = locLightColorAndFalloffAngle + maxShaderLightCount * 2;

// 画像処理シェーダ用のロケーション番号
constexpr GLint locColorMatrix = 200;
constexpr GLint locRadialBlur = 201;
constexpr GLint locHighPassFilter = 202;

// 点光源
struct LIGHT_DATA
{
	vec3 color;			// 色
	float intensity;	// 明るさ
	vec3 position;		// 位置
	float radius;		// ライトが届く最大半径

	// スポットライト用の変数
	vec3 direction;		// ライトの向き
	float coneAngle;	// スポットライトが照らす角度
	float falloffAngle;	// スポットライトの減衰開始角度

	bool isUsed = false;// 使用中かどうか
};

// 平行光源
struct DIRECTIONAL_LIGHT
{
  vec3 color = { 1.00f, 0.98f, 0.95f };		 // 色
  float intensity = 5.0f;					 // 明るさ
  vec3 direction = { 0.58f, -0.58f, 0.58f }; // 向き
};

/// <summary>
/// ゲームエンジン
/// </summary>
class Engine
{
public:
	// デフォルトコンストラクタ
	Engine() = default;
	// デフォルトデストラクタ
	virtual ~Engine() = default;

	// ゲームエンジンを実行する
	int Run();

	/// <summary>
	/// ゲームオブジェクトを作成する
	/// </summary>
	/// <typeparam name="T">型の違いの寛容化</typeparam>
	/// <param name="name">オブジェクトの名前</param>
	/// <param name="position">オブジェクトを配置する座標</param>
	/// <param name="rotation">オブジェクトの回転角度</param>
	/// <returns>ゲームオブジェクトのデータ</returns>
	template<typename T>
	std::shared_ptr<T> Create
	(
		const std::string& name,
		const vec3& position = { 0,0,0 },
		const vec3& rotation = { 0,0,0 }
	)
	{
		std::shared_ptr<T> p = std::make_shared<T>();
		p->engine = this;
		p->name = name;
		p->position = position;
		p->rotation = rotation;
		gameObjects.push_back(p); // エンジンに登録する
		return p;
	}

	/// <summary>
	/// UIオブジェクトを作成する
	/// </summary>
	/// <typeparam name="T">ゲームオブジェクトに割り当てるUILayoutまたはその派生クラス</typeparam>
	/// <param name="fileName">UIオブジェクトに表示する画像</param>
	/// <param name="position">UIオブジェクトの座標</param>
	/// <param name="scale">UIオブジェクトの大きさ</param>
	/// <returns>作成したUIオブジェクト</returns>
	template<typename T>
	std::pair<GameObjectPtr, std::shared_ptr<T>> CreateUIObject
	(
		const char* fileName,
		const vec2& position,
		float scale
	)
	{
		auto object = Create<GameObject>(fileName, { position.x,position.y,0 });
		object->render_queue = RENDER_QUEUE_OVERLAY;
		object->staticMesh = GetStaticMesh("plane_xy");

		// 固有マテリアルを作成し、テクスチャを差し替える
		auto texBaseColor = GetTexture(fileName);
		object->materials = CloneMaterialList(object->staticMesh);
		object->materials[0]->texBaseColor = texBaseColor;

		// 画像サイズに応じて拡大率を調整する
		const float aspectRatio = texBaseColor->GetAspectRatio();
		object->scale = { scale * aspectRatio,scale,1 };

		// コンポーネントを追加する
		auto component = object->AddComponent<T>();

		return{ object,component };
	}

	/// <summary>
	/// ゲームオブジェクト配置ファイルに含まれるメッシュ名と実際のファイル名の対応表
	/// ※first = メッシュ名、second = 実際のファイル名
	/// </summary>
	using FilepathMap = std::unordered_map<std::string, std::string>;

	// ゲームオブジェクト配置ファイルを読み込む
	GameObjectList LoadGameObjectMap(const char* filename, const FilepathMap&);

	// すべてのゲームオブジェクトを削除する
	void ClearGameObjectAll();

	// 視野角の管理
	void SetFovY(float fovY);
	float GetFovY() const { return degFovY; }
	float GetFovScale()const { return fovScale; }

	// 次のシーンを設定する
	template<typename T>
	void SetNextScene() { nextScene = std::make_shared<T>(); }

	// カメラを取得する
	GameObject& GetMainCamera() { return camera; }
	const GameObject& GetMainCamera() const { return camera; }

	/// <summary>
	/// 入力されたキーを取得する
	/// </summary>
	/// <param name="key">入力されたキー</param>
	/// <returns>true = 押されている、false = 押されていない</returns>
	bool GetKey(int key) const { return glfwGetKey(window, key) == GLFW_PRESS; }

	// マウスカーソルの座標を取得する
	vec2 GetMousePosition() const;

	// マウスカーソルの動作量を取得する
	vec2 GetMouseMovement() const { return mouseMovement; }

	// マウスカーソルを隠す
	void HideMouseCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); }
	// マウスカーソルを表示する
	void ShowMouseCursor() { glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); }

	// マウスのボタンの状態を取得する
	bool GetMouseButton(int button) const;

	// マウスボタンのクリック状態を取得する
	bool GetMouseClick(int button) const;

	// フレームバッファの大きさを取得する
	vec2 GetFramebufferSize() const;

	// フレームバッファのアスペクト比を取得する
	float GetAspectRatio() const;

	/// <summary>
	/// スタティックメッシュの取得
	/// </summary>
	/// <param name="name">メッシュ名</param>
	/// <returns>名前がnameと一致するスタティックメッシュ</returns>
	StaticMeshPtr GetStaticMesh(const char* name) { return meshBuffer->GetStaticMesh(name); }

	/// <summary>
	/// スケルタルメッシュの取得
	/// </summary>
	/// <param name="name">メッシュ名</param>
	/// <returns>名前がnameと一致するスケルタルメッシュ</returns>
	SkeletalMeshPtr GetSkeletalMesh(const char* name) { return meshBuffer->GetSkeletalMesh(name); }

	// テクスチャを取得する
	TexturePtr GetTexture(const char* name);

	// キューブマップテクスチャを読み込む
	TexturePtr LoadCubemapTexture(
		const char* name, const char* const cubemapFilenames[6]);

	// 環境マッピング用のキューブマップを設定する
	void SetEnvironmentCubemap(const TexturePtr & cubemap) { texEnvironment = cubemap; }

	// ライト配列を初期化する
	void InitializeLight();

	// 新しいライトを取得する
	int AllocateLight();

	// ライトを解放する
	void DeallocateLight(int index);

	// インデックスに対応するライトデータを取得する
	LIGHT_DATA* GetLight(int index);
	const LIGHT_DATA* GetLight(int index) const;

	// マウス座標から発射される光線を取得する
	RAY GetRayFromMousePosition() const;

	/// <summary>
	/// 光線の交差判定結果
	/// </summary>
	struct RAYCAST_HIT
	{
		ColliderPtr collider; // 最初に光線と交差したコライダー
		vec3 point;			  // 最初の交点の座標
		float distance;		  // 最初の交点までの距離
	};

	/// <summary>
	/// 交差判定の対象になるかどうかを調べる述語型
	/// ※collider = 判定するコライダー、distance = 光線とコライダーの交点までの距離
	/// 　true = 交差判定の対象内、false = 交差判定の対象外
	/// </summary>
	using RaycastPredicate =
		std::function <bool(const ColliderPtr& collider, float distance)>;

	// 光線とコライダーの交差判定を取得する
	bool Raycast(const RAY& ray, RAYCAST_HIT& hitInfo, const RaycastPredicate& pred) const;

private:
	// コンストラクタ、デストラクタを呼べるようにするための補助クラス
	struct TEX_HELPER
		: public Texture
	{
		TEX_HELPER(const char* p)
			: Texture(p)
		{}
		TEX_HELPER(const char* p, int w, int h, GLenum f, int levels = 1)
			: Texture(p, w, h, f, GL_CLAMP_TO_EDGE, levels)
		{}
		TEX_HELPER(const char* p, const char* const c[6])
			: Texture(p, c)
		{}
	};

	// ワールド座標系のコライダーを表す構造体
	struct WORLDCOLLIDER
	{
		/// <summary>
		/// 座標を変更する
		/// </summary>
		/// <param name="v">追加分の座標</param>
		void AddPosition(const vec3& v)
		{
			origin->GetOwner()->position += v;
			world->AddPosition(v);
		}

		ColliderPtr origin;
		ColliderPtr world;
	};
	using WorldColliderList = std::vector<WORLDCOLLIDER>;
	void ApplyPenetration(WorldColliderList*, GameObject*, const vec3&);

	int Initialize();
	void CreateMainFBO(int width, int height);
	void Update();
	void Render();
	void DrawGameObject
	(
		ProgramObject& prog,
		GameObjectList::iterator begin,
		GameObjectList::iterator end
	);
	void UpdateGameObject(float deltaTime);
	void UpdateJointMatrix();
	void HandleGameObjectCollision();
	void HandleWorldColliderCollision(WorldColliderList* a, WorldColliderList* b);
	void RemoveDestroyedGameObject();
	void UpdateShaderLight();
	void DrawSkySphere();
	void CreateShadowMap(
		GameObjectList::iterator begin,
		GameObjectList::iterator end);
	void DrawAmbientOcclusion();
	void DrawBloomEffect();
	void DrawMainImageToDefaultFramebuffer();

	GLFWwindow* window = nullptr;		   // ウインドウオブジェクト
	const std::string title = "Returnee";  // ウインドウタイトル
	float degFovY = 60;					   // 垂直視野角(度数法)
	float radFovY = degFovY * 3.1415926535f / 180;
	float fovScale = 1 / tan(radFovY / 2); // 視野角による拡大率の逆数

	// シェーダ管理用のメンバ変数
	ProgramObjectPtr prog3D;			 // ライト付きシェーダ
	ProgramObjectPtr progUnlit;			 // ライトなしシェーダ
	ProgramObjectPtr progShadow;		 // 影テクスチャ作成シェーダ
	ProgramObjectPtr progSkeletal;		 // ライト付きスケルタルメッシュシェーダ
	ProgramObjectPtr progShadowSkeletal; // スケルタルメッシュ用の影テクスチャ作成シェーダ

	// ブルームエフェクト
	ProgramObjectPtr progHighPassFilter; // 明るい領域を抽出するシェーダ
	ProgramObjectPtr progDownSampling;   // 縮小ぼかしシェーダ
	ProgramObjectPtr progUpSampling;     // 拡大ぼかしシェーダ
	float bloomThreshold = 1;			 // 明るいとみなす閾値
	float bloomIntensity = 2;			 // ブルームの強さ

	// SAO用のメンバ変数
	ProgramObjectPtr progSAORestoreDepth; // 線形距離変換シェーダ
	ProgramObjectPtr progSAODownSampling; // 縮小シェーダ
	ProgramObjectPtr progSAO;             // SAO計算シェーダ
	ProgramObjectPtr progSAOBlur;         // SAO用ぼかしシェーダ
	FramebufferObjectPtr fboSAODepth[4];  // 深度値の縮小バッファ
	FramebufferObjectPtr fboSAO;          // SAO計算結果バッファ
	FramebufferObjectPtr fboSAOBlur;      // ぼかし結果バッファ

	ProgramObjectPtr progSimple; // テクスチャを描画するだけのシェーダ

	ProgramObjectPtr progImageProcessing;	  // 画像処理シェーダ
	mat3 colorMatrix = mat3(1);				  // 画像処理で使う色変換行列
	vec2 radialBlurCenter = vec2(0.5f, 0.5f); // 放射状ブラーの中心
	float radialBlurLength = 0.02f;           // 放射状ブラーの長さ
	float radialBlurStart = 0.2f;             // 放射状ブラーの開始距離

	FramebufferObjectPtr fboMain;				// 3D描画用FBO
	std::vector<FramebufferObjectPtr> fboBloom; // ブルームエフェクト用FBO
	FramebufferObjectPtr fboShadow;				// デプスシャドウ用FBO

	// 関節行列の配列の最大バイト数
	static constexpr size_t jointMatricesBytes = 1024 * 1024;
	BufferObjectPtr ssboJointMatrices[3];	 // 関節行列の配列用のバッファ
	GLsync syncJointMatrix[3] = { 0, 0, 0 }; // 関節行列の同期オブジェクト
	size_t jointMatrixIndex = 0;			 // 関節行列の書き込み先インデックス

	MeshBufferPtr meshBuffer; // 図形データ管理オブジェクト
	std::unordered_map<std::string, TexturePtr> textureCache;

	GameObjectList gameObjects;	// ゲームオブジェクト配列
	double previousTime = 0;	// 前回更新時の時刻
	float deltaTime = 0;		// 前回更新からの経過時間
	ScenePtr scene;				// 実行中のシーン
	ScenePtr nextScene;			// 次のシーン

	// クリックと判定する速度
	static constexpr float mouseClickSpeed = 0.3f;

	// マウスボタンの状態
	struct MOUSEBUTTON
	{
		bool current = false;  // 現在フレームのボタンの状態
		bool previous = false; // 前フレームのボタンの状態
		bool click = false;	   // クリックの状態
		float timer = 0;	   // ボタンが押されている時間
	};
	MOUSEBUTTON mouseButtons[3];
	vec2 oldMousePosition = { 0, 0 }; // 現在のマウス位置
	vec2 mousePosition = { 0, 0 };	  // 変更後のマウス位置
	vec2 mouseMovement = { 0, 0 };	  // どれだけ動いたか

	// 点光源
	std::vector<LIGHT_DATA> lights;	// ライトデータの配列
	std::vector<int> usedLights;	// 使用中のライトのインデックス配列
	std::vector<int> freeLights;	// 未使用のライトのインデックス配列

	// 一度に増やすライト数
	static constexpr size_t lightResizeCount = 100;

	// 平行光源
	DIRECTIONAL_LIGHT directionalLight;

	// 環境光
	vec3 ambientLight = vec3(0);

	// 環境マッピング用のキューブマップ
	TexturePtr texEnvironment;

	// スカイスフィア用モデル
	StaticMeshPtr skySphere;

	// カメラオブジェクト
	GameObject camera;
	GLsizei indexCount = 0;
};

#endif // !ENGINE_H_INCLUDED