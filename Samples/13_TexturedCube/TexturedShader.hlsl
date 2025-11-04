//-----------------------------------------------------------------------------
// Constant Buffer
//-----------------------------------------------------------------------------
cbuffer MVPConstants : register(b0)
{
	float4x4 MVP;
};

//-----------------------------------------------------------------------------
// Texture & Sampler
//-----------------------------------------------------------------------------
Texture2D gDiffuseMap : register(t0);
Texture2D gNormalMap : register(t1);
Texture2D gSpecularMap : register(t2);
Texture2D gRoughnessMap : register(t3);
Texture2D gMetallicMap : register(t4);
Texture2D gAOMap : register(t5);
Texture2D gEmissiveMap : register(t6);

SamplerState gSampler : register(s0);

//-----------------------------------------------------------------------------
// Vertex Shader Input
//-----------------------------------------------------------------------------
struct VSInput
{
	float3 position : POSITION;
	float2 texCoord : TEXCOORD;
	float4 color : COLOR;
};

//-----------------------------------------------------------------------------
// Vertex Shader Output / Pixel Shader Input
//-----------------------------------------------------------------------------
struct PSInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 color : COLOR;
};

//-----------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------
PSInput VSMain(VSInput input)
{
	PSInput output;
	
	output.position = mul(float4(input.position, 1.0f), MVP);
	output.texCoord = input.texCoord;
	output.color = input.color;
	
	return output;
}

//-----------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------
float4 PSMain(PSInput input) : SV_TARGET
{
    // 일단 Diffuse만 사용
	float4 color = gDiffuseMap.Sample(gSampler, input.texCoord);
    
    // TODO: Normal, Specular 등은 조명 시스템에서 사용
    
	return color;
} 
