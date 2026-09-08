#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Chaos/HeightField.h"
#include "ClipmapCollisionComponent.generated.h"

class UTerrainCollisionData;

namespace Chaos
{
	class FHeightField;
	struct FPhysicsObject;
}

UCLASS()
class UClipmapCollisionComponent : public UPrimitiveComponent
{
	GENERATED_BODY()


public:

	Chaos::FHeightFieldPtr HeightfieldGeometry;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type reason) override;

protected:
	virtual void OnCreatePhysicsState() override;
	virtual void OnDestroyPhysicsState() override;
	virtual void OnUnregister() override;

	virtual bool ShouldCreatePhysicsState() const override { return HeightfieldGeometry.IsValid(); };
	//virtual bool IsNavigationRelevant() const override { return true; } // TODO
};