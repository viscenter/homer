#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "manuModel.h"
#include "MathDefs.h"

#define BUF_SIZE 256

class VertexGroup {
	public:
		std::vector<std::pair<tVector *, tVector *> > vertices;
		std::string groupName;
		std::string texName;
};

bool manuModel::readOBJ( char * filename )
{
	std::vector<tVector> vertices;
	std::vector<tVector> textureoffsets;
	std::vector<VertexGroup> groups;
	std::vector<Triangle> trigroups;
	std::vector<Triangle> texgroups;
	std::streampos rewind;

	std::ifstream objfile(filename);
	std::string line, ignore;

	if(!objfile.good()) {
		fprintf(stderr, "Unable to open OBJ file.\n");
		return false;
	}

	while(!objfile.eof()) {
		rewind = objfile.tellg();
		getline(objfile,line);
		// Read in vertices
		// Lines begin with v
		if((line[0] == 'v') && (line[1] == ' ')) {
			tVector thispoint;
			std::stringstream linereader;
			linereader << line;
			linereader >> ignore >> thispoint.x >> thispoint.y >> thispoint.z;
			vertices.push_back(thispoint);
		}
		// Read in texture offsets
		// Lines begin with vt
		else if((line[0] == 'v') && (line[1] == 't')) {
			tVector thisoffset;
			std::stringstream linereader;
			linereader << line;
			linereader >> ignore >> thisoffset.u >> thisoffset.v;
			// thisoffset.u = 1.0 - thisoffset.u;
			thisoffset.v = 1.0 - thisoffset.v;
			textureoffsets.push_back(thisoffset);
		}
		// Read in triangles
		// Lines begin with f
		else if((line[0] == 'f') && (line[1] == ' ')) {
			int v1,t1,v2,t2,v3,t3;
			if( sscanf(line.c_str(),
						"f %d/%d %d/%d %d/%d",&v1,&t1,&v2,&t2,&v3,&t3) == 6 ||
					sscanf(line.c_str(),
						"f %d/%d/%*d %d/%d/%*d %d/%d/%*d",&v1,&t1,&v2,&t2,&v3,&t3) == 6)
			{
				Triangle thistri;
				thistri.idx1 = v1-1;
				thistri.idx2 = v2-1;
				thistri.idx3 = v3-1;
				trigroups.push_back(thistri);
				
				thistri.idx1 = t1-1;
				thistri.idx2 = t2-1;
				thistri.idx3 = t3-1;
				texgroups.push_back(thistri);
			}
		}
		// This is kind of dangerous, but let's ignore bad/unknown formatting
		//else if((line[0] != '#') && (line[0] != 'g')) {
		//	break;
		//}
	}	
	
	// We now have vertices and texture offsets, copy them
	// into the manuModel data structure
	nVer = vertices.size();
	verList = new Point[nVer];
	originalList = new Point[nVer];
	for(int i=0; i < nVer; i++)
	{
		verList[i].u1 = textureoffsets[i].u;
		verList[i].v1 = textureoffsets[i].v;
		
		verList[i].x = vertices[i].x;
		verList[i].y = vertices[i].z;
		verList[i].z = vertices[i].y;
	}
	if( SMT_DEBUG ) printf("Read in %i Vertices \n", nVer );
		
	nTrig = trigroups.size();
	trigList = new Triangle[nTrig];

	for(unsigned int i = 0; i < nTrig; i++) {
		trigList[i] = trigroups[i];

		// Re-index textures, since their indices may not match vertices
		verList[trigroups[i].idx1].u1 = textureoffsets[texgroups[i].idx1].u;
		verList[trigroups[i].idx1].v1 = textureoffsets[texgroups[i].idx1].v;
		verList[trigroups[i].idx2].u1 = textureoffsets[texgroups[i].idx2].u;
		verList[trigroups[i].idx2].v1 = textureoffsets[texgroups[i].idx2].v;
		verList[trigroups[i].idx3].u1 = textureoffsets[texgroups[i].idx3].u;
		verList[trigroups[i].idx3].v1 = textureoffsets[texgroups[i].idx3].v;
	}
	if( SMT_DEBUG ) printf("Read in %i Triangles \n", nTrig );

	/*
	for(unsigned int i = 0; i < groups.size(); i++) {
				printf("Group name: %s\n",groups[i].groupName.c_str());
		printf("Texture name: %s\n",groups[i].texName.c_str());
		for(unsigned int j = 0; j < (groups[i].vertices.size() / 3); j++) {
			printf("%f,%f,%f\t%f,%f\n",
							groups[i].vertices[j].first->x,
							groups[i].vertices[j].first->y,
							groups[i].vertices[j].first->z,
							groups[i].vertices[j].second->u,
							groups[i].vertices[j].second->v);
		}
	}
	*/

	return true;
}
