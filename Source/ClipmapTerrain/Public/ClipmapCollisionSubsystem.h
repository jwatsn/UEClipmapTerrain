#pragma once
#include "Subsystems/WorldSubsystem.h"
#include "ClipmapCollisionSubsystem.generated.h"

UCLASS()
class UClipmapCollisionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	TSet<UActorComponent*> TerrainTargetComponents;

};