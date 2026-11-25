#include "common.hlsl"

#define SCREEN_WIDTH (1280.0f)
#define SCREEN_HEIGHT (720.0f)
#define NUM_SAMPLE (5.0f)



Texture2D g_Texture : register(t0); //通常のテクスチャ
SamplerState g_SamplerState : register(s0);
Texture2D g_TextureShadowDepth : register(t1); //テクスチャ1番
SamplerState g_SamplerState1 : register(s1); //サンプラー1番

void main(in PS_IN In, out float4 outDiffuse : SV_Target)
{
    //物体から光源へのベクトル
    float4 lv = In.WorldPosition - Light.Position;
    //物体と光源の距離
    float ld = length(lv);
    //ベクトルの正規化
    lv = normalize(lv);

    //減衰の計算
    float ofs = 1.0f - (1.0f / Light.PointLightparam.x) * ld;
    //減衰率0未満は0にする。
    ofs = max(0, ofs);

    //ピクセルの法線を正規化
    float4 normal = normalize(In.Normal);
    //光源計算（ハーフランバート版）
    float light = 0.5f - 0.5f * dot(normal.xyz, lv.xyz);
    light *= ofs; //明るさを減衰

    //テクスチャのピクセル色を取得
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
    outDiffuse.rgb *= In.Diffuse.rgb * light; //明るさを乗算
    outDiffuse.a *= In.Diffuse.a; //α値に明るさは関係ない

    //カメラからピクセルへ向かうベクトル
    float3 eyev = In.WorldPosition.xyz - CameraPosition.xyz;
    eyev = normalize(eyev);

    //ハーフベクトルを計算
    float3 halfv = eyev + lv.xyz; //視線ベクトル＋ライトベクトル
    halfv = normalize(halfv);

    //スペキュラーの計算
    float specular = -dot(halfv, normal.xyz); //ハーフベクトルと法線の内積
    specular = saturate(specular);
    specular = pow(specular, 30);
    outDiffuse.rgb += (specular * ofs);
    //ここまでは普通の光源計算

    // シャドウマップのピクセル色を取得
    float3 uv = In.LightPosition.xyz / In.LightPosition.w; // ★
    uv.xy *= float2(0.5f, -0.5f); //テクスチャ座標(0.0～1.0)へ変換
    uv.xy += 0.5f;

    //{//normal
    //    float depth = g_TextureShadowDepth.Sample(g_SamplerState1, uv.xy).r; // ★
    //    if (depth < uv.z - 0.003f) //Zファイティング対応                          // ★
    //    {
    //        outDiffuse.rgb *= 0.5f; //暗くする
    //    }
    //}
    
    {//PCF
        float2 offset = float2(1.0f / SCREEN_WIDTH, 1.0f / SCREEN_HEIGHT);
        float shadowRate = 0.0f;
        float zb[NUM_SAMPLE];
        zb[0] = g_TextureShadowDepth.Sample(g_SamplerState1, uv.xy);
        zb[1] = g_TextureShadowDepth.Sample(g_SamplerState1, uv.xy + float2(0, -offset.y));
        
        zb[2] = g_TextureShadowDepth.Sample(g_SamplerState1, uv.xy + float2(0, offset.y));
        zb[3] = g_TextureShadowDepth.Sample(g_SamplerState1, uv.xy + float2(-offset.x, 0));
        zb[4] = g_TextureShadowDepth.Sample(g_SamplerState1, uv.xy + float2(offset.x, 0));

        for (int i = 0; i < NUM_SAMPLE;i++)
        {
            if ((zb[i] < uv.z - 0.01f))
            {
                shadowRate++;
            }
        }
        shadowRate /= NUM_SAMPLE;
        float3 shadowColor = outDiffuse.rgb * 0.5f;
        outDiffuse.rgb = lerp(outDiffuse.rgb, shadowColor, shadowRate);
    }
}
 