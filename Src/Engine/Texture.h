/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include "glad/glad.h"
#include <string>
#include <memory>

// ��s�錾
class Texture;
using TexturePtr = std::shared_ptr<Texture>;

/// <summary>
/// �e�N�X�`���Ǘ��N���X
/// </summary>
class Texture
{
	friend class Engine;
public:
	// �R�s�[�Ƒ�����֎~����
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	/// <summary>
	/// �Ǘ��ԍ����擾����
	/// </summary>
	operator GLuint()const { return id; }

	/// <summary>
	/// �����擾����
	/// </summary>
	/// <returns>�e�N�X�`���̕�</returns>
	int GetWidth()const { return width; }
	/// <summary>
	/// �������擾����
	/// </summary>
	/// <returns>�e�N�X�`���̍���</returns>
	int GetHeight()const { return height; }

	/// <summary>
	/// �A�X�y�N�g����擾����
	/// </summary>
	/// <returns>�摜�̃A�X�y�N�g��</returns>
	float GetAspectRatio()const
	{
		return static_cast<float>(width) / static_cast<float>(height);
	}

private:
	// ����1�ŃR���X�g���N�^���Ăяo����ꍇ��explicit������
	// 2�ȏ���������ꍇ�͏����Ȃ�
	explicit Texture(const char* filename);

	// �L���[�u�}�b�v���쐬����R���X�g���N�^
	Texture(const char* name, const char* const cubemap_filenames[6]);

	// ��̃e�N�X�`�����쐬����R���X�g���N�^
	Texture(const char* name, int width, int height,
		GLenum format_gpu, GLenum mode_wrap = GL_CLAMP_TO_EDGE, int levels = 1);
	// �f�X�g���N�^
	virtual ~Texture();

	std::string name; // �e�N�X�`����(��Ƀf�o�b�O�p)
	GLuint id = 0;	  // �I�u�W�F�N�g�Ǘ��ԍ�
	int width = 0;	  // �e�N�X�`���̕�
	int height = 0;	  // �e�N�X�`���̍���
};

#endif // !TEXTURE_H_INCLUDED