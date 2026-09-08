#include "ClipmapCollisionComponent.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Chaos/ChaosArchive.h"
#include "Physics/Experimental/PhysInterface_Chaos.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"
#include "Chaos/ImplicitObject.h"
#include "Chaos/Core.h"
#include "Physics/PhysicsFiltering.h"

void UClipmapCollisionComponent::OnCreatePhysicsState()
{
	USceneComponent::OnCreatePhysicsState();

	if (!BodyInstance.IsValidBodyInstance())
	{
		if (HeightfieldGeometry.IsValid())
		{
			// Make transform for this landscape component PxActor
			FTransform LandscapeComponentTransform = GetComponentToWorld();
			FMatrix LandscapeComponentMatrix = LandscapeComponentTransform.ToMatrixWithScale();
			FVector LandscapeScale = LandscapeComponentMatrix.ExtractScaling();


			{
				FActorCreationParams Params;
				Params.InitialTM = LandscapeComponentTransform;
				Params.InitialTM.SetScale3D(FVector(0));
				Params.bQueryOnly = false;
				Params.bStatic = true;
				Params.Scene = GetWorld()->GetPhysicsScene();


				FPhysicsActorHandle PhysHandle;
				FPhysicsInterface::CreateActor(Params, PhysHandle, this);
				Chaos::FRigidBodyHandle_External& Body_External = PhysHandle->GetGameThreadAPI();

				Chaos::FShapesArray ShapeArray;
				TArray<Chaos::FImplicitObjectPtr> Geoms;

				// First add complex geometry
				Chaos::FImplicitObjectPtr ImplicitHeightField(HeightfieldGeometry);
				Chaos::FImplicitObjectPtr ChaosHeightFieldFromCooked = MakeImplicitObjectPtr<Chaos::TImplicitObjectTransformed<Chaos::FReal, 3>>(ImplicitHeightField, Chaos::FRigidTransform3(FTransform::Identity));

				TUniquePtr<Chaos::FPerShapeData> NewShape = Chaos::FShapeInstanceProxy::Make(ShapeArray.Num(), ChaosHeightFieldFromCooked);

				// Setup filtering
				FCollisionFilterData QueryFilterData, SimFilterData;
				CreateShapeFilterData(static_cast<uint8>(GetCollisionObjectType()), FMaskFilter(0), GetOwner()->GetUniqueID(), GetCollisionResponseToChannels(),
					GetUniqueID(), 0, QueryFilterData, SimFilterData, true, false, true);

				// Heightfield is used for simple and complex collision
				QueryFilterData.Word3 |= (EPDF_SimpleCollision | EPDF_ComplexCollision);
				SimFilterData.Word3 |= (EPDF_SimpleCollision | EPDF_ComplexCollision);

				NewShape->SetQueryData(QueryFilterData);
				NewShape->SetSimData(SimFilterData);
				//NewShape->SetMaterials(HeightfieldRef->UsedChaosMaterials);

				Geoms.Emplace(MoveTemp(ChaosHeightFieldFromCooked));
				ShapeArray.Emplace(MoveTemp(NewShape));
				if (Geoms.Num() == 1)
				{
					Body_External.SetGeometry(Geoms[0]);
				}
				else
				{
					Body_External.SetGeometry(MakeImplicitObjectPtr<Chaos::FImplicitObjectUnion>(MoveTemp(Geoms)));
				}

				// Construct Shape Bounds
				for (auto& Shape : ShapeArray)
				{
					Chaos::FRigidTransform3 WorldTransform = Chaos::FRigidTransform3(Body_External.X(), Body_External.R());
					Shape->UpdateShapeBounds(WorldTransform);
				}
				Body_External.MergeShapesArray(MoveTemp(ShapeArray));

				// Push the actor to the scene
				FPhysScene* PhysScene = GetWorld()->GetPhysicsScene();

				// Set body instance data
				BodyInstance.PhysicsUserData = FPhysicsUserData(&BodyInstance);
				BodyInstance.OwnerComponent = this;
				BodyInstance.ActorHandle = PhysHandle;

				Body_External.SetUserData(&BodyInstance.PhysicsUserData);

				TArray<FPhysicsActorHandle> Actors;
				Actors.Add(PhysHandle);

				FPhysicsCommand::ExecuteWrite(PhysScene, [&]()
					{
						bool bImmediateAccelStructureInsertion = true;
						PhysScene->AddActorsToScene_AssumesLocked(Actors, bImmediateAccelStructureInsertion);
					});

				PhysScene->AddToComponentMaps(this, PhysHandle);
				if (BodyInstance.bNotifyRigidBodyCollision)
				{
					PhysScene->RegisterForCollisionEvents(this);
				}
			}

		}
	}

}
void UClipmapCollisionComponent::OnDestroyPhysicsState()
{
	Super::OnDestroyPhysicsState();
	if (FPhysScene_Chaos* PhysScene = GetWorld()->GetPhysicsScene())
	{
		FPhysicsActorHandle& ActorHandle = BodyInstance.GetPhysicsActorHandle();
		if (FPhysicsInterface::IsValid(ActorHandle))
		{
			PhysScene->RemoveFromComponentMaps(ActorHandle);
		}
		if (BodyInstance.bNotifyRigidBodyCollision)
		{
			PhysScene->UnRegisterForCollisionEvents(this);
		}
	}
}

void UClipmapCollisionComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UClipmapCollisionComponent::EndPlay(EEndPlayReason::Type reason)
{
	Super::EndPlay(reason);
}
void UClipmapCollisionComponent::OnUnregister()
{
	Super::OnUnregister();
	HeightfieldGeometry = nullptr;
}
