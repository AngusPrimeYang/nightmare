// Copyright Epic Games, Inc. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NightmareProjectileActor.h"
#include "NightmareProjectileTypes.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(FNightmareProjectileSpec,
	"Nightmare.Projectile",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FNightmareProjectileSpec)

void FNightmareProjectileSpec::Define()
{
	Describe("ANightmareProjectileActor defaults", [this]()
	{
		It("ComputeDefaultSpeed equals WalkSpeed times 10", [this]()
		{
			TestEqual(TEXT("600 walk"), ANightmareProjectileActor::ComputeDefaultSpeed(600.0f), 6000.0f);
			TestEqual(TEXT("0 walk"), ANightmareProjectileActor::ComputeDefaultSpeed(0.0f), 0.0f);
		});

		It("ComputeDefaultDamage equals PlayerMaxStamina divided by 20", [this]()
		{
			TestEqual(TEXT("100 stamina"), ANightmareProjectileActor::ComputeDefaultDamage(100.0f), 5.0f);
			TestEqual(TEXT("0 stamina"), ANightmareProjectileActor::ComputeDefaultDamage(0.0f), 0.0f);
		});

		It("ConfigureProjectile stores speed damage trajectory and half extents", [this]()
		{
			ANightmareProjectileActor* Projectile = NewObject<ANightmareProjectileActor>(GetTransientPackage());
			const FVector Extents(50.0f, 6.0f, 6.0f);
			Projectile->ConfigureProjectile(
				FVector(0.0f, 1.0f, 0.0f),
				4200.0f,
				12.0f,
				ENightmareProjectileTrajectory::StraightLine,
				Extents);

			TestEqual(TEXT("speed"), Projectile->GetSpeed(), 4200.0f);
			TestEqual(TEXT("damage"), Projectile->GetDamage(), 12.0f);
			TestEqual(
				TEXT("trajectory"),
				static_cast<uint8>(Projectile->GetTrajectory()),
				static_cast<uint8>(ENightmareProjectileTrajectory::StraightLine));
			TestEqual(TEXT("extents"), Projectile->GetHalfExtents(), Extents);
			TestEqual(TEXT("dir y"), static_cast<float>(Projectile->GetFlightDirection().Y), 1.0f);
		});
	});

	Describe("ANightmareProjectileActor flight", [this]()
	{
		It("TickFlight StraightLine moves along direction by Speed times DeltaSeconds", [this]()
		{
			ANightmareProjectileActor* Projectile = NewObject<ANightmareProjectileActor>(GetTransientPackage());
			Projectile->ConfigureProjectile(
				FVector(1.0f, 0.0f, 0.0f),
				1000.0f,
				5.0f,
				ENightmareProjectileTrajectory::StraightLine,
				FVector(40.0f, 4.0f, 4.0f));
			Projectile->SetActorLocation(FVector::ZeroVector);

			const FVector Delta = Projectile->TickFlight(0.25f);
			TestEqual(TEXT("dx"), static_cast<float>(Delta.X), 250.0f);
			TestEqual(TEXT("dy"), static_cast<float>(Delta.Y), 0.0f);
			TestEqual(TEXT("dz"), static_cast<float>(Delta.Z), 0.0f);
		});

		It("TickFlight StraightLine keeps Z when direction is horizontal", [this]()
		{
			ANightmareProjectileActor* Projectile = NewObject<ANightmareProjectileActor>(GetTransientPackage());
			Projectile->ConfigureProjectile(
				FVector(0.707f, 0.707f, 0.0f),
				200.0f,
				5.0f,
				ENightmareProjectileTrajectory::StraightLine,
				FVector(40.0f, 4.0f, 4.0f));
			Projectile->SetActorLocation(FVector(0.0f, 0.0f, 180.0f));

			Projectile->TickFlight(1.0f);
			TestEqual(TEXT("z unchanged"), static_cast<float>(Projectile->GetActorLocation().Z), 180.0f);
		});

		It("TickFlight returns zero when already consumed", [this]()
		{
			ANightmareProjectileActor* Projectile = NewObject<ANightmareProjectileActor>(GetTransientPackage());
			Projectile->ConfigureProjectile(
				FVector(1.0f, 0.0f, 0.0f),
				500.0f,
				5.0f,
				ENightmareProjectileTrajectory::StraightLine,
				FVector(40.0f, 4.0f, 4.0f));
			Projectile->Consume();
			TestTrue(TEXT("consumed"), Projectile->IsConsumed());
			TestTrue(TEXT("no move"), Projectile->TickFlight(0.5f).IsNearlyZero());
		});
	});
}

#endif
