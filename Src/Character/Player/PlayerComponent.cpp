/**
* @file PlayerComponent.cpp
*/
#include "PlayerComponent.h"
#include "../../Engine/SphereCollider.h"
#include "../../Effect/SwordSwingEffect.h"
#include "../../Engine/Debug.h"

/// <summary>
/// プレイヤーのダメージ表現に使う赤枠
/// </summary>
class DamageFrame
	: public Component
{
public:
	/// <summary>
	/// 常にプレイヤーのダメージ表現に使う赤枠の更新をする
	/// </summary>
	/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
	virtual void Update(float deltaTime) override
	{
		auto owner = GetOwner();
		// 一定時間経ったら
		if (lifespan <= 0)
		{
			// フレームを破棄する
			owner->Destroy();
			return;
		}
		// 寿命と透明度を直結させる
		owner->color.w = lifespan;
		// 寿命を減らす
		lifespan -= deltaTime;
	}

private:
	float lifespan = 1.0f; // 赤枠の寿命
};

/// <summary>
/// プレイヤーを初期化する
/// </summary>
void PlayerComponent::Awake()
{
	auto owner = GetOwner();
	auto engine = owner->GetEngine();

	// コライダーを設定する
	// 球体コライダーを上下に並べて置くことで、縦長の衝突形状を再現
	for (float i = 0; i < 2; ++i)
	{
		auto collider = owner->AddComponent<SphereCollider>();
		collider->sphere.radius = 1;
		collider->sphere.position.y = -0.5f * i;
	}

	characterMovement = owner->AddComponent<CharacterMovement>();

	// 手を追加する
	auto hand = engine->Create<GameObject>("player.hand", { 0.2f, -0.2f, -0.2f });
	hand->rotation.x = -radians(70);
	hand->rotation.y = radians(180);
	hand->staticMesh = engine->GetStaticMesh(
		"Res/MeshData/arm_and_hand/arm_and_hand_grab.obj");
	hand->SetParent(owner);
	old_x = hand->rotation.x;
	old_y = hand->rotation.y;

	// 武器を追加する
	auto weapon = engine->Create<GameObject>("weapon", { -0.065f, -0.35f, 0.2f });
	weapon->rotation.x = radians(220);
	weapon->rotation.y = radians(180);
	weapon->staticMesh = engine->GetStaticMesh(
		"Res/MeshData/arm_and_hand/sword.obj");
	weapon->SetParent(hand);
	attackCollider_right_arm = weapon->AddComponent<WeaponAttackCollider>();

	// プレイヤーの影を追加する
	auto headMesh = engine->Create<GameObject>("player.head.mesh", { 0, 0.4f, 0 });
	headMesh->staticMesh = engine->GetStaticMesh("crystal");
	headMesh->type_lighting = GameObject::TYPE_LIGHTING::SHADOW;
	headMesh->SetParent(owner);
	headMesh->scale = vec3(0.3f, 0.25f, 0.3f);
	auto bodyMesh = engine->Create<GameObject>("player.body.mesh", { 0, -0.2f, 0 });
	bodyMesh->staticMesh = engine->GetStaticMesh("crystal");
	bodyMesh->type_lighting = GameObject::TYPE_LIGHTING::SHADOW;
	bodyMesh->scale = vec3(0.5f, 0.4f, 0.5f);
	bodyMesh->SetParent(owner);
	auto rightLegMesh = engine->Create<GameObject>("player.right_leg.mesh", { 0.125f, -0.7f, 0 });
	rightLegMesh->staticMesh = engine->GetStaticMesh("crystal");
	rightLegMesh->type_lighting = GameObject::TYPE_LIGHTING::SHADOW;
	rightLegMesh->scale = vec3(0.2f, 0.6f, 0.2f);
	rightLegMesh->SetParent(owner);
	auto leftLegMesh = engine->Create<GameObject>("player.left_leg.mesh", { -0.125f, -0.7f, 0 });
	leftLegMesh->staticMesh = engine->GetStaticMesh("crystal");
	leftLegMesh->type_lighting = GameObject::TYPE_LIGHTING::SHADOW;
	leftLegMesh->scale = vec3(0.2f, 0.6f, 0.2f);
	leftLegMesh->SetParent(owner);
}

/// <summary>
/// 常にプレイヤーの状態を更新する
/// </summary>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void PlayerComponent::Update(float deltaTime)
{
	Engine* engine = GetOwner()->GetEngine();
	GameObject& camera = *GetOwner(); // engine->GetMainCamera();

	characterMovement->DecelerateXZ(10 * deltaTime);

	// alive以外の状態は移動できない
	if (state_player != STATE_PLAYER::ALIVE)
	{
		// カーソルの表示
		engine->ShowMouseCursor();

		// 体力ゲージの破棄
		ui_hp_frame->GetOwner()->Destroy();
		ui_hp_gauge->GetOwner()->Destroy();

		// 攻撃判定を破棄する
		attackCollider_right_arm->Deactivate();

		return;
	}

	// 体力ゲージの表示・非表示、調整
	HpGauge(engine, deltaTime);

	// カーソルの非表示
	engine->HideMouseCursor();

	// 移動処理
	{
		isRunning = false;
		const float cameraSpeed = 5;
		const float cameraCos = cos(camera.rotation.y);
		const float cameraSin = sin(camera.rotation.y);

		// 前進する
		if (engine->GetKey(GLFW_KEY_W))
		{
			Move(camera, deltaTime, -1, cameraSpeed * cameraSin * 2, cameraSpeed * cameraCos * 2);
		}
		// 後退する
		if (engine->GetKey(GLFW_KEY_S))
		{
			Move(camera, deltaTime, 1, cameraSpeed * cameraSin * 1.5f, cameraSpeed * cameraCos * 1.5f);
		}
		// 左に移動する
		if (engine->GetKey(GLFW_KEY_A))
		{
			Move(camera, deltaTime, -1, cameraSpeed * cameraCos, cameraSpeed * -cameraSin);
		}
		// 右に移動する
		if (engine->GetKey(GLFW_KEY_D))
		{
			Move(camera, deltaTime, 1, cameraSpeed * cameraCos, cameraSpeed * -cameraSin);
		}

		// 走行音停止
		if (!isRunning)
			EasyAudio::Stop(AudioPlayer::run);
	}

	// 視点移動処理
	{
		static constexpr float SPEED_MOVE_CAMERA = 0.3f; // 視点移動の速さ
		static constexpr float CAMERA_ANGLE_MAX = 0.6f;	 // 上下視点の最大値

		// マウスを横方向に動かした分、カメラのY軸回転を行う
		if (abs(engine->GetMouseMovement().x) > 1)
		{
			camera.rotation.y -= engine->GetMouseMovement().x * deltaTime * SPEED_MOVE_CAMERA;
		}

		// マウスを縦方向に動かした分、カメラのX軸回転を行う
		if (abs(engine->GetMouseMovement().y) > 1 &&
			camera.rotation.x < CAMERA_ANGLE_MAX &&
			camera.rotation.x > -CAMERA_ANGLE_MAX)
		{
			camera.rotation.x -= engine->GetMouseMovement().y * deltaTime * SPEED_MOVE_CAMERA;
		}
		// 上向きの限度
		else if (camera.rotation.x >= CAMERA_ANGLE_MAX)
		{
			camera.rotation.x = CAMERA_ANGLE_MAX - 0.01f;
		}
		// 下向きの限度
		else if (camera.rotation.x <= -CAMERA_ANGLE_MAX)
		{
			camera.rotation.x = -CAMERA_ANGLE_MAX + 0.01f;
		}
	}

	// スペースキーが押されたら、ジャンプする
	Jump(camera, deltaTime);

#pragma region ATTACK_AND_GUARD
	auto hand = camera.GetChild(0);	// player.hand
	switch (state_sword)
	{
	case PlayerComponent::STATE_SWORD::IDLE:
		// 接地していたら、呼吸運動を行う
		if (camera.isGrounded)
		{
			hand->rotation.x = sin(breath_scale) * 0.01f * ((isRunning) ? 30 : 5) + old_x;
		}

		// 1撃目の準備
		if (time_swing <= 0)
		{
			if (engine->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
			{
				// 剣の回転角度の設定
				hand->rotation.x = old_x + radians(-45);
				hand->rotation.y = old_y;

				// 1撃目の準備を進める
				isNextSwinging = true;
				if (isNextSwinging)
				{
					// 1撃目の効果音を再生する
					EasyAudio::PlayOneShot(SE::player_attack_first);

					// 斬撃跡を生成する
					CreateSwordSlashEffect(camera, 0);

					AttackInitialize(STATE_SWORD::FIRST_SWING, STATE_SWORD::FIRST_SWING);
				}
				break;
			}
		}
		// ガードの準備
		if (time_guard <= 0)
		{
			if (engine->GetMouseButton(GLFW_MOUSE_BUTTON_RIGHT))
			{
				// 剣の回転角度を戻す
				hand->rotation.x = old_x;
				hand->rotation.y = old_y;

				// ガードする
				state_sword = STATE_SWORD::GUARD;
				time_guard = 3.0f;
				hand->rotation.y += radians(45);
				break;
			}
		}
		break;
	case PlayerComponent::STATE_SWORD::FIRST_SWING:
		// 剣を振り切ったら、2撃目の準備をする / 攻撃を終了する
		if (hand->rotation.x > old_x + radians(45))
		{
			if (time_swing <= 0)
			{
				// 剣の回転角度を戻す
				hand->rotation.x = old_x;
				hand->rotation.y = old_y;

				// 2撃目の準備を進める
				if (isNextSwinging)
				{
					// 2撃目の効果音を再生する
					EasyAudio::PlayOneShot(SE::player_attack_second);

					// 斬撃跡の生成
					CreateSwordSlashEffect(camera, -45);

					// 剣の回転角度の設定
					hand->rotation.x = old_x + radians(-45);
					hand->rotation.y = old_y + radians(-30);
				}

				AttackInitialize(STATE_SWORD::SECOND_SWING, STATE_SWORD::IDLE);
			}
		}
		else
		{
			// 1撃目の処理を行う
			SwordSwing(deltaTime, hand->rotation.x);

			// +1撃加えるか判断する
			ClickCheck();
		}
		break;
	case PlayerComponent::STATE_SWORD::SECOND_SWING:
		// 剣を振り切ったら、3撃目の準備をする / 攻撃を終了する
		if (hand->rotation.x > old_x + radians(30))
		{
			if (time_swing <= 0)
			{
				// 剣の回転角度を戻す
				hand->rotation.x = old_x;
				hand->rotation.y = old_y;

				// 3撃目の準備を進める
				if (isNextSwinging)
				{
					// 3撃目の効果音を再生する
					EasyAudio::PlayOneShot(SE::player_attack_third);

					// 斬撃跡の生成
					CreateSwordSlashEffect(camera, 45);

					// 剣の回転角度の設定
					hand->rotation.x = old_x + radians(-45);
					hand->rotation.y = old_y + radians(30);
				}

				AttackInitialize(STATE_SWORD::THIRD_SWING, STATE_SWORD::IDLE);
			}
		}
		else
		{
			// 2撃目の処理を行う
			SwordSwing(deltaTime, hand->rotation.x);
			anti_power -= 0.01f;
			SwordSwing(deltaTime, hand->rotation.y);

			// +1撃加えるか判断する
			ClickCheck();
		}
		break;
	case PlayerComponent::STATE_SWORD::THIRD_SWING:
		// 剣を振り切ったら、攻撃を終了する
		if (hand->rotation.x > old_x + radians(30))
		{
			if (time_swing <= 0)
			{
				// 剣の回転角度を戻す
				hand->rotation.x = old_x;
				hand->rotation.y = old_y;

				AttackInitialize(STATE_SWORD::IDLE, STATE_SWORD::IDLE);
			}
		}
		else
		{
			// 3撃目の処理を行う
			SwordSwing(deltaTime, hand->rotation.x);
			anti_power -= deltaTime * 0.01f;
			SwordSwing(-deltaTime, hand->rotation.y);
		}
		break;
	case PlayerComponent::STATE_SWORD::GUARD:
		// ガード終了
		if (time_guard <= 0 || isGuarded)
		{
			// 何もしていない状態に
			state_sword = STATE_SWORD::IDLE;
			isGuarded = false;

			// 剣の回転角度を戻す
			hand->rotation.y = old_y;
		}
		break;
	default:
		break;
	}
#pragma endregion

	// 時間経過
	{
		// 無敵時間を減らす
		if (time_invincible > 0)
		{
			time_invincible = std::max(time_invincible - deltaTime, 0.0f);
		}
		// 攻撃の待機時間
		if (time_swing > 0)
		{
			time_swing -= deltaTime;
		}
		// ガードの待機時間
		if (time_guard > 0)
		{
			time_guard -= deltaTime;
		}
		// 呼吸運動の大きさを加える
		if (isRunning)
		{
			breath_scale += deltaTime * 10;
		}
		else
		{
			breath_scale += deltaTime;
		}
	}
}

/// <summary>
/// 移動する
/// </summary>
/// <param name="camera">プレイヤー</param>
/// <param name="PlusOrMinus">正 or 負の方向</param>
/// <param name="x_movement_amount">x座標を動かす量</param>
/// <param name="z_movement_amount">z座標を動かす量</param>
void PlayerComponent::Move
(
	GameObject& camera,
	float deltaTime,
	int PlusOrMinus,
	float x_movement_amount,
	float z_movement_amount
)
{
	// 走行状態
	isRunning = true;
	if (!EasyAudio::IsPlaying(AudioPlayer::run))
		EasyAudio::Play(AudioPlayer::run, SE::player_run, 1, true);

	// PlusOrMinus方向に移動する
	camera.position.x += deltaTime * x_movement_amount * PlusOrMinus;
	camera.position.z += deltaTime * z_movement_amount * PlusOrMinus;
}

/// <summary>
/// スペースキーが押されたら、ジャンプする
/// </summary>
/// <param name="camera">プレイヤー</param>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void PlayerComponent::Jump
(
	GameObject& camera,
	float deltaTime
)
{
	// ジャンプ中なら
	if (isJumping)
	{
		camera.position.y += (POWER_BASE - gravity) * deltaTime;
		gravity += deltaTime * 0.01f;
		// 地面についたら、ジャンプ解除
		if (camera.isGrounded)
		{
			// 着地音を再生する
			EasyAudio::PlayOneShot(SE::player_land);

			// ジャンプ解除
			isJumping = false;
			gravity = 0;
		}
	}
	else
	{
		// spaceキーが押されたらジャンプする
		if (GetOwner()->GetEngine()->GetKey(GLFW_KEY_SPACE))
		{
			// ジャンプ音を再生する
			EasyAudio::PlayOneShot(SE::player_jump);

			// ジャンプ解除
			isJumping = true;
			gravity = 0;
		}
		else
		{
			characterMovement->DecelerateXZ(10 * deltaTime);
		}
	}
}

/// <summary>
/// 剣を振る
/// </summary>
/// <param name="rotation_hand">手の回転</param>
void PlayerComponent::SwordSwing
(
	float deltaTime,
	float& rotation_hand
)
{
	rotation_hand += (POWER_BASE - anti_power) * deltaTime;
	anti_power += deltaTime * 0.01f;
}

/// <summary>
/// +1撃加えるか判断する
/// </summary>
void PlayerComponent::ClickCheck()
{
	// +1撃追加する
	if (isFinishedClick)
	{
		if (GetOwner()->GetEngine()->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
		{
			isNextSwinging = true;
		}
	}

	// クリック判定の終了
	if (!isFinishedClick)
	{
		if (!GetOwner()->GetEngine()->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT))
		{
			isFinishedClick = true;
		}
	}
}

/// <summary>
/// 攻撃前に初期化する / 攻撃を終了する
/// </summary>
/// <param name="isNextSwinging">次攻撃するかどうか</param>
/// <param name="next">攻撃する場合の剣の状態</param>
/// <param name="finish">攻撃しない場合の剣の状態</param>
void PlayerComponent::AttackInitialize
(
	STATE_SWORD next,
	STATE_SWORD finish
)
{
	// 次の攻撃の準備をする
	if (isNextSwinging)
	{
		isNextSwinging = false;
		state_sword = next;
		isAttacking = true;
		time_swing = 0.5f;
		anti_power = 0;

		// ジャンプしてたら、ダメージ2倍
		if (isJumping)
		{
			attackCollider_right_arm->SetDamage(ATTACK_SPECIAL);
		}
		else
		{
			attackCollider_right_arm->SetDamage(ATTAK_NORMAL);
		}

		// 攻撃判定を設定する
		attackCollider_right_arm->Activate(2.0f);
	}
	else
	{
		// 攻撃を終了する
		state_sword = finish;
		isAttacking = false;
		time_swing = 1.0f;
	}

	// クリック終了判定を戻す
	isFinishedClick = false;
}

/// <summary>
/// 斬撃跡を生成する
/// </summary>
/// <param name="_camera">カメラ</param>
/// <param name="angle">斬撃跡の角度</param>
void PlayerComponent::CreateSwordSlashEffect
(
	GameObject& _camera,
	float angle
)
{
	// 正面を決定する
	const vec3 dirFront = { sin(_camera.rotation.y), 0, cos(_camera.rotation.y) };

	// 斬撃跡を表示する
	vec3 position_effect = _camera.position - dirFront;
	auto effect_swing = GetOwner()->GetEngine()->Create<GameObject>("swing effect", position_effect);
	effect_swing->AddComponent<SwordSwingEffect>();
	effect_swing->rotation.z = radians(angle);
}

/// <summary>
/// 衝突時に衝突した相手によって処理を行う
/// </summary>
/// <param name="self">自身</param>
/// <param name="other">衝突した相手</param>
void PlayerComponent::OnCollision
(
	const ComponentPtr& self,
	const ComponentPtr& other
)
{
	auto targetObject = other->GetOwner();

	// alive以外の状態は衝突に反応しない
	if (state_player != STATE_PLAYER::ALIVE)
	{
		return;
	}
	// Chestに触れたらゴールする
	else if (targetObject->name == "Chest")
	{
		state_player = STATE_PLAYER::GOAL;
	}
}

/// <summary>
/// ダメージを受ける
/// </summary>
/// <param name="damage">ダメージ量</param>
/// <param name="causer">自身</param>
void PlayerComponent::TakeDamage
(
	int damage,
	GameObject* causer
)
{
	// 無敵時間中は攻撃を無効化
	if (time_invincible > 0)
	{
		return;
	}

	// ダメージを受けた音を再生する
	EasyAudio::PlayOneShot(SE::player_damage);

	//// ダメージ倍率を適用
	//damage = int(float(damage * GameState::Instance()->takenDamageRatio));

	// 体力を減らす
	PlayerComponent::SetHp(damage);

	// ゲームオーバー
	if (PlayerComponent::GetHp() <= 0)
	{
		state_player = STATE_PLAYER::DEAD;
		PlayerComponent::SetHp(0);

		// ゲームオーバーBGM/SEを再生する
		EasyAudio::Stop(AudioPlayer::bgm);
		EasyAudio::Stop(AudioPlayer::run);
		EasyAudio::Play(AudioPlayer::bgm, BGM::game_over, 1, true);
		EasyAudio::PlayOneShot(SE::player_dead);
	}
	else
	{
		// 無敵時間をつくる
		time_invincible = 0.2f;
	}

	// ダメージの反動
	const vec3 v = normalize(GetOwner()->position - causer->position);
	characterMovement->velocity.x += 10 * v.x;
	characterMovement->velocity.z += 10 * v.z;

	// 命中による反作用
	auto cm = causer->GetComponent<CharacterMovement>();
	if (cm)
	{
		cm->velocity.x -= 5 * v.x;
		cm->velocity.z -= 5 * v.z;
	}

	// ダメージエフェクトを表示する
	Engine* engine = GetOwner()->GetEngine();
	auto damageFrame = engine->CreateUIObject<UILayout>("Res/UI_damage_frame.dds", vec2(0), 1);
	damageFrame.first->AddComponent<DamageFrame>();
}

/// <summary>
/// 体力ゲージの表示・非表示、調整を行う
/// </summary>
/// <param name="engine">ゲームエンジン</param>
/// <param name="deltaTime">前回の更新からの経過時間(秒)</param>
void PlayerComponent::HpGauge(Engine* engine, float deltaTime)
{
	// 体力ゲージのフレームを生成していなかったら
	if (!ui_hp_frame)
	{
		// 体力ゲージのフレームを生成する
		ui_hp_frame = engine->CreateUIObject<UILayout>("Res/UI_hp_frame.dds", { -0.98f, 0.95f }, 0.05f).second;
		ui_hp_frame->GetOwner()->render_queue = RENDER_QUEUE_OVERLAY + 1;
	}
	// 体力ゲージを生成していなかったら
	if (!ui_hp_gauge)
	{
		// 体力ゲージを生成する
		ui_hp_gauge = engine->CreateUIObject<UILayout>("Res/UI_hp_gauge.dds", { -0.98f, 0.95f }, 0.05f).second;
		ui_hp_gauge->GetOwner()->materials[0]->baseColor = { 0.0f, 1.0f, 0.0f, 1 };
	}
	// 体力ゲージを表示する
	auto gauge = ui_hp_gauge->GetOwner();
	const float aspectRatio = gauge->materials[0]->texBaseColor->GetAspectRatio();
	const float targetRatio = std::clamp(float(PlayerComponent::GetHp()) / float(HP_MAX), 0.0f, 1.0f);

	// 体力ゲージの増減を計算させる
	if (current_ratio > targetRatio)
	{
		current_ratio = std::max(current_ratio - deltaTime * 0.5f, targetRatio);
	}
	else if (current_ratio < targetRatio)
	{
		current_ratio = std::min(current_ratio + deltaTime * 0.5f, targetRatio);
	}

	// ゲージ全体のスケーリング
	switch (scalingMode)
	{
	case SCALING_MODE::OPEN:
		scale += deltaTime * 1.5f;
		if (scale >= 1)
		{
			scale = 1;
			scalingMode = SCALING_MODE::STATIONARY;
		}
		break;
	case SCALING_MODE::STATIONARY:
		if (current_ratio <= 0)
		{
			scalingMode = SCALING_MODE::CLOSE;
		}
		break;

	case SCALING_MODE::CLOSE:
		scale = std::max(scale - deltaTime * 1.5f, 0.0f);
		break;
	}

	// 計算結果を体力ゲージの座標、大きさに反映する
	ui_hp_gauge->position_base.x = -(1 - current_ratio) * aspectRatio * scale * 0.05f - 0.98f;
	gauge->scale.x = current_ratio * aspectRatio * scale * 0.05f;
	ui_hp_frame->GetOwner()->scale.x = aspectRatio * scale * 0.05f;
}