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
	std::streampos rewind;

	std::ifstream objfile(filename);
	std::string line, ignore;

	if(!objfile.good()) {
		fprintf(stderr, "Unable to open OBJ file.\n");
		return false;
	}

	// Read in all vertices
	// Lines begin with v
	while(!objfile.eof()) {
		rewind = objfile.tellg();
		getline(objfile,line);
		if((line[0] == 'v') && (line[1] == ' ')) {
			tVector thispoint;
			std::stringstream linereader;
			linereader << line;
			linereader >> ignore >> thispoint.x >> thispoint.y >> thispoint.z;
			vertices.push_back(thispoint);
		}
		else if(line[0] != '#') {
			break;
		}
	}

	objfile.clear();
	objfile.seekg(rewind);

	// Read in all texture offsets
	// Lines begin with vt
	while(!objfile.eof()) {
		rewind = objfile.tellg();
		getline(objfile,line);
		if((line[0] == 'v') && (line[1] == 't')) {
			tVector thisoffset;
			std::stringstream linereader;
			linereader << line;
			linereader >> ignore >> thisoffset.u >> thisoffset.v;
			textureoffsets.push_back(thisoffset);
		}
		else if(line[0] != '#') {
			break;
		}
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
		verList[i].y = vertices[i].y;
		verList[i].z = vertices[i].z;
	}
	if( SMT_DEBUG ) printf("Read in %i Vertices \n", nVer );
		
	

	objfile.clear();
	objfile.seekg(rewind);
	
	// Construct groups
	// Read g for name, look for map_Kd on next line, read all f %d/%d lines 
	// (ignoring comments) until we hit something else
	while(!objfile.eof()) {
		getline(objfile,line);
		if((line[0] == 'g') && (line[1] == ' ')) {
			VertexGroup thisgroup;
			char * namebuf = (char *)calloc(BUF_SIZE,sizeof(char));
			sscanf(line.c_str(),"g %s",namebuf);
			thisgroup.groupName = std::string(namebuf);
			
			getline(objfile,line);
			if(line.find("map_Kd",0) == 0) {
				sscanf(line.c_str(),"map_Kd %s",namebuf);
				thisgroup.texName = std::string(namebuf);
			}
			free(namebuf);
			
			// we now have the texture for this group, read triangles
			bool readingtriangles = true;
			while((!objfile.eof()) && readingtriangles) {
				rewind = objfile.tellg();
				getline(objfile,line);
				if((line[0] == 'f') && (line[1] == ' ')) {
					int v1,t1,v2,t2,v3,t3;
					sscanf(line.c_str(),"f %d/%d %d/%d %d/%d",&v1,&t1,&v2,&t2,&v3,&t3);

					Triangle thistri;
					thistri.idx1 = v1-1;
					thistri.idx2 = v2-1;
					thistri.idx3 = v3-1;
					trigroups.push_back(thistri);

					// printf("f %d/%d %d/%d %d/%d\n",v1,t1,v2,t2,v3,t3);
					thisgroup.vertices.push_back(std::pair<tVector *, tVector *>(&(vertices[v1-1]),&(textureoffsets[t1-1])));
					thisgroup.vertices.push_back(std::pair<tVector *, tVector *>(&(vertices[v2-1]),&(textureoffsets[t2-1])));
					thisgroup.vertices.push_back(std::pair<tVector *, tVector *>(&(vertices[v3-1]),&(textureoffsets[t3-1])));
					
					/*
					printf("%f,%f,%f\t%f,%f\n",
							thisgroup.vertices.back().first.x,
							thisgroup.vertices.back().first.y,
							thisgroup.vertices.back().first.z,
							thisgroup.vertices.back().second.u,
							thisgroup.vertices.back().second.v);
					*/
				}
				else if(line[0] != '#') {
					// not a triangle or a comment, go on to the next group
					readingtriangles = false;
					objfile.seekg(rewind);
				}
			}
			groups.push_back(thisgroup);
		}
	}

	nTrig = trigroups.size();
	trigList = new Triangle[nTrig];

	for(unsigned int i = 0; i < nTrig; i++) {
		trigList[i] = trigroups[i];
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
