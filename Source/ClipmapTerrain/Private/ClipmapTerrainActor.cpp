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

	RootComponent = CreateDefaultSubobject<USceneComponent>("Terrain Root");

	SeamMeshInstance->SetupAttachment(RootComponent);
	TrimMeshInstance->SetupAttachment(RootComponent);
	FillerMeshInstance->SetupAttachment(RootComponent);
	TileMeshInstance->SetupAttachment(RootComponent);
	CrossMeshInstance->SetupAttachment(RootComponent);

	PrimaryActorTick.bCanEverTick = true;
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
	CrossMeshInstance->SetStaticMesh(ClipmapBuilder::CrossMesh(ClipmapTileSize));
	TileMeshInstance->SetStaticMesh(ClipmapBuilder::TileMesh(ClipmapTileSize));
	FillerMeshInstance->SetStaticMesh(ClipmapBuilder::FillerMesh(ClipmapTileSize));
	TrimMeshInstance->SetStaticMesh(ClipmapBuilder::TrimMesh(ClipmapTileSize));
	SeamMeshInstance->SetStaticMesh(ClipmapBuilder::SeamMesh(ClipmapTileSize));
}
void AClipmapTerrainActor::InitClipmap()
{
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
		ClipmapMaterial->SetScalarParameterValue("HeightScale", HeightScale * 100.0);
		ClipmapMaterial->SetScalarParameterValue("WindowSize", ClipmapLevels * 4);
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

	FVector ViewPosition = GetLocalCameraLocation();

	FVector ViewGridPosition = FVector(FMath::Floor(ViewPosition.X / 100.0 / ClipmapTileSize), FMath::Floor(ViewPosition.Y / 100.0 / ClipmapTileSize), FMath::Floor(ViewPosition.Z / 100.0 / ClipmapTileSize)) * ClipmapTileSize * 100.0;
	ViewGridPosition += FVector(ClipmapTileSize * 50, ClipmapTileSize * 50, 0);
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
	FVector SnappedPos = FVector(FMath::Floor(ViewGridPosition.X / 100.0), FMath::Floor(ViewGridPosition.Y / 100.0), 0) * 100.0;
	if(!CrossInstanceID.IsValid())
	{
		CrossInstanceID = FClipmapMeshPiece(CrossMeshInstance->AddInstanceById(FTransform(FRotator::ZeroRotator, SnappedPos), true), SnappedPos);
	}
	else
	{
		CrossMeshInstance->UpdateInstanceTransformById(CrossInstanceID.Id, FTransform(FRotator::ZeroRotator, SnappedPos), true, true);
		CrossMeshInstance->SetCustomDataValueById(CrossInstanceID.Id, 0, 0);
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
void AClipmapTerrainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bClipmapDirty)
	{
		bClipmapDirty = false;
		InitClipmap();
	}

	UpdateClipmap();
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