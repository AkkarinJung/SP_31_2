#include "Common.hlsl"

SamplerState g_SamplerState : register(s1); //テクスチャサンプラー1番（リピートなし設定）
Texture2D g_Texture : register(t0); //テクスチャ0番

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    //テクセルの情報にピクセルのデフューズ色を乗算する
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
    outDiffuse *= In.Diffuse;
    //輝度抽出
    {
        float Luminance = dot(outDiffuse.rgb, float3(0.299f, 0.587f, 0.114f)); // ★ 輝度の計算
        Luminance = pow(Luminance, 1); //調節
        if (Luminance < Parameter.x)   //適当なしきい値
        {
            Luminance = 0.0f;
        }
        Luminance = saturate(Luminance);
        outDiffuse.rgb = Luminance;
    }
}