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

		const FBox ActorBox(Origin - Extent, Origin + Extent);

		FVector Vertices[8];
		ActorBox.GetVertices(Vertices);
		for (int i = 0; i < 8; i++)
		{
			OutVerts.Add(FMath::Lerp(Origin, Vertices[i], Scale));
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
