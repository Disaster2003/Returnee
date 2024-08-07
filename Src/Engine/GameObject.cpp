/**
* @file GameObject.cpp
*/
#include "GameObject.h"
#include <algorithm>

/// <summary>
/// デストラクタ
/// </summary>
GameObject::~GameObject()
{
	// 死ぬ前に親子関係を解除する
	SetParent(nullptr);
	for (auto child : children)
		child->parent = nullptr;
}

/// <summary>
/// ゲームオブジェクトから削除予定のコンポーネントを削除する
/// </summary>
void GameObject::RemoveDestroyedComponent()
{
	if (components.empty())
		return;	// コンポーネントを持っていなければ何もしない

	// 破棄予定の有無でコンポーネントを分ける
	const auto iter =
		std::stable_partition
		(
			components.begin(),
			components.end(),
			[](const auto& p) { return !p->IsDestroyed(); }
	);

	// 破棄予定のコンポーネントを別の配列に移動する
	std::vector<ComponentPtr> destroyList
	(
		std::move_iterator(iter),
		std::move_iterator(components.end())
	);

	// 配列から移動済みコンポーネントを削除する
	components.erase(iter, components.end());

	// コライダーはcollidersにも登録されているので、両方から削除する必要がある
	const auto iter2 =
		std::remove_if
		(
			colliders.begin(),
			colliders.end(),
			[](const auto& p) { return p->IsDestroyed(); }
		);
	colliders.erase(iter2, colliders.end());

	// 破棄予定のコンポーネントのOnDestroyを実行する
	for (auto& e : destroyList)
		e->OnDestroy();

	// ここで実際にコンポーネントが削除される(destroyListの寿命が終わるため)
}

/// <summary>
/// スタートイベント
/// </summary>
void GameObject::Start()
{
	for(auto& e : components)
	{
		if(!e->isStarted)
			e->Start();

		e->isStarted = true;
	}
}

/// <summary>
/// 更新イベント
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void GameObject::Update(float deltaTime)
{
	for(auto& e : components)
		e->Update(deltaTime);
	
	RemoveDestroyedComponent();
}

/// <summary>
/// 衝突イベント
/// </summary>
/// <param name="self">衝突したコンポーネント(自分)</param>
/// <param name="other">衝突したコンポーネント(相手)</param>
void GameObject::OnCollision
(
	const ComponentPtr& self,
	const ComponentPtr& other
)
{
	for(auto& e : components)
		e->OnCollision(self,other);
}

/// <summary>
/// 削除イベント
/// </summary>
void GameObject::OnDestroy()
{
	for(auto& e : components)
		e->OnDestroy();
}

/// <summary>
/// 親オブジェクトを設定する
/// </summary>
/// <param name="parent">
/// 親にするゲームオブジェクト、nullptrを指定すると親子関係を解消する
/// </param>
void GameObject::SetParent(GameObject* parent)
{
	// 同じ親を指定された場合は何もしない
	if (parent == this->parent)
		return;

	// 別の親がある場合、その親との関係を解除する
	if (this->parent)
	{
		// 自分の位置を検索する
		auto& c = this->parent->children;
		auto itr = std::find(c.begin(), c.end(), this);
		if (itr != c.end())
			c.erase(itr); // 配列から自分を削除する
	}

	// 新たな親子関係を設定する
	if (parent)
		parent->children.push_back(this);

	this->parent = parent;
	number_joint_parent = number_joint_invalid; // 無効な関節番号を設定する
}

/// <summary>
/// 親オブジェクトを設定する
/// </summary>
/// <param name="parent">
/// 親にするゲームオブジェクト、nullptrを指定すると親子関係を解消する
/// </param>
void GameObject::SetParent(const GameObjectPtr& parent)
{
	SetParent(parent.get());
}

/// <summary>
/// 親オブジェクトと関節番号を設定する
/// </summary>
/// <param name="parent">
/// 親にするゲームオブジェクト、nullptrを指定すると親子関係を解消する
/// </param>
/// <param name="number_joint">関節番号</param>
void GameObject::SetParentAndJoint
(
	const GameObjectPtr& parent,
	int number_joint
)
{
	SetParent(parent.get());
	number_joint_parent = number_joint;
}

/// <summary>
/// 親オブジェクトと関節番号を設定する
/// </summary>
/// <param name="parent">
/// 親にするゲームオブジェクト、nullptrを指定すると親子関係を解消する
/// </param>
/// <param name="number_joint">関節番号</param>
void GameObject::SetParentAndJoint
(
	GameObject* parent,
	int number_joint
)
{
	SetParent(parent);
	number_joint_parent = number_joint;
}