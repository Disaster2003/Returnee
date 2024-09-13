/**
* @file FramebufferObject.cpp
*/
#include "FramebufferObject.h"
#include "Debug.h"

/// <summary>
/// 既存のテクスチャからFBOを作成するコンストラクタ
/// </summary>
/// <param name="color">カラーテクスチャ</param>
/// <param name="depth">深度テクスチャ</param>
/// <param name="color_level">カラーテクスチャのミップレベル</param>
/// <param name="depth_level">深度テクスチャのミップレベル</param>
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

    // カラーバッファを設定する
    if (color)
    {
        // FBOにテクスチャの割り当てる
        // (割り当て先FBO,用途,テクスチャID,ミニマップレベル)
        glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, *color, color_level);
        width = texture_color->GetWidth();
        height = texture_color->GetHeight();
    }
    else
    {
        glNamedFramebufferDrawBuffer(fbo, GL_NONE);
    }

    // 深度バッファを設定する
    if (depth)
    {
        glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, *depth, depth_level);
        width = texture_depth->GetWidth();
        height = texture_depth->GetHeight();
    }

    // FBOのエラーチェック
    if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
        texture_color.reset();
        texture_depth.reset();
        LOG_ERROR("FBOの作成に失敗");
        return;
    }

    // 作成成功
    LOG("FBO(width=%d, height=%d)を作成", width, height);
}

/// <summary>
/// デストラクタ
/// </summary>
FramebufferObject::~FramebufferObject()
{
    glDeleteFramebuffers(1, &fbo);
}