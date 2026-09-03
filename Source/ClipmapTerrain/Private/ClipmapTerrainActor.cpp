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

	if (event.GetPropertyName() == "TileSize" || event.GetPropertyName() == "ClipmapLevels")
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

	TileMap.SetNum(ClipmapLevels * 16);
	Fillers.SetNum(ClipmapLevels);
	Trims.SetNum(ClipmapLevels);
	Seams.SetNum(ClipmapLevels);


	GenerateMesh();

	if (Material)
	{
		ClipmapMaterial = UMaterialInstanceDynamic::Create(Material, this);
	}
	UpdateWindowTexture();
	if (ClipmapMaterial)
	{

		ClipmapMaterial->SetTextureParameterValue("WindowTexture", WindowTexture);
		ClipmapMaterial->SetTextureParameterValue("NormalWindowTexture", NormalWindowTexture);
		ClipmapMaterial->SetScalarParameterValue("HeightScale", HeightScale * 100.0);
		ClipmapMaterial->SetScalarParameterValue("WindowSize", ClipmapTileSize * 4);
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

	NormalWindowTexture = UTexture2DArray::CreateTransient(WindowSize, WindowSize, ClipmapLevels, PF_FloatRGBA);
	NormalWindowTexture->SRGB = false;
	NormalWindowTexture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
#if WITH_EDITOR
	NormalWindowTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
#endif
	NormalWindowTexture->AddressX = TextureAddress::TA_Wrap;
	NormalWindowTexture->AddressY = TextureAddress::TA_Wrap;
	NormalWindowTexture->AddressZ = TextureAddress::TA_Clamp;
	NormalWindowTexture->UpdateResource();
}
void AClipmapTerrainActor::UpdateClipmap()
{
	const int TILE_RESOLUTION = ClipmapTileSize;
	const int CLIPMAP_RESOLUTION = TILE_RESOLUTION * 4 + 1;
	const int CLIPMAP_VERT_RESOLUTION = CLIPMAP_RESOLUTION + 1;
	const int NUM_CLIPMAP_LEVELS = ClipmapLevels;

	FVector ViewPosition = GetLocalCameraLocation();

	FVector ViewGridPosition = FVector(FMath::Floor(ViewPosition.X / 100.0 / ClipmapTileSize), FMath::Floor(ViewPosition.Y / 100.0 / ClipmapTileSize), FMath::Floor(ViewPosition.Z / 100.0 / ClipmapTileSize)) * ClipmapTileSize * 100.0;
	//ViewGridPosition += FVector(ClipmapTileSize * 50, ClipmapTileSize * 50, 0);
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
		ViewGridMovement = ViewGridPosition - LastViewGridPosition;
	}
	LastViewGridPosition = ViewGridPosition;
	
	bFirstUpdate = false;
	UpdateClipmapLevels();
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
		int scale = 1 << level;
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
	FFloat16Color* normals = chunk.NormalmapBuffers[level];
	double startX = (x) * ChunkSize * scalar;
	double startY = (y) * ChunkSize * scalar;

	// Offset outward by half the footprint at this level, so the
	// sampled region stays centered on the same point at every level


	NoiseNode->GenUniformGrid2D(pixels, startX, startY, ChunkSize, ChunkSize, scalar, scalar, Seed);
	for (int i = 1; i < ChunkSize-1; i++)
	{
		for (int t = 0; t < 2; t++)
		{
			int leftId = i * ChunkSize + t;
			int rightId = i * ChunkSize + (ChunkSize - 1 - t);
			{
				FVector normal = FVector::ZeroVector;
				int nY = i;
				int nX = t;
				for (int oX = 0; oX < 3; oX++)
				{
					for (int oY = 0; oY < 3; oY++)
					{
						double cX = startX + (nX * scalar) + (oX - 1) * scalar;
						double cY = startY + (nY * scalar) + (oY - 1) * scalar;
						normal += GetNormal(cX, cY, scalar).GetSafeNormal();
					}
				}
				FFloat16Color& outColor = normals[leftId];
				normal /= (3.0 * 3.0);
				normal.Normalize();
				outColor.R = 0.5f + normal.X * 0.5f; //(normalSum.X * 127) + 128;
				outColor.G = 0.5f + normal.Y * 0.5f;//(normalSum.Y * 127) + 128;
				outColor.B = 0.5f + normal.Z * 0.5f;//(normalSum.Z * 127) + 128;
				outColor.A = 1;
			}
			{
				FVector normal = FVector::ZeroVector;
				int nY = i;
				int nX = ChunkSize - 1 - t;
				for (int oX = 0; oX < 3; oX++)
				{
					for (int oY = 0; oY < 3; oY++)
					{
						double cX = startX + (nX * scalar) + (oX - 1) * scalar;
						double cY = startY + (nY * scalar) + (oY - 1) * scalar;
						normal += GetNormal(cX, cY, scalar).GetSafeNormal();
					}
				}
				FFloat16Color& outColor = normals[rightId];
				normal /= (3.0 * 3.0);
				normal.Normalize();
				outColor.R = 0.5f + normal.X * 0.5f; //(normalSum.X * 127) + 128;
				outColor.G = 0.5f + normal.Y * 0.5f;//(normalSum.Y * 127) + 128;
				outColor.B = 0.5f + normal.Z * 0.5f;//(normalSum.Z * 127) + 128;
				outColor.A = 1;
			}
		}
	}
	for (int i = 0; i < ChunkSize; i++)
	{
		for (int t = 0; t < 2; t++)
		{
		int topId = (ChunkSize - 1 - t) * ChunkSize + i;
		
		

			{
				FVector normal = FVector::ZeroVector;
				int nY = ChunkSize - 1 - t;
				int nX = i;
				for (int oX = 0; oX < 3; oX++)
				{
					for (int oY = 0; oY < 3; oY++)
					{
						double cX = startX + (nX * scalar) + (oX - 1) * scalar;
						double cY = startY + (nY * scalar) + (oY - 1) * scalar;
						normal += GetNormal(cX, cY, scalar).GetSafeNormal();
					}
				}
				FFloat16Color& outColor = normals[topId];
				normal /= (3.0 * 3.0);
				normal.Normalize();
				outColor.R = 0.5f + normal.X * 0.5f; //(normalSum.X * 127) + 128;
				outColor.G = 0.5f + normal.Y * 0.5f;//(normalSum.Y * 127) + 128;
				outColor.B = 0.5f + normal.Z * 0.5f;//(normalSum.Z * 127) + 128;
				outColor.A = 1;
			}
			{
				FVector normal = FVector::ZeroVector;
				int nY = t;
				int nX = i;
				for (int oX = 0; oX < 3; oX++)
				{
					for (int oY = 0; oY < 3; oY++)
					{
						double cX = startX + (nX * scalar) + (oX - 1) * scalar;
						double cY = startY + (nY * scalar) + (oY - 1) * scalar;
						normal += GetNormal(cX, cY, scalar).GetSafeNormal();
					}
				}
				FFloat16Color& outColor = normals[t*ChunkSize+nX];
				normal /= (3.0 * 3.0);
				normal.Normalize();
				outColor.R = 0.5f + normal.X * 0.5f; //(normalSum.X * 127) + 128;
				outColor.G = 0.5f + normal.Y * 0.5f;//(normalSum.Y * 127) + 128;
				outColor.B = 0.5f + normal.Z * 0.5f;//(normalSum.Z * 127) + 128;
				outColor.A = 1;
			}
		}
	}

	for (int nY = 2; nY < ChunkSize-2; nY++)
	{
		for (int nX = 2; nX < ChunkSize-2; nX++)
		{
			FFloat16Color& outColor = normals[nY * ChunkSize + nX];
			FVector normal = FVector::ZeroVector;// = GetNormal(startX + nX * scalar, startY + nY * scalar, scalar);
			for (int oX = 0; oX < 3; oX++)
			{
				for (int oY = 0; oY < 3; oY++)
				{
					int cX = nX + oX - 1;
					int cY = nY + oY - 1;
					normal += GetNormalUnsafe(cX, cY, pixels).GetSafeNormal();
				}
			}
			normal /= (3.0 * 3.0);
			normal.Normalize();
			outColor.R = 0.5f + normal.X * 0.5f; //(normalSum.X * 127) + 128;
			outColor.G = 0.5f + normal.Y * 0.5f;//(normalSum.Y * 127) + 128;
			outColor.B = 0.5f + normal.Z * 0.5f;//(normalSum.Z * 127) + 128;
			outColor.A = 1;
		}
	}

	chunk.bValid = true;
	chunk.bGenerating = false;

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
	FVector boundsExtension = FVector(0, 0, MaxHeight * HeightScale * 100.0);
	FVector negBoundsExtension = MinHeight < 0 ? FVector(0, 0, -MinHeight * HeightScale * 100.0) : FVector::ZeroVector;
	CrossMeshSection->SetPositiveBoundsExtension(boundsExtension);
	TileMeshSection->SetPositiveBoundsExtension(boundsExtension);
	FillerMeshSection->SetPositiveBoundsExtension(boundsExtension);
	TrimMeshSection->SetPositiveBoundsExtension(boundsExtension);
	SeamMeshSection->SetPositiveBoundsExtension(boundsExtension);

	CrossMeshSection->SetNegativeBoundsExtension(negBoundsExtension);
	TileMeshSection->SetNegativeBoundsExtension(negBoundsExtension);
	FillerMeshSection->SetNegativeBoundsExtension(negBoundsExtension);
	TrimMeshSection->SetNegativeBoundsExtension(negBoundsExtension);
	SeamMeshSection->SetNegativeBoundsExtension(negBoundsExtension);

	CrossMeshSection->CalculateExtendedBounds();
	TileMeshSection->CalculateExtendedBounds();
	FillerMeshSection->CalculateExtendedBounds();
	TrimMeshSection->CalculateExtendedBounds();
	SeamMeshSection->CalculateExtendedBounds();
}
void AClipmapTerrainActor::EmplaceWindowRegion(UTexture2D* Normalmap, UTexture2D* Heightmap, int level,double destX, double destY, int srcX, int srcY, int sizeX, int sizeY)
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
			QueuedUpdateRegions.Emplace(Normalmap,Heightmap, level, xSeg.dest, ySeg.dest, xSeg.src, ySeg.src, xSeg.size, ySeg.size);
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

			if (!chunk.bValid && !chunk.bGenerating)
			{
				ChunksToUpdate.Emplace(FIntVector2(x,y),key);
				chunk.bGenerating = true;
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
			rowCopySizeY = copySizeY;
			EmplaceWindowRegion(chunk.Normalmap[level], chunk.Heightmap[level], level, xOffset + windowX, yOffset + windowY, copyStartX, copyStartY, copySizeX, copySizeY);
			windowX += copySizeX;
		}
		windowY += rowCopySizeY;
	}
}
void AClipmapTerrainActor::UpdateClipmapLevels()
{
	int windowSize = ClipmapTileSize * 4;
	int windowSizeHalf = windowSize / 2;

	

	for (int i = 0; i < ClipmapLevels; i++)
	{
		double scalar = FMath::Pow(2.0, i);
		FVector2D pos = FVector2D(LastViewGridPosition.X, LastViewGridPosition.Y) / 100.0 / scalar;
		double x1 = pos.X - windowSizeHalf;
		double x2 = pos.X + windowSizeHalf;
		double y1 = pos.Y - windowSizeHalf;
		double y2 = pos.Y + windowSizeHalf;
		ChunksToWindow(i, 0, 0, x1, x2, y1, y2);
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
			chunk.NormalmapBuffers.SetNum(ClipmapLevels);
			for (int level = 0; level < ClipmapLevels; level++)
			{
				chunk.HeightmapBuffers[level] = (float*)chunk.Heightmap[level]->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
				chunk.NormalmapBuffers[level] = (FFloat16Color*)chunk.Normalmap[level]->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
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
					GenHeightmap(x, y, level, chunk);
				}
			});
		for (int i = 0; i < ChunksToUpdate.Num(); i++)
		{
			auto& chunkInfo = ChunksToUpdate[i];
			FRandomTerrainChunk& chunk = Chunks[chunkInfo.Value.Index];
			for (int level = 0; level < ClipmapLevels; level++)
			{
				chunk.Heightmap[level]->GetPlatformData()->Mips[0].BulkData.Unlock();
				chunk.Heightmap[level]->UpdateResource();
				chunk.Normalmap[level]->GetPlatformData()->Mips[0].BulkData.Unlock();
				chunk.Normalmap[level]->UpdateResource();
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
			([Regions = MoveTemp(QueuedUpdateRegions), windowTexture = WindowTexture,normalWindowTexture=NormalWindowTexture](FRHICommandList& RHICmdList)
				{
					FTextureRHIRef WindowTexture2DRHI = windowTexture->GetResource()->TextureRHI;
					for (const FUpdateHeightmapRegion& Region : Regions)
					{
						RHICmdList.CopyTexture(Region.SourceTexture->GetResource()->TextureRHI, windowTexture->GetResource()->TextureRHI, Region.Region);
						RHICmdList.CopyTexture(Region.NormalSourceTexture->GetResource()->TextureRHI, normalWindowTexture->GetResource()->TextureRHI, Region.Region);
					}




				});

	}
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
		ret.Index = Chunks.Num();
		FRandomTerrainChunk& newChunk = Chunks.AddDefaulted_GetRef();
		newChunk.Heightmap.SetNum(ClipmapLevels);
		newChunk.Normalmap.SetNum(ClipmapLevels);
		for (int i = 0; i < ClipmapLevels; i++)
		{
			newChunk.Heightmap[i] = UTexture2D::CreateTransient(ChunkSize, ChunkSize, EPixelFormat::PF_R32_FLOAT);
			newChunk.Normalmap[i] = UTexture2D::CreateTransient(ChunkSize, ChunkSize, EPixelFormat::PF_FloatRGBA);
		}
		ret.bValid = true;
		//Marked to regenerate
		newChunk.bValid = false;
	}
	return ret;
}