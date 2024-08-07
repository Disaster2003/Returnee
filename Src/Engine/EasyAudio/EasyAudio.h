/**
* @file EasyAudio.h
*/
#ifndef EASYAUDIO_H_INCLUDED
#define EASYAUDIO_H_INCLUDED

// 音声関連のコードを格納する名前空間
namespace EasyAudio
{
	struct Vector { float x, y, z; };
	
	bool Initialize();
	void Finalize();
	void Update();
	
	// プレイヤーを指定して再生・停止
	void Play(int playerId, const char* filename, float volume = 1.0f, bool isLoop = false);
	void Stop(int playerId);
	void Pause(int playerId);  // 一時停止
	void Resume(int playerId); // 再開(一時停止を解除)
	
	// 音量の設定・取得
	void SetVolume(int playerId, float volume);
	float GetVolume(int playerId);
	
	// ピッチの設定・取得
	void SetPitch(int playerId, float pitch);
	float GetPitch(int playerId);
	
	// パンの設定・取得
	void SetPan(int playerId, float pan);
	float GetPan(int playerId);
	
	// 疑似的な3D音源
	void SetListenr(const Vector& position, const Vector& right);
	void SetPanAndVolumeFromPosition(int playerId, const Vector& position, float volume);
	
	// プレイヤーが再生中かどうか(再生中ならtrue、それ以外はfalse)
	bool IsPlaying(int playerId);
	
	// 単発再生
	void PlayOneShot(const char* filename);
	void PlayOneShot(const char* filename, float volume);
	void PlayOneShot(const char* filename, float volume, float pitch);
	
	// マスターボリュームの設定・取得
	void SetMasterVolume(float volume);
	float GetMasterVolume();
}

#endif // EASYAUDIO_H_INCLUDED