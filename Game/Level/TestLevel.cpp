#include "TestLevel.h"
#include <Actor/TestActor.h>

using namespace Craft;

void TestLevel::OnInitialized()
{
	Level::OnInitialized();

	// 테스트 액터를 레벨에 추가
	SpawnActor<TestActor>();
	//SpawnActor<Level>(); // 불가능 
	                       // Level.h에서 보면 SFINAE를 걸어서 Actor의 하위 클래스만 T에 넣어줄 수 있어
	                       // 이러한 경우를 막아주려고 SFINAE 제한을 걸어준거야

}
