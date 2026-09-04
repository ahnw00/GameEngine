#include "Renderer.h"
#include "ScreenBuffer.h"

#include <Actor/Actor.h>
#include <Input/Input.h>
#include <Engine/Engine.h>
#include <Level/Level.h>

#include <cassert>
#include <iostream>
#include <Windows.h>
#include <cfloat>
#include <chrono>


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
		depthBuffer = std::make_unique<float[]>(static_cast<int>(screenSize.x));
		
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
		const Vector2& worldPosition,  Vector2& screenPosition) const
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
		if (renderMode == RenderMode::MENU)
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

					if (renderMode == RenderMode::MENU)
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
		const std::vector<std::string>& mapData,
		const std::vector<std::shared_ptr<Actor>>& actorList)
	{
		const int width = static_cast<int>(screenSize.x);
		const int height = static_cast<int>(screenSize.y);

		const float sightRadius = 30.f;

		for (int x = 0; x < width; ++x)
			depthBuffer[x] = FLT_MAX;

		constexpr float INF = 1e30f;
		constexpr float EPSILON = 0.0001f;

		// 화면의 가로 픽셀(x열) 수만큼 레이를 발사
		for (int x = 0; x < width; ++x)
		{
			// 현재 화면 x좌표를 -1.0 ~ 1.0 사이의 비율로 변환
			float cameraX = 2.0 * x / static_cast<float>(width) - 1.0;

			// 레이의 최종 방향 벡터 계산
			float rayDirX = playerDir.x + cameraPlane.x * cameraX;
			float rayDirY = playerDir.y + cameraPlane.y * cameraX;

			// 현재 레이가 위치한 맵의 정수 좌표
			int mapX = static_cast<int>(playerPos.x);
			int mapY = static_cast<int>(playerPos.y);

			// DDA 알고리즘
			float sideDistX, sideDistY;

			// deltaDist: 레이가 1칸 이동할 때 실제로 이동하는 총 거리
			// rayDir이 0일 때 0으로 나누는 에러 방지 위해 1e30 대입
			float deltaDistX = (std::abs(rayDirX) < EPSILON) ? INF : std::abs(1.0 / rayDirX);
			float deltaDistY = (std::abs(rayDirY) < EPSILON) ? INF : std::abs(1.0 / rayDirY);

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
			//while (hit == 0)
			//{
			//	if (sideDistX <= sideDistY + EPSILON)
			//	{
			//		sideDistX += deltaDistX; // x축에 평행하게 한 칸 이동
			//		mapX += stepX;
			//		side = 0;
			//	}
			//	else
			//	{
			//		sideDistY += deltaDistY; // y축에 평행하게 한 칸 이동
			//		mapY += stepY;
			//		side = 1;
			//	}

			//	if (mapX < 0 || mapX >= mapData[0].size() ||
			//		mapY < 0 || mapY >= mapData.size())
			//		break;

			//	// 시야 범위 벗어나면 중단
			//	if (min(sideDistX, sideDistY) >= sightRadius)
			//		break;

			//	if (mapData[mapY][mapX] == '#')
			//		hit = 1;
			//}

			while (!hit)
			{
				//--------------------------------------------------//
				// X / Y 경계까지의 거리가 거의 동일하다면
				// 레이가 셀 코너를 통과하는 상황
				//--------------------------------------------------//

				float difference = std::abs(sideDistX - sideDistY);

				if (difference <= EPSILON)
				{
					// 현재 코너까지의 거리
					float cornerDist = sideDistX;

					//------------------------------------------------//
					// X 방향으로 들어가는 셀
					//------------------------------------------------//

					int nextMapX = mapX + stepX;
					int nextMapY = mapY;

					if (nextMapX < 0 || nextMapX >= static_cast<int>(mapData[0].size()) ||
						nextMapY < 0 || nextMapY >= static_cast<int>(mapData.size()))
					{
						break;
					}

					//------------------------------------------------//
					// Y 방향으로 들어가는 셀
					//------------------------------------------------//

					int diagonalMapX = mapX;
					int diagonalMapY = mapY + stepY;

					if (diagonalMapX < 0 || diagonalMapX >= static_cast<int>(mapData[0].size()) ||
						diagonalMapY < 0 || diagonalMapY >= static_cast<int>(mapData.size()))
					{
						break;
					}

					//------------------------------------------------//
					// 코너 양쪽에 벽이 있는지 확인
					//------------------------------------------------//

					bool hitX = (mapData[nextMapY][nextMapX] == '#');
					bool hitY = (mapData[diagonalMapY][diagonalMapX] == '#');

					//------------------------------------------------//
					// 둘 중 하나라도 벽이면 충돌
					//------------------------------------------------//

					if (hitX || hitY)
					{
						hit = 1;

						// 어느 면을 사용할지 결정
						if (hitX && hitY)
						{
							// 양쪽 모두 벽이면 실제 이동 방향 기준
							// 한쪽을 선택
							if (rayDirX * rayDirX > rayDirY * rayDirY)
								side = 0;
							else
								side = 1;
						}
						else if (hitX)
						{
							side = 0;
						}
						else
						{
							side = 1;
						}

						//------------------------------------------------//
						// 코너까지의 거리를 유지
						//------------------------------------------------//

						sideDistX = cornerDist;
						sideDistY = cornerDist;

						break;
					}

					//------------------------------------------------//
					// 둘 다 벽이 아니라면 대각선 셀로 이동
					//------------------------------------------------//

					mapX += stepX;
					mapY += stepY;

					sideDistX += deltaDistX;
					sideDistY += deltaDistY;

					side = (rayDirX * rayDirX > rayDirY * rayDirY) ? 0 : 1;

					//------------------------------------------------//
					// 맵 범위 검사
					//------------------------------------------------//

					if (mapX < 0 || mapX >= static_cast<int>(mapData[0].size()) ||
						mapY < 0 || mapY >= static_cast<int>(mapData.size()))
					{
						break;
					}

					//------------------------------------------------//
					// 시야 거리 검사
					//------------------------------------------------//

					if (cornerDist > sightRadius + EPSILON)
						break;
				}
				else if (sideDistX < sideDistY)
				{
					//------------------------------------------------//
					// X 방향 셀 경계 통과
					//------------------------------------------------//

					float currentDist = sideDistX;

					sideDistX += deltaDistX;
					mapX += stepX;

					side = 0;

					//------------------------------------------------//
					// 맵 범위
					//------------------------------------------------//

					if (mapX < 0 || mapX >= static_cast<int>(mapData[0].size()) ||
						mapY < 0 || mapY >= static_cast<int>(mapData.size()))
					{
						break;
					}

					//------------------------------------------------//
					// 시야 거리
					//------------------------------------------------//

					if (currentDist > sightRadius + EPSILON)
						break;

					//------------------------------------------------//
					// 벽 검사
					//------------------------------------------------//

					if (mapData[mapY][mapX] == '#')
					{
						hit = 1;
						break;
					}
				}
				else
				{
					//------------------------------------------------//
					// Y 방향 셀 경계 통과
					//------------------------------------------------//

					float currentDist = sideDistY;

					sideDistY += deltaDistY;
					mapY += stepY;

					side = 1;

					//------------------------------------------------//
					// 맵 범위
					//------------------------------------------------//

					if (mapX < 0 || mapX >= static_cast<int>(mapData[0].size()) ||
						mapY < 0 || mapY >= static_cast<int>(mapData.size()))
					{
						break;
					}

					//------------------------------------------------//
					// 시야 거리
					//------------------------------------------------//

					if (currentDist > sightRadius + EPSILON)
						break;

					//------------------------------------------------//
					// 벽 검사
					//------------------------------------------------//

					if (mapData[mapY][mapX] == '#')
					{
						hit = 1;
						break;
					}
				}
			}
			
			float perpWallDist = FLT_MAX; // 기본값을 무한대로 (벽에 안 부딪혔을 때 대비)
			char wallShade = ' ';

			if (hit == 1)
			{
				// 수직 거리 계산
				if (side == 0) perpWallDist = (mapX - playerPos.x + (1 - stepX) / 2) / rayDirX;
				else           perpWallDist = (mapY - playerPos.y + (1 - stepY) / 2) / rayDirY;

				if (perpWallDist <= 0.0f) perpWallDist = 0.001f;
				wallShade = (side == 1) ? 178 : 219;
			}

			depthBuffer[x] = static_cast<float>(perpWallDist);

			// 벽이 화면에 그려질 시작과 끝 높이 계산
			int wallDrawStart = 0;
			int wallDrawEnd = -1;

			// DEBUGGING 시야처리 변수
			float wallMagnification = 16.0f;

			if (hit == 1)
			{
				float wallLineHeight = static_cast<float>(height) / perpWallDist * wallMagnification;

				wallDrawStart = static_cast<int>(std::floor(height * 0.5f - wallLineHeight * 0.5f));
				wallDrawEnd = static_cast<int>(std::ceil(height * 0.5f + wallLineHeight * 0.5f));

				wallDrawStart = max(wallDrawStart, 0);
				wallDrawEnd = min(wallDrawEnd, height - 1);
			}

			float rayLength = std::sqrt(rayDirX * rayDirX + rayDirY * rayDirY);

			// 수직 거리(perpDist)에 레이 길이를 곱하면 플레이어로부터의 실제 직선 거리(Euclidean Dist)가 됩니다.
			float trueWallDist = perpWallDist * rayLength;

			for (int y = 0; y < height; ++y)
			{
				const int index = y * width + x;

				// 1. 벽 영역 렌더링
				if (hit == 1 && y >= wallDrawStart && y <= wallDrawEnd)
				{
					// 실제 직선 거리(trueWallDist)로 비교하여 벽도 둥근 시야에 맞게 가려줍니다.
					if (trueWallDist <= sightRadius)
					{
						frame->charInfoArray[index].Char.AsciiChar = wallShade;
						frame->charInfoArray[index].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
					}
					else
					{
						// 시야 밖 벽 (어둠)
						frame->charInfoArray[index].Char.AsciiChar = ' ';
						frame->charInfoArray[index].Attributes = 0;
					}
				}
				// 2. 천장 영역 (y가 화면 절반 위쪽이면서 벽이 아닌 부분)
				else if (y < height / 2)
				{
					frame->charInfoArray[index].Char.AsciiChar = ' ';
					frame->charInfoArray[index].Attributes = 0;
				}
				// 3. 바닥 영역 (y가 화면 절반 아래쪽이면서 벽이 아닌 부분)
				else
				{
					float dy = static_cast<float>(y) - static_cast<float>(height) * 0.5f;
					if (dy < 0.001f) dy = 0.001f;

					// 보정 값
					float correctionValue = wallMagnification / 2.f;
					// 바닥의 수직 거리 계산
					float perpFloorDist = (static_cast<float>(height) * correctionValue) / dy;

					// 바닥 역시 수직 거리를 실제 3D 직선 거리로 변환합니다.
					float trueFloorDist = perpFloorDist * rayLength;

					if (trueFloorDist <= sightRadius)
					{
						// 시야 안의 바닥
						frame->charInfoArray[index].Char.AsciiChar = 177;
						frame->charInfoArray[index].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
					}
					else
					{
						// 시야 반경 밖의 바닥 (둥근 어둠)
						frame->charInfoArray[index].Char.AsciiChar = ' ';
						frame->charInfoArray[index].Attributes = 0;
					}
				}
			}

		}

		//auto start = std::chrono::high_resolution_clock::now();
		for (const auto& actor : actorList)
		{
			if (!actor)
				continue;

			//Render3DData& renderData = actor->GetRender3DData();
			if (!actor->CheckRender3DData())
				continue;

			float distance = (playerPos - actor->GetCenterPosition()).size();
			if (distance > sightRadius)
				continue;
			
			DrawActor3D(
				actor.get(),
				actor->GetRender3DData(),
				playerPos,
				playerDir,
				cameraPlane
			);
		}
		//auto end = std::chrono::high_resolution_clock::now();
		//float ms = std::chrono::duration<float, std::milli>(end - start).count();
		//std::cout << "DrawActor3D: " << ms << "ms\n";
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

	void Renderer::DrawActor3D(
		Actor* actor,
		const Render3DData& renderData,
		const Vector2& playerPos, 
		const Vector2& playerDir, 
		const Vector2& cameraPlane)
	{
		if (!actor) return;

		const int width = static_cast<int>(screenSize.x);
		const int height = static_cast<int>(screenSize.y);

		// 플레이어 기준 다른 액터의 상대적 위치
		Vector2 relative = actor->GetCenterPosition() - playerPos;
		// Camera Space 변환
		float determinant = cameraPlane.x * playerDir.y - playerDir.x * cameraPlane.y;

		if (std::abs(determinant) < 0.0001f)
			return;

		float inverseDeterminant = 1.f / determinant;

		float transformX = inverseDeterminant * (playerDir.y * relative.x - playerDir.x * relative.y);
		float transformY = inverseDeterminant * (-cameraPlane.y * relative.x + cameraPlane.x * relative.y);

		// 플레이어 뒤에 있는 Actor
		if (transformY <= 0.0f)
			return;

		int screenX = static_cast<int>((width / 2.f) * (1.f + transformX / transformY));
		if (screenX < 0 || screenX >= width)
			return;

		// Sprite 크기 계산
		//int spriteWidth = static_cast<int>(height * renderData.width / transformY);
		//int spriteHeight = static_cast<int>(height * renderData.height / transformY);

		float projectionScale = static_cast<float>(height) / transformY;
		int spriteWidth = static_cast<int>(renderData.width * projectionScale);
		int spriteHeight = static_cast<int>(renderData.height * projectionScale);

		if (spriteHeight <= 0 || spriteWidth <= 0)
			return;

		int drawStartX = screenX - spriteWidth / 2;
		int drawEndX = screenX + spriteWidth / 2;
		//int drawStartY = height / 2 - spriteHeight / 2;
		//int drawEndY = height / 2 + spriteHeight / 2;

		float correctionValue = 16.f / 2.f;

		float floorScreenY = height * 0.5f + (height * correctionValue) / transformY;
		int drawEndY = static_cast<int>(std::round(floorScreenY));
		int drawStartY = drawEndY - spriteHeight;
		
		// shape에 따라 렌더링
		for (int x = drawStartX; x < drawEndX; ++x)
		{
			if (x < 0 || x >= width)
				continue;

			// 벽보다 뒤에 있으면 렌더링X
			if (transformY >= depthBuffer[x])
				continue;

			for (int y = drawStartY; y <= drawEndY; ++y)
			{
				if (y < 0 || y >= height)
					continue;

				const int index = y * width + x;

				if (renderData.shape == Render3DData::Shape::Circle)
				{
					float centerX = (drawStartX + drawEndX) * 0.5f;
					float centerY = (drawStartY + drawEndY) * 0.5f;

					float radiusX = spriteWidth * 0.5f;
					float radiusY = spriteHeight * 0.5f;

					float dx = (x - centerX) / radiusX;
					float dy = (y - centerY) / radiusY;

					if (dx * dx + dy * dy > 1.0f)
						continue;
				}
				else if (renderData.shape == Render3DData::Shape::Rectangle)
				{
				}
				else if (renderData.shape == Render3DData::Shape::Billboard)
				{
					if (renderData.image.empty())
						continue;

					const int imageHeight = static_cast<int>(renderData.image.size());
					const int imageWidth = static_cast<int>(renderData.image[0].size());

					if (imageWidth <= 0 || imageHeight <= 0)
						continue;

					float u = static_cast<float>(x - drawStartX) / static_cast<float>(spriteWidth);
					float v = static_cast<float>(y - drawStartY) / static_cast<float>(spriteHeight);

					int imageX = static_cast<int>(u * imageWidth);
					int imageY = static_cast<int>(v * imageHeight);

					if (imageY < 0 || imageY >= imageHeight ||
						imageX < 0 || imageX >= static_cast<int>(renderData.image[imageY].size())						)
						continue;

					char pixel = renderData.image[imageY][imageX];

					if (pixel == ' ')
						continue;

					frame->charInfoArray[index].Char.AsciiChar = pixel;
					frame->charInfoArray[index].Attributes = static_cast<WORD>(Color::Green);

					continue;
				}

				frame->charInfoArray[index].Char.AsciiChar = 219;
				frame->charInfoArray[index].Attributes = static_cast<WORD>(renderData.color);
			}
		}
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