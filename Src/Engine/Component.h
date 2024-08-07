/**
* @file Component.h
*/
#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED
#include <memory>

//先行宣言
class GameObject;
using GameObjectPtr = std::shared_ptr<GameObject>;
class Component;
using ComponentPtr = std::shared_ptr<Component>;

/// <summary>
/// コンポーネントの基底クラス
/// </summary>
class Component
{
	friend GameObject;
public:
	// デフォルトコンストラクタ
	inline Component() = default;
	// デフォルトデストラクタ
	inline virtual ~Component() = default;

	/// <summary>
	/// コンポーネントの所有者を取得する
	/// </summary>
	inline GameObject* GetOwner() const { return owner; }

	/// <summary>
	/// コンポーネントをゲームオブジェクトから削除する
	/// </summary>
	inline void Destroy() { isDestroyed = true; }

	/// <summary>
	/// コンポーネントが破壊されているか取得する
	/// </summary>
	/// <returns>コンポーネントがtrue = 破壊されている、false = 破壊されていない</returns>
	inline bool IsDestroyed() const { return isDestroyed; }

	/// <summary>
	/// ゲームオブジェクトに追加された時に呼び出される
	/// 
	/// 派生クラスによって処理が異なるが、
	/// オーバライドする必然性はないため、
	/// 何も処理を書かない
	/// </summary>
	inline virtual void Awake() {}

	/// <summary>
	/// 最初のUpdateの直前で呼び出される
	/// 
	/// 派生クラスによって処理が異なるが、
	/// オーバライドする必然性はないため、
	/// 何も処理を書かない
	/// </summary>
	inline virtual void Start() {}

	/// <summary>
	/// 毎フレーム1回呼び出される
	/// 
	/// 派生クラスによって処理が異なるが、
	/// オーバライドする必然性はないため、
	/// 何も処理を書かない
	/// </summary>
	/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
	inline virtual void Update(float deltaTime) {}

	/// <summary>
	/// 衝突が起きたときに呼び出される
	/// 
	/// 派生クラスによって処理が異なるが、
	/// オーバライドする必然性はないため、
	/// 何も処理を書かない
	/// </summary>
	/// <param name="self">衝突したコンポーネント(自分)</param>
	/// <param name="other">衝突したコンポーネント(相手)</param>
	inline virtual void OnCollision(
		const ComponentPtr& self, const ComponentPtr& other) {}

	/// <summary>
	/// ゲームオブジェクトがエンジンから削除されるときに呼び出される
	/// 
	/// 派生クラスによって処理が異なるが、
	/// オーバライドする必然性はないため、
	/// 何も処理を書かない
	/// </summary>
	inline virtual void OnDestroy() {}

private:
	GameObject* owner = nullptr; // このコンポーネントの所有者
	bool isStarted = false;		 // Startが　true = 実行中、false =　実行されていない
	bool isDestroyed = false;	 // Destroyがtrue = 実行中、false =　実行されていない
};

#endif // !COMPONENT_H_INCLUDED