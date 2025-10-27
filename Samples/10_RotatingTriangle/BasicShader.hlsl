// BasicShader.hlsl
// 10_RotatingTriangle 샘플용 기본 셰이더 (CBV 사용)

//-----------------------------------------------------------------------------
// Constant Buffer
//-----------------------------------------------------------------------------
cbuffer MVPConstants : register(b0)
{
    float4x4 MVP;
};

//-----------------------------------------------------------------------------
// Vertex Shader Input
//-----------------------------------------------------------------------------
struct VSInput
{
    float3 position : POSITION;
    float4 color : COLOR;
};

//-----------------------------------------------------------------------------
// Vertex Shader Output / Pixel Shader Input
//-----------------------------------------------------------------------------
struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

//-----------------------------------------------------------------------------
// Vertex Shader
//-----------------------------------------------------------------------------
PSInput VSMain(VSInput input)
{
    PSInput output;
    
    // MVP 행렬로 정점 변환
    output.position = mul(float4(input.position, 1.0f), MVP);
    output.color = input.color;
    
    return output;
}

//-----------------------------------------------------------------------------
// Pixel Shader
//-----------------------------------------------------------------------------
float4 PSMain(PSInput input) : SV_TARGET
{
    // 정점 색상을 그대로 출력
    return input.color;
}