#include "ScreenBuffer.h"
#include <cassert>
#include <iostream>


namespace Craft
{
	ScreenBuffer::ScreenBuffer(const Vector2& worldSize, const Vector2& screenSize)
		: worldSize(worldSize), screenSize(screenSize)
	{
		// 콘솔 버퍼 생성
		buffer = CreateConsoleScreenBuffer(
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			CONSOLE_TEXTMODE_BUFFER,
			nullptr
		);

		// 값 확인
		assert(buffer != INVALID_HANDLE_VALUE);

		BOOL result = SetConsoleOutputCP(437);
		assert(result == TRUE);

		// 화면 창 크기 설정
		SMALL_RECT rect = {};
		rect.Top = 0;
		rect.Left = 0;
		rect.Right = static_cast<short>(screenSize.x - 1);
		rect.Bottom = static_cast<short>(screenSize.y - 1);
		result = SetConsoleWindowInfo(buffer, TRUE, &rect);

		// 결과 확인
		assert(result == TRUE);

		// 화면 버퍼 크기 설정
		result = SetConsoleScreenBufferSize(buffer, screenSize);
		assert(result == TRUE);

		// 직접 만든 콘솔의 커서 끄기
		CONSOLE_CURSOR_INFO info;
		result = GetConsoleCursorInfo(buffer, &info);
		assert(result == TRUE);

		// 커서 안보이게 설정
		info.bVisible = FALSE;
		result = SetConsoleCursorInfo(buffer, &info);
		assert(result == TRUE);
	}

	ScreenBuffer::~ScreenBuffer()
	{
		// 콘솔 닫기
		if (buffer)
		{
			CloseHandle(buffer);
		}
	}

	void ScreenBuffer::Clear() const
	{
		// 콘솔 전체를 지우는 함수
		// 공백 문자를 화면 전체에 한 번에 설정
		DWORD writtenCount = 0;

		BOOL result = FillConsoleOutputCharacterA(
			buffer,
			' ',
			screenSize.x * screenSize.y,
			Vector2::Zero,
			&writtenCount
		);

		assert(result == TRUE);
	}

	void ScreenBuffer::Draw(const CHAR_INFO* const charInfo) const
	{
		// charInfo는 2차원 배열(1차원 배열에 2차원 배열 정보를 기록)

		// 설정할 글자 영역
		SMALL_RECT rect = { 
			0,								// Left 
			0,								// Top
			static_cast<short>(screenSize.x - 1), // Right
			static_cast<short>(screenSize.y - 1)  // Bottom
		};

		// 콘솔에 CHAR_INFO 타입으로 글자 쓰는 함수
		BOOL result = WriteConsoleOutputA(
			buffer,
			charInfo,
			screenSize,
			Vector2::Zero,
			&rect
		);

		assert(result == TRUE);
	}
	void ScreenBuffer::SetFontSize(short width, short height)
	{
		CONSOLE_FONT_INFOEX fontInfo{};
		fontInfo.cbSize = sizeof(fontInfo);

		BOOL result = GetCurrentConsoleFontEx(
			buffer,
			FALSE,
			&fontInfo
		);

		assert(result == TRUE);

		std::cout << "Before Font : "
			<< fontInfo.dwFontSize.X << ", "
			<< fontInfo.dwFontSize.Y << '\n';

		fontInfo.dwFontSize.X = width;
		fontInfo.dwFontSize.Y = height;

		result = SetCurrentConsoleFontEx(
			buffer,
			FALSE,
			&fontInfo
		);

		std::cout << "SetFont Result : " << result << '\n';

		assert(result == TRUE);
	}
	void ScreenBuffer::Resize(const Vector2& newScreenSize)
	{
		screenSize = newScreenSize;

		// 1. 현재 폰트 기준으로 이 모니터에서 허용되는 가장 큰 콘솔 창 크기 가져오기
		COORD maxSize = GetLargestConsoleWindowSize(buffer);

		// 2. 목표 크기가 최대 크기를 넘지 않도록 보정 (이게 없으면 OS가 확장을 거부함)
		short windowWidth = min(static_cast<short>(newScreenSize.x), maxSize.X);
		short windowHeight = min(static_cast<short>(newScreenSize.y), maxSize.Y);

		COORD targetBufferSize = {
			static_cast<short>(newScreenSize.x),
			static_cast<short>(newScreenSize.y)
		};

		SMALL_RECT targetWindowSize = {
			0, 0,
			static_cast<short>(windowWidth - 1),
			static_cast<short>(windowHeight - 1)
		};

		// 3. 1x1로 먼저 찌그러뜨리기
		SMALL_RECT minWindow = { 0, 0, 1, 1 };
		SetConsoleWindowInfo(buffer, TRUE, &minWindow);

		// 4. 버퍼 늘리기 (버퍼는 모니터 크기와 상관없이 메모리상 커질 수 있음)
		SetConsoleScreenBufferSize(buffer, targetBufferSize);

		// 5. 창 크기를 안전하게 보정된 크기로 늘리기
		BOOL result = SetConsoleWindowInfo(buffer, TRUE, &targetWindowSize);

		if (result == FALSE)
		{
			std::cout << "\n[Resize Failed] Max Allowed: " << maxSize.X << "x" << maxSize.Y << '\n';
		}
	}
}