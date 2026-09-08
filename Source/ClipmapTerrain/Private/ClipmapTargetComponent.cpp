#include "ClipmapTargetComponent.h"
#include "ClipmapCollisionSubsystem.h"

UClipmapTargetComponent::UClipmapTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UClipmapTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* world = GetWorld();

	if (!world)
	{
		return;
	}
	UClipmapCollisionSubsystem* collisionSubsystem = world->GetSubsystem<UClipmapCollisionSubsystem>();
	if (!collisionSubsystem)
	{
		return;
	}
	collisionSubsystem->TerrainTargetComponents.Add(this);
}

void UClipmapTargetComponent::OnComponentDestroyed(bool parent)
{
	UWorld* world = GetWorld();

	if (!world)
	{
		return;
	}
	UClipmapCollisionSubsystem* collisionSubsystem = world->GetSubsystem<UClipmapCollisionSubsystem>();
	if (!collisionSubsystem)
	{
		return;
	}
	collisionSubsystem->TerrainTargetComponents.Remove(this);
	Super::OnComponentDestroyed(parent);
}
