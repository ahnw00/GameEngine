#pragma once

#include <Core/Core.h> // 외부에서 접근하게 하려고
#include <Math/Vector2.h>
#include <Math/Color.h>
#include <Render/Render3DData.h>
#include <string>
#include <vector>
#include <memory>

namespace Craft
{
	// 전방 선언
	class ScreenBuffer;
	class Actor;

	// 그리기 기능을 전담하는 전문 객체
	class CRAFT_API Renderer
	{
	public:
		enum class RenderMode
		{
			MENU,
			TwoDimension,
			ThreeDimension
		};

		enum class PlayMode
		{
			DEBUG,
			PLAY
		};

		enum class SightState
		{
			None,
			Range,
			Visible
		};

	private:
		// 프레임(이미지) 데이터 구조체
		struct Frame
		{
			Frame(int bufferCount);
			~Frame();

			// 프레임 초기화 함수
			void Clear(const Vector2& screenSize);

			// 화면에 그릴 2차원 배열 문자값(1차원 배열로 다뤄)
			std::unique_ptr<CHAR_INFO[]> charInfoArray;

			// 그리기 정렬 값 2차원 배열(1차원 배열로 다뤄)
			std::unique_ptr<int[]> sortingOrderArray;

			// 2차원 배열의 각 칸에 올라와있는 액터 리스트(1차원 배열 안에 리스트가 있어)
			std::unique_ptr<std::vector<Actor*>[]> actorArray;

			// 시야 영역 여부
			std::unique_ptr<bool[]> sightArray;
			std::unique_ptr<SightState[]> sightStateArray;
		};

		struct Frame3D
		{
			Frame3D(int bufferCount);
			~Frame3D();

			void Clear(const Vector2& screenSize);

			// 화면에 그릴 2차원 배열 문자값(1차원 배열로 다뤄)
			std::unique_ptr<CHAR_INFO[]> charInfoArray;
		};
		
		// 화면에 그릴 데이터를 명령 단위로 저장하기 위한 구조체
		struct RenderCommand
		{
			Actor* actor = nullptr;

			// 화면에 그릴 문자값
			std::vector<std::string> image;

			// 위치
			Vector2 position = Vector2::Zero;

			// 색상
			Color color = Color::White;

			// 그리기 정렬 순서, 값이 크면 우선순위가 높음(더 위에 그려짐)
			int sortingOrder = -1; // -1은 설정되지 않았다는 의미
		};

	public:
		Renderer(const Vector2& worldSize, const Vector2& screenSize);
		~Renderer();

		// 화면에 그릴 데이터를 제출(전달)하는 함수
		void Submit(
			Actor* actor,
			const std::vector<std::string>& image, 
			const Vector2& position,
			Color color = Color::White,
			int sortingOrder = 0
		);

		// Draw 이벤트 함수 - Engine에서 호출
		void Draw();

		// 전역 접근 함수
		static Renderer& Get();

		// 프레임 읽기 전용 함수
		inline const std::unique_ptr<Frame>& GetFrame() const { return frame; }

		void SetSight(const Vector2& position, const SightState& sightState);

		const std::vector<Actor*>& GetActorsAt(const Vector2& position);

		inline const RenderMode GetRenderMode() const { return renderMode; }
		inline void SetRenderMode(RenderMode newMode) { renderMode = newMode; }

		inline const PlayMode GetPlayMode() const { return playMode; }
		inline void SetPlayMode(PlayMode newMode) { playMode = newMode; }

		inline const Vector2 GetScreenSize() const { return screenSize; }
		inline const Vector2 GetWorldSize() const { return worldSize; }
		inline void SetWorldSize(const Vector2& newWorldSize) { worldSize = newWorldSize; }
		inline void SetScreenSize(const Vector2& newScreenSize) { screenSize = newScreenSize; }

		inline void SetRenderStartPosition(const Vector2& position) 
		{ 
			renderStartPosition = position; 
		}
		inline const Vector2 GetRenderStartPosition() const { return renderStartPosition; }

		bool WorldToScreenPosition(const Vector2& worldPosition, Vector2& screenPosition) const;

	public:
		void BeginFrame();

		// 표시한 시야 렌더
		void DrawSight();

		// 전달 받은 렌더 명령을 활용해 화면을 그리는 함수
		void DrawRenderQueue();

		// 마우스 커서 표시
		void DrawMouseCursor();

		// 3D 렌더링
		void Draw3DView(
			const Vector2& playerPos,
			const Vector2& playerDir,
			const Vector2& cameraPlane,
			const std::vector<std::string>& mapData,
			const std::vector<std::shared_ptr<Actor>>& actorList
		);

		// 그린 결과를 화면에 표시하는 함수
		void Present();

		void GetLargestWindowSize() const;

		void SwitchRenderMode(RenderMode newMode);

		inline void SetScreen3DSize(const Vector2& new3DSize) { screen3DSize = new3DSize; }

	private:
		// 그리기 작업을 시작할 때 프레임(화면)을 지우는 함수
		void Clear();

		// Getter
		const ScreenBuffer* const GetCurrentBuffer() const;

		void SetFontSize(short width, short height);

		void DrawActor3D(
			Actor* actor,
			const Render3DData& renderData,
			const Vector2& playerPos,
			const Vector2& playerDir,
			const Vector2& cameraPlane
		);

	private:
		// 전역 접근이 가능하도록 변수 선언
		static Renderer* instance;

		// 이번 프레임에 그릴 렌더 명령을 모아두는 배열
		// 큐(Queue)처럼 사용
		std::vector<RenderCommand> renderQueue;

		// 화면 크기
		Vector2 firstScreenSize = Vector2(169, 51);

		Vector2 screenSize;

		Vector2 screen3DSize = Vector2(1280, 342);

		// 월드 크기
		Vector2 worldSize;

		// 3D 렌더링용 깊이 버퍼
		// 가 화면 x열에서 가장 가까운 벽까지의 거리
		std::unique_ptr<float[]> depthBuffer;

		// 글자/그리기 순서 2차원 배열을 관리하는 프레임 객체
		std::unique_ptr<Frame> frame;

		// 이중 버퍼링 구현을 위한 화면 버퍼 2개
		std::unique_ptr<ScreenBuffer> screenBufferArray[2];

		std::unique_ptr<CHAR_INFO[]> screenCharInfoArray;
		
		// 글자/그리기 순서 2차원 배열을 관리하는 프레임 객체(3D)
		std::unique_ptr<Frame3D> frame3D;

		// 이중 버퍼링 구현을 위한 화면 버퍼 2개(3D)
		std::unique_ptr<ScreenBuffer> screenBuffer3DArray[2];

		std::unique_ptr<CHAR_INFO[]> screenCharInfo3DArray;

		// 버퍼 인덱스
		int currentBufferIndex = 0;

		// 렌더 모드
		RenderMode renderMode = RenderMode::TwoDimension;

		// 플레이 모드
		PlayMode playMode = PlayMode::PLAY;

		Vector2 renderStartPosition = Vector2::Zero;
	};
}