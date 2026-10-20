
#include "Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

//適当なSeed値をもとに疑似乱数を計算する
float Rand(float2 coord)
{
    //frac(x)=xの小数部を返す 乱数は0.0 <= rand < 1.0の範囲
    return frac(sin(dot(coord.xy, float2(12.9898f, 78.233f))) * 43758.5453f);
}

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET)
{
    // テクセルの取得
    float4 col = g_Texture.Sample(g_SamplerState, In.TexCoord);

    col.rgb += clamp(Rand(In.TexCoord * 300.0f), Parameter.y, Parameter.z);
    //col.rgb = (col.r * 0.3f) + (col.g * 0.6f) + (col.b * 0.1f);
    
    // 頂点カラー乗算
    col.rgb *= In.Diffuse.rgb;

    float div = 1.0f / Parameter.x; // 1段階あたりの幅を計算
    col.rgb = floor(col.rgb / div) * div; // 幅の値を使って色の値を求める
    col.rgb = saturate(col.rgb);

    // 結果出力
    outDiffuse.rgb = col.rgb;

    // アルファ値の処理
    outDiffuse.a = col.a * In.Diffuse.a;
}
