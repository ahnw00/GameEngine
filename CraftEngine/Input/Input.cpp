#include "Input.h"
#include <cassert>
#include <Windows.h>

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
		DWORD mode;
		GetConsoleMode(buffer, &mode);
		SetConsoleMode(buffer, mode | ENABLE_MOUSE_INPUT); // 마우스 입력 활성화
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

		// 현재 프레임에 마우스 입력이나 움직임이 있으면 위치 반환
		//INPUT_
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