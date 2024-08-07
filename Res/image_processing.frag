/**
* @file image_processing.frag
*/
#version 450

layout(location = 1) in vec2 inTexcoord;

out vec4 outColor;

layout(binding = 0) uniform sampler2D texColor;

// プログラムからの入力
layout(location = 200) uniform mat3 colorMatrix;

// 放射状ブラーのパラメータ
// xy=中心座標(NDC), z=長さ, w=開始距離
layout(location = 201) uniform vec4 radialBlur;

// エントリーポイント
void main()
{
    outColor = texture(texColor, inTexcoord);
    
    // 放射状ブラー
    if (radialBlur.z > 0)
    {
        // 中心へ向かうベクトルを計算する
        vec2 v = radialBlur.xy - inTexcoord;
        
        // ブラー開始距離以下の場合は方向ベクトルを0にする
        float lenA = length(v);
        float lenB = max(lenA - radialBlur.w, 0);
        v *= lenB / lenA;
        
        v *= radialBlur.z; // ベクトルの長さを調節
        
        // ベクトル方向のピクセルを複数サンプリングして画像をぼかす
        outColor.rgb *= 1.0 / 8.0;
        float ratio = 1.0 / 8.0;
        for (float i = 1; i < 8; ++i)
          outColor.rgb += texture(texColor, inTexcoord + v * i).rgb * ratio;
    } // if radialBlur.z
    
    // 色変換行列で色を変える
    outColor.rgb = colorMatrix * outColor.rgb;
    
    // ガンマ補正を行う
    outColor.rgb = pow(outColor.rgb, vec3(1 / 2.2));
}