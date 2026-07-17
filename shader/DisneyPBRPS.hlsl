#include "Common.hlsl"
#include "CookTorranceSub.hlsl"


Texture2D g_Texture : register(t0);
Texture2D g_TextureRoughness : register(t1);
Texture2D g_TextureMetalness : register(t2);

SamplerState g_SamplerState : register(s0);

float CalculateDiffuseFromFresnel(float3 N, float3 L, float3 V);
float CalculateCookTorranceSpecular(float3 L, float3 V, float3 N, float smooth, float metallic);

static const float PI = 3.1415926f;

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET)
{

    float4 normal = normalize(In.Normal);

    float4 albedocolor = g_Texture.Sample(g_SamplerState,In.TexCoord);
    float3 specColor = albedocolor;
    
    // 滑らかさを取得
    float smooth = g_TextureRoughness.Sample(g_SamplerState, In.TexCoord).r * 2.0f - 1.0f;
    // smooth *= Parameter.x; //これは無くてもよい
    smooth = saturate(smooth);

    // 金属度を取得
    float metallic = g_TextureMetalness.Sample(g_SamplerState, In.TexCoord).r * 2.0f - 1.0f;
    // metallic *= Parameter.y; //これは無くてもよい
    metallic = saturate(metallic);
    
    float3 eyev = normalize(CameraPosition.xyz - In.WorldPosition.xyz);
    
    //ライトマシマシ
    float3 lit = 0;
    for (int ligNo = 0; ligNo < Parameter.z; ligNo++)//ライトが複数あると仮定
    {
        float4 lv = Light.Position - In.WorldPosition; //光源へのベクトル

    // フレネル反射を考慮した拡散反射を計算
        float diffuseFromFresnel = CalculateDiffuseFromFresnel(
                                    normal.xyz, lv.xyz, eyev);

    // 正規化Lambert拡散反射を求める
        float nl = saturate(dot(normal.xyz, lv.xyz));
        float3 light = nl * Light.Diffuse.rgb / PI;

    // 最終的な拡散反射光を計算する
        float3 diffuse = albedocolor.rgb * diffuseFromFresnel * Light.Diffuse.rgb * light;

    // 鏡面反射率を計算する
        float3 spec = CalculateCookTorranceSpecular(
                    lv.xyz, eyev, normal.xyz, smooth, metallic) * Light.Diffuse;
    // 金属度が高ければ、鏡面反射はスペキュラカラー、低ければ白になるように補間
    // スペキュラカラーの強さを鏡面反射率として扱う
        spec *= lerp(float3(1.0f, 1.0f, 1.0f), specColor, metallic);

    // 滑らかさが高ければ、拡散反射は弱くなる
        lit += diffuse * (1.0f - smooth) + spec;
    }

// 環境光による底上げ
    lit += Light.Ambient.rgb * albedocolor.rgb;

//結果出力
    outDiffuse.rgb = lit;
    outDiffuse.a = albedocolor.a * In.Diffuse.a;

}

float CalculateCookTorranceSpecular(float3 L, float3 V, float3 N, float smooth, float metallic)
{
    // ライトに向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
    float3 H = normalize(L + V);

    // 各種ベクトルがどれくらい似ているかを内積を利用して求める
    float nh = saturate(dot(N, H));
    float vh = saturate(dot(V, H));
    float nl = saturate(dot(N, L));
    float nv = saturate(dot(N, V));

    // D項をベックマン分布を用いて計算する
    float D = CalculateBeckmann(smooth, nh);

    // F項をSchlick近似を用いて計算する
    float F = CalculateFresnel(metallic, vh);

    // G項を求める
    float G = CalculateGeometricDamping(nh, nv, nl, vh);
        // m項を求める
    float m = PI * nv * nh;

    // ここまで求めた、値を利用して、Cook-Torranceモデルの鏡面反射を求める
    return max(F * D * G / m, 0.0);
}

float CalculateDiffuseFromFresnel(float3 N, float3 L, float3 V)
{
    // 法線と光源に向かうベクトルがどれだけ似ているかを内積で求める
    float nl = saturate(dot(N, L));

    // 法線と視線に向かうベクトルがどれだけ似ているかを内積で求める
    float nv = saturate(dot(N, V));

    // 法線と光源への方向に依存する拡散反射率と、法線と視点ベクトルに依存する拡散反射率を
    // 乗算して最終的な拡散反射率を求めている。
    return (nl * nv);
}