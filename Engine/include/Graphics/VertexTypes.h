#pragma once
#include "Math/MathTypes.h"

namespace Graphics
{
	/**
	 * @brief 기본 버텍스 구조체 (위치 + 색상)
	 *
	 * 첫 삼각형 렌더링을 위한 단순한 버텍스 포맷
	 */
	struct BasicVertex
	{
		Math::Vector3 position;  // 로컬 좌표계 위치
		Math::Vector4 color;     // RGBA 색상 (0.0 ~ 1.0)

		/**
		 * @brief 이 Vertex 타입에 대한 Input Layout 반환
		 */
		static D3D12_INPUT_LAYOUT_DESC GetInputLayout()
		{
			static D3D12_INPUT_ELEMENT_DESC elements[] =
			{
				{
					"POSITION",
					0,
					DXGI_FORMAT_R32G32B32_FLOAT,
					0,
					0,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					0
				},
				{
					"COLOR",
					0,
					DXGI_FORMAT_R32G32B32A32_FLOAT,
					0,
					12,  // offsetof(BasicVertex, color)
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					0
				}
			};

			D3D12_INPUT_LAYOUT_DESC layout = {};
			layout.pInputElementDescs = elements;
			layout.NumElements = _countof(elements);
			return layout;
		}
	};

	struct TexturedVertex
	{
		Math::Vector3 position;  // POSITION
		Math::Vector2 texCoord;  // TEXCOORD0
		Math::Vector4 color;     // COLOR (옵션)

		static D3D12_INPUT_LAYOUT_DESC GetInputLayout()
		{
			static D3D12_INPUT_ELEMENT_DESC elements[] =
			{
				{
					"POSITION",
					0,
					DXGI_FORMAT_R32G32B32_FLOAT,
					0,
					0,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					0
				},
				{
					"TEXCOORD",
					0,
					DXGI_FORMAT_R32G32_FLOAT,
					0,
					12,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					0
				},
				{
					"COLOR",
					0,
					DXGI_FORMAT_R32G32B32A32_FLOAT,
					0,
					20,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					0
				}
			};

			D3D12_INPUT_LAYOUT_DESC layout = {};
			layout.pInputElementDescs = elements;
			layout.NumElements = _countof(elements);
			return layout;
		}
	};

	/**
	 * @brief 표준 버텍스 (Position + Normal + TexCoord + Tangent)
	 *
	 * 조명(Lighting)과 텍스처링을 모두 지원하는 가장 일반적인 포맷입니다.
	 * Phase 3.3 Phong Shading에 사용됩니다.
	 * 조명, 텍스처, Normal Mapping을 모두 지원하는 표준 포맷
	 */
	struct StandardVertex
	{
		Math::Vector3 position;
		Math::Vector3 normal;
		Math::Vector2 texCoord;
		Math::Vector3 tangent;

		static D3D12_INPUT_LAYOUT_DESC GetInputLayout()
		{
			static D3D12_INPUT_ELEMENT_DESC elements[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			D3D12_INPUT_LAYOUT_DESC layout = {};
			layout.pInputElementDescs = elements;
			layout.NumElements = _countof(elements);
			return layout;
		}
	};

	/**
	 * @brief Debug 렌더링용 정점 구조체
	 *
	 * 와이어프레임, 기즈모 등 디버그 시각화에 사용됩니다.
	 * 조명 계산이 필요 없으므로 Position만 포함합니다. (Color는 CBV로 전달)
	 */
	struct DebugVertex
	{
		Math::Vector3 position;

		static D3D12_INPUT_LAYOUT_DESC GetInputLayout()
		{
			static D3D12_INPUT_ELEMENT_DESC elements[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			return { elements, _countof(elements) };
		}
	};

} // namespace Graphics
