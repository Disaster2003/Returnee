/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include "glad/glad.h"
#include <string>
#include <memory>

// 先行宣言
class Texture;
using TexturePtr = std::shared_ptr<Texture>;

/// <summary>
/// テクスチャ管理クラス
/// </summary>
class Texture
{
	friend class Engine;
public:
	// コピーと代入を禁止する
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	/// <summary>
	/// 管理番号を取得する
	/// </summary>
	operator GLuint()const { return id; }

	/// <summary>
	/// 幅を取得する
	/// </summary>
	/// <returns>テクスチャの幅</returns>
	int GetWidth()const { return width; }
	/// <summary>
	/// 高さを取得する
	/// </summary>
	/// <returns>テクスチャの高さ</returns>
	int GetHeight()const { return height; }

	/// <summary>
	/// アスペクト比を取得する
	/// </summary>
	/// <returns>画像のアスペクト比</returns>
	float GetAspectRatio()const
	{
		return static_cast<float>(width) / static_cast<float>(height);
	}

private:
	// 引数1つでコンストラクタを呼び出せる場合はexplicitを書く
	// 2つ以上引数を持つ場合は書かない
	explicit Texture(const char* filename);

	// キューブマップを作成するコンストラクタ
	Texture(const char* name, const char* const cubemap_filenames[6]);

	// 空のテクスチャを作成するコンストラクタ
	Texture(const char* name, int width, int height,
		GLenum format_gpu, GLenum mode_wrap = GL_CLAMP_TO_EDGE, int levels = 1);
	// デストラクタ
	virtual ~Texture();

	std::string name; // テクスチャ名(主にデバッグ用)
	GLuint id = 0;	  // オブジェクト管理番号
	int width = 0;	  // テクスチャの幅
	int height = 0;	  // テクスチャの高さ
};

#endif // !TEXTURE_H_INCLUDED