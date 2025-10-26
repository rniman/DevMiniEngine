# 최종 설계 다이어그램 (Input Layout 멤버 추가 후)

## 전체 아키텍처 개요

```
┌─────────────────────────────────────────────────────────────┐
│                   Application Layer                         │
│                                                             │
│  void RenderFrame()                                         │
│  {                                                          │
│      D3D12_INPUT_LAYOUT_DESC layout = mesh.GetInputLayout();│
│      ID3D12PipelineState* pso =                             │
│          DX12PipelineStateCache.GetOrCreatePipelineState(   │
│          material, rootSignature, layout);                  │
│      cmdList->SetPipelineState(pso);                        │
│      mesh.Draw(cmdList);                                    │
│  }                                                          │
└─────────────────────────────────────────────────────────────┘
           ↓ 사용           ↓ 사용           ↓ 사용
    ┌──────────┐      ┌──────────┐      ┌──────────────────────┐
    │   Mesh   │      │ Material │      │DX12PipelineStateCache│
    └──────────┘      └──────────┘      └──────────────────────┘
```

---

## 1. DX12PipelineStateCache 구조

```
┌─────────────────────────────────────────────────────────────┐
│                   DX12PipelineStateCache                    │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [소유 - GPU 리소스]                                         │
│  ├── std::unordered_map<PSOKey, ComPtr<ID3D12PipelineState>>│
│  │   └── mPSOCache                                          │
│  │       ├── Key: { materialHash, inputLayoutHash, rootSig }│
│  │       └── Value: ComPtr<ID3D12PipelineState>             │
│  │           └── GPU에 존재하는 실제 PSO (강한 참조)          │
│  │                                                          │
│  [참조 - 외부 의존성, 소유 X]                                 │
│  ├── ID3D12Device* mDevice                                  │
│  │   └── DX12Device가 소유,                                 │
           DX12PipelineStateCache는 포인터만 저장              │
│  │                                                          │
│  └── DX12ShaderCompiler* mShaderCompiler                    │
│      └── 외부에서 주입, DX12PipelineStateCache는 포인터만 저장│
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  주요 메서드                                                 │
│  ├── bool Initialize(ID3D12Device*, DX12ShaderCompiler*)    │
│  ├── ID3D12PipelineState* GetOrCreatePipelineState(         │
│  │       const Material&,                                   │
│  │       ID3D12RootSignature*,                              │
│  │       const D3D12_INPUT_LAYOUT_DESC&)                    │
│  └── void Shutdown()                                        │
└─────────────────────────────────────────────────────────────┘

---

## 2. Material 구조

```
┌─────────────────────────────────────────────────────────────┐
│                          Material                           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [소유 - 렌더링 상태 설정]                                    │
│  ├── D3D12_BLEND_DESC mBlendDesc                            │
│  │   └── 알파 블렌딩, 컬러 쓰기 마스크 등                     │
│  │                                                          │
│  ├── D3D12_RASTERIZER_DESC mRasterizerDesc                  │
│  │   └── 컬링 모드, Fill 모드, 깊이 바이어스 등               │
│  │                                                          │
│  ├── D3D12_DEPTH_STENCIL_DESC mDepthStencilDesc             │
│  │   └── 깊이 테스트, 스텐실 테스트 설정                      │
│  │                                                          │
│  ├── std::wstring mVertexShaderPath                         │
│  │   └── 예: L"Shaders/BasicShader.hlsl"                    │
│  │                                                          │
│  ├── std::wstring mPixelShaderPath                          │
│  │   └── 예: L"Shaders/BasicShader.hlsl"                    │
│  │                                                          │
│  ├── std::string mVSEntryPoint                              │
│  │   └── 예: "VSMain"                                       │
│  │                                                          │
│  ├── std::string mPSEntryPoint                              │
│  │   └── 예: "PSMain"                                       │
│  │                                                          │
│  ├── D3D12_PRIMITIVE_TOPOLOGY_TYPE mPrimitiveTopology       │
│  │   └── TRIANGLE, LINE, POINT                              │
│  │                                                          │
│  ├── DXGI_FORMAT mRTVFormats[8]                             │
│  │   └── 렌더 타겟 포맷 배열                                 │
│  │                                                          │
│  ├── DXGI_FORMAT mDSVFormat                                 │
│  │   └── 깊이-스텐실 포맷                                    │
│  │                                                          │
│  ├── UINT mSampleCount                                      │
│  │   └── MSAA 샘플 개수                                      │
│  │                                                          │
│  └── UINT mSampleQuality                                    │
│      └── MSAA 품질 레벨                                      │
│                                                             │
│  [GPU 리소스 소유 X]                                         │
│  - 렌더링 상태만 정의, GPU 리소스 생성 안 함                   │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  주요 메서드                                                 │
│  ├── Material()                                             │
│  ├── Material(const MaterialDesc&)                          │
│  ├── size_t GetHash() const                                 │
│  ├── D3D12_BLEND_DESC GetBlendState() const                 │
│  ├── D3D12_RASTERIZER_DESC GetRasterizerState() const       │
│  └── D3D12_DEPTH_STENCIL_DESC GetDepthStencilState() const  │
└─────────────────────────────────────────────────────────────┘

---

## 3. Mesh 구조 (Input Layout 추가)

```
┌─────────────────────────────────────────────────────────────┐
│                            Mesh                             │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [소유 - Buffer 객체]                                        │
│  ├── DX12VertexBuffer mVertexBuffer                         │
│  │   ├── ComPtr<ID3D12Resource> mVertexBuffer               │
│  │   │   └── Default Heap (GPU 전용)                        │
│  │   ├── D3D12_VERTEX_BUFFER_VIEW mVertexBufferView         │
│  │   │   ├── BufferLocation (GPU 가상 주소)                  │
│  │   │   ├── SizeInBytes (전체 버퍼 크기)                    │
│  │   │   └── StrideInBytes (Vertex 하나 크기)                │
│  │   ├── Core::uint32 mVertexCount                          │
│  │   └── Core::uint32 mVertexStride                         │
│  │                                                          │
│  ├── DX12IndexBuffer mIndexBuffer                           │
│  │   ├── ComPtr<ID3D12Resource> mIndexBuffer                │
│  │   │   └── Default Heap (GPU 전용)                        │
│  │   ├── D3D12_INDEX_BUFFER_VIEW mIndexBufferView           │
│  │   │   ├── BufferLocation (GPU 가상 주소)                  │
│  │   │   ├── SizeInBytes (전체 버퍼 크기)                    │
│  │   │   └── Format (R16_UINT or R32_UINT)                  │
│  │   ├── Core::uint32 mIndexCount                           │
│  │   └── DXGI_FORMAT mIndexFormat                           │
│  │                                                          │
│  ├── D3D12_INPUT_LAYOUT_DESC mInputLayout  새로 추가!        │
│  │   ├── D3D12_INPUT_ELEMENT_DESC* pInputElementDescs       │
│  │   │   └── BasicVertex::GetInputLayout()의 static 배열    │
│  │   └── UINT NumElements                                   │
│  │       └── 2 (POSITION + COLOR)                           │
│  │                                                          │
│  └── bool mInitialized                                      │
│                                                             │
│  [일시적 소유 - Initialize() 중에만]                         │
│  └── ComPtr<ID3D12Resource> mUploadBuffer (각 Buffer에)     │
│      └── Upload Heap → GPU 복사 완료 후 즉시 해제            │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  주요 메서드                                                 │
│  ├── bool Initialize(ID3D12Device*,                         │
│  │       DX12CommandQueue*, DX12CommandContext*,            │
│  │       const BasicVertex*, uint32,                        │
│  │       const uint16*, uint32)                             │
│  ├── void Shutdown()                                        │
│  ├── void Draw(ID3D12GraphicsCommandList*)                  │
│  ├── uint32 GetVertexCount() const                          │
│  ├── uint32 GetIndexCount() const                           │
│  ├── bool IsInitialized() const                             │
│  ├── bool HasIndexBuffer() const                            │
│  └── D3D12_INPUT_LAYOUT_DESC GetInputLayout() const         │
└─────────────────────────────────────────────────────────────┘

---

## 4. DX12VertexBuffer 구조

```
┌─────────────────────────────────────────────────────────────┐
│                      DX12VertexBuffer                       │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [소유 - GPU 리소스]                                         │
│  ├── ComPtr<ID3D12Resource> mVertexBuffer                   │
│  │   └── Default Heap (D3D12_HEAP_TYPE_DEFAULT)             │
│  │       ├── GPU에서만 접근 가능                             │
│  │       ├── CPU 접근 불가                                   │
│  │       └── 고속 GPU 메모리                                 │
│  │                                                          │
│  ├── D3D12_VERTEX_BUFFER_VIEW mVertexBufferView             │
│  │   ├── BufferLocation: GPU 가상 주소                       │
│  │   ├── SizeInBytes: 전체 버퍼 크기                         │
│  │   └── StrideInBytes: sizeof(BasicVertex) = 28 bytes      │
│  │                                                          │
│  ├── Core::uint32 mVertexCount                              │
│  │   └── 예: 삼각형 = 3                                      │
│  │                                                          │
│  └── Core::uint32 mVertexStride                             │
│      └── sizeof(BasicVertex) = 28 bytes                     │
│          ├── Vector3 position: 12 bytes                     │
│          └── Vector4 color: 16 bytes                        │
│                                                             │
│  [일시적 소유 - Initialize() 중에만]                          │
│  └── ComPtr<ID3D12Resource> mUploadBuffer                   │
│      └── Upload Heap (D3D12_HEAP_TYPE_UPLOAD)               │
│          ├── CPU에서 GPU로 복사용                            │
│          ├── CopyResource() 후 해제                         │
│          └── WaitForIdle() 보장 후 안전하게 해제             │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  주요 메서드                                                 │
│  ├── bool Initialize(ID3D12Device*,                         │
│  │       DX12CommandQueue*, DX12CommandContext*,            │
│  │       const void*, uint32 count, uint32 stride)          │
│  ├── void Shutdown()                                        │
│  ├── void Bind(ID3D12GraphicsCommandList*, uint32 slot)     │
│  ├── uint32 GetVertexCount() const                          │
│  ├── uint32 GetVertexStride() const                         │
│  └── bool IsInitialized() const                             │
└─────────────────────────────────────────────────────────────┘

---

## 5. DX12IndexBuffer 구조

```
┌─────────────────────────────────────────────────────────────┐
│                       DX12IndexBuffer                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [소유 - GPU 리소스]                                         │
│  ├── ComPtr<ID3D12Resource> mIndexBuffer                    │
│  │   └── Default Heap (D3D12_HEAP_TYPE_DEFAULT)             │
│  │                                                          │
│  ├── D3D12_INDEX_BUFFER_VIEW mIndexBufferView               │
│  │   ├── BufferLocation: GPU 가상 주소                       │
│  │   ├── SizeInBytes: 전체 버퍼 크기                         │
│  │   └── Format: DXGI_FORMAT_R16_UINT or R32_UINT           │
│  │                                                          │
│  ├── Core::uint32 mIndexCount                               │
│  │   └── 예: 삼각형 = 3                                      │
│  │                                                          │
│  └── DXGI_FORMAT mIndexFormat                               │
│      ├── R16_UINT: 65,535개 미만 버텍스                      │
│      └── R32_UINT: 4,294,967,295개 미만 버텍스               │
│                                                             │
│  [일시적 소유 - Initialize() 중에만]                         │
│  └── ComPtr<ID3D12Resource> mUploadBuffer                   │
│      └── Upload Heap (D3D12_HEAP_TYPE_UPLOAD)               │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  주요 메서드                                                 │
│  ├── bool Initialize(ID3D12Device*,                         │
│  │       DX12CommandQueue*, DX12CommandContext*,            │
│  │       const void*, uint32 count, DXGI_FORMAT format)     │
│  ├── void Shutdown()                                        │
│  ├── void Bind(ID3D12GraphicsCommandList*)                  │
│  ├── uint32 GetIndexCount() const                           │
│  ├── DXGI_FORMAT GetIndexFormat() const                     │
│  └── bool IsInitialized() const                             │
└─────────────────────────────────────────────────────────────┘

---

## 6. BasicVertex 구조

```
┌─────────────────────────────────────────────────────────────┐
│                        BasicVertex                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  [데이터 멤버]                                               │
│  ├── Math::Vector3 position                                 │
│  │   ├── float x, y, z                                      │
│  │   └── 크기: 12 bytes                                     │
│  │                                                          │
│  └── Math::Vector4 color                                    │
│      ├── float r, g, b, a                                   │
│      └── 크기: 16 bytes                                     │
│                                                             │
│  [Static 메서드]                                             │
│  └── static D3D12_INPUT_LAYOUT_DESC GetInputLayout()        │
│      │                                                      │
│      └── 반환값                                              │
│          ├── pInputElementDescs: static 배열 포인터          │
│          │   └── D3D12_INPUT_ELEMENT_DESC[2]                │
│          │       ├─ [0] POSITION: R32G32B32_FLOAT, offset 0 │
│          │       └─ [1] COLOR: R32G32B32A32_FLOAT, offset 12│
│          └── NumElements: 2                                 │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  메모리 레이아웃                                              │
│  ┌────────────────────────────────────────────────┐         │
│  │ Offset 0-11:  position (Vector3)               │         │
│  │               [x(4)] [y(4)] [z(4)]             │         │
│  ├────────────────────────────────────────────────┤         │
│  │ Offset 12-27: color (Vector4)                  │         │
│  │               [r(4)] [g(4)] [b(4)] [a(4)]      │         │
│  └────────────────────────────────────────────────┘         │
│  Total Size: 28 bytes                                       │
└─────────────────────────────────────────────────────────────┘

---

## 7. 의존성 관계도

### 소유 관계 (→)
```
DX12PipelineStateCache
    → std::unordered_map<PSOKey, ComPtr<ID3D12PipelineState>>

Material
    → D3D12_BLEND_DESC
    → D3D12_RASTERIZER_DESC
    → D3D12_DEPTH_STENCIL_DESC
    → std::wstring × 2
    → std::string × 2

Mesh
    → DX12VertexBuffer
    → DX12IndexBuffer
    → D3D12_INPUT_LAYOUT_DESC

DX12VertexBuffer
    → ComPtr<ID3D12Resource>
    → D3D12_VERTEX_BUFFER_VIEW

DX12IndexBuffer
    → ComPtr<ID3D12Resource>
    → D3D12_INDEX_BUFFER_VIEW
```

### 참조 관계 (⇢)
```
DX12PipelineStateCache
    ⇢ ID3D12Device*
    ⇢ DX12ShaderCompiler*

Material
    ⇢ 없음 (자급자족)

Mesh
    ⇢ 없음 (자급자족)

DX12VertexBuffer
    ⇢ ID3D12Device* (Initialize 시에만)
    ⇢ DX12CommandQueue* (Initialize 시에만)
    ⇢ DX12CommandContext* (Initialize 시에만)

DX12IndexBuffer
    ⇢ ID3D12Device* (Initialize 시에만)
    ⇢ DX12CommandQueue* (Initialize 시에만)
    ⇢ DX12CommandContext* (Initialize 시에만)
```

### 사용 관계 (-->)
```
Application
    --> Mesh (Bind, Draw, GetInputLayout)
    --> Material (설정)
    --> DX12PipelineStateCache (GetOrCreatePipelineState)

DX12PipelineStateCache
    --> Material (Hash 계산)
    --> D3D12_INPUT_LAYOUT_DESC (Hash 계산)
    --> ID3D12RootSignature (Hash 키)

Mesh
    --> BasicVertex (GetInputLayout)

BasicVertex
    --> D3D12_INPUT_ELEMENT_DESC[] (static)
```

---

## 8. 데이터 흐름

### 초기화 시퀀스
```
1. Application
   ↓ Create
   Mesh, Material, DX12PipelineStateCache

2. Mesh::Initialize()
   ├─ DX12VertexBuffer::Initialize()
   │  ├─ Default Heap 버퍼 생성
   │  ├─ Upload Heap 버퍼 생성
   │  ├─ CPU → Upload Heap 복사
   │  ├─ Upload → Default 복사 (GPU)
   │  ├─ Resource Barrier
   │  ├─ WaitForIdle()
   │  └─ Upload Buffer 해제
   │
   ├─ DX12IndexBuffer::Initialize()
   │  └─ (동일 과정)
   │
   └─ mInputLayout = BasicVertex::GetInputLayout() 

3. DX12PipelineStateCache::Initialize()
   ├─ mDevice = device
   └─ mShaderCompiler = compiler
```

### 렌더링 시퀀스
```
1. Application::RenderFrame()
   ↓
2. layout = mesh.GetInputLayout()
   ├─ return mInputLayout
   └─ { pElements: static 배열, NumElements: 2 }
   ↓
3. pso = DX12PipelineStateCache.GetOrCreatePipelineState(material, rootSig, layout)
   ├─ PSOKey key = {
   │      materialHash: material.GetHash(),
   │      inputLayoutHash: HashInputLayout(layout),
   │      rootSignature: rootSig
   │  }
   │
   ├─ if (mPSOCache.find(key) != end)
   │    return cachedPSO;  // Cache Hit
   │
   └─ else
        ├─ CompileShaders(material)
        ├─ CreatePSO(material, rootSig, layout) 
        ├─ mPSOCache[key] = newPSO
        └─ return newPSO
   ↓
4. cmdList->SetPipelineState(pso)
   ↓
5. mesh.Draw(cmdList)
   ├─ mVertexBuffer.Bind()
   │  └─ IASetVertexBuffers()
   └─ mIndexBuffer.Bind()
   │  └─ IASetIndexBuffer()
   └─ DrawIndexedInstanced() or DrawInstanced()
```

---

## 9. 핵심 변경사항 요약

### Mesh 클래스에 추가된 것

```cpp
class Mesh
{
private:
    D3D12_INPUT_LAYOUT_DESC mInputLayout = {};
    
public:
    D3D12_INPUT_LAYOUT_DESC GetInputLayout() const
    {
        return mInputLayout;
    }
};
```

### Initialize()에서 설정

```cpp
bool Mesh::Initialize(...)
{
    // Vertex/Index Buffer 초기화...
    
    // Input Layout 저장
    mInputLayout = BasicVertex::GetInputLayout();
    
    return true;
}
```

### 장점

1. **명확한 소유권**: Mesh가 자신의 Input Layout을 직접 소유
2. **빠른 접근**: GetInputLayout() 호출 시 계산 불필요
3. **확장성**: 향후 다양한 Vertex 타입 지원 용이
4. **타입 안전성**: Initialize 시점에 Layout 고정

### 단점

- **메모리 증가**: Mesh당 16 bytes 추가
  - 하지만 무시할 수 있는 수준 (0.01%)

---

## 결론

모든 클래스가 명확한 소유권과 책임을 가지며, SOLID 원칙을 준수하는 설계

**주요 특징**:
- DX12PipelineStateCache: PSO 캐싱 및 생성
- Material: 렌더링 상태 관리
- Mesh: Vertex/Index Buffer + Input Layout 소유
- 명확한 의존성 방향
- 확장 가능한 구조