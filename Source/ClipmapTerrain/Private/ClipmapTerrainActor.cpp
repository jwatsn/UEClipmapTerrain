#include "ClipmapTerrainActor.h"
#include "ClipmapMeshHelper.h"
#include "Engine/Texture2DArray.h"

#if WITH_EDITOR
#include "Editor.h"
#include "LevelEditorViewport.h"
#endif

double AClipmapTerrainActor::Rotations[4] = {0,90.0,270.0,180.0};

AClipmapTerrainActor::AClipmapTerrainActor()
{
	CrossMeshInstance = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Cross Mesh Instance"), true);
	TileMeshInstance = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Tile Mesh Instance"), true);
	FillerMeshInstance = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Filler Mesh Instance"), true);
	TrimMeshInstance = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Trim Mesh Instance"), true);
	SeamMeshInstance = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Seam Mesh Instance"), true);

	CrossMeshInstance->SetFlags(RF_Transient);
	TileMeshInstance->SetFlags(RF_Transient);
	FillerMeshInstance->SetFlags(RF_Transient);
	TrimMeshInstance->SetFlags(RF_Transient);
	SeamMeshInstance->SetFlags(RF_Transient);

	CrossMeshInstance->bDisableCollision = true;
	CrossMeshInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CrossMeshInstance->SetCanEverAffectNavigation(false);
	CrossMeshInstance->SetMobility(EComponentMobility::Movable);
	CrossMeshInstance->SetGenerateOverlapEvents(false);


	TileMeshInstance->bDisableCollision = true;
	TileMeshInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TileMeshInstance->SetCanEverAffectNavigation(false);
	TileMeshInstance->SetMobility(EComponentMobility::Movable);
	TileMeshInstance->SetGenerateOverlapEvents(false);


	FillerMeshInstance->bDisableCollision = true;
	FillerMeshInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FillerMeshInstance->SetCanEverAffectNavigation(false);
	FillerMeshInstance->SetMobility(EComponentMobility::Movable);
	FillerMeshInstance->SetGenerateOverlapEvents(false);

	TrimMeshInstance->bDisableCollision = true;
	TrimMeshInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrimMeshInstance->SetCanEverAffectNavigation(false);
	TrimMeshInstance->SetMobility(EComponentMobility::Movable);
	TrimMeshInstance->SetGenerateOverlapEvents(false);

	SeamMeshInstance->bDisableCollision = true;
	SeamMeshInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SeamMeshInstance->SetCanEverAffectNavigation(false);
	SeamMeshInstance->SetReverseCulling(true);
	SeamMeshInstance->SetMobility(EComponentMobility::Movable);
	SeamMeshInstance->SetGenerateOverlapEvents(false);

	SeamMeshInstance->SetNumCustomDataFloats(1);
	TrimMeshInstance->SetNumCustomDataFloats(1);
	FillerMeshInstance->SetNumCustomDataFloats(1);
	TileMeshInstance->SetNumCustomDataFloats(1);
	CrossMeshInstance->SetNumCustomDataFloats(1);

	SeamMeshInstance->SetAbsolute(true, true, true);
	TrimMeshInstance->SetAbsolute(true, true, true);
	FillerMeshInstance->SetAbsolute(true, true, true);
	TileMeshInstance->SetAbsolute(true, true, true);
	CrossMeshInstance->SetAbsolute(true, true, true);

	RootComponent = CreateDefaultSubobject<USceneComponent>("Terrain Root");

	SeamMeshInstance->SetupAttachment(RootComponent);
	TrimMeshInstance->SetupAttachment(RootComponent);
	FillerMeshInstance->SetupAttachment(RootComponent);
	TileMeshInstance->SetupAttachment(RootComponent);
	CrossMeshInstance->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;

	Chunks.Reserve(1024);

}

#if WITH_EDITOR

void AClipmapTerrainActor::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	if (event.GetPropertyName() == "ClipmapTileSize" || event.GetPropertyName() == "ClipmapLevels" || event.GetPropertyName() == "HeightScale" || event.GetPropertyName() == "Seed" || event.GetPropertyName() == "ChunkSize" || event.GetPropertyName() == "FastNoiseEncodedString")
	{
		bClipmapDirty = true;
	}
}

#endif

void AClipmapTerrainActor::GenerateMesh()
{
	CrossMeshSection= ClipmapBuilder::CrossMesh(ClipmapTileSize);
	TileMeshSection = ClipmapBuilder::TileMesh(ClipmapTileSize);
	FillerMeshSection = ClipmapBuilder::FillerMesh(ClipmapTileSize);
	TrimMeshSection = ClipmapBuilder::TrimMesh(ClipmapTileSize);
	SeamMeshSection = ClipmapBuilder::SeamMesh(ClipmapTileSize);

	CrossMeshInstance->SetStaticMesh(CrossMeshSection);
	TileMeshInstance->SetStaticMesh(TileMeshSection);
	FillerMeshInstance->SetStaticMesh(FillerMeshSection);
	TrimMeshInstance->SetStaticMesh(TrimMeshSection);
	SeamMeshInstance->SetStaticMesh(SeamMeshSection);
}
void AClipmapTerrainActor::InitClipmap()
{
	auto encodedCast = StringCast<ANSICHAR>(*FastNoiseEncodedString);
	NoiseNode = FastNoise::NewFromEncodedNodeTree(encodedCast.Get());

	ChunkVisibilityIndex = 0;
	CurrentChunkId = 0;

	MinHeight = 0;
	MaxHeight = 0;

	SeamMeshInstance->ClearInstances();
	TrimMeshInstance->ClearInstances();
	FillerMeshInstance->ClearInstances();
	TileMeshInstance->ClearInstances();
	CrossMeshInstance->ClearInstances();

	CrossInstanceID.Reset();
	TileMap.Reset();
	Fillers.Reset();
	Trims.Reset();
	Seams.Reset();

	ChunksToUpdate.Reset();
	QueuedUpdateRegions.Reset();
	ChunkMap.Reset();
	Chunks.Reset();
	FreeChunkPool.Reset();
	

	TileMap.SetNum(ClipmapLevels * 16);
	Fillers.SetNum(ClipmapLevels);
	Trims.SetNum(ClipmapLevels);
	Seams.SetNum(ClipmapLevels);



	
	UVOffset = FVector::ZeroVector;
	int WindowSize = ClipmapTileSize * 4;

	
	GenerateMesh();

	if (Material)
	{
		ClipmapMaterial = UMaterialInstanceDynamic::Create(Material, this);
	}
	UpdateWindowTexture();
	if (ClipmapMaterial)
	{

		ClipmapMaterial->SetTextureParameterValue("WindowTexture", WindowTexture);
		//ClipmapMaterial->SetTextureParameterValue("NormalWindowTexture", NormalWindowTexture);
		ClipmapMaterial->SetScalarParameterValue("HeightScale", HeightScale * 100.0);
		ClipmapMaterial->SetScalarParameterValue("WindowSize", WindowSize);
		ClipmapMaterial->SetScalarParameterValue("NumLevels", ClipmapLevels);
		CrossMeshInstance->SetMaterial(0, ClipmapMaterial);
		TileMeshInstance->SetMaterial(0, ClipmapMaterial);
		FillerMeshInstance->SetMaterial(0, ClipmapMaterial);
		TrimMeshInstance->SetMaterial(0, ClipmapMaterial);
		SeamMeshInstance->SetMaterial(0, ClipmapMaterial);
	}

	bFirstUpdate = true;
}
void AClipmapTerrainActor::UpdateWindowTexture()
{
	int32 WindowSize = ClipmapTileSize * 4;
	WindowTexture = UTexture2DArray::CreateTransient(WindowSize, WindowSize, ClipmapLevels, PF_R32_FLOAT);
	WindowTexture->SRGB = false;
	WindowTexture->CompressionSettings = TextureCompressionSettings::TC_SingleFloat;
#if WITH_EDITOR
	WindowTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	WindowTexture->AddressX = TextureAddress::TA_Wrap;
	WindowTexture->AddressY = TextureAddress::TA_Wrap;
	WindowTexture->AddressZ = TextureAddress::TA_Clamp;
	WindowTexture->UpdateResource();

}
void AClipmapTerrainActor::UpdateClipmap()
{
	const int TILE_RESOLUTION = ClipmapTileSize;
	const int CLIPMAP_RESOLUTION = TILE_RESOLUTION * 4 + 1;
	const int CLIPMAP_VERT_RESOLUTION = CLIPMAP_RESOLUTION + 1;
	const int NUM_CLIPMAP_LEVELS = ClipmapLevels;
	const int WINDOW_SIZE = ClipmapTileSize * 4;


	FVector ViewPosition = GetLocalCameraLocation();

	FVector ViewGridPosition = ViewPosition.GridSnap(ClipmapTileSize * 100.0);

	if (!bFirstUpdate && ViewGridPosition == LastViewGridPosition)
	{
		return;
	}
	if (ClipmapMaterial)
	{
		ClipmapMaterial->SetVectorParameterValue("Offset", ViewGridPosition);
	}
	if (!bFirstUpdate)
	{
		ViewGridMovement = (ViewGridPosition - LastViewGridPosition)/100.0f;
	}
	else
	{
		ViewGridMovement = FVector(WINDOW_SIZE, WINDOW_SIZE, 0);
	}
	LastViewGridPosition = ViewGridPosition;
	SetActorLocation(ViewGridPosition);
	
	
	FVector SnappedPos = FVector(FMath::Floor(ViewGridPosition.X / 100.0), FMath::Floor(ViewGridPosition.Y / 100.0), 0) * 100.0;
	if(!CrossInstanceID.IsValid())
	{
		CrossInstanceID = FClipmapMeshPiece(CrossMeshInstance->AddInstanceById(FTransform(FRotator::ZeroRotator, SnappedPos), true), SnappedPos);
		CrossMeshInstance->SetCustomDataValueById(CrossInstanceID.Id, 0, 0);
	}
	else
	{
		CrossMeshInstance->UpdateInstanceTransformById(CrossInstanceID.Id, FTransform(FRotator::ZeroRotator, SnappedPos), true, true);
	}
	
	for (int level = 0; level < ClipmapLevels; level++)
	{
		double scale = FMath::Pow(2.0,level);
		SnappedPos = FVector(FMath::Floor(ViewGridPosition.X / scale / 100.0), FMath::Floor(ViewGridPosition.Y / scale / 100.0), 0) * scale * 100.0;
		
		
		const FVector tileSize = FVector(TILE_RESOLUTION * scale, TILE_RESOLUTION * scale, 0) * 100.0;

		const FVector base = FVector(SnappedPos.X, SnappedPos.Y, 0) - tileSize * 2;
		
		for (int y = 0; y < 4; y++)
		{
			for (int x = 0; x < 4; x++)
			{
				if (level != 0 && (x == 1 || x == 2) && (y == 1 || y == 2))
					continue;

				int tileId = (level * 16) + (y * 4 + x);

				const FVector fill = FVector(x >= 2 ? 1 : 0, y >= 2 ? 1 : 0, 0) * scale * 100.0;
				FVector tile_bl = base + FVector(x, y, 0) * tileSize + fill;
				FClipmapMeshPiece& idEntry = TileMap[tileId];
				if (idEntry.IsValid())
				{
					if (idEntry.LastLocation != tile_bl)
					{
						TileMeshInstance->UpdateInstanceTransformById(idEntry.Id, FTransform(FRotator::ZeroRotator, tile_bl, FVector(scale, scale, 1.0f)), true, true);
						idEntry.LastLocation = tile_bl;
					}
				}
				else
				{
					idEntry = FClipmapMeshPiece(TileMeshInstance->AddInstanceById(FTransform(FRotator::ZeroRotator, tile_bl, FVector(scale, scale, 1.0f)), true), tile_bl);
					TileMeshInstance->SetCustomDataValueById(idEntry.Id, 0, level);
				}
			}
		}
		FClipmapMeshPiece& fillerIdEntry = Fillers[level];
		if (fillerIdEntry.IsValid())
		{
			if (fillerIdEntry.LastLocation != SnappedPos)
			{
				FillerMeshInstance->UpdateInstanceTransformById(fillerIdEntry.Id, FTransform(FRotator::ZeroRotator, SnappedPos, FVector(scale, scale, 1.0f)), true, true);
				fillerIdEntry.LastLocation = SnappedPos;
			}
		}
		else
		{
			fillerIdEntry = FClipmapMeshPiece(FillerMeshInstance->AddInstanceById(FTransform(FRotator::ZeroRotator, SnappedPos, FVector(scale, scale, 1.0f)), true), SnappedPos);
			FillerMeshInstance->SetCustomDataValueById(fillerIdEntry.Id, 0, level);

		}
		if (level != NUM_CLIPMAP_LEVELS - 1)
		{
			const int next_scale = scale * 2;
			const FVector s = ((ViewGridPosition) / next_scale / 100.0);

			const FVector next_snapped_position = FVector(FMath::Floor(s.X), FMath::Floor(s.Y), 0.0f) * next_scale * 100;
			FVector tile_centre = SnappedPos + FVector(scale * 100.0 * 0.5, scale * 100.0 * 0.5, 0);
			const FVector d = (ViewGridPosition)-next_snapped_position;
			int r = 0;
			r |= d.X >= scale * 100 ? 0 : 2;
			r |= d.Y >= scale * 100 ? 0 : 1;
			const int offset = CLIPMAP_VERT_RESOLUTION * scale * 100;

			FVector next_base = next_snapped_position - FVector(TILE_RESOLUTION * next_scale, TILE_RESOLUTION * next_scale, 0) * 100.0;
			FClipmapMeshPiece& trimIdEntry = Trims[level];
			if (trimIdEntry.IsValid())
			{
				if (trimIdEntry.LastLocation != tile_centre)
				{
					TrimMeshInstance->UpdateInstanceTransformById(trimIdEntry.Id, FTransform(FRotator(0.0f, -Rotations[r], 0.0f), tile_centre, FVector(scale, scale, 1.0f)), true, true);
					trimIdEntry.LastLocation = tile_centre;
				}
			}
			else
			{
				trimIdEntry = FClipmapMeshPiece(TrimMeshInstance->AddInstanceById(FTransform(FRotator(0.0f, -Rotations[r], 0.0f), tile_centre, FVector(scale, scale, 1.0f)), true), tile_centre);
				TrimMeshInstance->SetCustomDataValueById(trimIdEntry.Id, 0, level);
			}
			
			FClipmapMeshPiece& seamIdEntry = Seams[level];
			if (seamIdEntry.IsValid())
			{
				if (seamIdEntry.LastLocation != next_base)
				{
					SeamMeshInstance->UpdateInstanceTransformById(seamIdEntry.Id, FTransform(FRotator::ZeroRotator, next_base, FVector(scale, scale, 1.0f)), true, true);
					seamIdEntry.LastLocation = next_base;
				}
			}
			else
			{
				seamIdEntry = FClipmapMeshPiece(SeamMeshInstance->AddInstanceById(FTransform(FRotator::ZeroRotator, next_base, FVector(scale, scale, 1.0f)), true), next_base);
				SeamMeshInstance->SetCustomDataValueById(seamIdEntry.Id, 0, level);
			}
			
		}
	}

	UpdateClipmapLevels();

	
	if (ClipmapMaterial)
	{
		ClipmapMaterial->SetVectorParameterValue("UVOffset", UVOffset/WINDOW_SIZE);
	}

	bFirstUpdate = false;
}
void GetSingleNormal(float L, float R, float U, float D,FVector& outNormal)
{
	// 2. Calculate the Gradient (Slope)
	// The "2.0f" represents the distance between x-1 and x+1 (2 grid units)
	float dzdx = (R - L) / 2.0f;
	float dzdy = (D - U) / 2.0f;

	// 3. Construct the Normal
	// In Unreal (Z is up), the normal is (-dz/dx, -dz/dy, 1.0)
	outNormal += FVector(-dzdx, -dzdy, 1.0f).GetSafeNormal();

}
FVector AClipmapTerrainActor::GetNormal(double x, double y,double step)
{
	float L = NoiseNode->GenSingle2D(x - step, y,Seed) * HeightScale;
	float U = NoiseNode->GenSingle2D(x, y+step, Seed) * HeightScale;
	float R = NoiseNode->GenSingle2D(x + step, y, Seed) * HeightScale;
	float D = NoiseNode->GenSingle2D(x, y-step, Seed) * HeightScale;

	return FVector(-((R - L) / 2.0f), -((D - U) / 2.0f), 1.0f);
}
FVector AClipmapTerrainActor::GetNormalUnsafe(int x, int y, float* buffer)
{

	float L = buffer[y * ChunkSize + (x - 1)] * HeightScale;
	float R = buffer[y * ChunkSize + (x + 1)] * HeightScale;
	float U = buffer[(y+1) * ChunkSize + x] * HeightScale;
	float D = buffer[(y - 1) * ChunkSize + x] * HeightScale;
	return FVector(-((R - L) / 2.0f), -((D - U) / 2.0f), 1.0f);
}
void AClipmapTerrainActor::GenHeightmap(int x, int y, int level, FRandomTerrainChunk& chunk)
{
	UTexture2D* texture = chunk.Heightmap[level];
	double scalar = FMath::Pow(2.0, level);

	float* pixels = chunk.HeightmapBuffers[level];

	double startX = (x) * ChunkSize * scalar;
	double startY = (y) * ChunkSize * scalar;




	NoiseNode->GenUniformGrid2D(pixels, startX, startY, ChunkSize, ChunkSize, scalar, scalar, Seed);
	

	
	chunk.bGenerating = false;
	chunk.LevelMask[level] = true;
	if (level == 0)
	{
		for (int i = 0; i < ChunkSize * ChunkSize; i++)
		{
			const float h = pixels[i];
			if (h < chunk.MinHeight)
			{
				chunk.MinHeight = h;
			}
			if (h > chunk.MaxHeight)
			{
				chunk.MaxHeight = h;
			}
		}
	}
}
void AClipmapTerrainActor::UpdateClipmapBounds()
{
	FVector boundsExtension = FVector(0, 0, MaxHeight * HeightScale * 200.0);

	FVector negBoundsExtension = MinHeight < 0 ? FVector(0, 0, -MinHeight * HeightScale * 100.0) : FVector::ZeroVector;
	CrossMeshSection->SetPositiveBoundsExtension(boundsExtension);
	TileMeshSection->SetPositiveBoundsExtension(boundsExtension);
	FillerMeshSection->SetPositiveBoundsExtension(boundsExtension);
	TrimMeshSection->SetPositiveBoundsExtension(boundsExtension);
	SeamMeshSection->SetPositiveBoundsExtension(boundsExtension);

	/*CrossMeshSection->SetNegativeBoundsExtension(negBoundsExtension);
	TileMeshSection->SetNegativeBoundsExtension(negBoundsExtension);
	FillerMeshSection->SetNegativeBoundsExtension(negBoundsExtension);
	TrimMeshSection->SetNegativeBoundsExtension(negBoundsExtension);
	SeamMeshSection->SetNegativeBoundsExtension(negBoundsExtension);*/

	CrossMeshSection->CalculateExtendedBounds();
	TileMeshSection->CalculateExtendedBounds();
	FillerMeshSection->CalculateExtendedBounds();
	TrimMeshSection->CalculateExtendedBounds();
	SeamMeshSection->CalculateExtendedBounds();

	CrossMeshInstance->UpdateBounds();
	TileMeshInstance->UpdateBounds();
	FillerMeshInstance->UpdateBounds();
	TrimMeshInstance->UpdateBounds();
	SeamMeshInstance->UpdateBounds();
}
void AClipmapTerrainActor::EmplaceWindowRegion(UTexture2D* Heightmap, int level,double destX, double destY, int srcX, int srcY, int sizeX, int sizeY)
{
	struct FSegment
	{
		FSegment(int inDest, int inSrc, int inSize) :
			dest(inDest),
			src(inSrc),
			size(inSize)
		{

		};
		int dest; int src; int size;
	};

	auto BuildSegments = [this,WindowSize=ClipmapTileSize*4](double dest, int src, int size) -> TArray<FSegment>
		{
			TArray<FSegment> segs;
			double d = dest;
			int s = src;
			int remaining = size;

			while (remaining > 0)
			{
				double wrappedStart = d - FMath::Floor(d / WindowSize) * WindowSize;


				int availableInWindow = WindowSize - FMath::FloorToInt(wrappedStart);
				int thisSize = FMath::Min(remaining, availableInWindow);

				segs.Add(FSegment(FMath::FloorToInt(wrappedStart), s, thisSize));

				d += thisSize;
				s += thisSize;
				remaining -= thisSize;
			}
			return segs;
		};

	TArray<FSegment> xSegs = BuildSegments(destX, srcX, sizeX);
	TArray<FSegment> ySegs = BuildSegments(destY, srcY, sizeY);

	for (const FSegment& xSeg : xSegs)
	{
		for (const FSegment& ySeg : ySegs)
		{
			QueuedUpdateRegions.Emplace(Heightmap, level, xSeg.dest, ySeg.dest, xSeg.src, ySeg.src, xSeg.size, ySeg.size);
		}
	}
}
void AClipmapTerrainActor::ChunksToWindow(int level, double xOffset, double yOffset, double x1, double x2, double y1, double y2)
{
	int startX = FMath::FloorToInt(x1 / ChunkSize);
	int endX = FMath::CeilToInt(x2 / ChunkSize);
	int startY = FMath::FloorToInt(y1 / ChunkSize);
	int endY = FMath::CeilToInt(y2 / ChunkSize);
	int windowY = 0;
	for (int y = startY; y < endY; y++)
	{
		int windowX = 0;
		int rowCopySizeY = 0;
		for (int x = startX; x < endX; x++)
		{
			FRandomTerrainChunkKey& key = GetChunk(x, y);
			FRandomTerrainChunk& chunk = Chunks[key.Index];

			if (!chunk.LevelMask[level] && !chunk.bGenerating)
			{
				chunk.DirtyLevels[level] = true;
				ChunksToUpdate.Emplace(FIntVector2(x,y),key);
				chunk.bGenerating = true;
			}
			else if (!chunk.LevelMask[level] && chunk.bGenerating)
			{
				chunk.DirtyLevels[level] = true;
			}
			
			double chunkWorldX = x * ChunkSize;
			double chunkWorldY = y * ChunkSize;

			double localStartX = FMath::Max(0.0, x1 - chunkWorldX);
			double localEndX = FMath::Max(FMath::Min((double)ChunkSize, x2 - chunkWorldX), 0);
			double localStartY = FMath::Max(0.0, y1 - chunkWorldY);
			double localEndY = FMath::Max(FMath::Min((double)ChunkSize, y2 - chunkWorldY), 0);

			int copyStartX = FMath::FloorToInt(localStartX);
			int copyEndX = FMath::FloorToInt(localEndX);
			int copyStartY = FMath::FloorToInt(localStartY);
			int copyEndY = FMath::FloorToInt(localEndY);

			int copySizeX = copyEndX - copyStartX;
			int copySizeY = copyEndY - copyStartY;
			if (copySizeX <= 0 || copySizeY <= 0)
			{
				continue;
			}
			chunk.DirtyLevels[level] = true;
			rowCopySizeY = copySizeY;
			EmplaceWindowRegion(chunk.Heightmap[level], level, xOffset + windowX, yOffset + windowY, copyStartX, copyStartY, copySizeX, copySizeY);
			windowX += copySizeX;
		}
		windowY += rowCopySizeY;
	}
}
void AClipmapTerrainActor::UpdateClipmapLevels()
{
	int windowSize = ClipmapTileSize * 4;
	int windowSizeHalf = windowSize / 2;
	
	const FVector& unscaledDiff = ViewGridMovement;

	bool bReset = false;
	for (int i = 0; i < ClipmapLevels; i++)
	{
		
		double scalar = FMath::Pow(2.0, i);
		FVector2D pos = FVector2D(LastViewGridPosition.X, LastViewGridPosition.Y) / 100.0 / scalar;
		
		if (unscaledDiff.IsNearlyZero())
		{
			continue;
		}
		if (FMath::Abs(unscaledDiff.X) < windowSize && FMath::Abs(unscaledDiff.Y) < windowSize)
		{
			
			FVector diff = unscaledDiff / scalar;
			

			bool ret = false;
			if (diff.X > 0)
			{

				double yOffset = UVOffset.Y/scalar + diff.Y;
				double xOffset = UVOffset.X/scalar;


				int xOffsetInt = FMath::FloorToInt(xOffset / windowSize);
				double xOffsetWrapped = xOffset - xOffsetInt * windowSize;

				int yOffsetInt = FMath::FloorToInt(yOffset / windowSize);
				double yOffsetWrapped = yOffset - yOffsetInt * windowSize;

				if (yOffsetWrapped != 0)
				{
					{
						double x1 = pos.X + windowSizeHalf - diff.X;
						double x2 = pos.X + windowSizeHalf;
						double y1 = pos.Y - windowSizeHalf;
						double y2 = pos.Y + windowSizeHalf - yOffsetWrapped;
						ChunksToWindow(i, xOffsetWrapped, yOffsetWrapped, x1, x2, y1, y2);
					}
					{
						double x1 = pos.X + windowSizeHalf - diff.X;
						double x2 = pos.X + windowSizeHalf;
						double y1 = pos.Y + windowSizeHalf - yOffsetWrapped;
						double y2 = pos.Y + windowSizeHalf;
						ChunksToWindow(i, xOffsetWrapped, 0, x1, x2, y1, y2);
					}
				}
				else
				{
					double x1 = pos.X + windowSizeHalf - diff.X;
					double x2 = pos.X + windowSizeHalf;
					double y1 = pos.Y - windowSizeHalf;
					double y2 = pos.Y + windowSizeHalf;



					ChunksToWindow(i, xOffsetWrapped, 0, x1, x2, y1, y2);

				}
				

				ret = true;
			}
			else if (diff.X < 0)
			{
				double yOffset = UVOffset.Y/scalar + diff.Y;
				double xOffset = UVOffset.X/scalar + diff.X;
				
				int xOffsetInt = FMath::FloorToInt(xOffset / windowSize);
				double xOffsetWrapped = xOffset - xOffsetInt * windowSize;

				int yOffsetInt = FMath::FloorToInt(yOffset / windowSize);
				double yOffsetWrapped = yOffset - yOffsetInt * windowSize;
				if (yOffsetWrapped != 0)
				{
					{
						double x1 = pos.X - windowSizeHalf;
						double x2 = pos.X - windowSizeHalf - diff.X;
						double y1 = pos.Y - windowSizeHalf;
						double y2 = pos.Y + windowSizeHalf - yOffsetWrapped;
						ChunksToWindow(i, xOffsetWrapped, yOffsetWrapped, x1, x2, y1, y2);
					}
					{
						double x1 = pos.X - windowSizeHalf;
						double x2 = pos.X - windowSizeHalf - diff.X;
						double y1 = pos.Y + windowSizeHalf - yOffsetWrapped;
						double y2 = pos.Y + windowSizeHalf;
						ChunksToWindow(i, xOffsetWrapped, 0, x1, x2, y1, y2);
					}
				}
				else
				{

					double x1 = pos.X - windowSizeHalf;
					double x2 = pos.X - windowSizeHalf - diff.X;
					double y1 = pos.Y - windowSizeHalf;
					double y2 = pos.Y + windowSizeHalf;


					ChunksToWindow(i, xOffsetWrapped, 0, x1, x2, y1, y2);
				}

				ret = true;
			}
			
			if (diff.Y > 0)
			{

				double yOffset = UVOffset.Y/scalar;
				double xOffset = UVOffset.X/scalar + diff.X;

				int xOffsetInt = FMath::FloorToInt(xOffset / windowSize);
				double xOffsetWrapped = xOffset - xOffsetInt * windowSize;

				int yOffsetInt = FMath::FloorToInt(yOffset / windowSize);
				double yOffsetWrapped = yOffset - yOffsetInt * windowSize;


				if (xOffsetWrapped != 0)
				{

					{
						double x1 = pos.X - windowSizeHalf;
						double x2 = pos.X + windowSizeHalf - xOffsetWrapped;
						double y1 = pos.Y + windowSizeHalf - diff.Y;
						double y2 = pos.Y + windowSizeHalf;
						ChunksToWindow(i, xOffsetWrapped, yOffsetWrapped, x1, x2, y1, y2);
					}

					{
						double x1 = pos.X + windowSizeHalf - xOffsetWrapped;
						double x2 = pos.X + windowSizeHalf;
						double y1 = pos.Y + windowSizeHalf - diff.Y;
						double y2 = pos.Y + windowSizeHalf;
						ChunksToWindow(i, 0, yOffsetWrapped, x1, x2, y1, y2);

					}
				}
				else
				{
					double x1 = pos.X - windowSizeHalf;
					double x2 = pos.X + windowSizeHalf;
					double y1 = pos.Y + windowSizeHalf - diff.Y;
					double y2 = pos.Y + windowSizeHalf;

					ChunksToWindow(i, 0, yOffsetWrapped, x1, x2, y1, y2);


				}
				ret = true;
				
			}
			else if (diff.Y < 0)
			{


				
				double yOffset = UVOffset.Y/scalar + diff.Y;
				double xOffset = UVOffset.X/scalar + diff.X;

				int xOffsetInt = FMath::FloorToInt(xOffset / windowSize);
				double xOffsetWrapped = xOffset - xOffsetInt * windowSize;

				int yOffsetInt = FMath::FloorToInt(yOffset / windowSize);
				double yOffsetWrapped = yOffset - yOffsetInt * windowSize;

				if (xOffsetWrapped != 0)
				{

					{
						double x1 = pos.X - windowSizeHalf;
						double x2 = pos.X + windowSizeHalf - xOffsetWrapped;
						double y1 = pos.Y - windowSizeHalf;
						double y2 = pos.Y - windowSizeHalf - diff.Y;
						ChunksToWindow(i, xOffsetWrapped, yOffsetWrapped, x1, x2, y1, y2);
					}

					{
						double x1 = pos.X + windowSizeHalf - xOffsetWrapped;
						double x2 = pos.X + windowSizeHalf;
						double y1 = pos.Y - windowSizeHalf;
						double y2 = pos.Y - windowSizeHalf - diff.Y;
						ChunksToWindow(i, 0, yOffsetWrapped, x1, x2, y1, y2);

					}
				}
				else
				{
					double x1 = pos.X - windowSizeHalf;
					double x2 = pos.X + windowSizeHalf;
					double y1 = pos.Y - windowSizeHalf;
					double y2 = pos.Y - windowSizeHalf - diff.Y;

					ChunksToWindow(i, 0, yOffsetWrapped, x1, x2, y1, y2);


				}
				ret = true;
				
			}
			if (ret)
				continue;
		}
		if (i == 0)
		{
			UVOffset.X = 0;
			UVOffset.Y = 0;
			bReset = true;
		}
		double x1 = pos.X - windowSizeHalf;
		double x2 = pos.X + windowSizeHalf;
		double y1 = pos.Y - windowSizeHalf;
		double y2 = pos.Y + windowSizeHalf;
		ChunksToWindow(i, 0, 0, x1, x2, y1, y2);
	}
	if (!bReset)
	{
		UVOffset += unscaledDiff;
	}
}
void AClipmapTerrainActor::UpdateVisibleChunks()
{
	uint32 numChunks = Chunks.Num();
	if (numChunks == 0)
	{
		return;
	}
	
	int loops = (loops = numChunks) > VisChecksPerUpdate ? VisChecksPerUpdate : loops;
	FVector2D pos = FVector2D(FMath::Floor(LastViewGridPosition.X / 100.0 / ChunkSize), FMath::Floor(LastViewGridPosition.Y / 100.0 / ChunkSize));

	for (int i = 0; i < loops; i++)
	{
		
		FRandomTerrainChunk& chunk = Chunks[ChunkVisibilityIndex];
		uint32 prevIndex = ChunkVisibilityIndex;
		ChunkVisibilityIndex = (ChunkVisibilityIndex += 1) < numChunks ? ChunkVisibilityIndex : 0;
		if (!chunk.bValid)
		{
			continue;
		}
		
		bool removeChunk = true;
		for (int l = 0; l < ClipmapLevels; l++)
		{
			if (!chunk.LevelMask[l])
			{
				
				continue;
			}
			double scalar = FMath::Pow(2.0, l);
			FVector2D scaledPos = pos / scalar;
			FVector2D diff = FVector2D(chunk.Key) - scaledPos;
			if (diff.SquaredLength() <= ViewDistanceSquared)
			{
				removeChunk = false;
				break;
			}
		}
		if (removeChunk)
		{
			FreeChunkPool.Add(prevIndex);
			ChunkMap.Remove(chunk.Key);
			chunk.bValid = false;
		}
		
	}
}
void AClipmapTerrainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bClipmapDirty)
	{
		bClipmapDirty = false;
		InitClipmap();
	}

	UpdateClipmap();

	if (!ChunksToUpdate.IsEmpty())
	{
		for (int i = 0; i < ChunksToUpdate.Num(); i++)
		{
			auto& chunkInfo = ChunksToUpdate[i];

			FRandomTerrainChunk& chunk = Chunks[chunkInfo.Value.Index];

			chunk.HeightmapBuffers.SetNum(ClipmapLevels);
			//chunk.NormalmapBuffers.SetNum(ClipmapLevels);
			for (int level = 0; level < ClipmapLevels; level++)
			{
				if (!chunk.DirtyLevels[level])
				{
					continue;
				}
				chunk.HeightmapBuffers[level] = (float*)chunk.Heightmap[level]->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				//chunk.NormalmapBuffers[level] = (FFloat16Color*)chunk.Normalmap[level]->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			}
		}
		ParallelFor(ChunksToUpdate.Num(), [&](int i)
			{
				auto& chunkInfo = ChunksToUpdate[i];
				FRandomTerrainChunk& chunk = Chunks[chunkInfo.Value.Index];
				int x = chunkInfo.Key.X;
				int y = chunkInfo.Key.Y;
				for (int level = 0; level < ClipmapLevels; level++)
				{
					if (!chunk.DirtyLevels[level])
					{
						continue;
					}
					GenHeightmap(x, y, level, chunk);
				}
			});
		for (int i = 0; i < ChunksToUpdate.Num(); i++)
		{
			auto& chunkInfo = ChunksToUpdate[i];
			FRandomTerrainChunk& chunk = Chunks[chunkInfo.Value.Index];
			for (int level = 0; level < ClipmapLevels; level++)
			{
				if (!chunk.DirtyLevels[level])
				{
					continue;
				}
				chunk.Heightmap[level]->GetPlatformData()->Mips[0].BulkData.Unlock();
				chunk.Heightmap[level]->UpdateResource();
				//chunk.Normalmap[level]->GetPlatformData()->Mips[0].BulkData.Unlock();
				//chunk.Normalmap[level]->UpdateResource();
				chunk.HeightmapBuffers.Reset();
			}
			if (chunk.MinHeight < MinHeight)
			{
				bBoundsNeedsUpdate = true;
				MinHeight = chunk.MinHeight;
			}
			if (chunk.MaxHeight > MaxHeight)
			{
				bBoundsNeedsUpdate = true;
				MaxHeight = chunk.MaxHeight;
			}
			chunk.DirtyLevels.Init(false, ClipmapLevels);
		}
		ChunksToUpdate.Reset();
	}

	if (bBoundsNeedsUpdate)
	{
		bBoundsNeedsUpdate = false;
		UpdateClipmapBounds();

	}

	if (!QueuedUpdateRegions.IsEmpty())
	{

		ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)
			([Regions = MoveTemp(QueuedUpdateRegions), windowTexture = WindowTexture](FRHICommandList& RHICmdList)
				{
					FTextureRHIRef WindowTexture2DRHI = windowTexture->GetResource()->TextureRHI;
					for (const FUpdateHeightmapRegion& Region : Regions)
					{
						if (!Region.SourceTexture->GetResource())
						{
							continue;
						}
						RHICmdList.CopyTexture(Region.SourceTexture->GetResource()->TextureRHI, windowTexture->GetResource()->TextureRHI, Region.Region);
						//RHICmdList.CopyTexture(Region.NormalSourceTexture->GetResource()->TextureRHI, normalWindowTexture->GetResource()->TextureRHI, Region.Region);
					}




				});

	}

	UpdateVisibleChunks();
}

FVector AClipmapTerrainActor::GetLocalCameraLocation() const
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* controller = Iterator->Get();
		if (controller->IsLocalController())
		{
			if (APlayerCameraManager* cameraManager = controller->PlayerCameraManager)
			{
				return cameraManager->GetCameraLocation();
			}
		}
	}
#if WITH_EDITOR
	UWorld* world = GetWorld();
	if (!world)
	{
		return FVector::ZeroVector;
	}
	if (world->IsEditorWorld() && GCurrentLevelEditingViewportClient)
	{
		FViewportCameraTransform& ViewTransform = GCurrentLevelEditingViewportClient->GetViewTransform();
		return ViewTransform.GetLocation();
	}
#endif

	return GetActorLocation();
}

FRandomTerrainChunkKey& AClipmapTerrainActor::GetChunk(int x, int y)
{
	FRandomTerrainChunkKey& ret = ChunkMap.FindOrAdd(FIntVector2(x, y));

	if (!ret.bValid)
	{
		ret.Id = CurrentChunkId;
		CurrentChunkId++;
		bool usePoolId = !FreeChunkPool.IsEmpty();
		if (usePoolId)
		{
			ret.Index = FreeChunkPool.Pop();
		}
		else
		{
			ret.Index = Chunks.Num();
		}
		FRandomTerrainChunk& newChunk = usePoolId ? Chunks[ret.Index] : Chunks.AddDefaulted_GetRef();
		newChunk.bValid = true;
		newChunk.bGenerating = false;
		newChunk.Heightmap.SetNum(ClipmapLevels);
		newChunk.Key.X = x;
		newChunk.Key.Y = y;
		newChunk.DirtyLevels.Init(false,ClipmapLevels);
		newChunk.LevelMask.Init(false, ClipmapLevels);

		for (int i = 0; i < ClipmapLevels; i++)
		{
			if (!newChunk.Heightmap[i])
			{
				newChunk.Heightmap[i] = UTexture2D::CreateTransient(ChunkSize, ChunkSize, EPixelFormat::PF_R32_FLOAT);
				newChunk.Heightmap[i]->UpdateResource();
			}
		}
		
		ret.bValid = true;
	}
	return ret;
}