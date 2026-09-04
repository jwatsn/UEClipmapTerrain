#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ClipmapTypes.h"
#include "FastNoise.h"
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
	void UpdateClipmapLevels();
	void UpdateWindowTexture();
	void ChunksToWindow(int level, double xOffset, double yOffset, double x1, double x2, double y1, double y2);
	void GenHeightmap(int x, int y, int level, FRandomTerrainChunk& chunk);
	void UpdateClipmapBounds();
	void EmplaceWindowRegion(UTexture2D* Heightmap, int level, double destX, double destY, int srcX, int srcY, int sizeX, int sizeY);
	FVector GetNormal(double x, double y,double step);
	FVector GetNormalUnsafe(int x, int y,float* buffer);
	FVector GetLocalCameraLocation() const;
	FRandomTerrainChunkKey& GetChunk(int x, int y);
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
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Generation Settings")
	int ChunkSize = 64;
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Generation Settings")
	int Seed = 1337;
	UPROPERTY(transient)
	TArray<FRandomTerrainChunk> Chunks;
	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Generation Settings")
	FString FastNoiseEncodedString = "AwQ=";
private:
	bool bBoundsNeedsUpdate = false;

	double MinHeight = 0;
	double MaxHeight = 0;

	uint32 CurrentChunkId = 0;

	
	UPROPERTY(transient)
	TMap<FIntVector2, FRandomTerrainChunkKey> ChunkMap;
	

	TArray<TPair<FIntVector2, FRandomTerrainChunkKey>> ChunksToUpdate;

	TArray<FUpdateHeightmapRegion> QueuedUpdateRegions;
	//Static mesh instance ID's
	FClipmapMeshPiece CrossInstanceID;
	TArray<FClipmapMeshPiece> TileMap;
	TArray<FClipmapMeshPiece> Fillers;
	TArray<FClipmapMeshPiece> Trims;
	TArray<FClipmapMeshPiece> Seams;

	TObjectPtr<UStaticMesh> CrossMeshSection;
	TObjectPtr<UStaticMesh> TileMeshSection;
	TObjectPtr<UStaticMesh> FillerMeshSection;
	TObjectPtr<UStaticMesh> TrimMeshSection;
	TObjectPtr<UStaticMesh> SeamMeshSection;

	

	FVector LastViewGridPosition;
	FVector ViewGridMovement;

	bool bFirstUpdate = true;

	FastNoise::SmartNode<> NoiseNode;
	
};