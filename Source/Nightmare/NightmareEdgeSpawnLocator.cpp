// Copyright Epic Games, Inc. All Rights Reserved.

#include "NightmareEdgeSpawnLocator.h"

UNightmareEdgeSpawnLocator::UNightmareEdgeSpawnLocator()
{
	Origin = FVector::ZeroVector;
	HalfExtentX = 2000.0f;
	HalfExtentY = 2000.0f;
	EdgeBandWidth = 400.0f;
	SpawnHeight = 100.0f;
	RandomSeed = 1;
	RandomStream.Initialize(RandomSeed);
}

void UNightmareEdgeSpawnLocator::ClampExtents()
{
	HalfExtentX = FMath::Max(1.0f, HalfExtentX);
	HalfExtentY = FMath::Max(1.0f, HalfExtentY);
	EdgeBandWidth = FMath::Clamp(EdgeBandWidth, 0.0f, FMath::Min(HalfExtentX, HalfExtentY));
}

void UNightmareEdgeSpawnLocator::SetOrigin(const FVector& InOrigin)
{
	Origin = InOrigin;
}

void UNightmareEdgeSpawnLocator::SetPlayAreaHalfExtent(float InHalfExtentX, float InHalfExtentY)
{
	HalfExtentX = InHalfExtentX;
	HalfExtentY = InHalfExtentY;
	ClampExtents();
}

void UNightmareEdgeSpawnLocator::SetEdgeBandWidth(float InEdgeBandWidth)
{
	EdgeBandWidth = InEdgeBandWidth;
	ClampExtents();
}

void UNightmareEdgeSpawnLocator::SetSpawnHeight(float InSpawnHeight)
{
	SpawnHeight = InSpawnHeight;
}

void UNightmareEdgeSpawnLocator::SetRandomSeed(int32 Seed)
{
	RandomSeed = Seed;
	RandomStream.Initialize(RandomSeed);
}

FVector UNightmareEdgeSpawnLocator::RollLocation()
{
	return RollLocationWithStream(RandomStream);
}

FVector UNightmareEdgeSpawnLocator::RollLocationWithStream(FRandomStream& Stream) const
{
	const float SafeHalfX = FMath::Max(1.0f, HalfExtentX);
	const float SafeHalfY = FMath::Max(1.0f, HalfExtentY);
	const float Band = FMath::Clamp(EdgeBandWidth, 0.0f, FMath::Min(SafeHalfX, SafeHalfY));

	// 0=+Y (north), 1=-Y (south), 2=+X (east), 3=-X (west)
	const int32 Side = Stream.RandRange(0, 3);
	float LocalX = 0.0f;
	float LocalY = 0.0f;

	switch (Side)
	{
	case 0: // +Y edge
		LocalY = SafeHalfY - Stream.FRandRange(0.0f, Band);
		LocalX = Stream.FRandRange(-SafeHalfX, SafeHalfX);
		break;
	case 1: // -Y edge
		LocalY = -SafeHalfY + Stream.FRandRange(0.0f, Band);
		LocalX = Stream.FRandRange(-SafeHalfX, SafeHalfX);
		break;
	case 2: // +X edge
		LocalX = SafeHalfX - Stream.FRandRange(0.0f, Band);
		LocalY = Stream.FRandRange(-SafeHalfY, SafeHalfY);
		break;
	default: // -X edge
		LocalX = -SafeHalfX + Stream.FRandRange(0.0f, Band);
		LocalY = Stream.FRandRange(-SafeHalfY, SafeHalfY);
		break;
	}

	return FVector(Origin.X + LocalX, Origin.Y + LocalY, Origin.Z + SpawnHeight);
}

bool UNightmareEdgeSpawnLocator::IsOnEdgeBand(const FVector& WorldLocation) const
{
	const float SafeHalfX = FMath::Max(1.0f, HalfExtentX);
	const float SafeHalfY = FMath::Max(1.0f, HalfExtentY);
	const float Band = FMath::Clamp(EdgeBandWidth, 0.0f, FMath::Min(SafeHalfX, SafeHalfY));

	const float RelX = WorldLocation.X - Origin.X;
	const float RelY = WorldLocation.Y - Origin.Y;

	const bool bInsideOuter =
		FMath::Abs(RelX) <= SafeHalfX + KINDA_SMALL_NUMBER &&
		FMath::Abs(RelY) <= SafeHalfY + KINDA_SMALL_NUMBER;
	if (!bInsideOuter)
	{
		return false;
	}

	const float InnerHalfX = SafeHalfX - Band;
	const float InnerHalfY = SafeHalfY - Band;
	const bool bInsideInner =
		FMath::Abs(RelX) < InnerHalfX - KINDA_SMALL_NUMBER &&
		FMath::Abs(RelY) < InnerHalfY - KINDA_SMALL_NUMBER;
	return !bInsideInner;
}
