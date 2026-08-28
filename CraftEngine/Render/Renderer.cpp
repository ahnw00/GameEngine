#include "Renderer.h"
#include "ScreenBuffer.h"
#include <cassert>
#include <iostream>
#include <Windows.h>
#include <Actor/Actor.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Level/Level.h>


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
		sightStateArray = std::make_unique<SightState[]>(bufferCount);
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

				sightStateArray[index] = SightState::None;
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
		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		//// 보이기 옵션을 false로
		info.bVisible = FALSE; // false를 Windows 스타일로 작성해준거
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

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

		// 콘솔 커서 다시 보이게 설정(복구)
		CONSOLE_CURSOR_INFO info;
		GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

		// 보이기 옵션을 true로
		info.bVisible = TRUE; // true를 Windows 스타일로 작성해준거
		SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);

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

	// 사용X(순서를 위해 Draw를 더 잘게 쪼갬)
	void Renderer::Draw()
	{
		// 화면(이미지/프레임) 지우기
		Clear();

		// 프레임 그리기
		DrawRenderQueue();

		// 시야 밖 처리
		DrawSight();

		// 마우스 커서 표시
		DrawMouseCursor();

		// 화면(이미지/프레임) 표시
		Present();
	}

	Renderer& Renderer::Get()
	{
		assert(instance && "instance should not be null");
		return *instance;
	}

	void Renderer::SetSight(const Vector2& position, const SightState& sightState)
	{
		Vector2 screenPosition = Vector2::Zero;

		if (WorldToScreenPosition(position, screenPosition))
		{
			const int index =
				screenPosition.y * static_cast<int>(screenSize.x) + screenPosition.x;

			frame->sightArray[index] = true;

			if (frame->sightStateArray[index] == SightState::Visible)
				return;

			frame->sightStateArray[index] = sightState;
		}
	}

	bool Renderer::WorldToScreenPosition(
		const Vector2& worldPosition, 
		Vector2& screenPosition) const
	{
		if (worldPosition.x < 0 || worldPosition.x >= worldSize.x ||
			worldPosition.y < 0 || worldPosition.y >= worldSize.y)
			return false;

		const int screenX =
			static_cast<int>(worldPosition.x) -
			static_cast<int>(renderStartPosition.x);
		const int screenY =
			static_cast<int>(worldPosition.y) -
			static_cast<int>(renderStartPosition.y);

		if (screenX < 0 || screenX >= screenSize.x ||
			screenY < 0 || screenY >= screenSize.y)
			return false;

		screenPosition = Vector2(screenX, screenY);
		return true;
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

		const int width = static_cast<int>(screenSize.x);
		const int height = static_cast<int>(screenSize.y);

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				const int index = y * width + x;

				bool ignoreSight = false;
				for (Actor* actor : frame->actorArray[index])
				{
					if (actor && actor->IgnoreSight())
					{
						ignoreSight = true;
						break;
					}
				}

				if (frame->sightArray[index])
				{
					frame->charInfoArray[index].Attributes |=
						BACKGROUND_RED |
						BACKGROUND_GREEN |
						BACKGROUND_BLUE;

					// 흰색 시야 안
					if (frame->sightStateArray[index] == SightState::Visible)
					{
						frame->charInfoArray[index].Attributes |=
							BACKGROUND_INTENSITY;

						continue;
					}

					if (ignoreSight)
						continue;

					// 회색 시야 안
					for (Actor* actor : frame->actorArray[index])
					{
						if (actor && !actor->IsVisibleOutsideSight())
						{
							frame->charInfoArray[index].Char.AsciiChar = ' ';
							break;
						}
					}

					continue;
				}

				// 시야 밖
				if (ignoreSight)
				{
					// 전경색은 그대로 유지
					// 배경색만 시야 밖 상태로 처리
					WORD& attributes = frame->charInfoArray[index].Attributes;

					attributes &= ~(BACKGROUND_RED |
						BACKGROUND_GREEN |
						BACKGROUND_BLUE |
						BACKGROUND_INTENSITY);

					continue;
				}

				bool visibleOutsideSight = false;

				// 이 칸에 실제로 렌더링된 Actor 검사
				for (Actor* actor : frame->actorArray[index])
				{
					if (actor &&
						actor->IsVisibleOutsideSight())
					{
						visibleOutsideSight = true;
						break;
					}
				}

				// 시야 밖에서도 보여야 하는 Actor
				if (visibleOutsideSight)
				{
					// 시야 밖
					WORD& attributes = frame->charInfoArray[index].Attributes;

					// 검은색으로 보여
					attributes &= ~(FOREGROUND_RED |
						FOREGROUND_GREEN |
						FOREGROUND_BLUE |
						FOREGROUND_INTENSITY);

					// 이거 하면 회색으로 보여
					attributes |= FOREGROUND_INTENSITY;
				}
				else
				{
					// 시야 밖에서는 문자 자체를 숨김
					//frame->charInfoArray[index].Char.AsciiChar = ' ';

					WORD& attributes = frame->charInfoArray[index].Attributes;

					// 검은색으로 보여
					attributes &= ~(FOREGROUND_RED |
						FOREGROUND_GREEN |
						FOREGROUND_BLUE |
						FOREGROUND_INTENSITY);
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

					int screenX, screenY;

					if (mode != RenderMode::PLAY)
					{
						screenX = worldX;
						screenY = worldY;
					}
					else
					{
						// 월드 좌표 -> 화면 좌표
						screenX = worldX - static_cast<int>(renderStartPosition.x);
						screenY = worldY - static_cast<int>(renderStartPosition.y);
					}

					// 화면 밖이면 렌더링하지 않음
					if (screenX < 0 || screenX >= screenSize.x ||
						screenY < 0 || screenY >= screenSize.y)
					{
						continue;
					}

					const int index = screenY * static_cast<int>(screenSize.x) + screenX;

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
					frame->charInfoArray[index].Char.AsciiChar = row[localX];

					// 색상 기록
					frame->charInfoArray[index].Attributes =
						static_cast<DWORD>(command.color);

					// 정렬 순서 기록
					frame->sortingOrderArray[index] = command.sortingOrder;
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

	//// 1. 플레이어의 현재 위치와 정면 벡터를 가져옵니다.
	//Vector2 playerPos = player->GetPosition();
	//Vector2 playerDir = player->GetForward();

	//// 2. Camera Plane(카메라 평면) 벡터 계산
	//// 정면 벡터(playerDir)와 수직(-y, x)이 되게 만듭니다.
	//// 0.66f를 곱하는 이유는 시야각(FOV)을 설정하기 위함입니다. (약 66도 시야각)
	//// 이 값을 키우면 광각 렌즈(FOV 넓어짐)가 되고, 줄이면 망원 렌즈(FOV 좁아짐)가 됩니다.
	//float fov = 0.66f;
	//Vector2 cameraPlane(-playerDir.y * fov, playerDir.x* fov);

	void Renderer::Draw3DView(
		const Vector2& playerPos, 
		const Vector2& playerDir, 
		const Vector2& cameraPlane, 
		const std::vector<std::string>& mapData)
	{
		const int width = static_cast<int>(screenSize.x);
		const int height = static_cast<int>(screenSize.y);

		// 화면의 가로 픽셀(x열) 수만큼 레이를 발사
		for (int x = 0; x < width; ++x)
		{
			// 현재 화면 x좌표를 -1.0 ~ 1.0 사이의 비율로 변환
			double cameraX = 2.0 * x / static_cast<double>(width) - 1.0;

			// 레이의 최종 방향 벡터 계산
			double rayDirX = playerDir.x + cameraPlane.x * cameraX;
			double rayDirY = playerDir.y + cameraPlane.y * cameraX;

			// 현재 레이가 위치한 맵의 정수 좌표
			int mapX = static_cast<int>(playerPos.x);
			int mapY = static_cast<int>(playerPos.y);

			// DDA 알고리즘
			double sideDistX, sideDistY;

			// deltaDist: 레이가 1칸 이동할 때 실제로 이동하는 총 거리
			// rayDir이 0일 때 0으로 나누는 에러 방지 위해 1e30 대입
			double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1.0 / rayDirX);
			double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1.0 / rayDirY);

			// 레이가 전진할 방향(-1 또는 1)
			int stepX, stepY;
			
			int hit = 0; // 벽에 부딪혔는지 여부(0: 안 부딪힘, 1: 부딪힘)
			int side = -1;    // 부딪힌 벽의 면(0: x축에 평행한 면, 1: y축에 평행한 면)

			// step과 초기 sideDist 세팅
			if (rayDirX < 0) // 왼쪽 방향으로 레이 발사
			{
				stepX = -1;
				sideDistX = (playerPos.x - mapX) * deltaDistX;
			}
			else // 오른쪽 방향으로 레이 발사
			{
				stepX = 1;
				sideDistX = (mapX + 1 - playerPos.x) * deltaDistX;
			}

			if (rayDirY < 0) // 위쪽 방향으로 레이 발사
			{
				stepY = -1;
				sideDistY = (playerPos.y - mapY) * deltaDistY;
			}
			else // 아래쪽 방향으로 레이 발사
			{
				stepY = 1;
				sideDistY = (mapY + 1 - playerPos.y) * deltaDistY;
			}

			// DDA 루프(벽에 부딪히거나 시야 거리 안까지 광선 1칸씩 전진)
			while (hit == 0)
			{
				// Todo: 시야 범위 정하기
				// 시야 범위 벗어나면 중단
				//if (min(sideDistX, sideDistY) >= 15.f)
				//	break;

				if (sideDistX < sideDistY)
				{
					sideDistX += deltaDistX; // x축에 평행하게 한 칸 이동
					mapX += stepX;
					side = 0;
				}
				else
				{
					sideDistY += deltaDistY; // y축에 평행하게 한 칸 이동
					mapY += stepY;
					side = 1;
				}

				if (mapX < 0 || mapX >= mapData[0].size() ||
					mapY < 0 || mapY >= mapData.size())
					break;

				if (mapData[mapY][mapX] == '#')
					hit = 1;
			}

			double perpWallDist; // 벽까지의 최종 수직 거리

			if (hit == 1)
			{
				// --- [ 1. 벽을 찾았을 때의 일반적인 그리기 로직 ] ---

				// 수직 거리 계산
				if (side == 0) perpWallDist = (mapX - playerPos.x + (1 - stepX) / 2) / rayDirX;
				else           perpWallDist = (mapY - playerPos.y + (1 - stepY) / 2) / rayDirY;

				if (perpWallDist <= 0.0) perpWallDist = 0.001;

				// Todo: 시야범위 변수화
				// (선택 사항) 만약 계산된 수직 거리가 sightLimit보다 멀다면 안 그려도 무방함
				//if (perpWallDist > 15.f) 
				//{
				//	// 거리가 너무 멀어서 안개(어둠) 속으로 사라짐
				//	for (int y = 0; y < height; ++y) 
				//	{
				//		frame->charInfoArray[y * width + x].Char.AsciiChar = ' ';
				//		frame->charInfoArray[y * width + x].Attributes = 0;
				//	}
				//	continue; // 다음 x열로 넘어감
				//}

				// 벽 높이 계산
				float lineHeight =
					static_cast<float>(height) / static_cast<float>(perpWallDist) * 2.5f;

				float drawStartF =
					static_cast<float>(height) * 0.5f - lineHeight * 0.5f;

				float drawEndF =
					static_cast<float>(height) * 0.5f + lineHeight * 0.5f;

				int drawStart = static_cast<int>(std::floor(drawStartF));
				int drawEnd = static_cast<int>(std::ceil(drawEndF));

				drawStart = max(drawStart, 0);
				drawEnd = min(drawEnd, height - 1);

				//int drawStart = -lineHeight / 2 + height / 2;
				//if (drawStart < 0) drawStart = 0;
				//int drawEnd = lineHeight / 2 + height / 2;
				//if (drawEnd >= height) drawEnd = height - 1;

				// 화면 프레임에 기록
				for (int y = 0; y < height; ++y)
				{
					const int index = y * width + x;
					if (y < drawStart) 
					{
						// 천장
						frame->charInfoArray[index].Char.AsciiChar = 176;
						frame->charInfoArray[index].Attributes = FOREGROUND_BLUE;
					}
					else if (y >= drawStart && y <= drawEnd) 
					{
						// 벽 그리기 (이전 답변의 거리별 색상 처리 로직 적용)
						//frame->charInfoArray[index].Char.AsciiChar = 219;
						//frame->charInfoArray[index].Attributes = FOREGROUND_GREEN;

						// 벽의 거리에 따라 음영 결정
						char shade = 178;

						//if (perpWallDist < 4.0)
						//{
						//	// 매우 가까운 벽
						//	shade = 219; // █
						//}
						//else if (perpWallDist < 7.0)
						//{
						//	// 가까운 벽
						//	shade = 178; // ▓
						//}
						//else if (perpWallDist < 10.0)
						//{
						//	// 중간 거리
						//	shade = 177; // ▒
						//}
						//else
						//{
						//	// 먼 벽
						//	shade = 176; // ░
						//}

						// side가 1이면 한 단계 어둡게
						if (side == 1)
						{
							shade = 176;
							//if (shade == 219)
							//	shade = 178;
							//else if (shade == 178)
							//	shade = 177;
							//else if (shade == 177)
							//	shade = 176;
						}

						frame->charInfoArray[index].Char.AsciiChar = shade;
						frame->charInfoArray[index].Attributes = FOREGROUND_GREEN;
					}
					else 
					{
						// 바닥
						frame->charInfoArray[index].Char.AsciiChar = 176;
						//frame->charInfoArray[index].Attributes |= FOREGROUND_INTENSITY;
					}
				}
			}
			else
			{
				// --- [ 2. 벽을 못 찾고 시야 한계(sightLimit)에서 끝났을 때 ] ---

				for (int y = 0; y < height; ++y)
				{
					const int index = y * width + x;

					// 시야 끝에는 아무것도 안 보이게 까맣게(혹은 바닥만) 처리

					// 예시: 벽 없이 위쪽 절반은 천장, 아래쪽 절반은 바닥으로 그리기
					if (y < height / 2) 
					{
						frame->charInfoArray[index].Char.AsciiChar = ' '; // 빈공간 (어둠)
						frame->charInfoArray[index].Attributes = 0;
					}
					else 
					{
						// 끝이 안 보이는 먼 바닥을 표현하고 싶다면 옅게 그림
						frame->charInfoArray[index].Char.AsciiChar = '.';
						frame->charInfoArray[index].Attributes = FOREGROUND_INTENSITY; // 어두운 색
					}
				}
			}
		}
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

	void Renderer::DrawMouseCursor()
	{
		Vector2 mousePosition = Input::Get().GetMousePosition();
		Vector2 screenPosition = mousePosition;

		if (!WorldToScreenPosition(mousePosition, screenPosition))
			return;

		const int x = static_cast<int>(screenPosition.x);
		const int y = static_cast<int>(screenPosition.y);

		const int index = y * static_cast<int>(screenSize.x) + x;

		frame->charInfoArray[index].Char.AsciiChar = '+';

		frame->charInfoArray[index].Attributes |= FOREGROUND_GREEN;
	}
}