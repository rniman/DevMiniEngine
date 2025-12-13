// PhongPS.hlsl - Phong Shading Pixel Shader
// Phase 3.3: Basic Phong Shading with Blinn-Phong Model

// ========== Constant Buffers ==========

cbuffer ObjectConstants : register(b0)
{
	float4x4 worldMatrix;
	float4x4 mvpMatrix;
};

cbuffer MaterialConstants : register(b1)
{
	float4 baseColor;
	float metallic;
	float roughness;
	uint textureFlags;
	float padding;
};

// 플래그 정의
static const uint TEX_ALBEDO = 1 << 0; // 1
static const uint TEX_NORMAL = 1 << 1; // 2
static const uint TEX_METALLIC = 1 << 2; // 4
static const uint TEX_ROUGHNESS = 1 << 3; // 8
static const uint TEX_AmbientOcclusion = 1 << 4; // 16
static const uint TEX_EMISSIVE = 1 << 5; // 32

// Directional Lights
struct DirectionalLight
{
	float4 direction; // xyz=방향, w=0.0
	float3 color;
	float intensity;
};

// Point Lights
struct PointLight
{
	float4 position; // xyz=위치, w=1.0
	float4 rangeAndColor; // x=range, yzw=color
	float4 intensityAndAttenuation; // x=intensity, yzw=attenuation (Kc, Kl, Kq)
};

cbuffer LightingData : register(b2)
{
	DirectionalLight dirLights[4];
	uint numDirLights;
	uint3 padding0;

	PointLight pointLights[8];
	uint numPointLights;
	uint3 padding2;

    // Camera
	float3 viewPos;
	float padding3;
};

// ========== Textures & Samplers ==========

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MetallicTexture : register(t2);
Texture2D RoughnessTexture : register(t3);
Texture2D AmbientOcclusionTexture : register(t4);
Texture2D EmissiveTexture : register(t5);
Texture2D HeightTexture : register(t6);

SamplerState DefaultSampler : register(s0);

// ========== Input from Vertex Shader ==========

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 WorldPos : POSITION0;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD0;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
};

// ========== Normal Mapping Functions ==========

/**
 * @brief TBN 행렬 생성 및 Normal Map 적용
 */
float3 ApplyNormalMap(
    float3 tangentNormal,
    float3 worldNormal,
    float3 worldTangent,
    float3 worldBitangent)
{
    // TBN 행렬 구성 (Tangent Space → World Space)
	float3x3 TBN = float3x3(
        normalize(worldTangent),
        normalize(worldBitangent),
        normalize(worldNormal)
    );

    // Tangent Space Normal을 World Space로 변환
	float3 worldSpaceNormal = mul(tangentNormal, TBN);

	return normalize(worldSpaceNormal);
}

/**
 * @brief Normal Map 샘플링 및 언패킹
 */
float3 SampleNormalMap(Texture2D normalTex, SamplerState samp, float2 uv)
{
    // Normal Map 샘플링 (RGB 값은 [0, 1] 범위)
	float3 normalMap = normalTex.Sample(samp, uv).rgb;

    // [0, 1] → [-1, 1] 변환
	normalMap = normalMap * 2.0f - 1.0f;

	return normalMap;
}

// ========== Phong Shading Functions ==========

/**
 * @brief Directional Light Blinn-Phong 계산
 */
float3 CalculateDirectionalLight(
    DirectionalLight light,
    float3 normal,
    float3 viewDir,
    float3 albedo,
    float shininess)
{
    // 빛이 오는 방향 (direction은 빛이 가는 방향이므로 반전)
	float3 lightDir = normalize(-light.direction.xyz);

    // 1. Ambient (글로벌 환경광)
	float3 ambient = light.color * light.intensity * 0.1f * albedo;

    // 2. Diffuse (Lambert)
	float diff = max(dot(normal, lightDir), 0.0f);
	float3 diffuse = diff * light.color * light.intensity * albedo;

    // 3. Specular (Blinn-Phong)
	float3 halfDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfDir), 0.0f), shininess);
	float3 specular = spec * light.color * light.intensity;

	return ambient + diffuse + specular;
}

/**
 * @brief Point Light Blinn-Phong 계산 (Attenuation 포함)
 */
float3 CalculatePointLight(
    PointLight light,
    float3 worldPos,
    float3 normal,
    float3 viewDir,
    float3 albedo,
    float shininess)
{
    // 데이터 추출
	float3 lightPos = light.position.xyz;
	float range = light.rangeAndColor.x;
	float3 lightColor = light.rangeAndColor.yzw;
	float intensity = light.intensityAndAttenuation.x;
	float3 attenuation = light.intensityAndAttenuation.yzw;

    // 1. 거리 및 방향 계산
	float3 lightVec = lightPos - worldPos;
	float distance = length(lightVec);

    // Range 체크 (조기 반환으로 성능 최적화)
	if(distance > range)
	{
		return float3(0.0f, 0.0f, 0.0f);
	}

	float3 lightDir = normalize(lightVec);

    // 2. Attenuation 계산
    // Attenuation = 1.0 / (Kc + Kl * d + Kq * d^2)
	float atten = 1.0f / (
        attenuation.x + // constant (Kc)
        attenuation.y * distance + // linear (Kl)
        attenuation.z * distance * distance // quadratic (Kq)
    );
	atten = max(atten, 0.0f);

    // 3. Ambient (Point Light는 약한 Ambient)
	float3 ambient = lightColor * intensity * 0.05f * albedo * atten;

    // 4. Diffuse (Lambert)
	float diff = max(dot(normal, lightDir), 0.0f);
	float3 diffuse = diff * lightColor * intensity * albedo * atten;

    // 5. Specular (Blinn-Phong)
	float3 halfDir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfDir), 0.0f), shininess);
	float3 specular = spec * lightColor * intensity * atten;

	return ambient + diffuse + specular;
}

// ========== Main Pixel Shader ==========

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    // 1. Normal 준비
	float3 normal = normalize(input.Normal);

    // Normal Map 사용 (Uniform 조건 - Warp Divergence 없음)
	if(textureFlags & TEX_NORMAL)
	{
		float3 tangentNormal = SampleNormalMap(NormalTexture, DefaultSampler, input.TexCoord);
		normal = ApplyNormalMap(
            tangentNormal,
            input.Normal,
            input.Tangent,
            input.Bitangent
        );
	}

    // 2. View Direction
	float3 viewDir = normalize(viewPos - input.WorldPos);

    // 3. Albedo
	float3 albedo = baseColor.rgb;
	if(textureFlags & TEX_ALBEDO)
	{
		albedo *= AlbedoTexture.Sample(DefaultSampler, input.TexCoord).rgb;
	}

    // 4. Roughness
	float finalRoughness = roughness;
	if(textureFlags & TEX_ROUGHNESS)
	{
		finalRoughness = RoughnessTexture.Sample(DefaultSampler, input.TexCoord).r;
	}

    // 5. Shininess (Roughness → Shininess 변환)
	float shininess = lerp(128.0f, 8.0f, finalRoughness);

    // 6. 조명 계산
	float3 finalColor = float3(0.0f, 0.0f, 0.0f);

    // 6-1. Directional Lights
	for(uint i = 0; i < numDirLights; ++i)
	{
		finalColor += CalculateDirectionalLight(
            dirLights[i],
            normal,
            viewDir,
            albedo,
            shininess
        );
	}

    // 6-2. Point Lights
	for(uint j = 0; j < numPointLights; ++j)
	{
		finalColor += CalculatePointLight(
            pointLights[j],
            input.WorldPos,
            normal,
            viewDir,
            albedo,
            shininess
        );
	}

    // 7. AmbientOcclusion (Ambient Occlusion)
	if(textureFlags & TEX_AmbientOcclusion)
	{
		float AmbientOcclusion = AmbientOcclusionTexture.Sample(DefaultSampler, input.TexCoord).r;
		finalColor *= AmbientOcclusion;
	}

    // 8. Emissive
	if(textureFlags & TEX_EMISSIVE)
	{
		float3 emissive = EmissiveTexture.Sample(DefaultSampler, input.TexCoord).rgb;
		finalColor += emissive;
	}

    // 9. Gamma Correction
	finalColor = pow(finalColor, 1.0f / 2.2f);

	return float4(finalColor, baseColor.a);
}
