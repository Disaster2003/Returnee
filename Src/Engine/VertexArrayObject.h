/**
* @file VertexArrayObject.h
*/
#ifndef VERTEXARRAYOBJECT_H_INCLUDED
#define VERTEXARRAYOBJECT_H_INCLUDED
#include "glad/glad.h"
#include <memory>

//��s�錾
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

/// <summary>
/// ���_�z��I�u�W�F�N�g(VAO)
/// 
/// ���_�o�b�t�@�Ɋi�[����Ă��钸�_�f�[�^�ɂ��āA
/// �e�v�f���ǂ̂悤�ɔz�u����Ă��邩���L�q����I�u�W�F�N�g�B
/// </summary>
class VertexArrayObject
{
public:
	/// <summary>
	/// VAO���쐬����
	/// </summary>
	/// <returns>�쐬����VAO�ւ̃|�C���^</returns>
	static inline VertexArrayObjectPtr Create()
	{
		return std::make_shared<VertexArrayObject>();
	}

	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	inline VertexArrayObject() { glCreateVertexArrays(1, &id); }

	/// <summary>
	/// �f�X�g���N�^
	/// </summary>
	inline virtual ~VertexArrayObject() { glDeleteVertexArrays(1, &id); }

	// �R�s�[�Ƒ�����֎~����
	inline VertexArrayObject(const VertexArrayObject&) = delete;
	inline VertexArrayObject& operator=(const VertexArrayObject&) = delete;

	/// <summary>
	/// �Ǘ��ԍ����擾����
	/// </summary>
	inline operator GLuint() const { return id; }

	/// <summary>
	/// ���_�A�g���r���[�g��ݒ肷��
	/// </summary>
	/// <param name="index">�C���f�b�N�X</param>
	/// <param name="size"> �v�f��</param>
	/// <param name="stride">�o�C�g��</param>
	/// <param name="offset">�I�u�W�F�N�g�̐擪����̃o�C�g�I�t�Z�b�g</param>
	inline void SetAttribute
	(
		GLuint index,
		GLint size,
		size_t stride,
		size_t offset
	)
	{
		// �w�肳�ꂽ�C���f�b�N�X�̒��_�����z���L���ɂ���
		glEnableVertexAttribArray(index);
		// �w�肳�ꂽ�C���f�b�N�X�̒��_�����̃f�[�^�`���Ɣz�u���`����
		glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), reinterpret_cast<void*>(offset));
	};

private:
	GLuint id = 0; // �I�u�W�F�N�g�Ǘ��ԍ�
};

#endif // !VERTEXARRAYOBJECT_H_INCLUDED