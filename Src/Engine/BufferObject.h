/**
* @file BufferObject.h
*/
#ifndef BUFFEROBJECT_H_INCLUDED
#define BUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include <memory>

// 先行宣言
class BufferObject;
using BufferObjectPtr = std::shared_ptr<BufferObject>;

/// <summary>
/// バッファオブジェクト(BO)
/// </summary>
class BufferObject
{
public:
	/// <summary>
	/// バッファオブジェクトを作成する
	/// </summary>
	/// <param name="size">バッファオブジェクトのバイトサイズ</param>
	/// <param name="data">バッファにコピーするデータのアドレス
	/// nullptrを指定すると空のバッファが作成される</param>
	/// <param name="flags">用途を示すビットフラグの論理和(glBufferStorageを参照)</param>
	/// <returns>作成したバッファオブジェクトへのポインタ</returns>
	static inline BufferObjectPtr Create
	(
		GLsizeiptr size,
		const void* data = nullptr,
		GLbitfield flags = 0
	)
	{
		return std::make_shared<BufferObject>(size, data, flags);
	}

	/// <summary>
	/// コンストラクタ
	/// </summary>
	inline BufferObject
	(
		GLsizeiptr size,
		const void* data,
		GLbitfield flags
	)
	{
		buffer_size = size;
		glCreateBuffers(1, &id);
		glNamedBufferStorage(id, buffer_size, data, flags);
	}

	/// <summary>
	/// デストラクタ
	/// </summary>
	inline virtual ~BufferObject() { glDeleteBuffers(1, &id); }

	// コピーと代入を禁止を禁止する
	inline BufferObject(const BufferObject&) = delete;
	inline BufferObject& operator=(const BufferObject&) = delete;

	/// <summary>
	/// 管理番号を取得を取得する
	/// </summary>
	inline operator GLuint() const { return id; }

	/// <summary>
	/// バッファサイズを取得する
	/// </summary>
	inline GLsizeiptr GetSize() const { return buffer_size; }

	/// <summary>
	/// GPUメモリにデータをコピーする
	/// </summary>
	/// <param name="offset">コピー先(GPUメモリ)の位置</param>
	/// <param name="data">コピーするデータのアドレス</param>
	/// <param name="size">コピーするバイト数</param>
	void CopyData
	(
		size_t offset,
		const void* data,
		size_t size
	)
	{
		GLuint tmp;
		glCreateBuffers(1, &tmp);
		glNamedBufferStorage(tmp, size, data, 0);
		glCopyNamedBufferSubData(tmp, id, 0, offset, size);
		glDeleteBuffers(1, &tmp);
	}

private:
	GLuint id = 0;			    // オブジェクト管理番号
	GLsizeiptr buffer_size = 0; // バッファサイズ(バイト数)
};

#endif // !BUFFEROBJECT_H_INCLUDED