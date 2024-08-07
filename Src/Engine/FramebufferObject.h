/**
* @file FramebufferObject.h
*/
#ifndef FRAMEBUFFEROBJECT_H_INCLUDED
#define FRAMEBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include "Texture.h"

/// <summary>
/// �t���[���o�b�t�@�E�I�u�W�F�N�g(FBO)
/// </summary>
class FramebufferObject
{
public:
	// �����̃e�N�X�`������FBO���쐬����R���X�g���N�^
	FramebufferObject(const TexturePtr& color, const TexturePtr& depth,
		int color_level = 0, int depth_level = 0);
	// �f�X�g���N�^
	virtual ~FramebufferObject();

	// �R�s�[�Ƒ�����֎~����
	FramebufferObject(const FramebufferObject&) = delete;
	FramebufferObject& operator=(const FramebufferObject&) = delete;

	/// <summary>
	/// �Ǘ��ԍ����擾����
	/// </summary>
	operator GLuint() const { return fbo; }

	/// <summary>
	/// �����擾����
	/// </summary>
	/// <returns>�E�B���h�E�̕�</returns>
	int GetWidth() const { return width; }

	/// <summary>
	/// �������擾����
	/// </summary>
	/// <returns>�E�B���h�E�̍���</returns>
	int GetHeight() const { return height; }

	/// <summary>
	/// �e�N�X�`���̐F���擾����
	/// </summary>
	const TexturePtr& GetTextureColor() const { return texture_color; }
	/// <summary>
	/// �e�N�X�`���̐[�x���擾����
	/// </summary>
	const TexturePtr& GetTextureDepth() const { return texture_depth; }

private:
	GLuint fbo = 0; // FBO�̊Ǘ��ԍ�
	int width = 0,  // FBO�̕�
		height = 0; // FBO�̍���

	TexturePtr texture_color; // �J���[�e�N�X�`��
	TexturePtr texture_depth; // �[�x�e�N�X�`��
};
using FramebufferObjectPtr = std::shared_ptr<FramebufferObject>;

#endif // FRAMEBUFFEROBJECT_H_INCLUDED