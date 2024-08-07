/**
* @file BufferObject.h
*/
#ifndef BUFFEROBJECT_H_INCLUDED
#define BUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include <memory>

// ��s�錾
class BufferObject;
using BufferObjectPtr = std::shared_ptr<BufferObject>;

/// <summary>
/// �o�b�t�@�I�u�W�F�N�g(BO)
/// </summary>
class BufferObject
{
public:
	/// <summary>
	/// �o�b�t�@�I�u�W�F�N�g���쐬����
	/// </summary>
	/// <param name="size">�o�b�t�@�I�u�W�F�N�g�̃o�C�g�T�C�Y</param>
	/// <param name="data">�o�b�t�@�ɃR�s�[����f�[�^�̃A�h���X
	/// nullptr���w�肷��Ƌ�̃o�b�t�@���쐬�����</param>
	/// <param name="flags">�p�r�������r�b�g�t���O�̘_���a(glBufferStorage���Q��)</param>
	/// <returns>�쐬�����o�b�t�@�I�u�W�F�N�g�ւ̃|�C���^</returns>
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
	/// �R���X�g���N�^
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
	/// �f�X�g���N�^
	/// </summary>
	inline virtual ~BufferObject() { glDeleteBuffers(1, &id); }

	// �R�s�[�Ƒ�����֎~���֎~����
	inline BufferObject(const BufferObject&) = delete;
	inline BufferObject& operator=(const BufferObject&) = delete;

	/// <summary>
	/// �Ǘ��ԍ����擾���擾����
	/// </summary>
	inline operator GLuint() const { return id; }

	/// <summary>
	/// �o�b�t�@�T�C�Y���擾����
	/// </summary>
	inline GLsizeiptr GetSize() const { return buffer_size; }

	/// <summary>
	/// GPU�������Ƀf�[�^���R�s�[����
	/// </summary>
	/// <param name="offset">�R�s�[��(GPU������)�̈ʒu</param>
	/// <param name="data">�R�s�[����f�[�^�̃A�h���X</param>
	/// <param name="size">�R�s�[����o�C�g��</param>
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
	GLuint id = 0;			    // �I�u�W�F�N�g�Ǘ��ԍ�
	GLsizeiptr buffer_size = 0; // �o�b�t�@�T�C�Y(�o�C�g��)
};

#endif // !BUFFEROBJECT_H_INCLUDED