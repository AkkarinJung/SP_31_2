#include "Common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

void main(in PS_IN In, out float4 outDiffuse : SV_TARGET)
{
    float4 normal = normalize(In.Normal);
    
    // Diffuse lighting
    float light = -dot(normal.xyz, Light.Direction.xyz);
    light = saturate(light);

    // Specular lighting (Reflection)
    float3 eyev = normalize(In.WorldPosition.xyz - CameraPosition.xyz);
    float3 refv = normalize(reflect(Light.Direction.xyz, normal.xyz));
    float specular = saturate(-dot(eyev, refv));
    specular = pow(specular, 30.0f);

    // Texture color
    outDiffuse = g_Texture.Sample(g_SamplerState, In.TexCoord);
    
    // Apply vertex diffuse and light
    outDiffuse.rgb *= In.Diffuse.rgb * light;
    
    // Add specular highlight
    outDiffuse.rgb += specular;
    
    // Apply alpha
    outDiffuse.a *= In.Diffuse.a;
}
