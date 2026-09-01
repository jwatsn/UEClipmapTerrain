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
};

USTRUCT()
struct FRandomTerrainChunk
{
	GENERATED_BODY()

	UPROPERTY(transient)
	TArray<TObjectPtr<UTexture2D>> Heightmap;

	Chaos::FHeightFieldPtr HeightField;

	bool bLevelMaskInit = false;
	TBitArray<> LevelMask;
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

		Region.SourceMipIndex = mipIndex;
		Region.DestSliceIndex = mipIndex;
		Region.DestMipIndex = 0;
	}
};