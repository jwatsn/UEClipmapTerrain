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

USTRUCT(BlueprintType)
struct FRandomTerrainChunk
{
	GENERATED_BODY()

	UPROPERTY(transient, VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UTexture2D>> Heightmap;

	UPROPERTY(transient, VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<UTexture2D>> Normalmap;

	TArray<float*> HeightmapBuffers;
	TArray<FFloat16Color*> NormalmapBuffers;
	Chaos::FHeightFieldPtr HeightField;

	bool bValid = false;
	bool bGenerating = false;

	double MinHeight = 0;
	double MaxHeight = 0;
};

struct FUpdateHeightmapRegion
{
	int MipIndex;
	FRHICopyTextureInfo Region;
	UTexture2D* SourceTexture;
	UTexture2D* NormalSourceTexture;
	FUpdateHeightmapRegion() {};

	FUpdateHeightmapRegion(UTexture2D* normalSourceTexture,UTexture2D* sourceTexture, int mipIndex, uint32 InDestX, uint32 InDestY, int32 InSrcX, int32 InSrcY, uint32 InWidth, uint32 InHeight) :
		MipIndex(mipIndex),
		NormalSourceTexture(normalSourceTexture),
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