
#include "Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET)
{
    float2 uv = In.TexCoord;

    //全画面モザイク
    //{
    //    uv *= float2(Parameter.x, Parameter.y); //UV座標に解像度を乗算して画面座標にする
    //    uv /= Parameter.z; //モザイクサイズで除算してブロック座標にする
    //    uv = floor(uv) * Parameter.z; //モザイクサイズを乗算してブロックの左上の座標にする
    //    uv /= float2(Parameter.x, Parameter.y); //モザイクサイズで除算してUV座標に戻す
    //}
    {
        float2 len = float2(0.5f, 0.5f);
        len -= In.TexCoord.xy;
        len.x *= (Parameter.x / Parameter.y);     
        len = length(len);
        if((len.x<Parameter.w))
        {
            uv *= float2(Parameter.x, Parameter.y); //UV座標に解像度を乗算して画面座標にする
            uv /= Parameter.z; //モザイクサイズで除算してブロック座標にする
            uv = floor(uv) * Parameter.z; //モザイクサイズを乗算してブロックの左上の座標にする
            uv /= float2(Parameter.x, Parameter.y); //モザイクサイズで除算してUV座標に戻す
            uv = clamp(uv, 0.001f, 0.999f);
            outDiffuse = g_Texture.SampleLevel(g_SamplerState, uv, 4.0f);
        }
        else
        {
            uv = clamp(uv, 0.001f, 0.999f);
            outDiffuse = g_Texture.SampleLevel(g_SamplerState, uv, 0.0f);
        }
    }
    outDiffuse *= In.Diffuse;
}



