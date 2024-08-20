/**
* @file AudioSettings.h
*/
#ifndef AUDIOSETTINGS_H_INCLUDED
#define AUDIOSETTINGS_H_INCLUDED

// �����Đ��v���C���[�ԍ�
namespace AudioPlayer
{
	inline constexpr int bgm = 0; // BGM�̍Đ��Ɏg���v���C���[�ԍ�
	inline constexpr int run = 1; // BGM�̍Đ��Ɏg���v���C���[�ԍ�
}

// BGM�ݒ�
namespace BGM
{
	inline constexpr char title[] = "Res/Audio/BGM/title.mp3";			 // �^�C�g����ʂ�BGM
	inline constexpr char play[] = "Res/Audio/BGM/play.mp3";			 // �v���C����BGM
	inline constexpr char vsBoss[] = "Res/Audio/BGM/vsBoss.mp3";		 // �{�X���BGM
	inline constexpr char game_clear[] = "Res/Audio/BGM/game_clear.mp3"; // �Q�[���N���A����BGM
	inline constexpr char game_over[] = "Res/Audio/BGM/game_over.mp3";   // �Q�[���I�[�o�[����BGM
}

// ���ʉ��ݒ�
namespace SE
{
	inline constexpr char click[] = "Res/Audio/SE/click.mp3";						// �N���b�N��
	inline constexpr char goal[] = "Res/Audio/SE/goal.mp3";							// �S�[���ɓ���
	inline constexpr char player_run[] = "Res/Audio/SE/player_run.wav";				// ����
	inline constexpr char player_damage[] = "Res/Audio/SE/player_damage.mp3";		// �U�����ꂽ
	inline constexpr char player_dead[] = "Res/Audio/SE/player_dead.mp3";			// �G�ɂ��ꂽ
	inline constexpr char player_jump[] = "Res/Audio/SE/player_jump.mp3";			// �W�����v
	inline constexpr char player_land[] = "Res/Audio/SE/player_land.mp3";			// ���n
	inline constexpr char player_attack_first[] =
		"Res/Audio/SE/player_attack_first.mp3";										// ���̑f�U��(1����)
	inline constexpr char player_attack_second[] =
		"Res/Audio/SE/player_attack_second.mp3";									// ���̑f�U��(2����)
	inline constexpr char player_attack_third[]	=
		"Res/Audio/SE/player_attack_third.mp3";										// ���̑f�U��(3����)
	inline constexpr char sword_hit[] = "Res/Audio/SE/sword_hit.mp3";				// �a�鉹
	inline constexpr char sword_guard[] = "Res/Audio/SE/sword_guard.mp3";			// �K�[�h����
	inline constexpr char enemy_attack[] = "Res/Audio/SE/enemy_attack.mp3";			// �v���C���[�ւ̍U��
	inline constexpr char enemy_magic[] = "Res/Audio/SE/enemy_magic.mp3";			// �΋�����
	inline constexpr char dragon_dead[] = "Res/Audio/SE/dragon_dead.mp3";			// �{�X���j
	inline constexpr char orc_dead[] = "Res/Audio/SE/orc_dead.mp3";					// �G�����j
	inline constexpr char rock_close[] = "Res/Audio/SE/rock_close.mp3";				// ��΂�����
	inline constexpr char rock_delete[] = "Res/Audio/SE/rock_delete.mp3";			// ��΂�����
}

#endif // !AUDIOSETTINGS_H_INCLUDED