#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ClipmapTypes.h"
#include "ClipmapTerrainActor.generated.h"




UCLASS()
class CLIPMAPTERRAIN_API AClipmapTerrainActor : public AActor
{
	GENERATED_BODY()

	struct FClipmapMeshPiece
	{
		FPrimitiveInstanceId Id;
		FVector LastLocation;

		FClipmapMeshPiece()
		{

		};
		FClipmapMeshPiece(FPrimitiveInstanceId id) :
			Id(id)
		{

		}
		FClipmapMeshPiece(FPrimitiveInstanceId id, const FVector& pos) :
			Id(id),
			LastLocation(pos)
		{

		}
		bool IsValid() const
		{
			return Id.IsValid();
		}

		void Reset()
		{
			Id = FPrimitiveInstanceId();
			LastLocation = FVector::ZeroVector;
		}
	};

	void InitClipmap();
	void GenerateMesh();
	void UpdateClipmap();
	void UpdateWindowTexture();
	FVector GetLocalCameraLocation() const;
public:

	AClipmapTerrainActor();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& event) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; };
#endif

	virtual void Tick(float DeltaTime) override;


public:

	static double Rotations[4];

	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2DArray> WindowTexture;

	UPROPERTY(transient)
	UMaterialInstanceDynamic* ClipmapMaterial = nullptr;


	UPROPERTY(BlueprintReadWrite, EditInstanceOnly)
	TObjectPtr<UMaterialInterface> Material;

	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> CrossMeshInstance;
	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> TileMeshInstance;
	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> FillerMeshInstance;
	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> TrimMeshInstance;
	UPROPERTY(Transient)
	TObjectPtr<UInstancedStaticMeshComponent> SeamMeshInstance;

	bool bClipmapDirty = true;

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Clipmap Settings")
	int ClipmapTileSize = 64;

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Clipmap Settings")
	int ClipmapLevels = 2;

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Terrain Settings")
	double HeightScale = 100;

	//Procedural gen info
	int ChunkSize = 128;

private:

	uint32 CurrentChunkId = 0;


	UPROPERTY(transient)
	TMap<FIntVector2, FRandomTerrainChunkKey> ChunkMap;
	TArray<FRandomTerrainChunk> Chunks;

	//Static mesh instance ID's
	FClipmapMeshPiece CrossInstanceID;
	TArray<FClipmapMeshPiece> TileMap;
	TArray<FClipmapMeshPiece> Fillers;
	TArray<FClipmapMeshPiece> Trims;
	TArray<FClipmapMeshPiece> Seams;

	FVector LastViewGridPosition;
	FVector ViewGridMovement;

	bool bFirstUpdate = true;
	
};