
#include "Common.hlsl"

Texture2D g_TextureBlur : register(t0);
Texture2D g_TextureScene : register(t1);
Texture2D g_TextureDepth : register(t2);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET )
{
        // テクスチャのデータを取得する (RGBA)
    float4 blur = g_TextureBlur.Sample(g_SamplerState, In.TexCoord); // ボケ画像
    float4 scene = g_TextureScene.Sample(g_SamplerState, In.TexCoord); // 通常シーン
    float4 depth = g_TextureDepth.Sample(g_SamplerState, In.TexCoord); // 深度

    float d = abs(Parameter.x - depth.r); // d = |ピントの距離 - 深度値|
    if (d > Parameter.y) // ピンの合う領域外
    {
        d = 0.0f; // 完全ピンボケ
    }
    else
    {
        // ピントからの距離を0-1で考える
        d = 1.0f - lerp(0.0f, 1.0f, d / Parameter.y);
    }
    // d=1.0に近いほどピントが合っているものとしてブレンド
    float4 color = lerp(blur, scene, smoothstep(0.0f, 1.0f, d));

    outDiffuse.rgb = color.rgb;
    outDiffuse.a = scene.a;
    outDiffuse.rgb *= In.Diffuse;
}



