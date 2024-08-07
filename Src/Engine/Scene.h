/**
* @file Scene.h
*/
#ifndef SCENE_H_INCLUDED
#define SCENE_H_INCLUDED
#include <memory>

// 先行宣言
class Engine;
struct MATERIAL;
using MaterialPtr = std::shared_ptr<MATERIAL>;

/// <summary>
/// シーンの基底クラス
/// </summary>
class Scene
{
public:
	// デフォルトコンストラクタ
	inline Scene() = default;
	// デフォルトデストラクタ
	inline virtual ~Scene() = default;

	// 基底となるInitialize
	inline virtual bool Initialize(Engine& engine) { return true; }
	// 基底となるUpdate
	inline virtual void Update(Engine& engine,float deltaTime){}
	// 基底となるFinalize
	inline virtual void Finalize(Engine& engine){}

	MaterialPtr material_skysphere; // スカイスフィア用のマテリアル
};

using ScenePtr = std::shared_ptr<Scene>; // シーンポインタ型

#endif // !SCENE_H_INCLUDED