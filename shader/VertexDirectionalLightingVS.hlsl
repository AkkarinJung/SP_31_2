
#include "Common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    Out.Position = mul(In.Position, wvp);
    
    float4 worldNormal, normal; //ローカル変数を作成
    normal = float4(In.Normal.xyz, 0.0f); //In.Normalをnormalへコピー（wは0.0fとしてコピー）
    worldNormal = mul(normal,World); //コピーされた法線をワールド行列で回転する
    worldNormal = normalize(worldNormal); //回転後の法線を正規化する
    Out.Normal = worldNormal; //回転後の法線出力 In.Normalでなく
    
    float light = -dot(Light.Direction.xyz, worldNormal.xyz);
    light = saturate(light); //明るさを0と1の間で飽和化する
    
    Out.Diffuse.rgb = light; //明るさを頂点色として出力
    Out.Diffuse.a = In.Diffuse.a; //αは頂点の物をそのまま出力
    Out.TexCoord = In.TexCoord; //テクスチャ座標も出力
}