#version 450

layout(binding=0) uniform sampler2D texColor; // 遮蔽率
layout(binding=1) uniform sampler2D texDepth; // 線形深度値

out vec4 outColor;

// ぼかし係数の配列
const float gaussian[5] = 
  { 0.153170, 0.444893, 0.422649, 0.392902, 0.362970 };

const int blurRadius = 4;        // ぼかし半径
const int blurScale = 2;         // ぼかし半径の拡大率
const float edgeDistance = 0.15; // 同じ物体とみなす深度差(単位=m)
const float edgeSharpness = 1 / edgeDistance; // 物体の輪郭をぼかさないための係数
const float far = 1000;          // ファー平面の深度値

// エントリーポイント
void main()
{
    // ピクセルの遮蔽率と深度値を取得する
    ivec2 uv = ivec2(gl_FragCoord.xy);
    outColor = texelFetch(texColor, uv, 0);
    float depth = texelFetch(texDepth, uv, 0).x;

    // 深度値がファー平面以上の場合、そのピクセルは背景なのでぼかさない
    if (depth >= far)
        return;

    // バイラテラル・フィルタを実行する
    outColor *= gaussian[0] * gaussian[0];
    float totalWeight = gaussian[0] * gaussian[0];
    for (int y = -blurRadius; y <= blurRadius; ++y)
        for (int x = -blurRadius; x <= blurRadius; ++x)
        {
            // 中心は取得済みなので無視する
            if (y == 0 && x == 0)
              continue;

            // サンプル点のウェイト(重要度)を取得する
            float weight = gaussian[abs(x)] * gaussian[abs(y)];

            // 中心との深度値の差によって物体の境界を判定し、ウェイトを調整する
            ivec2 sampleUV = uv + ivec2(x, y) * blurScale;
            float sampleDepth = texelFetch(texDepth, sampleUV, 0).x;
            weight *= max(0.0, 1.0 - edgeSharpness * abs(sampleDepth - depth));

            // 遮蔽率と合計ウェイトを更新する
            outColor += texelFetch(texColor, sampleUV, 0) * weight;
            totalWeight += weight;
        }

    outColor /= totalWeight;
}