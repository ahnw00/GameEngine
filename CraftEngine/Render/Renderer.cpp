#include "Renderer.h"
#include "ScreenBuffer.h"
#include <cassert>
#include <iostream>
#include <Windows.h>

namespace Craft
{
	//--------------------Frame------------------------//
	Renderer::Frame::Frame(int bufferCount)
	{
		// 2차원 배열 생성
		charInfoArray = std::make_unique<CHAR_INFO[]>(bufferCount);
		sortingOrderArray = std::make_unique<int[]>(bufferCount);
		actorArray = std::make_unique<std::vector<Actor*>[]>(bufferCount);
		sightArray = std::make_unique<bool[]>(bufferCount);
	}

	Renderer::Frame::~Frame()
	{}

	// 프레임 초기화 함수
	void Renderer::Frame::Clear(const Vector2& screenSize)
	{
		assert(sightArray != nullptr);

		// 이중 루프를 순회하면서 값 초기화
		const int width = screenSize.x;
		const int height = screenSize.y;

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				// 1차원 배열을 2차원 배열로 사용할 때
				// 필요한 인덱스 좌표 변환
				const int index = (y * width) + x;

				// 글자 항목 초기화
				CHAR_INFO& info = charInfoArray[index];
				// 빈문자 설정 - 기존의 설정된 값 지우기
				info.Char.AsciiChar = ' ';
				// 색상 표기 안함
				info.Attributes = 0;

				// 그리기 순서 배열 항목 초기화
				sortingOrderArray[index] = -1;

				// 해당 칸에 올라가 있던 Actor 정보 제거
				actorArray[index].clear();

				sightArray[index] = false;
			}
		}
	}
	//--------------------Frame------------------------//



	// static 변수 초기화
	Renderer* Renderer::instance = nullptr;

	Renderer::Renderer(const Vector2& worldSize, const Vector2& screenSize)
		: worldSize(worldSize), screenSize(screenSize)
	{
		assert(!instance && "instance should be null");
		instance = this;

		//// 콘솔 커서 안보이게 설정
		//CONSOLE_CURSOR_INFO info;
		//GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		//// 보이기 옵션을 false로
		//info.bVisible = FALSE; // false를 Windows 스타일로 작성해준거
		//SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		// 프레임 객체 생성
		const int bufferCount = screenSize.x * screenSize.y;
		frame = std::make_unique<Frame>(bufferCount);
		
		// 생성 후 프레임 지우기
		frame->Clear(screenSize);

		// 이중 버퍼 구현을 위한 콘솔 버퍼 생성 및 초기화
		screenBufferArray[0] = std::make_unique<ScreenBuffer>(worldSize, screenSize);
		screenBufferArray[0]->Clear();

		screenBufferArray[1] = std::make_unique<ScreenBuffer>(worldSize, screenSize);
		screenBufferArray[1]->Clear();

		// 화면에 0번 콘솔 버퍼 활성화
		SetConsoleActiveScreenBuffer(screenBufferArray[0]->GetBuffer());
	}

	Renderer::~Renderer()
	{
		instance = nullptr;

		//// 콘솔 커서 다시 보이게 설정(복구)
		//CONSOLE_CURSOR_INFO info;
		//GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		//// 보이기 옵션을 true로
		//info.bVisible = TRUE; // true를 Windows 스타일로 작성해준거
		//SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		// 콘솔 창 원래대로 복구
		SetConsoleActiveScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE));
	}

	void Renderer::Submit(
		Actor* actor,
		const std::vector<std::string>& image, 
		const Vector2& position, 
		Color color, 
		int sortingOrder)
	{
		// 렌더 명령 생성 및 값 설정
		RenderCommand command;

		command.actor = actor;
		command.image = image;
		command.position = position;
		command.color = color;
		command.sortingOrder = sortingOrder;

		// 렌더 큐에 명령 추가
		// 큐에 모아서 한번에 처리하는데 
		// 각각 따로 처리하면 최적화를 못해줘(계산 안해도 되는건 빼고 뭐 그런)
		renderQueue.emplace_back(command);
	}

	void Renderer::Draw()
	{
		// 화면(이미지/프레임) 지우기
		Clear();

		// 프레임 그리기
		DrawRenderQueue();

		// 시야 밖 처리
		DrawSight();

		// 화면(이미지/프레임) 표시
		Present();
	}

	Renderer& Renderer::Get()
	{
		assert(instance && "instance should not be null");
		return *instance;
	}

	void Renderer::SetSight(const Vector2& position)
	{
		const int worldX = static_cast<int>(position.x);
		const int worldY = static_cast<int>(position.y);

		if (worldX < 0 || worldX >= worldSize.x ||
			worldY < 0 || worldY >= worldSize.y)
		{
			return;
		}

		// 월드 좌표 -> 화면 좌표
		const int screenX =
			worldX - static_cast<int>(renderStartPosition.x);

		const int screenY =
			worldY - static_cast<int>(renderStartPosition.y);

		// 화면 밖이면 무시
		if (screenX < 0 || screenX >= screenSize.x ||
			screenY < 0 || screenY >= screenSize.y)
		{
			return;
		}

		const int index =
			screenY * static_cast<int>(screenSize.x) + screenX;

		frame->sightArray[index] = true;
	}

	void Renderer::BeginFrame()
	{
		Clear();
	}

	// 시야 안에 들어온 좌표들 처리
	void Renderer::DrawSight()
	{
		if (mode != RenderMode::PLAY)
			return;

		const int width =
			static_cast<int>(screenSize.x);

		const int height =
			static_cast<int>(screenSize.y);

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				const int index = y * width + x;

				if (frame->sightArray[index])
				{
					// 시야 안
					frame->charInfoArray[index].Attributes |=
						BACKGROUND_RED |
						BACKGROUND_GREEN |
						BACKGROUND_BLUE;
				}
				else
				{
					// 시야 밖
					WORD& attributes =
						frame->charInfoArray[index].Attributes;

					attributes &= ~(FOREGROUND_RED |
						FOREGROUND_GREEN |
						FOREGROUND_BLUE |
						FOREGROUND_INTENSITY);

					attributes |= FOREGROUND_INTENSITY;
				}
			}
		}
	}

	const std::vector<Actor*>& Renderer::GetActorsAt(const Vector2& position)
	{
		// static을 사용하는 이유는 다른 클래스에 보내주기 위해
		static const std::vector<Actor*> empty;

		const int worldX = static_cast<int>(position.x);
		const int worldY = static_cast<int>(position.y);

		if (worldX < 0 || worldX >= worldSize.x ||
			worldY < 0 || worldY >= worldSize.y)
		{
			return empty;
		}

		const int screenX =
			worldX - static_cast<int>(renderStartPosition.x);

		const int screenY =
			worldY - static_cast<int>(renderStartPosition.y);

		if (screenX < 0 || screenX >= screenSize.x ||
			screenY < 0 || screenY >= screenSize.y)
		{
			return empty;
		}

		const int index =
			screenY * static_cast<int>(screenSize.x) + screenX;

		return frame->actorArray[index];
	}

	void Renderer::Clear()
	{
		// 프레임 값 초기화
		frame->Clear(screenSize);

		// 콘솔 버퍼 초기화
		GetCurrentBuffer()->Clear();
	}

	// "***"
	// " * "
	// "***"

	void Renderer::DrawRenderQueue()
	{
		// 렌더 큐를 순회하면서 그리기 명령 실행
		for (const RenderCommand& command : renderQueue)
		{
			// 그릴 문자가 없으면 건너뛰기
			if (command.image.empty())
				continue;

			// 글자의 시작 위치
			const int startX = static_cast<int>(std::round(command.position.x));
			const int startY = static_cast<int>(std::round(command.position.y));

			const int height = static_cast<int>(command.image.size());

			for (int localY = 0; localY < height; ++localY)
			{
				const std::string& row = command.image[localY];

				const int width = static_cast<int>(row.length());

				for (int localX = 0; localX < width; ++localX)
				{
					const int worldX = startX + localX;
					const int worldY = startY + localY;

					// 화면 밖이면 건너뛰기
					if (worldX < 0 || worldX >= worldSize.x ||
						worldY < 0 || worldY >= worldSize.y)
						continue;

					// 빈 칸이면 그리지 않음
					if (row[localX] == ' ')
						continue;

					// 월드 좌표 -> 화면 좌표
					const int screenX =
						worldX - static_cast<int>(renderStartPosition.x);

					const int screenY =
						worldY - static_cast<int>(renderStartPosition.y);

					// 화면 밖이면 렌더링하지 않음
					if (screenX < 0 || screenX >= screenSize.x ||
						screenY < 0 || screenY >= screenSize.y)
					{
						continue;
					}

					const int index =
						screenY * static_cast<int>(screenSize.x) + screenX;

					// 이 칸에 Actor가 존재한다는 정보 저장
					if (command.actor)
					{
						frame->actorArray[index].emplace_back(command.actor);
					}

					// 정렬 순서 확인
					if (frame->sortingOrderArray[index] >
						command.sortingOrder)
					{
						continue;
					}

					// 문자 기록
					frame->charInfoArray[index].Char.AsciiChar =
						row[localX];

					// 색상 기록
					frame->charInfoArray[index].Attributes =
						static_cast<DWORD>(command.color);

					// 정렬 순서 기록
					frame->sortingOrderArray[index] =
						command.sortingOrder;
				}
			}
		}

		// 앞에서 설정한 2차원 배열을 콘솔에 그리기
		//GetCurrentBuffer()->Draw(frame->charInfoArray.get());

		// 렌더큐 비우기
		renderQueue.clear();

		// 콘솔 색상 초기화
		SetConsoleTextAttribute(
			GetCurrentBuffer()->GetBuffer(),
			static_cast<DWORD>(Color::White)
		);
	}

	void Renderer::Present()
	{
		
		GetCurrentBuffer()->Draw(frame->charInfoArray.get());

		// 현재 순번의 콘솔 버퍼를 활성화
		SetConsoleActiveScreenBuffer(GetCurrentBuffer()->GetBuffer());

		// 인덱스 업데이트
		// 마법의 공식 -> One Minus
		currentBufferIndex = 1 - currentBufferIndex;
	}

	const ScreenBuffer* const Renderer::GetCurrentBuffer() const
	{
		// const로 감싸서 원시 포인터로 값 변경 못하도록
		// unique_ptr<>&로 받아올 수 있는데 그러면 unique_ptr의 성격 때문에 불가
		// 스마트 포인터라서 get()을 이용해 원시 포인터 얻어내
		return screenBufferArray[currentBufferIndex].get(); 
	}
}