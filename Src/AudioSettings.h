/**
* @file AudioSettings.h
*/
#ifndef AUDIOSETTINGS_H_INCLUDED
#define AUDIOSETTINGS_H_INCLUDED

// 音声再生プレイヤー番号
namespace AudioPlayer
{
	inline constexpr int bgm = 0; // BGMの再生に使うプレイヤー番号
}

// BGM設定
namespace BGM
{
	inline constexpr char title[] = "Res/Audio/bgm/title.mp3";			 // タイトル画面のBGM
	inline constexpr char play[] = "Res/Audio/bgm/play.mp3";			 // プレイ画面のBGM
	inline constexpr char boss[] = "Res/Audio/bgm/boss.mp3";			 // ボス戦のBGM
	inline constexpr char game_clear[] = "Res/Audio/bgm/game_clear.mp3"; // ゲームクリア画面のBGM
	inline constexpr char game_over[] = "Res/Audio/bgm/game_over.mp3";   // ゲームオーバー画面のBGM
}

// 効果音設定
namespace SE
{
	inline constexpr char click[] = "Res/Audio/se/click.mp3";						// クリック音
	inline constexpr char goal[] = "Res/Audio/se/goal.mp3";							// ゴールに到達した
	inline constexpr char player_dead[] = "Res/Audio/se/player_dead.mp3";			// 敵にやられた
	inline constexpr char player_jump[] = "Res/Audio/se/player_jump.mp3";			// ジャンプ
	inline constexpr char player_land[] = "Res/Audio/se/player_land.mp3";			// 着地
	inline constexpr char player_attack_first[] =
		"Res/Audio/se/player_attack_first.mp3";										// 剣の素振り(1撃目)
	inline constexpr char player_attack_second[] =
		"Res/Audio/se/player_attack_second.mp3";									// 剣の素振り(2撃目)
	inline constexpr char player_attack_third[]	=
		"Res/Audio/se/player_attack_third.mp3";										// 剣の素振り(3撃目)
	inline constexpr char sword_hit[] = "Res/Audio/se/sword_hit.mp3";				// 斬る音
	inline constexpr char sword_guard[] = "Res/Audio/se/sword_guard.mp3";			// 剣が弾かれる
	inline constexpr char enemy_attack[] = "Res/Audio/se/enemy_attack.mp3";			// 攻撃
	inline constexpr char enemy_magic[] = "Res/Audio/se/enemy_magic.mp3";			// 火球
	inline constexpr char enemy_hit_attack[] = "Res/Audio/se/enemy_hit_attack.mp3";	// やられた
	inline constexpr char dragon_dead[] = "Res/Audio/se/dragon_dead.mp3";			// ボス撃破
	inline constexpr char orc_dead[] = "Res/Audio/se/orc_dead.mp3";					// 雑魚撃破
	inline constexpr char rock_close[] = "Res/Audio/se/rock_close.mp3";				// 岩石が動く
	inline constexpr char rock_delete[] = "Res/Audio/se/rock_delete.mp3";			// 岩石が壊れる
}

#endif // !AUDIOSETTINGS_H_INCLUDED