#pragma once

#include <Core/Core.h>
#include <Math/Vector2.h>

namespace Craft
{
	class CRAFT_API Input
	{
		// Engine 클래스 friend 선언
		friend class Engine;

		// 키 입력 상태를 저장하기 위한 구조체
		struct KeyState
		{
			// 현재 프레임에 키가 눌렸는지
			bool isKeyDown = false;

			// 이전 프레임에 키가 눌렸는지
			bool wasKeyDown = false;
		};

	public:
		Input();
		~Input() = default;

		// 키 눌림/해제 여부 확인
		// 이전 프레임에 안눌렀다가 이번 프레임에 눌리면 true 반환
		bool GetKeyDown(int keyCode) const;

		// 이전 프레임에 눌렀다가 이번 프레임에 안눌리면 true 반환
		bool GetKeyUp(int keyCode) const;

		// 현재 프레임에 입력이 눌리면 반복해서 true를 반환
		bool GetKey(int keyCode) const;

		// 마우스 위치 Getter
		inline Vector2 GetMousePosition() const { return mousePosition; }

		// 이전 마우스 위치 Getter
		inline Vector2 GetPrevMousePosition() const { return prevMousePosition; }

		// 마우스 델타 값 가져오기
		inline Vector2 GetMouseDelta() const { return mouseDelta; }

		// 마우스 고정 모드 켜기/끄기
		inline void SetCursorLock(bool lock) { isCursorLocked = lock; }

		// 외부에서 접근이 가능하도록
		static Input& Get();

	private:
		// 현재 프레임에 특정 키 입력이 발생했는지를 처리
		void ProcessInput();

		// 이전 프레임의 키 눌림 상태를 저장
		void SavePreviousStates();

	private:
		// 가상 키의 수(=처리할 키의 수)
		const int keyCount = 256;

		// 키 상태를 관리할 배열
		KeyState keyStates[256] = {};

		// 전역 접근이 가능하도록 변수 추가
		static Input* instance;

		// 마우스 위치 저장 변수
		Vector2 mousePosition = Vector2::Up;

		// 마우스 이전 위치 저장 변수
		Vector2 prevMousePosition = Vector2::Up;

		// 화면 버퍼 핸들
		HANDLE buffer = nullptr;

		HWND consoleWindow = nullptr; // 콘솔 창 핸들
		bool isCursorLocked = true;  // 마우스 중앙 고정 모드 플래그
		Vector2 mouseDelta;           // 마우스가 중앙에서 얼마나 이동했는지 저장
	};
}
