struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
};

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    // 입력 색상 그대로 출력
	return float4(input.Color, 1.0f);
}
