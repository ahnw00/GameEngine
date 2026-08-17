#include "Sight.h"

#include <Actor/Player.h>
#include <Actor/Enemy.h>
#include <Actor/Wall.h>
#include <Engine/Engine.h>
#include <Render/Renderer.h>

#include <cmath>


using namespace Craft;

Sight::Sight(Character* owner)
    : owner(owner)
{
    ownerType = owner->GetCharacterType();

    // 화면 범위 제한
    width = Engine::Get().GetWidth();
    height = Engine::Get().GetHeight();
}

void Sight::Tick(float deltaTime)
{
    startPoint = owner->GetCenterPosition();

    // 반지름 안에 들어올 수 있는 정사각형 범위만 검사
    minX = static_cast<int>(startPoint.x - radius);
    maxX = static_cast<int>(startPoint.x + radius);
    minY = static_cast<int>(startPoint.y - radius);
    maxY = static_cast<int>(startPoint.y + radius);

    minX = max(0, minX);
    maxX = min(width - 1, maxX);
    minY = max(0, minY);
    maxY = min(height - 1, maxY);

    forward = owner->GetForward();

    if (ownerType == Character::Type::Enemy)
    {
        Player* detectedPlayer = DetectPlayer();
        SetTarget(detectedPlayer);
    }
}

Player* Sight::DetectPlayer()
{
    // 시작 지점부터 반경 radius이고 중심각 degree인 부채꼴 모양으로
    // 좌표들을 검사해서 액터가 있는지 없는지 판단

    if (!owner)
        return nullptr;

    Vector2 startPoint = owner->GetCenterPosition();

    // 현재 프레임에 화면에 그려지고 있는 구조체 Frame 가져오기
    const auto& frame = Renderer::Get().GetFrame();

    if (!frame)
        return nullptr;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            Vector2 point(
                static_cast<float>(x),
                static_cast<float>(y)
            );

            if (!CheckRange(point, startPoint))
                continue;
            if (IsBehindWall(point, startPoint))
                continue;

            // 현재 포인트 위에 올라와있는 액터들의 리스트 가져오기
            const auto& actors = Renderer::Get().GetActorsAt(point);

            for (auto actor : actors)
            {
                if (actor->IsTypeOf<Player>())
                {
                    return static_cast<Player*>(actor);
                }
            }
        }
    }

    return nullptr;
}

void Sight::CalculateSight()
{
    // 시작 지점부터 반경 radius이고 중심각 degree인 부채꼴 모양으로
    // 좌표들을 검사해서 액터가 있는지 없는지 판단

    if (!owner)
        return;

    Vector2 startPoint = owner->GetCenterPosition();

    // 현재 프레임에 화면에 그려지고 있는 구조체 Frame 가져오기
    const auto& frame = Renderer::Get().GetFrame();

    if (!frame)
        return;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            Vector2 point(
                static_cast<float>(x),
                static_cast<float>(y)
            );

            //// 현재 체크하는 위치에 올라와있는 액터들 가져와
            //const auto& actors = Renderer::Get().GetActorsAt(point);

            //// DEBUGGING
            //bool checked = false;
            //for (const auto& actor : actors)
            //{
            //    if (actor->IsTypeOf<Player>() /*|| actor->IsTypeOf<Enemy>()*/)
            //    {
            //        Renderer::Get().SetSight(point);
            //        checked = true;
            //    }
            //}
            //if (checked) continue;

            Vector2 toPoint = point - startPoint;
            float distance = toPoint.size();

            if (distance > radius || distance <= 0.f)
                continue;
            if (IsBehindWall(point, startPoint))
                continue;

            Renderer::Get().SetSight(point, Renderer::SightState::Range);

            if (!CheckRange(point, startPoint))
                continue;

            Renderer::Get().SetSight(point, Renderer::SightState::Visible);
        }
    }
}

bool Sight::CheckRange(Vector2 point, Vector2 startPoint)
{
    Vector2 toPoint = point - startPoint;
    float distance = toPoint.size();

    // 원 밖
    if (distance > radius)
        return false;

    // 시작점
    if (distance <= 0.f)
        return false;

    toPoint = toPoint.normalized();

    // 시야각 검사
    float dot = forward.dot(toPoint);

    float cosHalfAngle = std::cosf(
        (degree * 0.5f) * 3.14159265f / 180.f
    );

    if (dot < cosHalfAngle)
        return false;

    return true;
}

bool Sight::IsBehindWall(Craft::Vector2 point, Craft::Vector2 startPoint)
{
    // 시작점과 목표점을 정수 타일 좌표로 변환
    int x0 = static_cast<int>(std::round(startPoint.x));
    int y0 = static_cast<int>(std::round(startPoint.y));

    int x1 = static_cast<int>(std::round(point.x));
    int y1 = static_cast<int>(std::round(point.y));

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int error = dx - dy;

    while (true)
    {
        // 시작점은 검사하지 않음
        if (!(x0 == static_cast<int>(std::round(startPoint.x)) &&
            y0 == static_cast<int>(std::round(startPoint.y))))
        {
            Vector2 checkPoint(
                static_cast<float>(x0),
                static_cast<float>(y0)
            );

            const auto& actors = Renderer::Get().GetActorsAt(checkPoint);

            for (const auto& actor : actors)
            {
                if (actor->IsTypeOf<Wall>())
                {
                    return true;
                }
            }
        }

        // 목표 지점까지 도착
        if (x0 == x1 && y0 == y1)
            break;

        int error2 = error * 2;

        if (error2 > -dy)
        {
            error -= dy;
            x0 += sx;
        }

        if (error2 < dx)
        {
            error += dx;
            y0 += sy;
        }
    }

    return false;
}

void Sight::SetTarget(Player* detectedPlayer)
{
    Enemy* enemy = dynamic_cast<Enemy*>(owner);
    if (!enemy)
        return;

    enemy->SetTarget(detectedPlayer);
}
