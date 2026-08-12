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

		DWORD eventCount = 0;
		GetNumberOfConsoleInputEvents(buffer, &eventCount);

		while(eventCount > 0)
		{
			// 현재 프레임에 마우스 입력이나 움직임이 있으면 위치 반환
			INPUT_RECORD record;
			DWORD count = 0;
			
			BOOL result = ReadConsoleInput(buffer, &record, 1, &count);
			
			if (!result)
				break;

			if (record.EventType == MOUSE_EVENT)
			{
				const auto& mouseEvent = record.Event.MouseEvent;

				mousePosition = Vector2(
					static_cast<float>(mouseEvent.dwMousePosition.X),
					static_cast<float>(mouseEvent.dwMousePosition.Y)
				);
			}
		
			--eventCount;
		}

		std::string temp =
			std::to_string(mousePosition.x) +
			", " +
			std::to_string(mousePosition.y);

		Renderer::Get().Submit(
			nullptr,
			{ temp },
			Vector2(20, 0)
		);
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