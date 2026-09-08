#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClipmapTargetComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLIPMAPTERRAIN_API UClipmapTargetComponent : public UActorComponent
{
	GENERATED_BODY()


public:
	// Sets default values for this component's properties
	UClipmapTargetComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnComponentDestroyed(bool parent) override;
};
