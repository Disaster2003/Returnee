/**
* @file FramebufferObject.cpp
*/
#include "FramebufferObject.h"
#include "Debug.h"

/// <summary>
/// �����̃e�N�X�`������FBO���쐬����R���X�g���N�^
/// </summary>
/// <param name="color">�J���[�e�N�X�`��</param>
/// <param name="depth">�[�x�e�N�X�`��</param>
/// <param name="color_level">�J���[�e�N�X�`���̃~�b�v���x��</param>
/// <param name="depth_level">�[�x�e�N�X�`���̃~�b�v���x��</param>
FramebufferObject::FramebufferObject
(
    const TexturePtr& color,
    const TexturePtr& depth,
    int color_level,
    int depth_level
)
    : texture_color(color)
    , texture_depth(depth)
{
    glCreateFramebuffers(1, &fbo);

    // �J���[�o�b�t�@��ݒ肷��
    if (color)
    {
        // FBO�Ƀe�N�X�`���̊��蓖�Ă�
        // (���蓖�Đ�FBO,�p�r,�e�N�X�`��ID,�~�j�}�b�v���x��)
        glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, *color, color_level);
        width = texture_color->GetWidth();
        height = texture_color->GetHeight();
    }
    else
    {
        glNamedFramebufferDrawBuffer(fbo, GL_NONE);
    }

    // �[�x�o�b�t�@��ݒ肷��
    if (depth)
    {
        glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, *depth, depth_level);
        width = texture_depth->GetWidth();
        height = texture_depth->GetHeight();
    }

    // FBO�̃G���[�`�F�b�N
    if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
        texture_color.reset();
        texture_depth.reset();
        LOG_ERROR("FBO�̍쐬�Ɏ��s");
        return;
    }

    // �쐬����
    LOG("FBO(width=%d, height=%d)���쐬", width, height);
}

/// <summary>
/// �f�X�g���N�^
/// </summary>
FramebufferObject::~FramebufferObject()
{
    glDeleteFramebuffers(1, &fbo);
}