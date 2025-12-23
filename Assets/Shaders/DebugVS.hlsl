cbuffer DebugConstants : register(b0)
{
	float4x4 mvpMatrix;
	float3 gizmoColor;
	float padding;
};

struct VS_INPUT
{
	float3 Position : POSITION;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
};

VS_OUTPUT VSMain(VS_INPUT input)
{
	VS_OUTPUT output;
	output.Position = mul(float4(input.Position, 1.0f), mvpMatrix);
	output.Color = gizmoColor; // Constant Buffer에서 색상 사용
	return output;
}
