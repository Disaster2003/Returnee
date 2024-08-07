/**
* @file Engine.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Engine.h"
#include "ProgramObject.h"
#include "EasyAudio/EasyAudio.h"
#include "AabbCollider.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Debug.h"
#include "SkeletalMeshRenderer.h"

// 図形データ
#include "../../Res/MeshData/crystal_mesh.h"
#include "../../Res/MeshData/plane_xy_mesh.h"

/// <summary>
/// OpenGLからのメッセージを処理するコールバック関数
/// </summary>
/// <param name="source">メッセージの発信者(OpenGL、Windows、シェーダなど)</param>
/// <param name="type">メッセージの種類(エラー、警告など)</param>
/// <param name="id">メッセージを一位に識別する値</param>
/// <param name="severity">メッセージの重要度(高、中、低、最低)</param>
/// <param name="length">メッセージの文字数.負数ならメッセージは0終端されている</param>
/// <param name="message">メッセージ本体</param>
/// <param name="userParam">コールバック設定時に指定したポインタ</param>
void APIENTRY DebugCallback
(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
)
{
	std::string s;
	// string.assign : 文字列の書き換え(first→secondに)
	(length < 0) ? s = message : s.assign(message, message + length);
	
	s += '\n'; // メッセージには改行がないので追加する
	// string.c_str() : string型をchar型に変換する関数
	LOG(s.c_str());
}

/// <summary>
/// ゲームエンジンを実行する
/// </summary>
/// <returns>0 = 正常に実行が完了した、0以外 = エラーが発生した</returns>
int Engine::Run()
{
	const int result = Initialize();
	if(result)
		return result;
	// 音声ライブラリを初期化する
	if (!EasyAudio::Initialize())
		return 1; // 初期化失敗
	
	// 終了応答が来ていなければ
	while(!glfwWindowShouldClose(window))
	{
		// ゲームを実行する
		Update();
		Render();
		RemoveDestroyedGameObject();

		// 音声ライブラリを更新する
		EasyAudio::Update();
	}

	// 音声ライブラリを終了する
	EasyAudio::Finalize();

	// GLFWを終了する
	glfwTerminate();
	return 0;
}

/// <summary>
/// ゲームオブジェクト配置ファイルを読み込む
/// </summary>
/// <param name="filename">ファイル名</param>
/// <param name="filepathMap"></param>
/// <returns>空のオブジェクト = 読み込み失敗、ゲームオブジェクト配列 = 読み込み成功</returns>
GameObjectList Engine::LoadGameObjectMap
(
	const char* filename,
	const FilepathMap& filepathMap
)
{
	// ファイルを開く
	std::ifstream ifs(filename);
	if (!ifs)
	{
		LOG_WARNING("%sを開けません", filename);
		return {};
	}

	struct PARENT_DATA
	{
		std::string parent;
		GameObjectPtr go;
	};
	std::vector<PARENT_DATA> objectMap;

	GameObjectList gameObjectList;
	gameObjectList.reserve(1000); // 適当な数を予約

	// ファイルを読み込む
	while (!ifs.eof())
	{
		std::string line;
		std::getline(ifs, line);
		const char* p = line.c_str();
		int readByte = 0; // 読み込んだバイト数

		// ゲームオブジェクト定義の開始判定(先頭には必ずname要素があるはず)
		char name[256] = { 0 }; // ゲームオブジェクト名
		if (sscanf(p, R"( { "name" : "%255[^"]" %n)", name, &readByte) != 1)
			continue;  // 定義ではない
		
		p += readByte; // 読み取り位置を更新

		/* 名前以外の要素を読み込む */

		char parentName[256] = { 0 }; // 親オブジェクト名
		if (sscanf(p, R"(, "parent" : "%255[^"]" %n)", parentName, &readByte) == 1)
			p += readByte;

		char meshName[256] = { 0 };	  // メッシュ名
		if (sscanf(p, R"(, "mesh" : "%255[^"]" %n)", meshName, &readByte) == 1)
			p += readByte;

		char renderType[256] = { 0 }; // 描画タイプ
		if (sscanf(p, R"(, "renderType" : "%255[^"]" %n)", renderType, &readByte) == 1)
			p += readByte;

		vec3 t(0); // 平行移動
		if (sscanf(p, R"(, "translate" : [ %f, %f, %f ] %n)",
			&t.x, &t.y, &t.z, &readByte) == 3)
			p += readByte;

		vec3 r(0); // 回転
		if (sscanf(p, R"(, "rotate" : [ %f, %f, %f ] %n)",
			&r.x, &r.y, &r.z, &readByte) == 3)
			p += readByte;

		vec3 s(1); // 拡大率
		if (sscanf(p, R"(, "scale" : [ %f, %f, %f ] %n)",
			&s.x, &s.y, &s.z, &readByte) == 3)
			p += readByte;

		// ゲームオブジェクトを作成する
		// Unityは左手座標系なので、平行移動と回転方向を右手座標系に変換する
		auto gameObject = Create<GameObject>(name);
		gameObject->position = vec3(t.x, t.y, -t.z);
		gameObject->rotation =
			vec3(radians(-r.x), radians(-r.y), radians(r.z));
		gameObject->scale = s;

		// メッシュを読み込む
		if (meshName[0])
		{
			// メッシュ名に対応するファイルパスを検索
			// 対応するファイルパスが見つかればメッシュを読み込む
			const auto itr = filepathMap.find(meshName);
			if (itr != filepathMap.end())
			{
				const auto& path = itr->second.c_str();
				gameObject->staticMesh = meshBuffer->LoadOBJ(path);
			}
		} // if meshName[0]

		// ボックスコライダーを作成する
		const char strBoxCollider[] = R"(, "BoxCollider" : [)";
		if (strncmp(p, strBoxCollider, sizeof(strBoxCollider) - 1) == 0)
		{
			p += sizeof(strBoxCollider) - 1;
			BOX box = { vec3(0), { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } }, vec3(1) };
			for (;;)
			{
				if (sscanf(p, R"( { "center" : [ %f, %f, %f ], "size" : [ %f, %f, %f ] } %n)",
					&box.position.x, &box.position.y, &box.position.z,
					&box.scale.x, &box.scale.y, &box.scale.z, &readByte) != 6)
					break;
				
				auto collider = gameObject->AddComponent<BoxCollider>();
				collider->box = box;
				collider->box.position.z *= -1; // 左手座標系を右手座標系に変換
				collider->box.scale *= 0.5f;	// UnityのBoxColliderは1x1m(0.5m四方)
				collider->isStatic = true;
				p += readByte;

				// 末尾にカンマがある場合は飛ばす
				if (*p == ',')
					++p;
			}
		} // if strBoxCollider

		// ゲームオブジェクトをリストに追加する
		gameObjectList.push_back(gameObject);
		objectMap.push_back({ parentName, gameObject });
	} // while

	for (auto& e : objectMap)
	{
		auto itr =
			std::find_if
			(
				gameObjectList.begin(),
				gameObjectList.end(),
				[&e](const GameObjectPtr& go)
				{
					return e.parent == go->name;
				}
			);
		if (itr != gameObjectList.end())
			e.go->SetParent(*itr);
	}

	// 作成したゲームオブジェクト配列を返す
	LOG("ゲームオブジェクト配置ファイル%sを読み込みました", filename);
	return gameObjectList;
}

/// <summary>
/// ゲームエンジンから全てのゲームオブジェクトを破棄する
/// </summary>
void Engine::ClearGameObjectAll()
{
	for (auto& e : gameObjects)
		e->OnDestroy();

	gameObjects.clear();
}

/// <summary>
/// 垂直視野角を設定する
/// </summary>
/// <param name="fovY">設定する垂直視野角(度数法)</param>
void Engine::SetFovY(float fovY)
{
	degFovY = fovY;
	radFovY = degFovY * 3.1415926535f / 180; // 弧度法に変換
	fovScale = 1 / tan(radFovY / 2);		 // 視野角による拡大率
}

/// <summary>
/// マウスカーソルの座標を取得する
/// </summary>
/// <returns>カメラ座標系のカーソル座標</returns>
vec2 Engine::GetMousePosition() const
{
	// スクリーン座標系のカーソル座標を取得する
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	// 戻り型に合わせるためにfloatにキャスト
	const vec2 pos = { static_cast<float>(x),static_cast<float>(y) };

	// UILayerの座標系に合わせるために、スクリーン座標系からカメラ座標系(Z=-1)に変換する
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	const vec2 framebufferSize = { static_cast<float>(w),static_cast<float>(h) };
	const float aspectRatio = framebufferSize.x / framebufferSize.y;
	return
	{
		(pos.x / framebufferSize.x * 2 - 1) * aspectRatio,
		(pos.y / framebufferSize.y * 2 - 1) * -1
	};
}

/// <summary>
/// マウスボタンの状態を取得する
/// </summary>
/// <param name="button">入力されたマウスボタンを示すマクロ定数(GLFW_MOUSE_BUTTON_LEFT等)</param>
/// <returns>true = 押されている、false = 押されていない</returns>
bool Engine::GetMouseButton(int button) const
{
	return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

/// <summary>
/// マウスボタンのクリック状態を取得する
/// </summary>
/// <param name="button">入力されたマウスボタン(ボタンを示すマクロ定数(GLFW_MOUSE_BUTTON_LEFT等))</param>
/// <returns>false = 押されていない、false以外 = 押された</returns>
bool Engine::GetMouseClick(int button) const
{
	// 範囲外のボタン番号は無視
	if (button<GLFW_MOUSE_BUTTON_LEFT
		|| button>GLFW_MOUSE_BUTTON_MIDDLE)
		return false;
	
	return mouseButtons[button].click;
}

/// <summary>
/// フレームバッファの大きさを取得する
/// </summary>
/// <returns>フレームバッファの縦と横のサイズ</returns>
vec2 Engine::GetFramebufferSize() const
{
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	return { static_cast<float>(w),static_cast<float>(h) };
}

/// <summary>
/// フレームバッファのアスペクト比を取得する
/// </summary>
/// <returns>フレームバッファのアスペクト比</returns>
float Engine::GetAspectRatio() const
{
	const vec2 size = GetFramebufferSize();
	return size.x / size.y;
}

/// <summary>
/// テクスチャを取得する
/// </summary>
/// <param name="name">テクスチャファイル名</param>
/// <returns>名前がnameと一致するテクスチャ</returns>
TexturePtr Engine::GetTexture(const char* name)
{
	// キャッシュにあれば、キャッシュされたテクスチャを返す
	auto itr = textureCache.find(name);
	if (itr != textureCache.end())
		return itr->second; // キャッシュされたテクスチャを返す

	// キャッシュになければ、テクスチャを作成してキャッシュに登録
	auto tex = std::make_shared<TEX_HELPER>(name);
	textureCache.emplace(name, tex);
	return tex; // 作成したテクスチャを返す
}

/// <summary>
/// キューブマップテクスチャを読み込む
/// </summary>
/// <param name="name">テクスチャ名</param>
/// <param name="cubemapFilenames">キューブマップ用の画像ファイル名の配列</param>
/// <returns>名前がnameと一致するテクスチャ</returns>
TexturePtr Engine::LoadCubemapTexture
(
	const char* name,
	const char* const cubemapFilenames[6]
)
{
	// キャッシュにあれば、キャシュされたテクスチャを返す
	auto itr = textureCache.find(name);
	if (itr != textureCache.end())
		return itr->second; // キャッシュされたテクスチャを返す

	// キャッシュになければ、テクスチャを作成してキャッシュに登録
	auto tex = std::make_shared<TEX_HELPER>(name, cubemapFilenames);
	textureCache.emplace(name, tex);
	return tex; // 作成したテクスチャを返す
}

// ライト配列を初期化する
void Engine::InitializeLight()
{
	// 指定された数のライトを生成する
	lights.resize(lightResizeCount);
	usedLights.reserve(lightResizeCount);

	// すべてのライトを未使用ライト配列に「逆順」で追加する
	freeLights.resize(lightResizeCount);
	for (int i = 0; i < lightResizeCount; ++i)
		freeLights[i] = static_cast<int>(lightResizeCount - i - 1);
}

/// <summary>
/// 新しいライトを取得する
/// </summary>
/// <returns>ライトのインデックス</returns>
int Engine::AllocateLight()
{
	// 未使用のライトがなければライト配列を拡張する
	if (freeLights.empty())
	{
		const size_t oldSize = lights.size();
		lights.resize(oldSize + lightResizeCount);
		// 拡張したライトを未使用ライト配列に「逆順」で追加する
		for (size_t i = lights.size()-1; i >= oldSize; --i)
			freeLights.push_back(static_cast<int>(i));
	}
	
	// 未使用ライト配列の末尾からインデックスを取り出す
	const int index = freeLights.back();
	freeLights.pop_back();

	// 取り出したインデックスを使用中ライト配列に追加する
	usedLights.push_back(index);

	// ライトの状態を「使用中」にする
	lights[index].isUsed = true;

	// 取り出したインデックスを返す
	return index;
}

/// <summary>
/// ライトを解放する
/// </summary>
/// <param name="index">解放するライトのインデックス</param>
void Engine::DeallocateLight(int index)
{
	if (index >= 0 && index < lights.size())
	{
		// インデックスを未使用ライト配列に追加する
		freeLights.push_back(index);

		// ライトの状態を「未使用」にする
		lights[index].isUsed = false;
	}
}

/// <summary>
/// インデックスに対応するライトデータを取得する
/// </summary>
/// <param name="index">ライトのインデックス</param>
/// <returns>nullptr以外 = indexに対応するライトデータのアドレス、nullptr = 対応するライトデータが存在しない</returns>
LIGHT_DATA* Engine::GetLight(int index)
{
	if (index >= 0 && index < lights.size() && lights[index].isUsed)
		return &lights[index];

	return nullptr;
}

/// <summary>
/// インデックスに対応するライトデータを取得する
/// </summary>
/// <param name="index">ライトのインデックス</param>
/// <returns>nullptr以外 = indexに対応するライトデータのアドレス、nullptr = 対応するライトデータが存在しない</returns>
const LIGHT_DATA* Engine::GetLight(int index) const
{
	return const_cast<Engine*>(this)->GetLight(index);
}

// マウス座標から発射される光線を取得する
RAY Engine::GetRayFromMousePosition() const
{
	// 座標系のマウスカーソル座標を取得する
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	// スクリーン座標系からNDC座標系に変換する
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	vec3 nearPos = {
		static_cast<float>(x / w * 2 - 1),
		-static_cast<float>(y / h * 2 - 1),
		-1
	};
	vec3 farPos = { nearPos.x,nearPos.y,1 };

	// 深度値の計算結果が-1~+1になるようなパラメータA,Bを計算する
	// 頂点シェーダの値を一致させること
	const float near = 0.35f;
	const float far = 1000;
	const float A = -2 * far * near / (far - near);
	const float B = (far + near) / (far - near);

	// NDC座標系からクリップ座標系に変換する
	nearPos *= near;
	farPos *= far;
	nearPos.z=(nearPos.z - A) / B;
	farPos.z = (farPos.z - A) / B;

	// クリップ座標系からビュー座標系に変換する
	const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
	const float invFovScale = 1.0f/GetFovScale();
	nearPos.x *= invFovScale * aspectRatio;
	nearPos.y *= invFovScale;
	farPos.x *= invFovScale * aspectRatio;
	farPos.y *= invFovScale;

	// ビュー座標系からワールド座標系に変換する
	const float cameraSinY = std::sin(camera.rotation.y);
	const float cameraCosY = std::cos(camera.rotation.y);
	nearPos = 
	{
		nearPos.x * cameraCosY - near * cameraSinY,
		nearPos.y,
		nearPos.x * -cameraSinY - near * cameraCosY
	};
	nearPos += camera.position;

	farPos = 
	{
		farPos.x * cameraCosY - far * cameraSinY,
		farPos.y,
		farPos.x * -cameraSinY - far * cameraCosY
	};
	farPos += camera.position;

	// 近平面の座標と遠平面の座標から光線の向きベクトルを求める
	vec3 direction = farPos - nearPos;
	const float length =
		sqrt
		(
			direction.x * direction.x +
			direction.y * direction.y +
			direction.z * direction.z
		);
	direction *= 1.0f / length;

	return RAY{ nearPos,direction };
}

/// <summary>
/// 光線とコライダーの交差判定
/// </summary>
/// <param name="ray">光線</param>
/// <param name="hitInfo">光線と最初に交差したコライダーの情報</param>
/// <param name="pred">交差判定を行うコライダーを選別する述語</param>
/// <returns>true = コライダーと交差した、false = どのコライダーとも交差しなかった</returns>
bool Engine::Raycast
(
	const RAY& ray,
	RAYCAST_HIT& hitInfo,
	const RaycastPredicate& pred
) const
{
	// 交点の情報を初期化
	hitInfo.collider = nullptr;
	hitInfo.distance = FLT_MAX;

	for (const auto& go : gameObjects)
		for (const auto& collider : go->colliders)
		{
			// コライダーをワールド座標系に変換する
			const auto worldCollider =
				collider->GetTransformedCollider(go->GetTransformMatrix());

			// 光線との交差判定
			float d;
			bool hit = false;
			switch (collider->GetType())
			{
			case Collider::TYPE_SHAPE::AABB:
				hit = Intersect(static_cast<AabbCollider&>(*worldCollider).aabb, ray, d);
				break;
			case Collider::TYPE_SHAPE::SPHERE:
				hit = Intersect(static_cast<SphereCollider&>(*worldCollider).sphere, ray, d);
				break;
			case Collider::TYPE_SHAPE::BOX:
				hit = Intersect(static_cast<BoxCollider&>(*worldCollider).box, ray, d);
				break;
			}
			if (!hit)
				continue;

			// 交差判定の対象でなければ飛ばす
			if (!pred(collider, d))
				continue;

			// より発射点に近い交点を持つコライダーを選ぶ
			if (d < hitInfo.distance)
			{
				hitInfo.collider = collider;
				hitInfo.distance = d;
			}
		} // for colliders

	// 交差するコライダーがあればtrue、なければfalseを返す
	if (hitInfo.collider)
	{
		//交点の座標を計算する
		hitInfo.point 
			= ray.start + ray.direction * hitInfo.distance;
		return true;
	}

	return false;
}

/// <summary>
/// ゲームエンジンを初期化する
/// </summary>
/// <returns>0 = 正常に初期化された、0以外 = エラーが発生した</returns>
int Engine::Initialize()
{
	// GLFWライブラリを初期化する
	if (glfwInit() != GLFW_TRUE)
		return 1; // 初期化失敗

	// 描画ウインドウを作成する
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	window =
		glfwCreateWindow
		(
			1280,
			720,
			title.c_str(),
			nullptr,
			nullptr
		);
	if (!window)
	{
		glfwTerminate();
		return 1; //ウインドウ作成失敗
	}

	// OpenGLコンテキストを作成する
	glfwMakeContextCurrent(window);

	// キャスト(型変換)について
	// const_cast・・・constの付け外し
	// static_cast・・・ポインタを除く型変換
	// reinterpret_cast・・・ポインタの型変換
	// dynamic_cast・・・基底クラスから派生クラスへのキャスト

	// OpenGL関数のアドレスを取得する
	// ライブラリ間の不整合を解消するためにreinterpret_castを使う
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		glfwTerminate();
		return 1; // アドレス取得失敗
	}

	// メッセージコールバックを設定する
	glDebugMessageCallback(DebugCallback, nullptr);

	// ライトデータを初期化する
	InitializeLight();

	// シェーダを読み込んでコンパイル
	prog3D =
		std::make_shared<ProgramObject>
		(
			"Res/standard.vert",
			"Res/standard.frag"
		);

	// スケルタルメッシュ・シェーダを作成する
	progSkeletal =
		std::make_shared<ProgramObject>
		(
			"Res/skeletal.vert",
			"Res/standard.frag"
		);

	progShadowSkeletal =
		std::make_shared<ProgramObject>
		(
			"Res/shadow_skeletal.vert",
			"Res/shadow.frag"
		);

	progHighPassFilter =
		std::make_shared<ProgramObject>
		(
			"Res/simple.vert",
			"Res/high_pass_filter.frag"
		);
	progDownSampling =
		std::make_shared<ProgramObject>
		(
			"Res/simple.vert",
			"Res/down_sampling.frag"
		);
	progUpSampling =
		std::make_shared<ProgramObject>
		(
			"Res/simple.vert",
			"Res/up_sampling.frag"
		);

	progImageProcessing =
		std::make_shared<ProgramObject>
		(
			"Res/simple.vert",
			"Res/image_processing.frag"
		);

	progSAORestoreDepth = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/sao_restore_depth.frag");
	progSAODownSampling = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/sao_down_sampling.frag");
	progSAO = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/sao.frag");
	progSAOBlur = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/sao_blur.frag");

	// テクスチャを描画するだけのシェーダ
	progSimple = std::make_shared<ProgramObject>(
		"Res/simple.vert", "Res/simple.frag");

	// シャドウ・シェーダを作成
	progShadow =std::make_shared<ProgramObject>(
		"Res/shadow.vert","Res/shadow.frag"	);

	// カットオフ値を設定しておく
	glProgramUniform1f(*progShadow, locAlphaCutoff, 0.5f);
	glProgramUniform1f(*progShadowSkeletal, locAlphaCutoff, 0.5f);

	// FBOを作成する
	CreateMainFBO(1920, 1080);
	auto texShadow =
		std::make_shared<TEX_HELPER>
		(
			"FBO(depth)",
			2048,
			2048,
			GL_DEPTH_COMPONENT32
		);
	fboShadow = 
		std::make_shared<FramebufferObject>
		(
			nullptr,
			texShadow
		);

	// アンリット・シェーダを作成する
	progUnlit =
		std::make_shared<ProgramObject>
		(
			"Res/unlit.vert",
			"Res/unlit.frag"
		);

	// 関節用のSSBOを作成する
	for (size_t i = 0; i < std::size(ssboJointMatrices); ++i)
		ssboJointMatrices[i] = BufferObject::Create(jointMatricesBytes);
	jointMatrixIndex = 0;

	// 頂点データをGPUメモリにコピー
	const VERTEX vertexData[] =
	{
		// +Z(手前の面)
		{{-1,-1,1},{0,0}},
		{{1,-1,1},{1,0}},
		{{1,1,1},{1,1}},
		{{-1,1,1},{0,1}},

		// -Z(奥の面)
		{{1,-1,-1},{0,0}},
		{{-1,-1,-1},{1,0}},
		{{-1,1,-1},{1,1}},
		{{1,1,-1},{0,1}},

		// +Y(上の面)
		{{1,1,1},{0.0}},
		{{1,1,-1},{1,0}},
		{{-1,1,-1},{1,1}},
		{{-1,1,1},{0,1}},

		// -Y(下の面)
		{{1,-1,-1},{0,0}},
		{{1,-1,1},{1,0}},
		{{-1,-1,1},{1,1}},
		{{-1,-1,-1},{0,1}},

		// +X
		{{1,-1,1},{0,0}},
		{{1,-1,-1},{1,0}},
		{{1,1,-1},{1,1}},
		{{1,1,1},{0,1}},

		// -X
		{{-1,-1,1},{0,0}},
		{{-1,-1,-1},{1,0}},
		{{-1,1,-1},{1,1}},
		{{-1,1,1},{0,1}},
	};

	const GLushort indexData[] =
	{
		0,1,2,2,3,0,
		4,5,6,6,7,4,
		8,9,10,10,11,8,
		12,13,14,14,15,12,
		16,17,18,18,19,16,
		20,21,22,22,23,20,
	};

	//図形データの情報
	struct MESH_DATA
	{
		const char* name;            // メッシュ名
		const char* textureFilename; // テクスチャファイル名
		size_t vertexSize;			 // 頂点データのバイト数
		size_t indexSize;			 // インデックスデータのバイト数
		const void* vertexData;		 // 頂点データのアドレス
		const void* indexData;		 // インデックスデータのアドレス
	};
	const MESH_DATA meshes[] =
	{
		{ 
			"crystal",
			nullptr,
			sizeof(crystal_vertices),
			sizeof(crystal_indices),
			crystal_vertices,
			crystal_indices,
		},
		{
			"plane_xy",
			nullptr,
			sizeof(plane_xy_vertices),
			sizeof(plane_xy_indices),
			plane_xy_vertices,
			plane_xy_indices
		},
	};

	// 図形データから描画パラメータを作成し、データをGPUメモリにコピーする
	meshBuffer = MeshBuffer::Create(32'000'000);
	meshBuffer->SetTextureCallback([this](const char* filename)
		{ return GetTexture(filename); });
	for (const auto& e : meshes)
	{
		// 法線を設定するために図形データのコピーを作る
		auto pVertex = static_cast<const VERTEX*>(e.vertexData);
		auto pIndex = static_cast<const uint16_t*>(e.indexData);
		std::vector<VERTEX> v(pVertex, pVertex + e.vertexSize / sizeof(VERTEX));

		// コピーした図形データに法線を設定する
		for (auto& e : v)
			e.normal = { 0,0,0 };
		FillMissingNormals(v.data(), v.size(), pIndex, e.indexSize / sizeof(uint16_t));

		// 法線を設定した図形データをGPUメモリにコピー
		meshBuffer->AddVertexData(v.data(), e.vertexSize, pIndex, e.indexSize);

		// 直前のAddVertexDataで作成した描画パラメータを取得する
		const DRAW_PARAMS& drawParams =
			meshBuffer->GetDrawParams(meshBuffer->GetDrawParamsCount() - 1);

		// テクスチャを作成する
		TexturePtr texBaseColor;
		if (e.textureFilename)
			texBaseColor = GetTexture(e.textureFilename);

		// 描画パラメータとテクスチャからスタティックメッシュを作成する
		meshBuffer->CreateStaticMesh(e.name, drawParams, texBaseColor);
	}

	// OBJファイルを読み込む
	skySphere = meshBuffer->LoadOBJ("Res/MeshData/sky_sphere/sky_sphere.obj");

	// ドラゴン
	{
		MeshBuffer::SKELETON skeleton;
		skeleton.bones.resize(9);
		skeleton.bones[0] = { { 0.00f, 0.00f, 0.00f }, { 0.00f, 0.38f, 0.87f },-1, 0.7f }; // 胴体
		skeleton.bones[1] = { { 0.00f, 0.86f, 2.86f }, { 0.00f, 0.31f, 2.79f }, 0, 0.5f }; // 頭
		skeleton.bones[2] = { {-0.40f, 0.50f, 1.80f }, {-1.51f, 0.61f, 1.48f }, 0, 0.5f }; // 右腕
		skeleton.bones[3] = { { 0.40f, 0.50f, 1.80f }, {-1.51f, 0.61f, 1.48f }, 0, 0.5f }; // 左腕
		skeleton.bones[4] = { {-0.34f, 0.22f, 1.23f }, {-0.48f,-0.25f, 0.17f }, 0, 0.3f }; // 右足
		skeleton.bones[5] = { { 0.34f, 0.22f, 1.23f }, {-0.48f,-0.25f, 0.17f }, 0, 0.3f }; // 左足
		skeleton.bones[6] = { {-0.60f,-0.03f, 2.21f }, {-2.08f,-0.21f, 2.50f }, 0, 0.3f }; // 右翼
		skeleton.bones[7] = { { 0.60f,-0.03f, 2.21f }, { 2.08f,-0.21f, 2.50f }, 0, 0.3f }; // 左翼
		skeleton.bones[8] = { { 0.00f,-0.66f, 0.93f }, { 0.00f,-3.45f, 0.35f }, 0, 0.5f }; // 尾
		meshBuffer->LoadOBJ("Res/MeshData/free_rocks/Dragon.obj", skeleton);
	}
	// オーク
	{
		MeshBuffer::SKELETON skeleton;
		skeleton.bones.resize(6);
		skeleton.bones[0] = { { 0.0f, 1.00f,-0.2f }, { 0.0f, 1.80f,-0.2f },-1, 0.7f }; // 胴体
		skeleton.bones[1] = { { 0.0f, 1.70f, 0.1f }, { 0.0f, 1.70f, 0.3f }, 0, 0.5f }; // 頭
		skeleton.bones[2] = { {-0.4f, 1.65f,-0.2f }, {-1.6f, 1.65f,-0.2f }, 0, 0.5f }; // 右腕
		skeleton.bones[3] = { { 0.4f, 1.65f,-0.2f }, { 1.6f, 1.65f,-0.2f }, 0, 0.5f }; // 左腕
		skeleton.bones[4] = { {-0.2f, 0.90f,-0.2f }, {-0.2f, 0.00f,-0.2f }, 0, 0.3f }; // 右足
		skeleton.bones[5] = { { 0.2f, 0.90f,-0.2f }, { 0.2f, 0.00f,-0.2f }, 0, 0.3f }; // 左足
		meshBuffer->LoadOBJ("Res/MeshData/orcshaman/orcshaman_fighter.obj",skeleton);
		meshBuffer->LoadOBJ("Res/MeshData/orcshaman/orcshaman_magician.obj",skeleton);
	}
	meshBuffer->LoadOBJ("Res/MeshData/arm_and_hand/arm_and_hand_grab.obj");
	meshBuffer->LoadOBJ("Res/MeshData/arm_and_hand/sword.obj");

	// ゲームオブジェクト配列の容量を予約する
	gameObjects.reserve(1000);

	// カメラの初期設定をする
	camera.position = { 3,1,3 };
	camera.rotation.y = 3.14159265f;

	return 0;
}

/// <summary>
/// FBOを作成する
/// </summary>
/// <param name="width">メインFBOの幅(ピクセル単位)</param>
/// <param name="height">メインFBOの高さ(ピクセル単位)</param>
void Engine::CreateMainFBO
(
	int width,
	int height
)
{
	// 以前のFBOを削除する
	fboMain.reset();

	// 新しいFBOを作成する
	auto texMainColor =
		std::make_shared<TEX_HELPER>
		(
			"FBO(main color)",
			width,
			height,
			GL_RGBA16F
		);
	auto texMainDepth =
		std::make_shared<TEX_HELPER>
		(
			"FBO(main depth)",
			width,
			height,
			GL_DEPTH_COMPONENT32
		);
	fboMain = 
		std::make_shared<FramebufferObject>
		(
			texMainColor,
			texMainDepth
		);

	// SAO用FBOを作成する
	const int maxMipLevel = static_cast<int>(std::size(fboSAODepth));
	auto texSAODepth = std::make_shared<TEX_HELPER>(
		"FBO(sao depth)", width / 2, height / 2, GL_R32F, maxMipLevel);
	for (int level = 0; level < maxMipLevel; ++level)
		fboSAODepth[level] = std::make_shared<FramebufferObject>(texSAODepth, nullptr, level, 0);

	auto texSAO = std::make_shared<TEX_HELPER>(
		"FBO(sao)", width / 2, height / 2, GL_R8);
	fboSAO = std::make_shared<FramebufferObject>(texSAO, nullptr);
	auto texSAOBlur = std::make_shared<TEX_HELPER>(
		"FBO(sao blur)", width / 2, height / 2, GL_R8);
	glTextureParameteri(*texSAOBlur, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
	glTextureParameteri(*texSAOBlur, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
	glTextureParameteri(*texSAOBlur, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
	glTextureParameteri(*texSAOBlur, GL_TEXTURE_SWIZZLE_A, GL_RED);
	fboSAOBlur = std::make_shared<FramebufferObject>(texSAOBlur, nullptr);

	// ブルーム用FBOを作成する
	fboBloom.clear();
	fboBloom.resize(6);
	width /= 4; // 0番のブルーム用FBOは縦横1/4に縮小
	height /= 4;
	char name[] = "FBO(bloom[0])";
	for (int i = 0; i < fboBloom.size(); ++i)
	{
		name[10] = '0' + i;
		auto texColor = 
			std::make_shared<TEX_HELPER>
			(
				name,
				width,
				height,
				GL_RGBA16F
			);
		fboBloom[i] = 
			std::make_shared<FramebufferObject>
			(
				texColor,
				nullptr
			);
		width /= 2; // 1番以降のFBOは前段の縦横1/2に縮小
		height /= 2;
	} // for i
}

/// <summary>
/// ゲームエンジンの状態を更新する
/// </summary>
void Engine::Update()
{
	// デルタタイム(前回の更新からの経過時間)を計算
	const double currentTime = glfwGetTime(); // 現在時刻
	deltaTime = static_cast<float>(currentTime - previousTime);
	previousTime = currentTime;

	// 経過時間が長すぎる場合は適当に短くする(主にデバッグ対策)
	if (deltaTime >= 0.5f)
		deltaTime = 1.0f / 60.0f;

	// マウスボタンの状態を取得
	for (int i = 0; i < std::size(mouseButtons); ++i)
	{
		MOUSEBUTTON& e = mouseButtons[i];
		e.previous = e.current;
		e.current = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT + i);

		// 押されている時間を更新
		(e.previous) ? e.timer += deltaTime : e.timer = 0;

		// 以下の条件をすべて満たす場合はクリックと判定
		// -前フレームで押されている
		// -現在フレームで押されていない
		// -押されている時間が0.3秒以下
		(e.previous && !e.current && e.timer <= mouseClickSpeed) ? e.click = true : e.click = false;
	} // for i

	// マウス座標を取得する
	oldMousePosition = mousePosition;
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	mousePosition.x = static_cast<float>(x);
	mousePosition.y = static_cast<float>(y);
	mouseMovement = mousePosition - oldMousePosition;

	// シーンを切り替える
	if (nextScene)
	{
		if (scene)
			scene->Finalize(*this);
		
		nextScene->Initialize(*this);
		scene = std::move(nextScene);
	}

	// シーンを更新する
	if (scene)
		scene->Update(*this, deltaTime);

	UpdateGameObject(deltaTime);
	UpdateJointMatrix();
	HandleGameObjectCollision();
}

/// <summary>
/// ゲームエンジンの状態を描画する
/// </summary>
void Engine::Render()
{
	// ゲームオブジェクトをレンダーキュー順に並べ替える
	std::stable_sort
	(
		gameObjects.begin(),
		gameObjects.end(),
		[](const GameObjectPtr& a, const GameObjectPtr& b)
		{
			return a->render_queue < b->render_queue;
		}
	);

	// transparentキューの先頭を検索する
	const auto transparentBegin = std::lower_bound
	(
		gameObjects.begin(),
		gameObjects.end(),
		RENDER_QUEUE_TRANSPARENT,
		[](const GameObjectPtr& e, int value)
		{
			return e->render_queue < value;
		}
	);

	// フレームバッファの大きさを取得する
	int fbWidth, fbHeight;
	glfwGetFramebufferSize
	(
		window,
		&fbWidth,
		&fbHeight
	);

	// ブルームエフェクトを描画する
	DrawBloomEffect();

	// 描画先をデフォルトフレームバッファに戻す
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, fbWidth, fbHeight);

	// 3D描画の結果を画像処理してデフォルトフレームバッファに書き込む
	DrawMainImageToDefaultFramebuffer();

	// overlayキューの先頭を検索する
	const auto overlayBegin = std::lower_bound
	(
		transparentBegin,
		gameObjects.end(),
		RENDER_QUEUE_OVERLAY,
		[](const GameObjectPtr& e, int value)
		{
			return e->render_queue < value;
		}
	);

	// デプスシャドウマップを作成する
	CreateShadowMap(gameObjects.begin(), transparentBegin);

	// 描画先を3D描画用FBOに変更する
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMain);
	glViewport(0, 0, fboMain->GetWidth(), fboMain->GetHeight());

	// バックバッファをクリア
	glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const GLuint programs[] = { *prog3D,*progUnlit,*progSkeletal };
	for (auto prog : programs)
	{
		// アスペクト比と視野角を設定する
		const float aspectRatio =
			static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
		glProgramUniform2f(prog, 3, fovScale / aspectRatio, fovScale);

		// カメラパラメータを設定する
		glProgramUniform3fv(prog, 4, 1, &camera.position.x);
		glProgramUniform2f(prog, 5, sin(-camera.rotation.y), cos(-camera.rotation.y));
		glProgramUniform2f(prog, 6, sin(-camera.rotation.x), cos(-camera.rotation.x));
	}
	// 深度テストを有効化する
	glEnable(GL_DEPTH_TEST);

	// 半透明合成を有効化する
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	UpdateShaderLight();

	// 環境マッピング用キューブマップを設定
	if (texEnvironment)
	{
		const GLuint tex = *texEnvironment;
		glBindTextures(4, 1, &tex);
	}

	glUseProgram(*prog3D);

	// transparent以前のキューを描画する
	glProgramUniform1f(*prog3D, locAlphaCutoff, 0.5f);
	DrawGameObject(*prog3D, gameObjects.begin(), transparentBegin);
	glProgramUniform1f(*prog3D, locAlphaCutoff, 0);

	// アンビエントオクルージョンを描画する
	DrawAmbientOcclusion();

	// スカイスフィアを描画する
	DrawSkySphere();

	// transparentからoverlayまでのキューを描画する
	glDepthMask(GL_FALSE); // 深度バッファへの書き込みを禁止
	DrawGameObject(*prog3D, transparentBegin, overlayBegin);
	glDepthMask(GL_TRUE);  // 深度バッファへの書き込みを許可

	// overlay以降のキューを描画する
	glDisable(GL_DEPTH_TEST); // 深度テストを無効化
	glUseProgram(*progUnlit);
	DrawGameObject(*progUnlit, overlayBegin, gameObjects.end());

	// 描画完了待ちのため、同期オブジェクトを作成する
	syncJointMatrix[jointMatrixIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	// トリプルバッファのインデックスを更新する
	jointMatrixIndex = (jointMatrixIndex + 1) % std::size(ssboJointMatrices);

#if 0
	// シャドウマップのチェック用
	{
		glBindVertexArray(*meshBuffer->GetVAO());
		MaterialList materials(1, std::make_shared<MATERIAL>());
		materials[0]->texBaseColor = fboShadow->GetDepthTexture();
		mat4 m = GetTransformMatrix(vec3(1), vec3(0), vec3(0, 1.5f, -4));
		glProgramUniformMatrix4fv(*progUnlit, locTransformMatrix, 1, GL_FALSE, &m[0].x);
		Draw(*meshBuffer->GetStaticMesh("plane_xy"), *progUnlit, materials);
		glBindVertexArray(0);
	}
#endif

	glfwSwapBuffers(window);
	glfwPollEvents();
}

/// <summary>
/// ゲームオブジェクト配列を描画する
/// </summary>
/// <param name="prog">描画に使うプログラムパイプラインオブジェクト
/// <param name="begin">描画するゲームオブジェクト範囲の先頭</param>
/// <param name="end">描画するゲームオブジェクト範囲の終端</param>
void Engine::DrawGameObject
(
	ProgramObject& prog,
	GameObjectList::iterator begin,
	GameObjectList::iterator end
)
{
	glBindVertexArray(*meshBuffer->GetVAO());
	for (GameObjectList::iterator i = begin; i != end; ++i)
	{
		const auto& e = *i;

		// メッシュが全く設定されていない場合は描画しない
		if (!(e->staticMesh || e->renderer))
			continue;

		// ユニフォーム変数にデータをコピー
		glProgramUniform4fv(prog, 100, 1, &e->color.x);

		// 座標変換ベクトルの配列をGPUメモリにコピー
		glProgramUniformMatrix4fv(prog, 0, 1, GL_FALSE, &e->GetTransformMatrix()[0].x);
		if (prog != *progUnlit)
			glProgramUniformMatrix3fv(prog, 1, 1, GL_FALSE, &e->GetNormalMatrix()[0].x);

		// スタティックメッシュを描画する
		if (e->staticMesh)
			if (e->materials.empty())
				Draw(*e->staticMesh, prog, e->staticMesh->materials);
			else
				Draw(*e->staticMesh, prog, e->materials);

		// スケルタルメッシュを描画する
		if (e->renderer)
		{
			// スケルタルメッシュ用のシェーダとVAOを割り当てる
			glUseProgram(*progSkeletal);
			glBindVertexArray(*meshBuffer->GetVAOSkeletal());

			e->renderer->Draw(*e, *progSkeletal,
				*ssboJointMatrices[jointMatrixIndex], bpJointMatrices);

			// 元のシェーダとVAOに戻す
			glBindVertexArray(*meshBuffer->GetVAO());
			glUseProgram(prog);
		} // if e->renderer
	}
	glBindVertexArray(0);
}

/// <summary>
/// ゲームオブジェクトの状態を更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void Engine::UpdateGameObject(float deltaTime)
{
	// 要素の追加に対応するため添字を選択する
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* e = gameObjects[i].get();
		if (!e->IsDestroyed())
		{
			e->Start();
			e->Update(deltaTime);
		}
	}

	// ローカル座標変換行列を計算する
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* e = gameObjects[i].get();
		e->matrix_transform = GetTransformMatrix(e->scale, e->rotation, e->position);
		e->matrix_normal = GetRotationMatrix(e->rotation);
	}

	// ワールド座標変換行列を計算する
	std::vector<mat4> worldTransforms(gameObjects.size());
	std::vector<mat3> worldNormals(gameObjects.size());
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		GameObject* e = gameObjects[i].get();
		mat4 m = e->matrix_transform;
		mat3 n = e->matrix_normal;
		int parentJointNo = e->number_joint_parent; // 親の関節番号

		// すべての祖先の座標変換行列を乗算する
		for (e = e->parent; e; e = e->parent)
		{
			// 親の関節番号がある場合、先に関節行列を乗算する
			if (parentJointNo >= 0 && e->renderer)
			{
				const mat4& matJoint = e->renderer->GetJointMatrix(parentJointNo);

				// 関節行列から回転成分を取り出す
				vec3 t, s; // 位置と拡大率は使わない
				mat3 matJointNormal;
				Decompose(matJoint, t, s, matJointNormal);

				// 関節行列を乗算する
				m = matJoint * m;
				n = matJointNormal * n;

				// 次の親の関節番号を取得する
				parentJointNo = e->number_joint_parent;
			}      m = e->matrix_transform * m;
			n = e->matrix_normal * n;
		}
		worldTransforms[i] = m;
		worldNormals[i] = n;
	}

	// ワールド座標変換行列をゲームオブジェクトに設定する
	for (int i = 0; i < gameObjects.size(); ++i)
	{
		gameObjects[i]->matrix_transform = worldTransforms[i];
		gameObjects[i]->matrix_normal = worldNormals[i];
	}
} // UpdateGameObject

/// <summary>
/// 関節行列を更新する
/// </summary>
void Engine::UpdateJointMatrix()
{
	const size_t jointMatrixSize = jointMatricesBytes / sizeof(mat4);
	std::shared_ptr<mat4[]> jointMatrices(new mat4[jointMatrixSize]);
	size_t totalJointCount = 0; // コピーした関節行列の数

	// 書き込み処理を1回で済ませるために、全データをバッファに蓄積
	for (const auto& go : gameObjects)
	{
		if (!go->renderer)
			continue;

		// データサイズを256バイト境界(mat4換算で4個)に切り上げる
		const size_t jointCount =
			((go->renderer->GetJointMatrixCount() + 3) / 4) * 4;

		// 座標変換行列を計算してバッファに設定する
		go->renderer->CalculateJointMatrix(&jointMatrices[totalJointCount]);

		go->renderer->gpuOffset = totalJointCount * sizeof(mat4); //設定した位置を記録する
		totalJointCount += jointCount; // 関節行列の総数を更新する
	}

	// 描画完了を待つ
	if (syncJointMatrix[jointMatrixIndex])
	{
		const GLuint64 timeout = 16'700'000; // 16.7ミリ秒(約1/60秒)
		glClientWaitSync(syncJointMatrix[jointMatrixIndex],
			GL_SYNC_FLUSH_COMMANDS_BIT, timeout);

		// 同期オブジェクトを削除する
		glDeleteSync(syncJointMatrix[jointMatrixIndex]);
		syncJointMatrix[jointMatrixIndex] = 0;
	}

	// 関節行列をGPUメモリにコピー
	if (totalJointCount > 0)
	{
		const size_t size = sizeof(mat4) * totalJointCount;
		ssboJointMatrices[jointMatrixIndex]->CopyData(0, jointMatrices.get(), size);
	}
} // UpdateJointMatrix

/// <summary>
/// ゲームオブジェクト間の衝突を処理する
/// </summary>
void Engine::HandleGameObjectCollision()
{
	// ワールド座標系の衝突判定を作成する
	std::vector<WorldColliderList> colliders;
	colliders.reserve(gameObjects.size());
	for (const auto& e : gameObjects)
	{
		if (e->colliders.empty())
			continue;

		// 「接地していない」状態にする
		e->isGrounded = false;

		// 衝突判定を作成
		WorldColliderList list(e->colliders.size());
		for (int i = 0; i < e->colliders.size(); ++i)
		{
			// オリジナルのコライダーをコピー
			list[i].origin = e->colliders[i];

			// コライダーの座標をワールド座標に変換
			list[i].world = e->colliders[i]->GetTransformedCollider(e->GetTransformMatrix());
		}
		colliders.push_back(list);
	}

	if (colliders.size() >= 2)
	{
		// ゲームオブジェクト単位の衝突判定
		for (auto a = colliders.begin(); a != colliders.end() - 1; ++a)
		{
			const GameObject* goA = a->at(0).origin->GetOwner();
			if (goA->IsDestroyed())
				continue; // 削除済みなので飛ばす
			for (auto b = a + 1; b != colliders.end(); ++b)
			{
				const GameObject* goB = b->at(0).origin->GetOwner();
				if (goB->IsDestroyed())
					continue; //削除済みなので飛ばす
				HandleWorldColliderCollision(&*a, &*b);	// コライダー単位の衝突判定
			} // for b
		} // for a
	}
}

// 

/// <summary>
/// 型によって交差判定関数を呼び分けるための関数テンプレート
/// </summary>
template<typename T, typename U>
bool CallIntersect
(
	const ColliderPtr& a,
	const ColliderPtr& b,
	vec3& p
)
{
	return
		Intersect
		(
			static_cast<T&>(*a).GetShape(),
			static_cast<U&>(*b).GetShape(),
			p
		);
}


/// <summary>
/// 型によって交差判定関数を呼び分けるための関数テンプレート
/// 交差判定関数に渡す引数を逆にするバージョン
/// </summary>
template<typename T, typename U>
bool CallIntersectReverse
(
	const ColliderPtr& a,
	const ColliderPtr& b,
	vec3& p
)
{
	if (Intersect(static_cast<U&>(*b).GetShape(),
		static_cast<T&>(*a).GetShape(), p))
	{
		p *= -1; // 貫通ベクトルを逆向きにする
		return true;
	}
	return false;
}


/// <summary>
/// 常にfalseを返す関数テンプレート
/// 未実装または実装予定のない組み合わせ用
/// </summary>
template<typename T, typename U>
bool NotImplemented
(
	const ColliderPtr& a,
	const ColliderPtr& b,
	vec3& p
)
{
	return false;
}

/// <summary>
/// 貫通ベクトルをゲームオブジェクトに反映する
/// </summary>
/// <param name="worldColliders">
/// 衝突判定に使用されたワールド座標系のコライダーリストへのポインタ
/// </param>
/// <param name="gameObject">
/// 衝突判定の対象となったゲームオブジェクトへのポインタ
/// </param>
/// <param name="penetration">衝突後のオブジェクトの貫通量</param>
void Engine::ApplyPenetration
(
	WorldColliderList* worldColliders,
	GameObject* gameObject,
	const vec3& penetration
)
{
	// 接地判定
	// 衝突ベクトルが垂直に近い場合に、床に触れたとみなす
	static const float cosGround = cos(radians(30)); // 床とみなす角度
	if (penetration.y > 0)
	{
		// 対象が単位垂直ベクトルであることを利用して、内積による角度の比較を単純化
		const float d = length(penetration);
		if (penetration.y >= d * cosGround)
			gameObject->isGrounded = true; // 接地した
	} // if penetration.y

	// ゲームオブジェクトを移動
	gameObject->position += penetration;

	// 全てのワールドコライダーを移動
	for (auto& e : *worldColliders)
		e.world->AddPosition(penetration);
}

/// <summary>
/// コライダー単位の衝突判定
/// </summary>
/// <param name="a">判定対象のワールドコライダー配列その1</param>
/// <param name="b">判定対象のワールドコライダー配列その2</param>
void Engine::HandleWorldColliderCollision(WorldColliderList* a, WorldColliderList* b)
{
	// 関数ポインタ型を定義する
	using FuncType = bool(*)(const ColliderPtr&, const ColliderPtr&, vec3&);

	// 組み合わせに対応する交差判定関数を選ぶための配列
	static const FuncType funcList[3][3] =
	{
		{
			CallIntersect<AabbCollider, AabbCollider>,
			CallIntersect<AabbCollider, SphereCollider>,
			NotImplemented<AabbCollider, BoxCollider>,
		},
		{
			CallIntersectReverse<SphereCollider, AabbCollider>,
			CallIntersect<SphereCollider, SphereCollider>,
			CallIntersectReverse<SphereCollider, BoxCollider>,
		},
		{
			NotImplemented<BoxCollider, AabbCollider>,
			CallIntersect<BoxCollider, SphereCollider>,
			NotImplemented<BoxCollider, BoxCollider>,
		},
	};

	// コライダー単位の衝突判定
	for (auto& colA : *a)
	{
		for (auto& colB : *b)
		{
			// スタティックコライダー同士は衝突しない
			if (colA.origin->isStatic && colB.origin->isStatic)
				continue;

			// 衝突判定を行う
			vec3 penetration;
			const int typeA = static_cast<int>(colA.origin->GetType());
			const int typeB = static_cast<int>(colB.origin->GetType());
			if (funcList[typeA][typeB](colA.world, colB.world, penetration))
			{
				GameObject* goA = colA.origin->GetOwner();
				GameObject* goB = colB.origin->GetOwner();

				// コライダーが重ならないように座標を調整
				if (!colA.origin->isTrigger && !colB.origin->isTrigger)
					if (colA.origin->isStatic)
						//Aは動かないのでBを移動させる
						ApplyPenetration(b, goB, penetration);
					else if (colB.origin->isStatic)
						//Bは動かないのでAを移動させる
						ApplyPenetration(a, goA, -penetration);
					else
					{
						//AとBを均等に移動させる
						ApplyPenetration(b, goB, penetration * 0.5f);
						ApplyPenetration(a, goA, penetration * -0.5f);
					}

				//イベント関数の呼び出し
				goA->OnCollision(colA.origin, colB.origin);
				goB->OnCollision(colB.origin, colA.origin);

				//イベントの結果、どちらかのゲームオブジェクトが破棄されたらループ終了
				if (goA->IsDestroyed() || goB->IsDestroyed())
					return;	// 関数を終了する
			} // if (funcList[typeA][typeB](colA.world, colB.world, penetration))
		} // for colB
	} // for colA
} // HandleGameObjectCollision

/// <summary>
/// 破棄予定のゲームオブジェクトを削除する
/// </summary>
void Engine::RemoveDestroyedGameObject()
{
	if (gameObjects.empty())
		return;	// ゲームオブジェクトを持っていなければ何もしない

	// 破棄予定の有無でゲームオブジェクトを分ける
	const auto iter =
		std::partition
		(
			gameObjects.begin(),
			gameObjects.end(),
			[](const GameObjectPtr& e) { return !e->IsDestroyed(); }
	);

	// 破棄予定のオブジェクトを別の配列に移動
	GameObjectList destroyList
	(
		std::make_move_iterator(iter),
		std::make_move_iterator(gameObjects.end())
	);

	// 配列から移動済みオブジェクトを削除
	gameObjects.erase(iter, gameObjects.end());

	// 破棄予定のオブジェクトのOnDestroyを実行
	for (auto& e : destroyList)
		e->OnDestroy();

	// ここで実際にゲームオブジェクトが削除される(destroyListの寿命が終わるため)
}

/// <summary>
/// カメラに近いライトを選んでGPUメモリにコピーする
/// </summary>
void Engine::UpdateShaderLight()
{
	const GLuint programs[] = { *prog3D, *progSkeletal };
	for (auto prog : programs)
	{
		// 環境光をGPUメモリにコピー
		glProgramUniform3fv(prog, locAmbientLight, 1, &ambientLight.x);

		// 平行光源のパラメータをGPUメモリにコピー
		const vec3 color = directionalLight.color * directionalLight.intensity;
		glProgramUniform3fv(prog, locDirectionalLightColor, 1, &color.x);
		glProgramUniform3fv(prog, locDirectionalLightDirection,
			1, &directionalLight.direction.x);
	} // for programs

	// コピーするライトがなければライト数を0に設定する
	if (usedLights.empty())
	{
		for (auto prog : programs)
			glProgramUniform1i(prog, locLightCount, 0);
		
		return;
	}

	// 使用中ライトの配列から、未使用になったライトを除外する
	const auto itrUnused = std::remove_if(usedLights.begin(), usedLights.end(),
		[&](int i) { return !lights[i].isUsed; });
	usedLights.erase(itrUnused, usedLights.end());

	// 重複する番号を除去する
	std::sort(usedLights.begin(), usedLights.end());
	auto itr = std::unique(usedLights.begin(), usedLights.end());
	usedLights.erase(itr, usedLights.end());

	// カメラの正面ベクトルを計算する
	const vec3 front = { -sin(camera.rotation.y),0,-cos(camera.rotation.y) };

	// カメラからライトまでの距離を計算する
	struct DISTANCE
	{
		float distance;		 // カメラからライトの半径までの距離
		const LIGHT_DATA* p; // ライトのアドレス
	};
	std::vector<DISTANCE> distanceList;
	distanceList.reserve(lights.size());
	for (auto index : usedLights)
	{
		const auto& light = lights[index];
		const vec3 v = light.position - camera.position;
		// カメラの後ろにあるライトを除外する
		if (dot(front, v) <= -light.radius)
			continue;
		
		const float d = length(v) - light.radius; // カメラからライトの半径までの距離
		distanceList.push_back({ d,&light });
	}

	// 画面に影響するライトがなければライト数を0に設定する
	if (distanceList.empty())
	{
		for (auto prog : programs)
			glProgramUniform1i(prog, locLightCount, 0);

		return;
	}

	// カメラに近いライトを優先する
	std::stable_sort(distanceList.begin(), distanceList.end(),
		[&](const auto& a, const auto& b) { return a.distance < b.distance; });

	// ライトデータをGPUメモリにコピー
	const int lightCount = static_cast<int>(
		std::min(distanceList.size(), maxShaderLightCount)); // コピーするライト数
	std::vector<vec4> colorAndFalloffAngle(lightCount);
	std::vector<vec4> positionAndRadius(lightCount);
	std::vector<vec4> directionAndConeAngle(lightCount);
	for (int i = 0; i < lightCount; ++i)
	{
		const LIGHT_DATA* p = distanceList[i].p;
		const vec3 color = p->color * p->intensity;
		colorAndFalloffAngle[i] =
		{
			color.x,color.y,color.z,p->falloffAngle
		};
		positionAndRadius[i] =
		{
			p->position.x,p->position.y,p->position.z,p->radius
		};
		directionAndConeAngle[i] =
		{
			p->direction.x,p->direction.y,p->direction.z,p->coneAngle
		};
	}
	for (auto prog : programs)
	{
		glProgramUniform4fv(prog, locLightColorAndFalloffAngle, lightCount, &colorAndFalloffAngle[0].x);
		glProgramUniform4fv(prog, locLightPositionAndRadius, lightCount, &positionAndRadius[0].x);
		glProgramUniform4fv(prog, locLightDirectionAndConeAngle, lightCount, &directionAndConeAngle[0].x);
		glProgramUniform1i(prog, locLightCount, lightCount);
	} // for program
}

/// <summary>
/// スカイスフィアを描画する
/// </summary>
void Engine::DrawSkySphere()
{
	// シーンにスカイスフィアが設定されていない場合は描画しない
	if (!skySphere || !scene->material_skysphere)
		return;

	// アンリットシェーダで描画
	glUseProgram(*progUnlit);
	glBindVertexArray(*meshBuffer->GetVAO());
	glDepthMask(GL_FALSE); // 深度バッファへの書き込みを禁止する

	// スカイスフィアモデルの半径を0.5mと仮定し、描画範囲の95%の大きさに拡大する
	static const float far = 1000; // 描画範囲の最大値
	static const float scale = far * 0.95f / 0.5f;
	static const mat4 transformMatrix =
	{
	  vec4(scale, 0,    0,  0),
	  vec4(0, scale,    0,  0),
	  vec4(0,     0, scale, 0),
	  vec4(0,     0,    0,  1),
	};
	glProgramUniformMatrix4fv(*progUnlit,
		locTransformMatrix, 1, GL_FALSE, &transformMatrix[0].x);

	// 色はマテリアルカラーで調整するのでオブジェクトカラーは白に設定する
	static const vec4 color = { 1, 1, 1, 1 };
	glProgramUniform4fv(*progUnlit, locColor, 1, &color.x);

	// スカイスフィアはカメラを原点として描画する
	static const vec3 skySphereCameraPosition = { 0, 0, 0 };
	glProgramUniform3fv(*progUnlit, 4, 1, &skySphereCameraPosition.x);

	// スカイスフィアを描画する
	const MaterialList materials(1, scene->material_skysphere);
	Draw(*skySphere, *progUnlit, materials);

	glProgramUniform3fv(*progUnlit, 4, 1, &camera.position.x); // カメラ座標を元に戻す
	glDepthMask(GL_TRUE);  // 深度バッファへの書き込みを許可する
	glUseProgram(*prog3D); // 3D描画用シェーダに戻す
}

/// <summary>
/// デプスシャドウマップを作成する
/// </summary>
void Engine::CreateShadowMap
(
	GameObjectList::iterator begin,
	GameObjectList::iterator end
)
{
	// 描画先フレームバッファを変更する
	glBindFramebuffer(GL_FRAMEBUFFER, *fboShadow);

	// ビューポートをフレームバッファのサイズに合わせる
	const auto& texShadow = fboShadow->GetTextureDepth();
	glViewport(0, 0, texShadow->GetWidth(), texShadow->GetHeight());

	glEnable(GL_DEPTH_TEST); // 深度テストを有効化
	glDisable(GL_BLEND);     // 半透明合成を無効化

	// 深度バッファをクリア
	glClear(GL_DEPTH_BUFFER_BIT);

	// VAOとシェーダをOpenGLコンテキストに割り当てる
	glBindVertexArray(*meshBuffer->GetVAO());
	glUseProgram(*progShadow);

	// 影の描画パラメータ
	const float shadowAreaSize = 100;  // 影の描画範囲(XY平面)
	const float shadowNearZ = 1;       // 影の描画範囲(近Z平面)
	const float shadowFarZ = 200;      // 影の描画範囲(遠Z平面)
	const float shadowCenterZ = (shadowNearZ + shadowFarZ) * 0.5f; // 描画範囲の中心
	const vec3 target = { 0, 0, 50 }; // カメラの注視点
	const vec3 eye =
		target - directionalLight.direction * shadowCenterZ; // カメラの位置

	// ビュープロジェクション行列を計算する
	const mat4 matShadowView = LookAt(eye, target, vec3(0, 1, 0));
	const mat4 matShadowProj = Orthogonal(
		-shadowAreaSize / 2, shadowAreaSize / 2,
		-shadowAreaSize / 2, shadowAreaSize / 2,
		shadowNearZ, shadowFarZ);
	const mat4 matShadow = matShadowProj * matShadowView;

	// ビュープロジェクション行列をGPUメモリにコピー
	glProgramUniformMatrix4fv(*progShadow,
		locViewProjectionMatrix, 1, GL_FALSE, &matShadow[0].x);
	glProgramUniformMatrix4fv(*progShadowSkeletal,
		locViewProjectionMatrix, 1, GL_FALSE, &matShadow[0].x);

	// メッシュを描画する
	std::vector<const GameObject*> skeletalMeshList;
	skeletalMeshList.reserve(end - begin);
	for (GameObjectList::iterator i = begin; i != end; ++i)
	{
		const auto& e = *i;

		if (!(int(e->type_lighting) & int(GameObject::TYPE_LIGHTING::SHADOW)))
			continue;

		// スケルタルメッシュを記録する
		if (e->renderer)
			skeletalMeshList.push_back(e.get());

		if (!e->staticMesh)
			continue;

		glProgramUniformMatrix4fv(*progShadow,
			locTransformMatrix, 1, GL_FALSE, &e->GetTransformMatrix()[0].x);
		if (e->materials.empty())
			Draw(*e->staticMesh, *progShadow, e->staticMesh->materials);
		else
			Draw(*e->staticMesh, *progShadow, e->materials);

	} // for

	 // スケルタルメッシュを描画する
	glUseProgram(*progShadowSkeletal);
	glBindVertexArray(*meshBuffer->GetVAOSkeletal());
	for (const GameObject* e : skeletalMeshList)
		e->renderer->Draw(*e, *progShadowSkeletal, *ssboJointMatrices[jointMatrixIndex], bpJointMatrices);

	// 深度テクスチャを割り当てる
	glBindTextureUnit(2, *texShadow);

	// 深度テクスチャ座標への変換行列を作成する
	static const mat4 matTexture = {
	  { 0.5f, 0.0f, 0.0f, 0.0f },
	  { 0.0f, 0.5f, 0.0f, 0.0f },
	  { 0.0f, 0.0f, 0.5f, 0.0f },
	  { 0.5f, 0.5f, 0.5f, 1.0f } };

	// シャドウテクスチャ行列をGPUメモリにコピー
	const mat4 matShadowTexture = matTexture * matShadowProj * matShadowView;
	glProgramUniformMatrix4fv(*prog3D,
		locShadowTextureMatrix, 1, GL_FALSE, &matShadowTexture[0].x);

	// 法線方向の補正値を設定する
	const float texelSize = shadowAreaSize / static_cast<float>(texShadow->GetWidth());
	glProgramUniform1f(*prog3D, locShadowNormalOffset, texelSize);
}

/// <summary>
/// アンビエントオクルージョンを描画する
/// </summary>
void Engine::DrawAmbientOcclusion()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glBindVertexArray(*meshBuffer->GetVAO());
	const auto& drawParams =
		meshBuffer->GetStaticMesh("plane_xy")->drawParamsList[0];

	// 深度値を線形距離に変換(fboMain -> fboSAODepth[0])
	glUseProgram(*progSAORestoreDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboSAODepth[0]);
	glViewport(0, 0, fboSAODepth[0]->GetWidth(), fboSAODepth[0]->GetHeight());
	GLuint tex0 = *fboMain->GetTextureDepth();
	glBindTextures(0, 1, &tex0);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// 縮小バッファを作成(fboSAODepth[i - 1] -> fboSAODepth[i])
	glUseProgram(*progSAODownSampling);
	tex0 = *fboSAODepth[0]->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	for (int i = 1; i < std::size(fboSAODepth); ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *fboSAODepth[i]);
		glViewport(0, 0, fboSAODepth[i]->GetWidth(), fboSAODepth[i]->GetHeight());
		glProgramUniform1i(*progSAODownSampling, 200, i - 1);
		glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
			GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);
	}

	// SAO制御用パラメータを設定する
	const float radius = 1.0f; // ワールド座標系におけるAOのサンプリング半径(単位=m)
	const float bias = 0.012f; // 平面とみなして無視する角度のコサイン
	const float intensity = 5; // AO効果の強さ
	glProgramUniform4f(*progSAO, 200, radius * radius,
		0.5f * radius * GetFovScale(), bias, intensity / pow(radius, 6.0f));

	// NDC座標を視点座標に変換するパラメータを設定する
	const float w = static_cast<float>(fboSAODepth[0]->GetWidth());
	const float h = static_cast<float>(fboSAODepth[0]->GetHeight());
	const float aspectRatio = w / h;
	const float invFovScale = 1.0f / GetFovScale();
	glProgramUniform2f(*progSAO, 201, invFovScale * aspectRatio, invFovScale);

	// SAOを計算する
	glUseProgram(*progSAO);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboSAO);
	tex0 = *fboSAODepth[0]->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// SAOの結果をぼかす
	glUseProgram(*progSAOBlur);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboSAOBlur);
	tex0 = *fboSAO->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	GLuint tex1 = *fboSAODepth[0]->GetTextureColor();
	glBindTextures(1, 1, &tex1);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// アンビエントオクルージョンを合成
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(*progSimple);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMain);
	glViewport(0, 0, fboMain->GetWidth(), fboMain->GetHeight());
	tex0 = *fboSAOBlur->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

/// <summary>
/// ブルームエフェクトを描画する
/// </summary>
void Engine::DrawBloomEffect()
{
	// シェーダ共通の設定
	const int bufferCount = static_cast<int>(fboBloom.size());
	const auto& drawParams =
		meshBuffer->GetStaticMesh("plane_xy")->drawParamsList[0];
	glBindVertexArray(*meshBuffer->GetVAO());
	glDisable(GL_DEPTH_TEST);

	// 明るい成分を抽出する
	glDisable(GL_BLEND);
	glUseProgram(*progHighPassFilter);
	glBindFramebuffer(GL_FRAMEBUFFER, *fboBloom[0]);
	glViewport(0, 0, fboBloom[0]->GetWidth(), fboBloom[0]->GetHeight());
	const GLuint tex0 = *fboMain->GetTextureColor();
	glBindTextures(0, 1, &tex0);
	glProgramUniform2f(*progHighPassFilter,
		locHighPassFilter, bloomThreshold, bloomIntensity);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// 縮小ぼかし
	glUseProgram(*progDownSampling);
	for (int i = 1; i < bufferCount; ++i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *fboBloom[i]);
		glViewport(0, 0, fboBloom[i]->GetWidth(), fboBloom[i]->GetHeight());
		const GLuint tex = *fboBloom[i - 1]->GetTextureColor();
		glBindTextures(0, 1, &tex);
		glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
			GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);
	}

	// 拡大ぼかし
	glEnable(GL_BLEND); // 半透明合成を有効化
	glBlendFunc(GL_ONE, GL_ONE); // 1:1の比率で加算合成
	glUseProgram(*progUpSampling);
	for (int i = bufferCount - 2; i >= 0; --i)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, *fboBloom[i]);
		glViewport(0, 0, fboBloom[i]->GetWidth(), fboBloom[i]->GetHeight());
		const GLuint tex = *fboBloom[i + 1]->GetTextureColor();
		glBindTextures(0, 1, &tex);
		glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
			GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);
	}

	// メインFBOにブルームエフェクトを反映する
	glBlendFunc(GL_CONSTANT_COLOR, GL_ONE); // 定数:1の比率で加算合成
	const float c = 1.0f / static_cast<float>(bufferCount); // 定数
	glBlendColor(c, c, c, c); // 定数を設定する
	glBindFramebuffer(GL_FRAMEBUFFER, *fboMain);
	glViewport(0, 0, fboMain->GetWidth(), fboMain->GetHeight());
	const GLuint tex1 = *fboBloom[0]->GetTextureColor();
	glBindTextures(0, 1, &tex1);
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	// ブレンド式を元に戻す
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


/// <summary>
/// 3D描画の結果を画像処理してデフォルトフレームバッファに書き込む
/// </summary>
void Engine::DrawMainImageToDefaultFramebuffer()
{
	// フレームバッファの内容を消去する
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST); // 深度テストを無効化
	glDisable(GL_BLEND);	  // 半透明合成を無効化

	glUseProgram(*progImageProcessing);
	glBindVertexArray(*meshBuffer->GetVAO());

	// OpenGLコンテキストにテクスチャを設定する
	const GLuint tex[] = { *fboMain->GetTextureColor() };
	glBindTextures(0, GLsizei(std::size(tex)), tex);

	//// グレースケールにする色変換行列を設定
	//colorMatrix = {
	//{ 0.2126f, 0.2126f, 0.2126f },
	//{ 0.7152f, 0.7152f, 0.7152f },
	//{ 0.0722f, 0.0722f, 0.0722f }
	// };

	//// セピア調にする色変換行列を設定
	//colorMatrix = {
	//{ 0.346f, 0.278f, 0.196f },
	//{ 0.678f, 0.546f, 0.385f },
	//{ 0.151f, 0.124f, 0.089f },
	//};

	// 色変換行列をGPUメモリにコピー
	glProgramUniformMatrix3fv(*progImageProcessing,
		locColorMatrix, 1, GL_FALSE, &colorMatrix[0].x);

	//// 放射状ブラーのパラメータをGPUメモリにコピー
	//glProgramUniform4f(*progImageProcessing, locRadialBlur,
	//    radialBlurCenter.x, radialBlurCenter.y, radialBlurLength, radialBlurStart);

	// 画面全体に四角形を描画する
	const auto& drawParams =
		meshBuffer->GetStaticMesh("plane_xy")->drawParamsList[0];
	glDrawElementsBaseVertex(drawParams.mode, drawParams.count,
		GL_UNSIGNED_SHORT, drawParams.indices, drawParams.baseVertex);

	glEnable(GL_BLEND); // 半透明合成を有効化
}