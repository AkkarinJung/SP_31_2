#include "Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET , out float4 outDoF : SV_Target1)
{
    float4 normal = normalize(In.Normal);
    
    // Diffuse lighting
    float light = -dot(normal.xyz, Light.Direction.xyz);
    light = saturate(light);
    
    // Texture color
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
    
    // Apply vertex diffuse and light
    outDiffuse.rgb *= In.Diffuse.rgb * Light.Diffuse.rgb * light + Light.Ambient.rgb;
    
    // Apply alpha
    outDiffuse.a *= In.Diffuse.a;

    // Specular lighting (Reflection)
    float3 eyev = normalize(In.WorldPosition.xyz - CameraPosition.xyz);
    float3 halfv = normalize(eyev + Light.Direction.xyz);
    float specular = saturate(-dot(normal.xyz, halfv));
    specular = pow(specular, 30.0f);

    // Add specular highlight
    outDiffuse.rgb += specular;
    outDoF = outDiffuse;
}
