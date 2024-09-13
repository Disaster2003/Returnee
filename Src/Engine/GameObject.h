/**
* @file GameObject.h
*/
#ifndef GAMEOBJECT_H_INCLUDED
#define GAMEOBJECT_H_INCLUDED
#include "AabbCollider.h"
#include "Texture.h"
#include <vector>
#include <type_traits>

//先行宣言
class Engine;
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
using GameObjectList = std::vector<GameObjectPtr>;

struct MESH_STATIC;								   
using StaticMeshPtr = std::shared_ptr<MESH_STATIC>;

class SkeletalMeshRenderer;
using SkeletalMeshRendererPtr = std::shared_ptr<SkeletalMeshRenderer>;

struct MATERIAL;
using MaterialPtr = std::shared_ptr<MATERIAL>;
using MaterialList = std::vector<MaterialPtr>;

// 無効な関節番号
constexpr int number_joint_invalid = -1;

// レンダー・キュー
enum RENDER_QUEUE
{
	RENDER_QUEUE_GEOMETRY = 2000,	 // 一般的な図形
	RENDER_QUEUE_TRANSPARENT = 3000, // 半透明な図形
	RENDER_QUEUE_OVERLAY = 4000,	 // UI、全画面エフェクトなど
	RENDER_QUEUE_MAX = 5000,		 // キューの最大値
};

/// <summary>
/// ゲームに登場するさまざまなオブジェクトを表す基本クラス
/// </summary>
class GameObject
{
	friend Engine;
public:
	// デフォルトコンストラクタ
	GameObject() = default;
	// デストラクタ
	virtual ~GameObject();

	// コピーと代入を禁止する
	GameObject(GameObject&) = delete;
	GameObject& operator=(GameObject&) = delete;

	/// <summary>
	/// ゲームエンジンを取得する
	/// </summary>
	Engine* GetEngine() const { return engine; }

	/// <summary>
	/// ゲームオブジェクトをエンジンから削除する
	/// </summary>
	void Destroy() { isDestroyed = true; }

	/// <summary>
	/// ゲームオブジェクトが破壊されているかを取得する
	/// </summary>
	/// <returns>ゲームオブジェクトがtrue = 破壊されている、false = 破壊されていない</returns>
	bool IsDestroyed() const { return isDestroyed; }


	/// <summary>
	/// ゲームオブジェクトにコンポーネントを追加する
	/// </summary>
	/// <returns>追加したコンポーネント</returns>
	template<typename T>
	std::shared_ptr<T> AddComponent()
	{
		auto p = std::make_shared<T>();
		p->owner = this;
		if constexpr (std::is_base_of_v<Collider, T>)
			colliders.push_back(p);
		components.push_back(p);
		p->Awake();
		return p;
	}

	/// <summary>
	/// ゲームオブジェクトのコンポーネントを取得する
	/// </summary>
	/// <returns>nullptr = コンポーネントなし、nullptr != 発見されたコンポーネント</returns>
	template<typename T>
	std::shared_ptr<T> GetComponent()
	{
		for (auto& e : components)
		{
			// shared_ptrの場合はdynamic_castではなくdynamic_pointer_castを使う
			auto p = std::dynamic_pointer_cast<T>(e);
			if (p)
			{
				return p; // 見つかったコンポーネントを返す
			}
		}
		return nullptr;	  // 見つからなかったのでnullptrを返す
	}

	// ゲームオブジェクトから削除予定のコンポーネントを削除する
	void RemoveDestroyedComponent();

	// イベント制御
	virtual void Start();
	virtual void Update(float deltaTime);
	virtual void OnCollision(const ComponentPtr& self, const ComponentPtr& other);
	virtual void OnDestroy();

	/// <summary>
	/// 親オブジェクトを取得する
	/// </summary>
	/// <returns>nullptr = 親がいない、nullptr != 親オブジェクトのアドレス</returns>
	GameObject* GetParent() const { return parent; }

	// 親オブジェクトを設定する
	void SetParent(GameObject* parent);
	void SetParent(const GameObjectPtr& parent);
	
	// 親オブジェクトと関節番号を設定する
	void SetParentAndJoint(const GameObjectPtr& parent, int number_joint);
	void SetParentAndJoint(GameObject* parent, int number_joint);

	/// <summary>
	/// 子オブジェクトの数を取得する
	/// </summary>
	size_t GetChildCount() const { return children.size(); }

	/// <summary>
	/// 子オブジェクトを取得する
	/// </summary>
	/// <param name="index">
	/// 子オブジェクト配列の添字、GetChildCountが返す以上の値を渡すとエラーになるので注意
	/// </param>
	/// <returns>index番目の子オブジェクト</returns>
	GameObject* GetChild(size_t index) const { return children[index]; }

	/// <summary>
	/// 座標変換行列を取得する
	/// </summary>
	const mat4& GetTransformMatrix() const { return matrix_transform; }

	/// <summary>
	/// ワールド座標を取得する
	/// </summary>
	vec3 GetWorldPosition() const { return vec3(matrix_transform.data[3]); }

	/// <summary>
	/// 法線変換行列を取得する
	/// </summary>
	const mat3& GetNormalMatrix() const { return matrix_normal; }


	std::string name;
	vec3 position = { 0,0,0	};	 // 物体の位置
	vec3 rotation = { 0,0,0	};	 // 物体の回転角度
	vec3 scale = { 1,1,1 };		 // 物体の拡大率
	vec4 color = { 1, 1, 1, 1 }; // 物体の色
	StaticMeshPtr staticMesh;	 // 表示するスタティックメッシュ
	SkeletalMeshRendererPtr renderer; // 表示するスケルタルメッシュ
	MaterialList materials;		 // ゲームオブジェクト固有のマテリアル配列
	int render_queue = RENDER_QUEUE_GEOMETRY; // 描画順
	bool isGrounded = false;	 // 足場となる物体の上にtrue = 乗っている、false = 乗っていない

	int countOfkeepingGrounded = 0;	  // 接地状態を維持する交差判定回数
	vec3 ground = vec3(0, 1, 0);

	/// <summary>
	/// 地面から浮いた
	/// </summary>
	void LiftOffGround()
	{
		isGrounded = false;
		countOfkeepingGrounded = 0;
		ground = vec3(0, 1, 0);
	}

	// どれだけ描画するか(色や影)
	enum class TYPE_LIGHTING : uint8_t
	{
		COLOR = 1,
		SHADOW = 2,
		COLOR_AND_SHADOW = 3,
	};
	TYPE_LIGHTING type_lighting = TYPE_LIGHTING::COLOR_AND_SHADOW;

private:
	Engine* engine = nullptr;			  // エンジンのアドレス
	bool isDestroyed = false;			  // 死亡フラグ
	mat4 matrix_transform = mat4(1);	  // 座標変換行列
	mat3 matrix_normal = mat3(1);		  // 法線変換行列
	GameObject* parent = nullptr;         // 親オブジェクト
	int number_joint_parent = number_joint_invalid; // 親の関節番号
	std::vector<GameObject*> children;    // 子オブジェクト
	std::vector<ComponentPtr> components; // コンポーネント配列
	std::vector<ColliderPtr> colliders;	  // コライダー配列
};

#endif // !GAMEOBJECT_H_INCLUDED