/**
* @file VertexArrayObject.h
*/
#ifndef VERTEXARRAYOBJECT_H_INCLUDED
#define VERTEXARRAYOBJECT_H_INCLUDED
#include "glad/glad.h"
#include <memory>

//先行宣言
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

/// <summary>
/// 頂点配列オブジェクト(VAO)
/// 
/// 頂点バッファに格納されている頂点データについて、
/// 各要素がどのように配置されているかを記述するオブジェクト。
/// </summary>
class VertexArrayObject
{
public:
	/// <summary>
	/// VAOを作成する
	/// </summary>
	/// <returns>作成したVAOへのポインタ</returns>
	static inline VertexArrayObjectPtr Create()
	{
		return std::make_shared<VertexArrayObject>();
	}

	/// <summary>
	/// コンストラクタ
	/// </summary>
	inline VertexArrayObject() { glCreateVertexArrays(1, &id); }

	/// <summary>
	/// デストラクタ
	/// </summary>
	inline virtual ~VertexArrayObject() { glDeleteVertexArrays(1, &id); }

	// コピーと代入を禁止する
	inline VertexArrayObject(const VertexArrayObject&) = delete;
	inline VertexArrayObject& operator=(const VertexArrayObject&) = delete;

	/// <summary>
	/// 管理番号を取得する
	/// </summary>
	inline operator GLuint() const { return id; }

	/// <summary>
	/// 頂点アトリビュートを設定する
	/// </summary>
	/// <param name="index">インデックス</param>
	/// <param name="size"> 要素数</param>
	/// <param name="stride">バイト数</param>
	/// <param name="offset">オブジェクトの先頭からのバイトオフセット</param>
	inline void SetAttribute
	(
		GLuint index,
		GLint size,
		size_t stride,
		size_t offset
	)
	{
		// 指定されたインデックスの頂点属性配列を有効にする
		glEnableVertexAttribArray(index);
		// 指定されたインデックスの頂点属性のデータ形式と配置を定義する
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), reinterpret_cast<void*>(offset));
	};

private:
	GLuint id = 0; // オブジェクト管理番号
};

#endif // !VERTEXARRAYOBJECT_H_INCLUDED