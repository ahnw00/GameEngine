#include "Sight.h"

#include <Actor/Player.h>
#include <Actor/Enemy.h>
#include <Engine/Engine.h>
#include <Render/Renderer.h>
#include <Actor/Character.h>

#include <cmath>


using namespace Craft;

Sight::Sight(Character* owner)
    : owner(owner)
{
    ownerType = owner->GetCharacterType();
}

void Sight::Tick(float deltaTime)
{
    if (ownerType == Character::Type::Player)
    {
        forward = owner->GetForward();

        //Detect();
    }
    else if (ownerType == Character::Type::Enemy)
    {
        // Todo: Enemy 시야 방향 설정
    }
}

bool Sight::Detect()
{
    // 시작 지점부터 반경 radius이고 중심각 degree인 부채꼴 모양으로
    // 좌표들을 검사해서 액터가 있는지 없는지 판단
    
    if (!owner)
        return false;

    Vector2 startPoint = owner->GetCenterPosition();

    // 반지름 안에 들어올 수 있는 정사각형 범위만 검사
    int minX = static_cast<int>(startPoint.x - radius);
    int maxX = static_cast<int>(startPoint.x + radius);
    int minY = static_cast<int>(startPoint.y - radius);
    int maxY = static_cast<int>(startPoint.y + radius);

    // 화면 범위 제한
    int width = Engine::Get().GetWidth();
    int height = Engine::Get().GetHeight();

    minX = max(0, minX);
    maxX = min(width - 1, maxX);
    minY = max(0, minY);
    maxY = min(height - 1, maxY);

    float cosHalfAngle = std::cosf(
        (degree * 0.5f) * 3.14159265f / 180.f
    );

    // 현재 프레임에 화면에 그려지고 있는 구조체 Frame 가져오기
    const auto& frame = Renderer::Get().GetFrame();
    
    if (!frame)
        return false;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            // 해당 픽셀에 Actor가 있는지 검사
            int idx = y * width + x;

            Vector2 point(
                static_cast<float>(x),
                static_cast<float>(y)
            );

            // 플레이어면 일단 렌더
            const auto& actors = Renderer::Get().GetActorsAt(point);

            bool isPlayer = false;
            for (const auto& actor : actors)
            {
                if (actor->IsTypeOf<Player>())
                {
                    Renderer::Get().SetSight(point);
                    isPlayer = true;
                    break;
                }
            }
            if (isPlayer) continue;

            Vector2 toPoint = point - startPoint;
            float distance = toPoint.size();

            // 원 밖
            if (distance > radius)
                continue;

            // 시작점
            if (distance <= 0.f)
                continue;

            toPoint = toPoint.normalized();

            // 시야각 검사
            float dot = forward.dot(toPoint);

            if (dot < cosHalfAngle)
                continue;

            Renderer::Get().SetSight(point);

            //Renderer::Get()

            //const auto& actors = frame->actorArray[idx];

            //for (Actor* actor : actors)
            //{
            //    if (!actor) continue;

            //    // 자기 자신 제외
            //    if (actor == ownerPtr.get())
            //        continue;

            //    return true;
            //}
        }
    }
    
    return false;
}