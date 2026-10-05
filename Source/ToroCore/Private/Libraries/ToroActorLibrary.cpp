// Copyright (C) RedCraft86. Licensed under LGPL-3.0, see LICENSE file for details.

#include "Libraries/ToroActorLibrary.h"
#include "GameFramework/Actor.h"

void UToroActorLibrary::GetBoundingBoxVertices(TSet<FVector>& OutVerts, FVector& Origin, FVector& Extent,
	const AActor* Target, FVector Scale, bool bOnlyColliding, bool bChildActors)
{
	OutVerts.Empty();
	Origin = FVector::ZeroVector;
	Extent = FVector::ZeroVector;

	if (IsValid(Target))
	{
		Target->GetActorBounds(bOnlyColliding, Origin, Extent, bChildActors);
		if (Scale.Size() < 0.05f)
		{
			OutVerts.Add(Origin);
			return;
		}

		constexpr uint8 NumCorners = 8;
		//     (+Z)
		//      |
		//      |
		//      +--------(+Y)
		//     /
		//    /
		//  (+X)
		//
		//    (5)---------------(4)
		//    /|                /|
		//   / |               / |
		// (6)---------------(7) |
		//  |  |              |  |
		//  |  |     (O)      |  |
		//  | (0)-------------|-(3)
		//  | /               | /
		//  |/                |/
		// (1)---------------(2)
		//
		// O = Origin
		// 0..7 = Extent Multipliers (Index)
		const FVector Multipliers[NumCorners]{
			FVector(-1, -1, -1), // A
			FVector( 1, -1, -1), // B
			FVector( 1,  1, -1), // C
			FVector(-1,  1, -1), // D
			FVector(-1,  1,  1), // E
			FVector(-1, -1,  1), // F
			FVector( 1, -1,  1), // G
			FVector( 1,  1,  1)  // H
		};

		OutVerts.Reserve(NumCorners);
		for (int i = 0; i < NumCorners; i++)
		{
			OutVerts.Add(Origin + (Extent * Multipliers[i] * Scale));
		}
	}
}

void UToroActorLibrary::AddActorTag(AActor* Target, FName InTag)
{
	if (IsValid(Target))
	{
		Target->Tags.AddUnique(InTag);
	}
}

void UToroActorLibrary::RemoveActorTag(AActor* Target, FName InTag)
{
	if (IsValid(Target))
	{
		Target->Tags.Remove(InTag);
	}
}
