#include "Common.hlsl"
#include "CookTorranceSub.hlsl"


Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET)
{
    float4 lv = Light.Position - In.WorldPosition;
    float4 ld = length(lv);
    //lv = normalize(ld);
    lv = normalize(lv);
    
    float ofs = 1.0f - (1.0f / Light.PointLightparam.x) * ld;
    ofs = max(0.0f, ofs);
    
    float4 normal = normalize(In.Normal);
    float light = 0.5f + 0.5f * dot(normal.xyz, lv.xyz);
    light = saturate(light);
    light *= ofs;
    
    
    float4 albedocolor = g_Texture.Sample(g_SamplerState,In.TexCoord);
    albedocolor = float4(0.7f, 0.6f, 0.3f, 1.0f);
    outDiffuse = albedocolor;
    outDiffuse.rgb *= In.Diffuse.rgb * Light.Diffuse.rgb * light + Light.Ambient.rgb;
    outDiffuse.a *= In.Diffuse.a;
    
    float3 eyev = normalize(CameraPosition.xyz - In.WorldPosition.xyz);
    float3 halfv = normalize(eyev + lv.xyz);
 
    //スペキュラーの計算
    float3 specular;
    float nv = saturate(dot(normal.xyz, eyev));
    float nh = saturate(dot(normal.xyz, halfv));
    float vh = saturate(dot(eyev, halfv));
    float nl = saturate(dot(normal.xyz, lv.xyz));
    
    //今回はテクスチャからの情報でなく設定した数値で代用
    float smooth = saturate(Parameter.x);
    float metallic = saturate(Parameter.y);
    
    float D = CalculateBeckmann(smooth, nh); //面の粗さ
    float G = CalculateGeometricDamping(nh, nv, nl, vh);
    float F = CalculateFresnel(metallic, dot(lv.xyz, halfv));
    
    specular = max(0.0f, F * D * G / nv) * albedocolor.rgb;

    outDiffuse.rgb += (specular * ofs); // = のみにしてスペキュラだけにすると分かりやすい
}
