/**
* @file AudioSettings.h
*/
#ifndef AUDIOSETTINGS_H_INCLUDED
#define AUDIOSETTINGS_H_INCLUDED

// 音声再生プレイヤー番号
namespace AudioPlayer
{
	inline constexpr int bgm = 0; // BGMの再生に使うプレイヤー番号
	inline constexpr int run = 1; // BGMの再生に使うプレイヤー番号
}

// BGM設定
namespace BGM
{
	inline constexpr char title[] = "Res/Audio/BGM/title.mp3";			 // タイトル画面のBGM
	inline constexpr char play[] = "Res/Audio/BGM/play.mp3";			 // プレイ中のBGM
	inline constexpr char vsBoss[] = "Res/Audio/BGM/vsBoss.mp3";		 // ボス戦のBGM
	inline constexpr char game_clear[] = "Res/Audio/BGM/game_clear.mp3"; // ゲームクリア時のBGM
	inline constexpr char game_over[] = "Res/Audio/BGM/game_over.mp3";   // ゲームオーバー時のBGM
}

// 効果音設定
namespace SE
{
	inline constexpr char click[] = "Res/Audio/SE/click.mp3";						// クリック音
	inline constexpr char goal[] = "Res/Audio/SE/goal.mp3";							// ゴールに到着
	inline constexpr char player_run[] = "Res/Audio/SE/player_run.wav";				// 走る
	inline constexpr char player_damage[] = "Res/Audio/SE/player_damage.mp3";		// 攻撃された
	inline constexpr char player_dead[] = "Res/Audio/SE/player_dead.mp3";			// 敵にやられた
	inline constexpr char player_jump[] = "Res/Audio/SE/player_jump.mp3";			// ジャンプ
	inline constexpr char player_land[] = "Res/Audio/SE/player_land.mp3";			// 着地
	inline constexpr char player_attack_first[] =
		"Res/Audio/SE/player_attack_first.mp3";										// 剣の素振り(1撃目)
	inline constexpr char player_attack_second[] =
		"Res/Audio/SE/player_attack_second.mp3";									// 剣の素振り(2撃目)
	inline constexpr char player_attack_third[]	=
		"Res/Audio/SE/player_attack_third.mp3";										// 剣の素振り(3撃目)
	inline constexpr char sword_hit[] = "Res/Audio/SE/sword_hit.mp3";				// 斬る音
	inline constexpr char sword_guard[] = "Res/Audio/SE/sword_guard.mp3";			// ガード成功
	inline constexpr char enemy_attack[] = "Res/Audio/SE/enemy_attack.mp3";			// プレイヤーへの攻撃
	inline constexpr char enemy_magic[] = "Res/Audio/SE/enemy_magic.mp3";			// 火球発射
	inline constexpr char dragon_dead[] = "Res/Audio/SE/dragon_dead.mp3";			// ボス撃破
	inline constexpr char orc_dead[] = "Res/Audio/SE/orc_dead.mp3";					// 雑魚撃破
	inline constexpr char rock_close[] = "Res/Audio/SE/rock_close.mp3";				// 岩石が動く
	inline constexpr char rock_delete[] = "Res/Audio/SE/rock_delete.mp3";			// 岩石が壊れる
}

#endif // !AUDIOSETTINGS_H_INCLUDED