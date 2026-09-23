#include "ZippyCharacter.h"
#include "ZippyCharacterMovementComponent.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FZippyWallRunTest, "Zippy.Movement.WallRun",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FZippyWallRunTest::RunTest(const FString& Parameters)
{
	// Use an isolated, transient physics world so the user's level and Blueprint stay untouched.
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	auto AddBox = [World](const FVector& Location, const FVector& Extent)
	{
		AActor* Actor = World->SpawnActor<AActor>();
		UBoxComponent* Box = NewObject<UBoxComponent>(Actor);
		Actor->SetRootComponent(Box);
		Box->SetBoxExtent(Extent);
		Box->SetCollisionProfileName(TEXT("BlockAll"));
		Box->RegisterComponent();
		Actor->SetActorLocation(Location);
		return Actor;
	};
	AActor* Wall = AddBox(FVector(0, 100, 500), FVector(10000, 50, 500));
	AddBox(FVector(0, 0, -10), FVector(10000, 10000, 10));

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AZippyCharacter* Character = World->SpawnActor<AZippyCharacter>(FVector(0, 0, 300), FRotator::ZeroRotator, SpawnParameters);
	UZippyCharacterMovementComponent* Movement = CastChecked<UZippyCharacterMovementComponent>(Character->GetCharacterMovement());
	Movement->bRunPhysicsWithNoController = true;
	constexpr float Step = 1.f / 60.f;
	auto Reset = [&](const FVector& Location, const FVector& Velocity)
	{
		Movement->StopMovementImmediately();
		Character->SetActorLocationAndRotation(Location, FRotator::ZeroRotator, false, nullptr, ETeleportType::TeleportPhysics);
		Movement->SetMovementMode(MOVE_Falling);
		Movement->Velocity = Velocity;
	};

	// Parallel contact must work on both sides and cancel all inherited jump/fall velocity.
	for (const float SideY : {0.f, 200.f})
	{
		for (const float InitialZSpeed : {700.f, -100.f})
		{
			Reset(FVector(0, SideY, 300), FVector(500, 0, InitialZSpeed));
			for (int32 Frame = 0; Frame < 60; ++Frame)
			{
				Movement->TickComponent(Step, LEVELTICK_All, nullptr);
				TestTrue(TEXT("Parallel run stays active"), Movement->IsWallRunning());
				TestTrue(TEXT("Horizontal run holds height"), FMath::IsNearlyEqual(Character->GetActorLocation().Z, 300.0, 0.01));
				TestTrue(TEXT("Horizontal run has no vertical velocity"), FMath::IsNearlyZero(Movement->Velocity.Z));
			}
			TestTrue(TEXT("Run moves along wall"), Character->GetActorLocation().X > 400.f);
			TestEqual(TEXT("Correct wall side"), Movement->WallRunningIsRight(), SideY == 0.f);
		}
	}

	Reset(FVector(0, 0, 300), FVector(100, 0, 0));
	Movement->TickComponent(Step, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Below minimum along-wall speed rejects entry"), Movement->IsWallRunning());
	Reset(FVector(0, 0, 300), FVector(500, -300, 0));
	Movement->TickComponent(Step, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Moving away from wall rejects entry"), Movement->IsWallRunning());
	Reset(FVector(0, 0, 300), FVector(500, 0, -900));
	Movement->TickComponent(Step, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Excessive falling speed rejects entry"), Movement->IsWallRunning());
	Reset(FVector(0, 0, 110), FVector(500, 0, 0));
	Movement->TickComponent(Step, LEVELTICK_All, nullptr);
	TestFalse(TEXT("Too close to floor rejects entry"), Movement->IsWallRunning());

	Reset(FVector(0, 0, 300), FVector(500, 0, 700));
	Movement->TickComponent(Step, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Jump test starts on wall"), Movement->IsWallRunning());
	TestTrue(TEXT("Wall jump remains available"), Movement->DoJump(false, Step));
	TestTrue(TEXT("Wall jump gives upward and outward velocity"), Movement->Velocity.Z > 0.f && Movement->Velocity.Y < 0.f);
	Movement->TickComponent(Step, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Wall jump does not immediately reattach"), Movement->IsFalling());

	Reset(FVector(0, 0, 300), FVector(500, 0, 0));
	Movement->TickComponent(Step, LEVELTICK_All, nullptr);
	Wall->SetActorLocation(FVector(0, 1000, 500));
	Movement->TickComponent(Step, LEVELTICK_All, nullptr);
	TestTrue(TEXT("Losing wall resumes falling"), Movement->IsFalling());
	Wall->SetActorLocation(FVector(0, 100, 500));

	// The optional descending mode must safely handle an unassigned curve.
	FBoolProperty* HorizontalOnly = FindFProperty<FBoolProperty>(Movement->GetClass(), TEXT("bWallRunHorizontalOnly"));
	if (TestNotNull(TEXT("Horizontal-only setting is reflected"), HorizontalOnly))
	{
		HorizontalOnly->SetPropertyValue_InContainer(Movement, false);
		Reset(FVector(0, 0, 300), FVector(500, 0, 700));
		Movement->TickComponent(Step, LEVELTICK_All, nullptr);
		TestTrue(TEXT("Descending mode starts a run"), Movement->IsWallRunning());
		TestTrue(TEXT("Missing curve falls safely without rising"), Movement->Velocity.Z < 0.f && Character->GetActorLocation().Z <= 300.f);
	}

	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	return true;
}
#endif
