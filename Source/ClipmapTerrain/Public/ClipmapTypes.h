#pragma once

#include "Chaos/HeightField.h"
#include "ClipmapTypes.generated.h"


USTRUCT()
struct FRandomTerrainChunkKey
{
	GENERATED_BODY()
	uint64 Id;
	uint32 Index;

	bool bValid;

	bool operator==(const FRandomTerrainChunkKey& Other) const
	{
		return Index == Other.Index && Id == Other.Id;
	}

	// Friend function to expose GetTypeHash for standard UE containers
	friend uint32 GetTypeHash(const FRandomTerrainChunkKey& StructInstance)
	{
		return HashCombine(GetTypeHash(StructInstance.Id), GetTypeHash(StructInstance.Index));
	}
};

USTRUCT(BlueprintType)
struct FRandomTerrainChunk
{
	GENERATED_BODY()

	UPROPERTY(transient)
	TArray<TObjectPtr<UTexture2D>> Heightmap;

	UPROPERTY(transient)
	TArray<TObjectPtr<UTexture2D>> Normalmap;

	FIntVector2 Key;

	TBitArray<> DirtyLevels;
	TBitArray<> LevelMask;

	TArray<float*> HeightmapBuffers;
	TArray<FFloat16Color*> NormalmapBuffers;
	Chaos::FHeightFieldPtr HeightField;

	bool bGenerating = false;
	bool bValid = false;

	double MinHeight = 0;
	double MaxHeight = 0;
};

struct FUpdateHeightmapRegion
{
	int MipIndex;
	FRHICopyTextureInfo Region;
	UTexture2D* SourceTexture;
	FUpdateHeightmapRegion() {};

	FUpdateHeightmapRegion(UTexture2D* sourceTexture, int mipIndex, uint32 InDestX, uint32 InDestY, int32 InSrcX, int32 InSrcY, uint32 InWidth, uint32 InHeight) :
		MipIndex(mipIndex),
		SourceTexture(sourceTexture)
	{
		Region.Size.X = InWidth;
		Region.Size.Y = InHeight;
		Region.DestPosition.X = InDestX;
		Region.DestPosition.Y = InDestY;
		Region.SourcePosition.X = InSrcX;
		Region.SourcePosition.Y = InSrcY;

		Region.SourceMipIndex = 0;
		Region.DestSliceIndex = mipIndex;
		Region.DestMipIndex = 0;
	}
};