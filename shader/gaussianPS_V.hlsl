#include "Common.hlsl"

Texture2D g_Texture : register(t0); //テクスチャ0番
SamplerState g_SamplerState : register(s1); //サンプラー1番（CLAMP）※0番だと画面端にノイズが出る

//サンプリング間隔（1.0:スライド通り  2.0:ボケを強調）※縦横で同じ値にすること
#define BLUR_STEP (1.0f)

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET, out float4 outDoF : SV_Target1)
{
    // テクセルの取得（代表点）
    float4 col = g_Texture.Sample(g_SamplerState, In.TexCoord);

    // 真ん中（代表点）
    outDiffuse.rgb = Weight[0].x * col.rgb;

    //代表点から下へサンプリング
    outDiffuse.rgb += Weight[0].y * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, 1.0f * BLUR_STEP / Parameter.y)).rgb; //下1個目
    outDiffuse.rgb += Weight[0].z * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, 2.0f * BLUR_STEP / Parameter.y)).rgb; //下2個目
    outDiffuse.rgb += Weight[0].w * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, 3.0f * BLUR_STEP / Parameter.y)).rgb; //下3個目
    outDiffuse.rgb += Weight[1].x * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, 4.0f * BLUR_STEP / Parameter.y)).rgb; //下4個目
    outDiffuse.rgb += Weight[1].y * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, 5.0f * BLUR_STEP / Parameter.y)).rgb; //下5個目
    outDiffuse.rgb += Weight[1].z * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, 6.0f * BLUR_STEP / Parameter.y)).rgb; //下6個目
    outDiffuse.rgb += Weight[1].w * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, 7.0f * BLUR_STEP / Parameter.y)).rgb; //下7個目

    //代表点から上へサンプリング
    outDiffuse.rgb += Weight[0].y * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, -1.0f * BLUR_STEP / Parameter.y)).rgb; //上1個目
    outDiffuse.rgb += Weight[0].z * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, -2.0f * BLUR_STEP / Parameter.y)).rgb; //上2個目
    outDiffuse.rgb += Weight[0].w * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, -3.0f * BLUR_STEP / Parameter.y)).rgb; //上3個目
    outDiffuse.rgb += Weight[1].x * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, -4.0f * BLUR_STEP / Parameter.y)).rgb; //上4個目
    outDiffuse.rgb += Weight[1].y * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, -5.0f * BLUR_STEP / Parameter.y)).rgb; //上5個目
    outDiffuse.rgb += Weight[1].z * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, -6.0f * BLUR_STEP / Parameter.y)).rgb; //上6個目
    outDiffuse.rgb += Weight[1].w * g_Texture.Sample(g_SamplerState,
                    In.TexCoord + float2(0.0f, -7.0f * BLUR_STEP / Parameter.y)).rgb; //上7個目

    // アルファ値の処理
    outDiffuse.a = col.a * In.Diffuse.a;
    outDoF = outDiffuse;
}