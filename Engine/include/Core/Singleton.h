#pragma once

#include "Core/Assert.h"

#include <utility>

namespace Core
{
	//=========================================================================
	// Explicit Lifecycle Singleton
	//=========================================================================

	/**
	 * @brief 명시적 수명주기 관리가 가능한 싱글톤 베이스 클래스
	 *
	 * Create()/Destroy()를 통해 생성/파괴 시점을 명시적으로 제어할 수 있습니다.
	 * 시스템 간 초기화 순서가 중요한 엔진 코어 시스템에 적합합니다.
	 *
	 * @tparam T 싱글톤으로 만들 파생 클래스 타입
	 *
	 * @note 스레드 안전하지 않음. 엔진 초기화는 단일 스레드에서 수행한다고 가정
	 *
	 * 사용 예:
	 * @code
	 * class Renderer : public Singleton<Renderer>
	 * {
	 *     friend class Singleton<Renderer>;
	 *
	 * private:
	 *     Renderer(Device* device) : mDevice(device) {}
	 *     ~Renderer() = default;
	 *
	 *     Device* mDevice;
	 * };
	 *
	 * // 엔진 초기화 (순서 제어)
	 * Renderer::Create(device);
	 * Renderer::Get().Draw();
	 * Renderer::Destroy();
	 * @endcode
	 */
	template <typename T>
	class Singleton
	{
	protected:
		Singleton() = default;
		virtual ~Singleton() = default;

	public:
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton&) = delete;

	protected:
		static T* sInstance;

	public:
		/**
		 * @brief 싱글톤 인스턴스를 생성
		 * @tparam Args 생성자에 전달할 인자 타입들
		 * @param args 생성자 인자들 (완벽 전달)
		 * @warning 이미 생성된 경우 Assert 발생
		 */
		template <typename... Args>
		static void Create(Args&&... args)
		{
			CORE_ASSERT(sInstance == nullptr, "Singleton already created!");
			sInstance = new T(std::forward<Args>(args)...);
		}

		/**
		 * @brief 싱글톤 인스턴스를 파괴
		 * @note 여러 번 호출해도 안전 (멱등성 보장)
		 */
		static void Destroy()
		{
			delete sInstance;
			sInstance = nullptr;
		}

		/**
		 * @brief 싱글톤 인스턴스 참조를 반환
		 * @return 싱글톤 인스턴스 참조
		 * @warning 생성 전 호출 시 Assert 발생
		 */
		static T& GetInstance()
		{
			CORE_ASSERT(sInstance != nullptr, "Singleton not initialized!");
			return *sInstance;
		}

		static T* GetPtr() { return sInstance; }
		static bool IsValid() { return sInstance != nullptr; }
	};

	template <typename T>
	T* Singleton<T>::sInstance = nullptr;

	//=========================================================================
	// Meyer's Singleton (Lazy Initialization)
	//=========================================================================

	/**
	 * @brief 지연 초기화 싱글톤 베이스 클래스 (Meyer's Singleton)
	 *
	 * 첫 호출 시 자동 생성되고 프로그램 종료 시 자동 파괴됩니다.
	 * 다른 시스템에 의존하지 않는 유틸리티 시스템에 적합합니다.
	 *
	 * @tparam T 싱글톤으로 만들 파생 클래스 타입
	 *
	 * @note C++11 이후 스레드 안전성 보장 (magic statics)
	 *
	 * 사용 예:
	 * @code
	 * class Logger : public LazySingleton<Logger>
	 * {
	 *     friend class LazySingleton<Logger>;
	 *
	 * private:
	 *     Logger() = default;
	 *     ~Logger() = default;
	 * };
	 *
	 * // 첫 호출 시 자동 생성
	 * Logger::Get().Log("Hello");
	 * // 프로그램 종료 시 자동 파괴
	 * @endcode
	 */
	template <typename T>
	class LazySingleton
	{
	protected:
		LazySingleton() = default;
		virtual ~LazySingleton() = default;

	public:
		LazySingleton(const LazySingleton&) = delete;
		LazySingleton& operator=(const LazySingleton&) = delete;

	public:
		/**
		 * @brief 싱글톤 인스턴스 참조를 반환
		 *
		 * 첫 호출 시 인스턴스가 생성됩니다 (Lazy Initialization).
		 * C++11 이후 스레드 안전성이 보장됩니다.
		 *
		 * @return 싱글톤 인스턴스 참조
		 */
		static T& GetInstance()
		{
			static T sInstance;
			return sInstance;
		}
	};

} // namespace Core
