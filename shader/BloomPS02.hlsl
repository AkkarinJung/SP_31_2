#include "Common.hlsl"

//C言語側で設定したサンプラー設定を受け取る
SamplerState g_SamplerState : register(s1); //テクスチャサンプラー1番
//テクスチャを受け取る
Texture2D g_Texture : register(t0); //テクスチャ0番（通常シーン）
Texture2D g_Texture1 : register(t1); //テクスチャ1番（輝度マップ）

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    //このピクセルに使うテクセルを取得
    outDiffuse = g_Texture.SampleLevel(g_SamplerState, In.TexCoord, 0);
    //ここではミップマップチェーンを3つブレンドして使う
    float3 Luminance = 0;
    Luminance += g_Texture1.SampleLevel(g_SamplerState, In.TexCoord, 2.5f).rgb * Parameter.y;
    Luminance += g_Texture1.SampleLevel(g_SamplerState, In.TexCoord, 4.5f).rgb * Parameter.z;
    Luminance += g_Texture1.SampleLevel(g_SamplerState, In.TexCoord, 6.5f).rgb * Parameter.w;

    outDiffuse.rgb += saturate(Luminance);
    outDiffuse *= In.Diffuse;
}