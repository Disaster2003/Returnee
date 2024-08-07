/**
* @file AudioSettings.h
*/
#ifndef AUDIOSETTINGS_H_INCLUDED
#define AUDIOSETTINGS_H_INCLUDED

// �����Đ��v���C���[�ԍ�
namespace AudioPlayer
{
	inline constexpr int bgm = 0; // BGM�̍Đ��Ɏg���v���C���[�ԍ�
}

// BGM�ݒ�
namespace BGM
{
	inline constexpr char title[] = "Res/Audio/bgm/title.mp3";			 // �^�C�g����ʂ�BGM
	inline constexpr char play[] = "Res/Audio/bgm/play.mp3";			 // �v���C��ʂ�BGM
	inline constexpr char boss[] = "Res/Audio/bgm/boss.mp3";			 // �{�X���BGM
	inline constexpr char game_clear[] = "Res/Audio/bgm/game_clear.mp3"; // �Q�[���N���A��ʂ�BGM
	inline constexpr char game_over[] = "Res/Audio/bgm/game_over.mp3";   // �Q�[���I�[�o�[��ʂ�BGM
}

// ���ʉ��ݒ�
namespace SE
{
	inline constexpr char click[] = "Res/Audio/se/click.mp3";						// �N���b�N��
	inline constexpr char goal[] = "Res/Audio/se/goal.mp3";							// �S�[���ɓ��B����
	inline constexpr char player_dead[] = "Res/Audio/se/player_dead.mp3";			// �G�ɂ��ꂽ
	inline constexpr char player_jump[] = "Res/Audio/se/player_jump.mp3";			// �W�����v
	inline constexpr char player_land[] = "Res/Audio/se/player_land.mp3";			// ���n
	inline constexpr char player_attack_first[] =
		"Res/Audio/se/player_attack_first.mp3";										// ���̑f�U��(1����)
	inline constexpr char player_attack_second[] =
		"Res/Audio/se/player_attack_second.mp3";									// ���̑f�U��(2����)
	inline constexpr char player_attack_third[]	=
		"Res/Audio/se/player_attack_third.mp3";										// ���̑f�U��(3����)
	inline constexpr char sword_hit[] = "Res/Audio/se/sword_hit.mp3";				// �a�鉹
	inline constexpr char sword_guard[] = "Res/Audio/se/sword_guard.mp3";			// �����e�����
	inline constexpr char enemy_attack[] = "Res/Audio/se/enemy_attack.mp3";			// �U��
	inline constexpr char enemy_magic[] = "Res/Audio/se/enemy_magic.mp3";			// �΋�
	inline constexpr char enemy_hit_attack[] = "Res/Audio/se/enemy_hit_attack.mp3";	// ���ꂽ
	inline constexpr char dragon_dead[] = "Res/Audio/se/dragon_dead.mp3";			// �{�X���j
	inline constexpr char orc_dead[] = "Res/Audio/se/orc_dead.mp3";					// �G�����j
	inline constexpr char rock_close[] = "Res/Audio/se/rock_close.mp3";				// ��΂�����
	inline constexpr char rock_delete[] = "Res/Audio/se/rock_delete.mp3";			// ��΂�����
}

#endif // !AUDIOSETTINGS_H_INCLUDED