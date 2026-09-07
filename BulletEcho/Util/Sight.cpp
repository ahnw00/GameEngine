#include "Sight.h"

#include <Actor/Actor.h>
#include <Actor/Player.h>
#include <Actor/Enemy.h>
#include <Actor/Wall.h>

#include <Engine/Engine.h>
#include <Render/Renderer.h>
#include <Physics/CollisionSystem.h>
#include <Level/GameLevel.h>

#include <cmath>
#include <iostream>


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
        //Player* detectedPlayer = DetectPlayer();
        Character* detectedPlayer = Detect();

        //SetTarget(dynamic_cast<Player*>(detectedPlayer));
    }
    else if (ownerType == Character::Type::Player)
    {
        //Player* detectedPlayer = DetectPlayer();
        Detect();

        //SetTarget(dynamic_cast<Player*>(detectedPlayer));
    }
}

Player* Sight::DetectPlayer()
{
    // 시작 지점부터 반경 radius이고 중심각 degree인 부채꼴 모양으로
    // 좌표들을 검사해서 액터가 있는지 없는지 판단

    if (!owner)
        return nullptr;

    Vector2 startPoint = owner->GetCenterPosition();

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

            std::shared_ptr<Level> level = owner->GetOwner();
            if (!level) return nullptr;

            std::shared_ptr<GameLevel> gameLevel = Cast<GameLevel>(level);
            if (!gameLevel) return nullptr;

            if (CollisionSystem::Get().FindActorOn(gameLevel->GetPlayer(), point))
                return gameLevel->GetPlayer();
        }
    }

    return nullptr;
}

Character* Sight::Detect()
{
    if (!owner)
        return nullptr;

    detectedEnemies.clear();

    Vector2 startPoint = owner->GetCenterPosition();

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

            std::shared_ptr<Level> level = owner->GetOwner();
            if (!level) return nullptr;

            std::shared_ptr<GameLevel> gameLevel = Cast<GameLevel>(level);
            if (!gameLevel) return nullptr;

            if (ownerType == Character::Type::Enemy)
            {
                if (CollisionSystem::Get().FindActorOn(gameLevel->GetPlayer(), point))
                    return gameLevel->GetPlayer(); // 강제 형변환 일어나는데 업캐스팅이라 괜찮아
            }
            else if (ownerType == Character::Type::Player)
            {
                std::vector<Craft::Actor*> actors = CollisionSystem::Get().GetActorsOn(point);

                for (const auto& actor : actors)
                {
                    if (dynamic_cast<Enemy*>(actor))
                        detectedEnemies.insert(actor);
                }
            }
        }
    }

    if (!detectedEnemies.empty())
    {
        for (auto actor : detectedEnemies)
        {
            //Vector2 enemyForward = enemy->GetForward();
            ////Vector2 playerForward = owner->GetForward();
            //Vector2 playerForward = owner->GetCenterPosition() - enemy->GetCenterPosition();
            //playerForward = playerForward.normalized();

            //const float PI = 3.141592f;

            //// 외적
            //float cross = playerForward.y * enemyForward.x - playerForward.x * enemyForward.y;
            //float dot = playerForward.dot(enemyForward);
            //float angle = std::atan2(cross, dot) * 180.f / PI;

            //// 적이 플레이어를 바라보았을때를 0도로
            //angle += 180.f;

            //if (angle < 0.f)
            //    angle += 360.f;
            //if (angle >= 360.f)
            //    angle -= 360.f;

            //int idx = static_cast<int>(std::round(angle / 45.f)) % 8;

            ////std::cout
            ////    << "PlayerForward: (" << playerForward.x << ", " << playerForward.y << ") "
            ////    << "EnemyForward: (" << enemyForward.x << ", " << enemyForward.y << ") "
            ////    << "Angle: " << angle << " "
            ////    << "Index: " << idx
            ////    << "\n\n";

            //static_cast<Enemy*>(enemy)->SetDirection(static_cast<Craft::Actor::Direction>(idx));

            Enemy* enemy = dynamic_cast<Enemy*>(actor);

            if (!enemy)
                continue;

            // 적 -> 플레이어 방향
            Vector2 toPlayer = owner->GetCenterPosition() - enemy->GetCenterPosition();
            if (toPlayer.size() <= 0.f)
                continue;

            toPlayer = toPlayer.normalized();

            Vector2 enemyForward = enemy->GetForward();

            float cross = toPlayer.x * enemyForward.y - toPlayer.y * enemyForward.x;
            float dot = toPlayer.dot(enemyForward);

            const float PI = 3.14159265f;
            float angle = std::atan2(cross, dot) * 180.f / PI;

            //angle += 180.f;

            if (angle < 0.f)
                angle += 360.f;
            if (angle >= 360.f)
                angle -= 360.f;


            // 현재 Direction의 순서
            //
            // N  = 0
            // NE = 1
            // E  = 2
            // SE = 3
            // S  = 4
            // SW = 5
            // W  = 6
            // NW = 7
            //
            // atan2 기준은 E가 0도이므로
            // Direction 기준으로 90도 회전시켜준다.
            // 8방향으로 반올림
            int idx = static_cast<int>(std::round(angle / 45.f)) % 8;

            enemy->SetDirection(static_cast<Craft::Actor::Direction>(idx));
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

bool Sight::IsBehindWall(Vector2 point, Vector2 startPoint)
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

            //const auto& actors = Renderer::Get().GetActorsAt(checkPoint);

            //for (const auto& actor : actors)
            //{
            //    if (actor->IsTypeOf<Wall>())
            //    {
            //        return true;
            //    }
            //}

            std::shared_ptr<GameLevel> gameLevel = Cast<GameLevel>(owner->GetOwner());
            if (!gameLevel)
                return false;

            if (gameLevel->IsWall(checkPoint))
                return true;
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
