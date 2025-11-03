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
Texture2D<float4> diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

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
	float4 texColor = diffuseTexture.Sample(diffuseSampler, input.texCoord);
	return texColor * input.color;
} 