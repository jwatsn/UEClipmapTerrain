#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "MeshDescription.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshAttributes.h"
#include "Materials/Material.h"

void InitMeshDescription(FMeshDescription& outMeshDescription, FMeshDescriptionBuilder& outBuilder, FPolygonGroupID& outPolygonGroupID)
{
	FStaticMeshAttributes Attributes(outMeshDescription);
	Attributes.Register();
	outBuilder.SetMeshDescription(&outMeshDescription);
	outBuilder.EnablePolyGroups();
	outBuilder.SetNumUVLayers(1);
	outPolygonGroupID = outBuilder.AppendPolygonGroup();
};
UStaticMesh* FinalizeGenMesh(const FMeshDescription& meshDesc)
{
	UStaticMesh* staticMesh = NewObject<UStaticMesh>();
	TArray<FStaticMaterial> staticMaterials;
	staticMaterials.Add(FStaticMaterial(UMaterial::GetDefaultMaterial(MD_Surface)));
	staticMesh->SetStaticMaterials(staticMaterials);
	UStaticMesh::FBuildMeshDescriptionsParams mdParams;
	staticMesh->bGenerateMeshDistanceField = false;
#if WITH_EDITOR
	staticMesh->NaniteSettings.bEnabled = false;
	staticMesh->RayTracingProxySettings.bEnabled = false;
	staticMesh->bSupportRayTracing = false;
#endif
	//staticMesh->LightMapResolution = 512;
	mdParams.bCommitMeshDescription = false;
	mdParams.bFastBuild = true;
	mdParams.bAllowCpuAccess = true;
	// Build static mesh
	TArray<const FMeshDescription*> meshDescPtrs;
	meshDescPtrs.Add(&meshDesc);

	staticMesh->BuildFromMeshDescriptions(meshDescPtrs, mdParams);
	staticMesh->InitResources();
	return staticMesh;
}
int _patch_2d(const int x, const int y, const int PATCH_VERT_RESOLUTION)
{
	return y * PATCH_VERT_RESOLUTION + x;
};
void InitCrossMesh(FMeshDescriptionBuilder& meshDescBuilder, FPolygonGroupID& polygonGroup, int tileSize)
{
	const int TILE_RESOLUTION = tileSize;
	const int PATCH_VERT_RESOLUTION = tileSize + 1;
	int vertices_index = 0;
	TArray< FVertexID > vertexIDs; vertexIDs.SetNum(PATCH_VERT_RESOLUTION * 8);

	for (int i = 0; i < PATCH_VERT_RESOLUTION * 2; i++)
	{
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(1, i - TILE_RESOLUTION, 0) * 100);
		vertices_index += 1;
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(0, i - TILE_RESOLUTION, 0) * 100);
		vertices_index += 1;
	}


	int start_of_vertical = vertices_index;

	for (int i = 0; i < PATCH_VERT_RESOLUTION * 2; i++)
	{
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(i - TILE_RESOLUTION, 1, 0) * 100);
		vertices_index += 1;
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(i - TILE_RESOLUTION, 0, 0) * 100);
		vertices_index += 1;
	}

	const int numIndices = TILE_RESOLUTION * 24 + 6;
	for (int i = 0; i < TILE_RESOLUTION * 2 + 1; i++)
	{
		int bl = i * 2 + 0;
		int br = i * 2 + 1;
		int tl = i * 2 + 2;
		int tr = i * 2 + 3;

		FVertexInstanceID instance1 = meshDescBuilder.AppendInstance(vertexIDs[br]);
		meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceTangentSpace(instance1, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance2 = meshDescBuilder.AppendInstance(vertexIDs[tl]);
		meshDescBuilder.SetInstanceNormal(instance2, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance2, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance2, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance2, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance3 = meshDescBuilder.AppendInstance(vertexIDs[bl]);
		meshDescBuilder.SetInstanceNormal(instance3, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance3, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance3, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance3, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));


		FVertexInstanceID instance4 = meshDescBuilder.AppendInstance(vertexIDs[tl]);
		meshDescBuilder.SetInstanceNormal(instance4, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance4, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance4, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance4, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance5 = meshDescBuilder.AppendInstance(vertexIDs[br]);
		meshDescBuilder.SetInstanceNormal(instance5, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance5, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance5, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance5, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance6 = meshDescBuilder.AppendInstance(vertexIDs[tr]);
		meshDescBuilder.SetInstanceNormal(instance6, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance6, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance6, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance6, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		meshDescBuilder.AppendTriangle(instance1, instance2, instance3, polygonGroup);
		meshDescBuilder.AppendTriangle(instance4, instance5, instance6, polygonGroup);
	}

	for (int i = 0; i < TILE_RESOLUTION * 2 + 1; i++)
	{
		if (i == TILE_RESOLUTION)
			continue;

		int bl = i * 2 + 0;
		int br = i * 2 + 1;
		int tl = i * 2 + 2;
		int tr = i * 2 + 3;

		FVertexInstanceID instance1 = meshDescBuilder.AppendInstance(vertexIDs[start_of_vertical + br]);
		meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance1, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance2 = meshDescBuilder.AppendInstance(vertexIDs[start_of_vertical + tl]);
		meshDescBuilder.SetInstanceNormal(instance2, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance2, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance2, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance2, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance3 = meshDescBuilder.AppendInstance(vertexIDs[start_of_vertical + tr]);
		meshDescBuilder.SetInstanceNormal(instance3, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance3, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance3, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance3, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));


		FVertexInstanceID instance4 = meshDescBuilder.AppendInstance(vertexIDs[start_of_vertical + br]);
		meshDescBuilder.SetInstanceNormal(instance4, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance4, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance4, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance4, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance5 = meshDescBuilder.AppendInstance(vertexIDs[start_of_vertical + bl]);
		meshDescBuilder.SetInstanceNormal(instance5, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance5, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance5, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance5, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance6 = meshDescBuilder.AppendInstance(vertexIDs[start_of_vertical + tl]);
		meshDescBuilder.SetInstanceNormal(instance6, FVector::UpVector);
		meshDescBuilder.SetInstanceTangentSpace(instance6, FVector(0., 0., 1.0), FVector(1.0, .0, .0), 0);
		meshDescBuilder.SetInstanceUV(instance6, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance6, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		meshDescBuilder.AppendTriangle(instance1, instance2, instance3, polygonGroup);
		meshDescBuilder.AppendTriangle(instance4, instance5, instance6, polygonGroup);

	}
}
void InitTileMesh(FMeshDescriptionBuilder& meshDescBuilder, FPolygonGroupID& polygonGroup, int tileSize)
{
	const int PATCH_VERT_RESOLUTION = tileSize + 1;
	const int TILE_RESOLUTION = tileSize;
	TArray< FVertexID > vertexIDs; vertexIDs.SetNum(PATCH_VERT_RESOLUTION * PATCH_VERT_RESOLUTION);
	int vertices_index = 0;

	for (int x = 0; x < PATCH_VERT_RESOLUTION; x++)
	{
		for (int y = 0; y < PATCH_VERT_RESOLUTION; y++)
		{
			vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(x, y, 0) * 100);
			vertices_index += 1;
		}
	}


	for (int y = 0; y < TILE_RESOLUTION; y++)
	{
		for (int x = 0; x < TILE_RESOLUTION; x++)
		{
			FVertexInstanceID instance1 = meshDescBuilder.AppendInstance(vertexIDs[_patch_2d(x, y, PATCH_VERT_RESOLUTION)]);
			meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance2 = meshDescBuilder.AppendInstance(vertexIDs[_patch_2d(x + 1, y + 1, PATCH_VERT_RESOLUTION)]);
			meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance3 = meshDescBuilder.AppendInstance(vertexIDs[_patch_2d(x, y + 1, PATCH_VERT_RESOLUTION)]);
			meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance4 = meshDescBuilder.AppendInstance(vertexIDs[_patch_2d(x, y, PATCH_VERT_RESOLUTION)]);
			meshDescBuilder.SetInstanceNormal(instance4, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance4, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance4, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance5 = meshDescBuilder.AppendInstance(vertexIDs[_patch_2d(x + 1, y, PATCH_VERT_RESOLUTION)]);
			meshDescBuilder.SetInstanceNormal(instance5, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance5, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance5, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance6 = meshDescBuilder.AppendInstance(vertexIDs[_patch_2d(x + 1, y + 1, PATCH_VERT_RESOLUTION)]);
			meshDescBuilder.SetInstanceNormal(instance6, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance6, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance6, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			meshDescBuilder.AppendTriangle(instance1, instance2, instance3, polygonGroup);
			meshDescBuilder.AppendTriangle(instance4, instance5, instance6, polygonGroup);
		}
	}
}
void InitSeamMesh(FMeshDescriptionBuilder& meshDescBuilder, FPolygonGroupID& polygonGroup, int tileSize)
{
	const int CLIPMAP_RESOLUTION = tileSize * 4 + 1;
	const int CLIPMAP_VERT_RESOLUTION = CLIPMAP_RESOLUTION + 1;
	TArray< FVertexID > vertexIDs; vertexIDs.SetNum(CLIPMAP_VERT_RESOLUTION * 4);
	int vertices_index = 0;

	for (int i = 0; i < CLIPMAP_VERT_RESOLUTION; i++)
	{
		vertexIDs[CLIPMAP_VERT_RESOLUTION * 0 + i] = meshDescBuilder.AppendVertex(FVector(i, 0, 0) * 100);
		vertexIDs[CLIPMAP_VERT_RESOLUTION * 1 + i] = meshDescBuilder.AppendVertex(FVector(CLIPMAP_VERT_RESOLUTION, i, 0) * 100);
		vertexIDs[CLIPMAP_VERT_RESOLUTION * 2 + i] = meshDescBuilder.AppendVertex(FVector(CLIPMAP_VERT_RESOLUTION - i, CLIPMAP_VERT_RESOLUTION, 0) * 100);
		vertexIDs[CLIPMAP_VERT_RESOLUTION * 3 + i] = meshDescBuilder.AppendVertex(FVector(0, CLIPMAP_VERT_RESOLUTION - i, 0) * 100);
	}
	TArray<int32> indices;
	indices.SetNum(CLIPMAP_VERT_RESOLUTION * 6);
	int indices_index = 0;

	for (int i = 0; i < CLIPMAP_VERT_RESOLUTION * 4; i += 2)
	{
		indices[indices_index] = i + 1;
		indices_index += 1;
		indices[indices_index] = i;
		indices_index += 1;
		indices[indices_index] = i + 2;
		indices_index += 1;
	}
	indices[indices.Num() - 1] = 0;

	for (int i = 0; i < indices.Num(); i += 3)
	{
		FVertexInstanceID instance1 = meshDescBuilder.AppendInstance(vertexIDs[indices[i]]);
		meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance2 = meshDescBuilder.AppendInstance(vertexIDs[indices[i + 1]]);
		meshDescBuilder.SetInstanceNormal(instance2, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance2, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance2, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance3 = meshDescBuilder.AppendInstance(vertexIDs[indices[i + 2]]);
		meshDescBuilder.SetInstanceNormal(instance3, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance3, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance3, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		meshDescBuilder.AppendTriangle(instance1, instance2, instance3, polygonGroup);
	}
};
void InitTrimMesh(FMeshDescriptionBuilder& meshDescBuilder, FPolygonGroupID& polygonGroup, int tileSize)
{
	const int CLIPMAP_RESOLUTION = tileSize * 4 + 1;
	const int CLIPMAP_VERT_RESOLUTION = CLIPMAP_RESOLUTION + 1;
	TArray< FVertexID > vertexIDs; vertexIDs.SetNum((CLIPMAP_VERT_RESOLUTION * 2 + 1) * 2);
	int vertices_index = 0;

	FVector offset = FVector(0.5 * (CLIPMAP_VERT_RESOLUTION + 1), 0.5 * (CLIPMAP_VERT_RESOLUTION + 1), 0);

	for (int i = 0; i < CLIPMAP_VERT_RESOLUTION + 1; i++)
	{
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex((FVector(0, CLIPMAP_VERT_RESOLUTION - i, 0) - offset) * 100);
		vertices_index += 1;
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex((FVector(1, CLIPMAP_VERT_RESOLUTION - i, 0) - offset) * 100);
		vertices_index += 1;
	}
	int start_of_horizontal = vertices_index;

	for (int i = 0; i < CLIPMAP_VERT_RESOLUTION; i++)
	{
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex((FVector(i + 1, 0, 0) - offset) * 100);
		vertices_index += 1;
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex((FVector(i + 1, 1, 0) - offset) * 100);
		vertices_index += 1;
	}


	/*for (int i=0; i<data.vertices.Num(); i++)
			data.vertices[i] -= FVector3f(0.5 * (CLIPMAP_VERT_RESOLUTION + 1), 0.5 * (CLIPMAP_VERT_RESOLUTION + 1), 0);*/



	for (int i = 0; i < CLIPMAP_VERT_RESOLUTION; i++)
	{
		FVertexInstanceID instance1 = meshDescBuilder.AppendInstance(vertexIDs[(i + 0) * 2 + 1]);
		meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance2 = meshDescBuilder.AppendInstance(vertexIDs[(i + 1) * 2 + 0]);
		meshDescBuilder.SetInstanceNormal(instance2, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance2, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance2, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance3 = meshDescBuilder.AppendInstance(vertexIDs[(i + 0) * 2 + 0]);
		meshDescBuilder.SetInstanceNormal(instance3, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance3, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance3, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));


		FVertexInstanceID instance4 = meshDescBuilder.AppendInstance(vertexIDs[(i + 1) * 2 + 1]);
		meshDescBuilder.SetInstanceNormal(instance4, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance4, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance4, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance5 = meshDescBuilder.AppendInstance(vertexIDs[(i + 1) * 2 + 0]);
		meshDescBuilder.SetInstanceNormal(instance5, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance5, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance5, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance6 = meshDescBuilder.AppendInstance(vertexIDs[(i + 0) * 2 + 1]);
		meshDescBuilder.SetInstanceNormal(instance6, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance6, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance6, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		meshDescBuilder.AppendTriangle(instance1, instance2, instance3, polygonGroup);
		meshDescBuilder.AppendTriangle(instance4, instance5, instance6, polygonGroup);
	}
	for (int i = 0; i < CLIPMAP_VERT_RESOLUTION - 1; i++)
	{
		FVertexInstanceID instance1 = meshDescBuilder.AppendInstance(vertexIDs[start_of_horizontal + (i + 0) * 2 + 1]);
		meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance2 = meshDescBuilder.AppendInstance(vertexIDs[start_of_horizontal + (i + 1) * 2 + 0]);
		meshDescBuilder.SetInstanceNormal(instance2, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance2, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance2, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance3 = meshDescBuilder.AppendInstance(vertexIDs[start_of_horizontal + (i + 0) * 2 + 0]);
		meshDescBuilder.SetInstanceNormal(instance3, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance3, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance3, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));


		FVertexInstanceID instance4 = meshDescBuilder.AppendInstance(vertexIDs[start_of_horizontal + (i + 1) * 2 + 1]);
		meshDescBuilder.SetInstanceNormal(instance4, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance4, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance4, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance5 = meshDescBuilder.AppendInstance(vertexIDs[start_of_horizontal + (i + 1) * 2 + 0]);
		meshDescBuilder.SetInstanceNormal(instance5, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance5, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance5, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		FVertexInstanceID instance6 = meshDescBuilder.AppendInstance(vertexIDs[start_of_horizontal + (i + 0) * 2 + 1]);
		meshDescBuilder.SetInstanceNormal(instance6, FVector::UpVector);
		meshDescBuilder.SetInstanceUV(instance6, FVector2D::ZeroVector, 0);
		meshDescBuilder.SetInstanceColor(instance6, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

		meshDescBuilder.AppendTriangle(instance1, instance2, instance3, polygonGroup);
		meshDescBuilder.AppendTriangle(instance4, instance5, instance6, polygonGroup);
	}
}
void InitFillerMesh(FMeshDescriptionBuilder& meshDescBuilder, FPolygonGroupID& polygonGroup, int tileSize)
{
	const int TILE_RESOLUTION = tileSize;
	const int PATCH_VERT_RESOLUTION = tileSize + 1;
	TArray< FVertexID > vertexIDs; vertexIDs.SetNum(PATCH_VERT_RESOLUTION * 8);
	int vertices_index = 0;
	int offset = TILE_RESOLUTION;

	for (int i = 0; i < PATCH_VERT_RESOLUTION; i++)
	{
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(offset + i + 1, 0, 0) * 100);
		vertices_index += 1;
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(offset + i + 1, 1, 0) * 100);
		vertices_index += 1;
	}
	for (int i = 0; i < PATCH_VERT_RESOLUTION; i++)
	{
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(1, offset + i + 1, 0) * 100);
		vertices_index += 1;
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(0, offset + i + 1, 0) * 100);
		vertices_index += 1;
	}
	for (int i = 0; i < PATCH_VERT_RESOLUTION; i++)
	{
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(-(offset + i), 1, 0) * 100);
		vertices_index += 1;
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(-(offset + i), 0, 0) * 100);
		vertices_index += 1;
	}
	for (int i = 0; i < PATCH_VERT_RESOLUTION; i++)
	{
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(0, -(offset + i), 0) * 100);
		vertices_index += 1;
		vertexIDs[vertices_index] = meshDescBuilder.AppendVertex(FVector(1, -(offset + i), 0) * 100);
		vertices_index += 1;
	}




	int indices_index = 0;

	for (int i = 0; i < TILE_RESOLUTION * 4; i++)
	{
		int arm = i / TILE_RESOLUTION;

		int bl = (arm + i) * 2 + 0;
		int br = (arm + i) * 2 + 1;
		int tl = (arm + i) * 2 + 2;
		int tr = (arm + i) * 2 + 3;

		if (arm % 2 == 0)
		{
			FVertexInstanceID instance1 = meshDescBuilder.AppendInstance(vertexIDs[br]);
			meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance2 = meshDescBuilder.AppendInstance(vertexIDs[tr]);
			meshDescBuilder.SetInstanceNormal(instance2, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance2, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance2, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance3 = meshDescBuilder.AppendInstance(vertexIDs[bl]);
			meshDescBuilder.SetInstanceNormal(instance3, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance3, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance3, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));


			FVertexInstanceID instance4 = meshDescBuilder.AppendInstance(vertexIDs[bl]);
			meshDescBuilder.SetInstanceNormal(instance4, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance4, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance4, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance5 = meshDescBuilder.AppendInstance(vertexIDs[tr]);
			meshDescBuilder.SetInstanceNormal(instance5, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance5, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance5, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance6 = meshDescBuilder.AppendInstance(vertexIDs[tl]);
			meshDescBuilder.SetInstanceNormal(instance6, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance6, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance6, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			meshDescBuilder.AppendTriangle(instance1, instance2, instance3, polygonGroup);
			meshDescBuilder.AppendTriangle(instance4, instance5, instance6, polygonGroup);

		}
		else
		{
			FVertexInstanceID instance1 = meshDescBuilder.AppendInstance(vertexIDs[br]);
			meshDescBuilder.SetInstanceNormal(instance1, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance1, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance1, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance2 = meshDescBuilder.AppendInstance(vertexIDs[tl]);
			meshDescBuilder.SetInstanceNormal(instance2, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance2, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance2, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance3 = meshDescBuilder.AppendInstance(vertexIDs[bl]);
			meshDescBuilder.SetInstanceNormal(instance3, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance3, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance3, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));


			FVertexInstanceID instance4 = meshDescBuilder.AppendInstance(vertexIDs[br]);
			meshDescBuilder.SetInstanceNormal(instance4, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance4, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance4, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance5 = meshDescBuilder.AppendInstance(vertexIDs[tr]);
			meshDescBuilder.SetInstanceNormal(instance5, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance5, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance5, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			FVertexInstanceID instance6 = meshDescBuilder.AppendInstance(vertexIDs[tl]);
			meshDescBuilder.SetInstanceNormal(instance6, FVector::UpVector);
			meshDescBuilder.SetInstanceUV(instance6, FVector2D::ZeroVector, 0);
			meshDescBuilder.SetInstanceColor(instance6, FVector4f(1.0f, 1.0f, 1.0f, 1.0f));

			meshDescBuilder.AppendTriangle(instance1, instance2, instance3, polygonGroup);
			meshDescBuilder.AppendTriangle(instance4, instance5, instance6, polygonGroup);
		}
	}
}
namespace ClipmapBuilder
{
	static UStaticMesh* CrossMesh(int tileSize)
	{
		FMeshDescription meshDesc;
		FMeshDescriptionBuilder meshDescBuilder;
		FPolygonGroupID polygonGroupID;
		InitMeshDescription(meshDesc, meshDescBuilder, polygonGroupID);
		InitCrossMesh(meshDescBuilder, polygonGroupID, tileSize);

		return FinalizeGenMesh(meshDesc);
	}

	static UStaticMesh* TileMesh(int tileSize)
	{
		FMeshDescription meshDesc;
		FMeshDescriptionBuilder meshDescBuilder;
		FPolygonGroupID polygonGroupID;
		InitMeshDescription(meshDesc, meshDescBuilder, polygonGroupID);
		InitTileMesh(meshDescBuilder, polygonGroupID, tileSize);

		return FinalizeGenMesh(meshDesc);
	}

	static UStaticMesh* TrimMesh(int tileSize)
	{
		FMeshDescription meshDesc;
		FMeshDescriptionBuilder meshDescBuilder;
		FPolygonGroupID polygonGroupID;
		InitMeshDescription(meshDesc, meshDescBuilder, polygonGroupID);
		InitTrimMesh(meshDescBuilder, polygonGroupID, tileSize);

		return FinalizeGenMesh(meshDesc);
	}

	static UStaticMesh* FillerMesh(int tileSize)
	{
		FMeshDescription meshDesc;
		FMeshDescriptionBuilder meshDescBuilder;
		FPolygonGroupID polygonGroupID;
		InitMeshDescription(meshDesc, meshDescBuilder, polygonGroupID);
		InitFillerMesh(meshDescBuilder, polygonGroupID, tileSize);

		return FinalizeGenMesh(meshDesc);
	}

	static UStaticMesh* SeamMesh(int tileSize)
	{
		FMeshDescription meshDesc;
		FMeshDescriptionBuilder meshDescBuilder;
		FPolygonGroupID polygonGroupID;
		InitMeshDescription(meshDesc, meshDescBuilder, polygonGroupID);
		InitSeamMesh(meshDescBuilder, polygonGroupID, tileSize);

		return FinalizeGenMesh(meshDesc);
	}
};