
#include "Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET)
{
    float2 uv = In.TexCoord;

    //Rの処理
    outDiffuse.r =
    g_Texture.Sample(
        g_SamplerState,
        float2(uv.x + Parameter.x, uv.y + Parameter.y)
    ).r;

    //Gの処理
    outDiffuse.g =
        g_Texture.Sample(g_SamplerState, float2(uv.x, uv.y)).g;

    //Bの処理
    outDiffuse.b =
    g_Texture.Sample(
        g_SamplerState,
        float2(uv.x - Parameter.x, uv.y - Parameter.y)
    ).b;

    //Aの処理
    outDiffuse.a =
        g_Texture.Sample(g_SamplerState, float2(uv.x, uv.y)).a;

    //頂点の色を合成
    outDiffuse *= In.Diffuse; //頂点（ピクセル）の色を合成
}



