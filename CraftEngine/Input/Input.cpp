#include "Input.h"
#include <cassert>
#include <Windows.h>
#include <iostream>
#include <Render/Renderer.h>
#include <cassert>

namespace Craft
{
	// static 변수 초기화
	Input* Input::instance = nullptr;

	Input::Input()
	{
		// 시작할 때 instance 값은 null이어야 함
		assert(!instance && "instance should be null here");
		instance = this;

		// 마우스 위치를 가져오기 위해 콘솔 모드 설정
		buffer = GetStdHandle(STD_INPUT_HANDLE);
		assert(buffer != INVALID_HANDLE_VALUE && "Invalid handle value");

		DWORD mode;

		BOOL result = GetConsoleMode(buffer, &mode);
		assert(result);

		mode |= ENABLE_MOUSE_INPUT;
		mode |= ENABLE_EXTENDED_FLAGS;
		mode &= ~ENABLE_QUICK_EDIT_MODE;

		result = SetConsoleMode(buffer, mode); // 마우스 입력 활성화
		assert(result);

		consoleWindow = GetForegroundWindow();

		std::cout << "consoleWindow: " << consoleWindow << "\n";

		RECT rect;
		result = GetWindowRect(consoleWindow, &rect);

		std::cout << "GetWindowRect result = " << result << "\n";
		std::cout << "rect = "
			<< rect.left << ", "
			<< rect.top << ", "
			<< rect.right << ", "
			<< rect.bottom << "\n";
	}

	bool Input::GetKeyDown(int keyCode) const
	{
		return !keyStates[keyCode].wasKeyDown 
			&& keyStates[keyCode].isKeyDown;
	}

	bool Input::GetKeyUp(int keyCode) const
	{
		return keyStates[keyCode].wasKeyDown
			&& !keyStates[keyCode].isKeyDown;
	}

	bool Input::GetKey(int keyCode) const
	{
		return keyStates[keyCode].isKeyDown;
	}

	Input& Input::Get()
	{
		// 여기에서 instance는 null이면 안됨
		assert(instance && "instance should not be null here");
		return *instance;
	}

	void Input::ProcessInput()
	{
		// 현재 프레임에 키 입력이 발생했는지 확인
		for (int ix = 0; ix < keyCount; ++ix)
		{
			// 키 눌림 여부 저장
			// 0x8000은 최상위 비트
			keyStates[ix].isKeyDown = ((GetAsyncKeyState(ix) & 0x8000) != 0);
		}

		Renderer::RenderMode curRenderMode = Renderer::Get().GetRenderMode();


		if (curRenderMode != Renderer::RenderMode::ThreeDimension)
		{
			DWORD eventCount = 0;
			GetNumberOfConsoleInputEvents(buffer, &eventCount);

			while (eventCount > 0)
			{
				// 현재 프레임에 마우스 입력이나 움직임이 있으면 위치 반환
				INPUT_RECORD record;
				DWORD count = 0;

				BOOL result = ReadConsoleInput(buffer, &record, 1, &count);

				if (!result)
					break;

				if (record.EventType == MOUSE_EVENT)
				{
					prevMousePosition = mousePosition;
					const auto& mouseEvent = record.Event.MouseEvent;

					mousePosition = Vector2(
						static_cast<float>(mouseEvent.dwMousePosition.X),
						static_cast<float>(mouseEvent.dwMousePosition.Y)
					);

					mousePosition += Renderer::Get().GetRenderStartPosition();
				}

				--eventCount;
			}
		}

		// 3차원일 때 마우스 회전 처리
		if (consoleWindow != nullptr &&
			curRenderMode == Renderer::RenderMode::ThreeDimension)
		{
			// 1. 현재 화면에서 콘솔 창이 위치한 영역(Rect)을 가져옵니다.
			RECT rect;
			GetClientRect(consoleWindow, &rect);

			// 2. 콘솔 창의 정중앙 픽셀 좌표 계산
			POINT center;
			center.x = (rect.left + rect.right) / 2;
			center.y = (rect.top + rect.bottom) / 2;

			// 3. 현재 실제 마우스 커서의 화면 위치 가져오기
			POINT currentPos;
			GetCursorPos(&currentPos);

			// 4. 중앙 좌표와 현재 마우스 위치의 차이(Delta)를 계산
			mouseDelta.x = static_cast<float>(currentPos.x - center.x);
			mouseDelta.y = static_cast<float>(currentPos.y - center.y);

			// 5. 계산이 끝났으면 마우스 커서를 다시 콘솔 창 정중앙으로 강제 이동!
			SetCursorPos(center.x, center.y);
		}
		else
		{
			// 고정 모드가 아닐 때는 델타값을 0으로 유지
			mouseDelta = Vector2(0.f, 0.f);
		}
	}

	void Input::SavePreviousStates()
	{
		// 이전 프레임 입력 값 저장
		for (KeyState& state : keyStates)
		{
			// 현재 프레임 입력 값을 이전 프레임 값으로 저장
			state.wasKeyDown = state.isKeyDown;
		}
	}
}