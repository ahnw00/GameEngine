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
    forward = owner->GetForward();

    Player* detectedPlayer = DetectPlayer();

    if (ownerType == Character::Type::Enemy)
    {
        SetTarget(detectedPlayer);
    }
}

void Sight::CalculatePlayerSight()
{
    // 시작 지점부터 반경 radius이고 중심각 degree인 부채꼴 모양으로
    // 좌표들을 검사해서 액터가 있는지 없는지 판단
    
    if (!owner)
        return;

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
        return;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
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
        }
    }
}

Player* Sight::DetectPlayer()
{
    // 시작 지점부터 반경 radius이고 중심각 degree인 부채꼴 모양으로
    // 좌표들을 검사해서 액터가 있는지 없는지 판단

    if (!owner)
        return nullptr;

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
        return nullptr;

    Player* player = nullptr;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            Vector2 point(
                static_cast<float>(x),
                static_cast<float>(y)
            );

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

            // 현재 포인트 위에 올라와있는 액터들의 리스트 가져오기
            const auto& actors = Renderer::Get().GetActorsAt(point);

            for (auto actor : actors)
            {
                if (actor->IsTypeOf<Player>())
                {
                    player = static_cast<Player*>(actor);
                }
            }
        }
    }

    return player;
}

void Sight::CalculateSight()
{
    // 시작 지점부터 반경 radius이고 중심각 degree인 부채꼴 모양으로
    // 좌표들을 검사해서 액터가 있는지 없는지 판단

    if (!owner)
        return;

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
        return;

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            Vector2 point(
                static_cast<float>(x),
                static_cast<float>(y)
            );

            // 본인이면 일단 렌더
            const auto& actors = Renderer::Get().GetActorsAt(point);

            bool isMe = false;
            for (const auto& actor : actors)
            {
                if (actor->IsTypeOf<Enemy>())
                {
                    Renderer::Get().SetSight(point);
                    isMe = true;
                    break;
                }
            }
            if (isMe) continue;

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
        }
    }
}

//Player* Sight::CalculateSight()
//{
//    if (!owner)
//        return nullptr;
//
//    Vector2 startPoint = owner->GetCenterPosition();
//
//    int minX = static_cast<int>(startPoint.x - radius);
//    int maxX = static_cast<int>(startPoint.x + radius);
//    int minY = static_cast<int>(startPoint.y - radius);
//    int maxY = static_cast<int>(startPoint.y + radius);
//
//    int width = Engine::Get().GetWidth();
//    int height = Engine::Get().GetHeight();
//
//    minX = max(0, minX);
//    maxX = min(width - 1, maxX);
//    minY = max(0, minY);
//    maxY = min(height - 1, maxY);
//
//    float cosHalfAngle = std::cosf(
//        (degree * 0.5f) * 3.14159265f / 180.f
//    );
//
//    const auto& frame = Renderer::Get().GetFrame();
//
//    if (!frame)
//        return nullptr;
//
//    Player* player = nullptr;
//
//    for (int y = minY; y <= maxY; ++y)
//    {
//        for (int x = minX; x <= maxX; ++x)
//        {
//            Vector2 point(
//                static_cast<float>(x),
//                static_cast<float>(y)
//            );
//
//            // 현재 좌표의 액터
//            const auto& actors = Renderer::Get().GetActorsAt(point);
//
//            
//
//            for (const auto& actor : actors)
//            {
//                if (actor->IsTypeOf<Player>())
//                {
//                    player = static_cast<Player*>(actor);
//                    break;
//                }
//            }
//
//            // Player가 자기 자신의 시야를 계산하는 경우
//            if (ownerType == Character::Type::Player && player)
//            {
//                Renderer::Get().SetSight(point);
//                continue;
//            }
//
//            // 거리 검사
//            Vector2 toPoint = point - startPoint;
//            float distance = toPoint.size();
//
//            if (distance > radius)
//                continue;
//
//            if (distance <= 0.f)
//                continue;
//
//            // 시야각 검사
//            toPoint = toPoint.normalized();
//
//            float dot = forward.dot(toPoint);
//
//            if (dot < cosHalfAngle)
//                continue;
//
//            // Enemy가 Player를 탐지하는 경우
//            //if (ownerType == Character::Type::Enemy)
//            //{
//            //    if (player)
//            //        return player;
//            //}
//
//            // Player의 시야 표시
//            if (ownerType == Character::Type::Player)
//            {
//            }
//            Renderer::Get().SetSight(point);
//        }
//    }
//
//    return player;
//}

void Sight::SetTarget(Player* detectedPlayer)
{
    Enemy* enemy = dynamic_cast<Enemy*>(owner);
    if (!enemy)
        return;

    enemy->SetTarget(detectedPlayer);
}
