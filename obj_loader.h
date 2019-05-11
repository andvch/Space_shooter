#ifndef _OBJ_LOADER_H
#define _OBJ_LOADER_H

#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include "LiteMath.h"

bool loadOBJ(
	const char * path, 
	std::vector<float> & out_vertices, 
	std::vector<float> & out_uvs,
	std::vector<float> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<LiteMath::float3> temp_vertices; 
	std::vector<LiteMath::float2> temp_uvs;
	std::vector<LiteMath::float3> temp_normals;
	LiteMath::float3 min(0, 0, 0), max(0, 0, 0);

	FILE * file = fopen(path, "r");
	if( file == NULL )
		return false;

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ) {
			LiteMath::float3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			if (vertex.x < min.x)
				min.x = vertex.x;
			if (vertex.y < min.y)
				min.y = vertex.y;
			if (vertex.z < min.z)
				min.z = vertex.z;
			if (vertex.x > max.x)
				max.x = vertex.x;
			if (vertex.y > max.y)
				max.y = vertex.y;
			if (vertex.z > max.z)
				max.z = vertex.z;
			temp_vertices.push_back(vertex);
		} else if ( strcmp( lineHeader, "vt" ) == 0 ) {
			LiteMath::float2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y );
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		} else if ( strcmp( lineHeader, "vn" ) == 0 ) {
			LiteMath::float3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
			temp_normals.push_back(normal);
		} else if ( strcmp( lineHeader, "f" ) == 0 ) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
			if (matches != 9){
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				fclose(file);
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices    .push_back(uvIndex[0]);
			uvIndices    .push_back(uvIndex[1]);
			uvIndices    .push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		} else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}
	
	LiteMath::float3 center = (max - min) / 2;
	float compress = (center.x > center.y) ? center.x : center.y;
	compress = (compress > center.z) ? compress : center.z;
	center += min;
	
	out_vertices.clear();
	out_uvs     .clear();
	out_normals .clear();

	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];
		
		// Get the attributes thanks to the index
		LiteMath::float3 vertex = temp_vertices[ vertexIndex-1 ];
		LiteMath::float2 uv = temp_uvs[ uvIndex-1 ];
		LiteMath::float3 normal = temp_normals[ normalIndex-1 ];
		
		vertex = (vertex - center) / compress;
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex.x);
		out_vertices.push_back(vertex.y);
		out_vertices.push_back(vertex.z);
		out_uvs     .push_back(uv.x);
		out_uvs     .push_back(uv.y);
		out_normals .push_back(normal.x);
		out_normals .push_back(normal.y);
		out_normals .push_back(normal.z);
	
	}
	fclose(file);
	printf("OK (%u triangles)\n", vertexIndices.size() / 3);
	return true;
}

#endif