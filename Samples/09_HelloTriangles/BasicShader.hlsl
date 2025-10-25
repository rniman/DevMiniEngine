// BasicShader.hlsl
// 09_HelloTriangle 샘플용 기본 셰이더

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
    
    // 단순 Pass-through (변환 없음)
    output.position = float4(input.position, 1.0f);
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