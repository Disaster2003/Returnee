/**
* @file FramebufferObject.h
*/
#ifndef FRAMEBUFFEROBJECT_H_INCLUDED
#define FRAMEBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include "Texture.h"

/// <summary>
/// フレームバッファ・オブジェクト(FBO)
/// </summary>
class FramebufferObject
{
public:
	// 既存のテクスチャからFBOを作成するコンストラクタ
	FramebufferObject(const TexturePtr& color, const TexturePtr& depth,
		int color_level = 0, int depth_level = 0);
	// デストラクタ
	virtual ~FramebufferObject();

	// コピーと代入を禁止する
	FramebufferObject(const FramebufferObject&) = delete;
	FramebufferObject& operator=(const FramebufferObject&) = delete;

	/// <summary>
	/// 管理番号を取得する
	/// </summary>
	operator GLuint() const { return fbo; }

	/// <summary>
	/// 幅を取得する
	/// </summary>
	/// <returns>ウィンドウの幅</returns>
	int GetWidth() const { return width; }

	/// <summary>
	/// 高さを取得する
	/// </summary>
	/// <returns>ウィンドウの高さ</returns>
	int GetHeight() const { return height; }

	/// <summary>
	/// テクスチャの色を取得する
	/// </summary>
	const TexturePtr& GetTextureColor() const { return texture_color; }
	/// <summary>
	/// テクスチャの深度を取得する
	/// </summary>
	const TexturePtr& GetTextureDepth() const { return texture_depth; }

private:
	GLuint fbo = 0; // FBOの管理番号
	int width = 0,  // FBOの幅
		height = 0; // FBOの高さ

	TexturePtr texture_color; // カラーテクスチャ
	TexturePtr texture_depth; // 深度テクスチャ
};
using FramebufferObjectPtr = std::shared_ptr<FramebufferObject>;

#endif // FRAMEBUFFEROBJECT_H_INCLUDED