// PhongVS.hlsl - Phong Shading Vertex Shader
// Phase 3.3: Basic Phong Shading (No Normal Mapping)

// Constant Buffers
cbuffer ObjectConstants : register(b0)
{
	float4x4 worldMatrix;
	float4x4 mvpMatrix;
	float4x4 worldInvTranspose;
};

// Input Layout (StandardVertex)
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
	float4 Tangent : TANGENT;
};

// Output to Pixel Shader
struct VS_OUTPUT
{
	float4 Position : SV_POSITION; // Clip Space 위치
	float3 WorldPos : POSITION0; // 월드 좌표 (Point Light 거리 계산용)
	float3 Normal : NORMAL; // 월드 노멀 (조명 계산용)
	float2 TexCoord : TEXCOORD0; // 텍스처 좌표
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
	VS_OUTPUT output;
    
    // 1. Clip Space 위치 (미리 계산된 MVP 사용)
	output.Position = mul(float4(input.Position, 1.0f), mvpMatrix);
    
    // 2. 월드 좌표 (Point Light 거리 계산용)
	output.WorldPos = mul(float4(input.Position, 1.0f), worldMatrix).xyz;
    
    // 3. 월드 노멀 변환
    // worldMatrix의 3x3 부분만 사용 (회전/스케일만, 평행이동 제외)
	output.Normal = mul(input.Normal, (float3x3) worldInvTranspose);
	output.Normal = normalize(output.Normal);
    
    // 4. 월드 Tangent 변환
	output.Tangent = mul(input.Tangent.xyz, (float3x3) worldInvTranspose);
	output.Tangent = normalize(output.Tangent);
    
    // 5. Bitangent 계산 (부호 적용)
	output.Bitangent = cross(output.Normal, output.Tangent) * input.Tangent.w; 
	output.Bitangent = normalize(output.Bitangent);
	
    // 6. 텍스처 좌표
	output.TexCoord = input.TexCoord;
    
	return output;
}
