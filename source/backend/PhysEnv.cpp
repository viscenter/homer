// Altered Load And Save - removing sphere information - BOOL to bool
//
// Updated July 23, 2003
//
///////////////////////////////////////////////////////////////////////////////
//
// PhysEnv.cpp : Physical World implementation file
//
// Purpose:	Implementation of Particle Physics System
//
// Created:
//		JL 2/10/99	Modified from Mass-Spring Particle demo to handle cloth
// Modified:
//		JL 3/6/99 - FIXED GRAVITY FORCE CALCULATION BUG
//		JL 3/8/99 - ADDED MORE POSSIBLE CONTACTS AS EACH VERTEX CAN CONTACT MORE 
//					THEN ONE COLLISION SURFACE (SHOULD IT BE DYNAMICALLY ALLOC'ED?)
//		JL 3/20/99 - ADDED THE MIDPOINT AND RK INTEGRATOR NEEDED TO ALLOC 5 TEMP PARTICLE ARRAYS
//
// Notes:	A bit of this along with the organization comes from Chris Hecker's
//			Physics Articles from last year www.d6.com.  Hopefully this will get 
//			everyone back up to speed before we dig deeper into the world of Dynamics.
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright 1998-1999 Jeff Lander, All Rights Reserved.
//  For educational purposes only.
//  Please do not republish in electronic or print form without permission
//  Thanks - jeffl@darwin3d.com
//
///////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "PhysEnv.h"
#include "manuModel.h"
#include "smc.h"

extern manuModel *manu;

/////////////////////////////////////////////////////////////////////////////
// CPhysEnv

// INITIALIZE THE SIMULATION WORLD
CPhysEnv::CPhysEnv()
{
	m_IntegratorType = RK4_INTEGRATOR;

	m_Pick[0] = -1;
	m_Pick[1] = -1;
	m_ParticleSys[0] = NULL;
	m_ParticleSys[1] = NULL;
	m_ParticleSys[2] = NULL;	// RESET BUFFER
	// THESE TEMP PARTICLE BUFFERS ARE NEEDED FOR THE MIDPOINT AND RK4 INTEGRATOR
	for (int i = 0; i < 5; i++)
		m_TempSys[i] = NULL;
	m_ParticleCnt = 0;
	m_Contact = NULL;
	m_Spring = NULL;
	m_SpringCnt = 0;		
	m_MouseForceActive = false;
	
	m_UseXAxis = true;
	m_UseYAxis = true;
	m_UseZAxis = true;

	m_UseGravity = true;
	m_UseDamping = true;
	m_DrawSprings = true;
	m_DrawStructural = true;	// By default only draw structural springs
	m_DrawVertices	= true;
//	m_CollisionActive = true;
	m_CollisionActive = false;
	m_CollisionRootFinding = false;		// I AM NOT LOOKING FOR A COLLISION RIGHT AWAY

	MAKEVECTOR(m_Gravity, 0.0f, -0.21f, 0.0f)
	m_UserForceMag = 100.0;
	m_UserForceActive = false;
	m_MouseForceKs = 2.0f;	// MOUSE SPRING CONSTANT
	m_Kd	= 0.04f;	// DAMPING FACTOR
	m_Kr	= 0.1f;		// 1.0 = SUPERBALL BOUNCE 0.0 = DEAD WEIGHT
	m_Ksh	= 5.0f;		// HOOK'S SPRING CONSTANT
	m_Ksd	= 0.1f;		// SPRING DAMPING CONSTANT
	
	vertexPointSize = 5.0f;

	// CREATE THE SIZE FOR THE SIMULATION WORLD
	m_WorldSizeX = 15.0f;
	m_WorldSizeY = 15.0f;
	m_WorldSizeZ = 15.0f;

	m_CollisionPlane = (tCollisionPlane	*)malloc(sizeof(tCollisionPlane) * 6);
	m_CollisionPlaneCnt = 6;

	// MAKE THE TOP PLANE (CEILING)
	MAKEVECTOR(m_CollisionPlane[0].normal,0.0f, -1.0f, 0.0f)
	m_CollisionPlane[0].d = m_WorldSizeY / 2.0f;

	// MAKE THE BOTTOM PLANE (FLOOR)
	MAKEVECTOR(m_CollisionPlane[1].normal,0.0f, 1.0f, 0.0f)
	m_CollisionPlane[1].d = m_WorldSizeY / 2.0f;

	// MAKE THE LEFT PLANE
	MAKEVECTOR(m_CollisionPlane[2].normal,-1.0f, 0.0f, 0.0f)
	m_CollisionPlane[2].d = m_WorldSizeX / 2.0f;

	// MAKE THE RIGHT PLANE
	MAKEVECTOR(m_CollisionPlane[3].normal,1.0f, 0.0f, 0.0f)
	m_CollisionPlane[3].d = m_WorldSizeX / 2.0f;

	// MAKE THE FRONT PLANE
	MAKEVECTOR(m_CollisionPlane[4].normal,0.0f, 0.0f, -1.0f)
	m_CollisionPlane[4].d = m_WorldSizeZ / 2.0f;

	// MAKE THE BACK PLANE
	MAKEVECTOR(m_CollisionPlane[5].normal,0.0f, 0.0f, 1.0f)
	m_CollisionPlane[5].d = m_WorldSizeZ / 2.0f;
	
//	m_Unscrolling = false;

}

void CPhysEnv::setWorldSize( float x, float y, float z )
{
	m_WorldSizeX = x;
	m_WorldSizeY = y;
	m_WorldSizeZ = z;
	
	// MAKE THE TOP PLANE (CEILING)
	m_CollisionPlane[0].d = m_WorldSizeY / 2.0f;

	// MAKE THE BOTTOM PLANE (FLOOR)
	m_CollisionPlane[1].d = m_WorldSizeY / 2.0f;

	// MAKE THE LEFT PLANE
	m_CollisionPlane[2].d = m_WorldSizeX / 2.0f;

	// MAKE THE RIGHT PLANE
	m_CollisionPlane[3].d = m_WorldSizeX / 2.0f;

	// MAKE THE FRONT PLANE
	m_CollisionPlane[4].d = m_WorldSizeZ / 2.0f;

	// MAKE THE BACK PLANE
	m_CollisionPlane[5].d = m_WorldSizeZ / 2.0f;
}

CPhysEnv::~CPhysEnv()
{
	if (m_ParticleSys[0])
		free(m_ParticleSys[0]);
	if (m_ParticleSys[1])
		free(m_ParticleSys[1]);
	if (m_ParticleSys[2])
		free(m_ParticleSys[2]);
	for (int i = 0; i < 5; i++)
	{
		if (m_TempSys[i])
			free(m_TempSys[i]);
	}
	if (m_Contact)
		free(m_Contact);
	free(m_CollisionPlane);
	free(m_Spring);
//free(lockedArray);
}

void renderBestTextureSplit(int idx, int cell_x, int cell_y)
{
	// render a tex coord for u,v for cell_x,cell_y
	int tileW = manu->tileW;
	int tileH = manu->tileH;
	int border = manu->border;

	int imaW = manu->imaW, imaH = manu->imaH;
	
	int orig_tileH = (int)ceil((double) imaH/(double) TEXH);
	int orig_tileW = (int)ceil((double) imaW/(double) TEXW);

	int orig_pixelH = orig_tileH * TEXH;
	int orig_pixelW = orig_tileW * TEXW;

	double orig_sizeH = (double)1.0/(double)orig_tileH;
	double orig_sizeW = (double)1.0/(double)orig_tileW;

	double cur_u = manu->verList[idx].u1;
	double cur_v = manu->verList[idx].v1;

	// scaled position in a non-bordered tiling
	double u_scaled = cur_u*imaW/(orig_tileW*TEXW);
	double v_scaled = cur_v*imaH/(orig_tileH*TEXH);

	// cut position
	cur_u = u_scaled - ((cell_x * (TEXW - border))/orig_pixelW);
	cur_v = v_scaled - ((cell_y * (TEXH - border))/orig_pixelH);

	// scale the cut position to the local tile
	double cell_percent_x = ((double)1.0/(double)tileW);
	cur_u = cur_u/cell_percent_x;
	cur_u -= cell_x;
	cur_u += ((double)border/(double)TEXW)*cell_x;
	double cell_percent_y = ((double)1.0/(double)tileH);
	cur_v = cur_v/cell_percent_y;
	cur_v -= cell_y;
	cur_v += ((double)border/(double)TEXH)*cell_y;
	// cur_v = (double)1.0 - cur_v;

	// cur_u = (u_scaled-(cell_percent_x*(double)cell_x))/cell_percent_x;
	// cur_u = cur_u * (imaW/(tileW*TEXW));
	
	// cur_v = (((double)1.0-v_scaled)-(cell_percent_y*(double)cell_y))/cell_percent_y;
	// cur_v -= (border/TEXH)*cell_y;
	// cur_v = (double)1.0 - cur_v;

	/*
	if(cell_x || cell_y) {
		printf("%d,%d:\n",cell_x,cell_y);
		printf("%f,%f\t%f,%f\n",u_scaled,v_scaled,cur_u,cur_v);
	}
	*/

	glTexCoord2f( cur_u, cur_v );
}

void bindBestTextureSplit(int idx1, int idx2, int idx3, int &best_x, int &best_y)
{
	int tileW = manu->tileW;
	int tileH = manu->tileH;
	int border = manu->border;

	int imaW = manu->imaW, imaH = manu->imaH;
	
	int orig_tileH = (int)ceil((double) imaH/(double) TEXH);
	int orig_tileW = (int)ceil((double) imaW/(double) TEXW);

	int orig_pixelH = orig_tileH * TEXH;
	int orig_pixelW = orig_tileW * TEXW;

	double orig_sizeH = (double)1.0/(double)orig_tileH;
	double orig_sizeW = (double)1.0/(double)orig_tileW;

	int indexes[3];

	indexes[0] = idx1;
	indexes[1] = idx2;
	indexes[2] = idx3;

	// array containing votes for each texture tile
	// 3 = all three vertices are within this tile
	int* votes = (int*)calloc(tileW*tileH,sizeof(int));

	for(int i = 0; i < 3; i++) {
		double cur_u = manu->verList[indexes[i]].u1;
		double cur_v = manu->verList[indexes[i]].v1;

		// scaled position in a non-bordered tiling
		double u_scaled = cur_u*imaW/(orig_tileW*TEXW);
		double v_scaled = cur_v*imaH/(orig_tileH*TEXH);
		// v_scaled = (double)1.0 - v_scaled;

		for(int y = 0; y < tileH; y++) {
			if(((y * (TEXH - border)) <= (v_scaled * orig_pixelH)) &&
				 ((y * (TEXH - border) + TEXH) > (v_scaled * orig_pixelH)) ) {
				for(int x = 0; x < tileW; x++) {
					// check to see if u,v is in this tile
					if(((x * (TEXW - border)) <= (u_scaled * orig_pixelW)) &&
						 ((x * (TEXW - border) + TEXW) > (u_scaled * orig_pixelW)) ) {
						// votes[(((tileH - 1) - y) * tileW) + x]++;
						votes[(y * tileW) + x]++;
					}
				}
			}
		}
	}

	/*
	for(int i = 0; i < 3; i++) {
		double cur_u = manu->verList[indexes[i]].u1;
		double cur_v = manu->verList[indexes[i]].v1;

		// scaled position in a non-bordered tiling
		double u_scaled = cur_u*imaW/(orig_tileW*TEXW);
		double v_scaled = cur_v*imaH/(orig_tileH*TEXH);
		v_scaled = (double)1.0 - v_scaled;

		printf("\t%f,%f",u_scaled,v_scaled);
	}
	printf(":\n");
	*/

	bool fully_contained = false;

	for(int y = 0; (y < tileH) && (!fully_contained); y++) {
		for(int x = 0; (x < tileW) && (!fully_contained); x++) {
			if(votes[(y * tileW) + x] == 3) {
				best_x = x;
				best_y = y;
				manu->BindArrTexture((y * tileW) + x);
				fully_contained = true;
			}
			// printf("\t%d,%d\t%d\n",x,y,votes[(y * tileW) + x]);
		}
	}

	if(!fully_contained) {
		printf("Found not fully contained polygon\n");
	}

	free(votes);
}

void bindTextureSplit(int idx)
{
	int tileW = manu->tileW;
	int tileH = manu->tileH;

	int imaW = manu->imaW, imaH = manu->imaH;
	
	double u_scaled = manu->verList[idx].u1*imaW/(tileW*TEXW);
	double v_scaled = manu->verList[idx].v1*imaH/(tileH*TEXH);

	int cell_x = (int)floor((double)tileW * u_scaled);
	int cell_y = (int)floor((double)tileH * (1.0 - v_scaled));
	cell_y = (tileH - 1) - cell_y;

	printf("bound: %d,%d\n\n",cell_x,cell_y);
	manu->BindArrTexture((cell_y * tileW) + cell_x);
}

void renderTextureSplit(int idx)
{
	int tileW = manu->tileW;
	int tileH = manu->tileH;

	int imaW = manu->imaW, imaH = manu->imaH;
	
	double u_scaled = manu->verList[idx].u1*imaW/(tileW*TEXW);
	double v_scaled = manu->verList[idx].v1*imaH/(tileH*TEXH);

	int cell_x = (int)floor((double)tileW * u_scaled);
	int cell_y = (int)floor((double)tileH * (1.0 - v_scaled));
	// cell_y = (tileH - 1) - cell_y;

	double cell_percent_x = ((double)1.0/(double)tileW);
	double cur_u = (u_scaled-(cell_percent_x*(double)cell_x))/cell_percent_x;
	// cur_u = cur_u * (imaW/(tileW*TEXW));
	
	double cell_percent_y = ((double)1.0/(double)tileH);
	double cur_v = (((double)1.0-v_scaled)-(cell_percent_y*(double)cell_y))/cell_percent_y;
	// cur_v = (((double)1.0) - cur_v) * (imaH/(tileH*TEXH));
	cur_v = (double)1.0 - cur_v;
	// cur_v = cur_v * ((double)imaH/(double)(tileH*TEXH));

	// printf("%f,%f\t%f,%f\n",u_scaled,v_scaled,cur_u,cur_v);
	
	glTexCoord2f( cur_u, cur_v );
}

void CPhysEnv::RenderWorld()
{
	tParticle	*tempParticle;
	tSpring		*tempSpring;

	// FIRST DRAW THE WORLD CONTAINER  
	glColor3f(1.0f,1.0f,1.0f);
  	/*  
		// do a big linestrip to get most of edges
    glBegin(GL_LINE_STRIP);
        glVertex3f(-m_WorldSizeX/2.0f, m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f, m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f, m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f, m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f, m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
    glEnd();
    // fill in the stragglers
    glBegin(GL_LINES);
        glVertex3f( m_WorldSizeX/2.0f, m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f,-m_WorldSizeZ/2.0f);

        glVertex3f( m_WorldSizeX/2.0f, m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);

        glVertex3f(-m_WorldSizeX/2.0f, m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f, m_WorldSizeZ/2.0f);
    glEnd();
    
    // draw floor
    glBegin(GL_QUADS);
        glColor3f(0.3f,0.3f,0.3f);
        glVertex3f(-m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f-0.1,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f-0.1,-m_WorldSizeZ/2.0f);
        glVertex3f( m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f-0.1, m_WorldSizeZ/2.0f);
        glVertex3f(-m_WorldSizeX/2.0f,-m_WorldSizeY/2.0f-0.1, m_WorldSizeZ/2.0f);
    glEnd();
		*/
	
	if (m_ParticleSys)
	{
		if (m_Spring && m_DrawSprings)
		{
			glLineWidth(2);
			

			glBegin(GL_LINES);

			glColor3f(0.0f,1.0f,0.0f);

			tempSpring = m_Spring;

			for (int loop = 0; loop < m_SpringCnt; loop++)
			{
				// Only draw normal springs or the cloth "structural" ones
				if ((tempSpring->type == MANUAL_SPRING) ||
					(tempSpring->type == STRUCTURAL_SPRING && m_DrawStructural))
				{
					glVertex3fv((float *)&m_CurrentSys[tempSpring->p1].pos);
					glVertex3fv((float *)&m_CurrentSys[tempSpring->p2].pos);
				}
				tempSpring++;
			}
			
		
			if (m_MouseForceActive)	// DRAW MOUSESPRING FORCE
			{
				if (m_Pick[0] > -1)
				{
					glColor3f(0.8f,0.0f,0.8f);
					glVertex3fv((float *)&m_CurrentSys[m_Pick[0]].pos);
					glVertex3fv((float *)&m_MouseDragPos[0]);
				}
				if (m_Pick[1] > -1)
				{
					glColor3f(0.8f,0.0f,0.8f);
					glVertex3fv((float *)&m_CurrentSys[m_Pick[1]].pos);
					glVertex3fv((float *)&m_MouseDragPos[1]);
				}
			}
			
			glEnd();
		}
		
		if (manu)
		{
			glColor3f(1.0, 1.0, 1.0 );
			glPolygonMode( GL_FRONT, GL_FILL );
			glPolygonMode( GL_BACK, GL_FILL );
			
			tempParticle = m_CurrentSys;

			if (manu->YL_UseQuad)
			{
				manu->BindNextTexture();		
				for(int j=0; j < manu->ySamples-1; j++)
					for(int i=0; i < manu->xSamples-1; i++)
					{
						int offset;
						int xSamples = manu->xSamples;
		
						glBegin(GL_POLYGON);
						offset = j*(xSamples)+i;
						tempParticle = &m_CurrentSys[ offset ];	
						glTexCoord2f( manu->verList[offset].u1,
										manu->verList[offset].v1  );

						glVertex3fv((float *)&tempParticle->pos);

						offset = j * (xSamples)+i+1;
						tempParticle = &m_CurrentSys[ offset ];
						glTexCoord2f( manu->verList[offset].u1,
								  		manu->verList[offset].v1);
					

						glVertex3fv((float *)&tempParticle->pos);	
					

						offset = (j+1) * (xSamples)+i+1;
						tempParticle = &m_CurrentSys[ offset ];
						glTexCoord2f( manu->verList[offset].u1,
										manu->verList[offset].v1 );
						

						glVertex3fv((float *)&tempParticle->pos);	

						offset = (j+1) * (xSamples)+i;
						tempParticle = &m_CurrentSys[ offset ];
						glTexCoord2f( manu->verList[offset].u1,
										manu->verList[offset].v1);
						
						glVertex3fv((float *)&tempParticle->pos);

						glEnd();
				}
			}
			// For the case of Triangular Mesh
			else if( manu->YL_UseTriangularTextureMap )
			{
				int imaW = manu->imaW, imaH = manu->imaH;

				if( manu->firstRun )
				{
					int minusTriangles = 0, index;
					int idx, trigtexres = manu->TRIGTEXRES + 1, numtrigperrow = manu->NUMTRIGPERROW;
					for (int i = 0; i < manu->nTrig; i++ )
					{
						if( ( i % manu->numberOfTrianglesInATexture ) == 0 )
						{
							manu->BindNextTexture();
							minusTriangles = manu->numberOfTrianglesInATexture * ( i / manu->numberOfTrianglesInATexture );
						}
						
						index = i - minusTriangles;
						glBegin(GL_POLYGON);
						idx = manu->trigList[i].idx1;
						tempParticle = &m_CurrentSys[idx];
						manu->trigList[i].textureVertex1[0] = (float)(((index%numtrigperrow)*trigtexres)+1)/(float)TEXW;
						manu->trigList[i].textureVertex1[1] = (float)(((index/numtrigperrow)*trigtexres)+1)/(float)TEXH;
						glTexCoord2fv( manu->trigList[i].textureVertex1 );
						glVertex3fv((float *)&tempParticle->pos);
						
						idx = manu->trigList[i].idx2;
						tempParticle = &m_CurrentSys[idx];
						manu->trigList[i].textureVertex2[0] = (float)((((index%numtrigperrow)+1)*trigtexres)-1)/(float)TEXW;
						manu->trigList[i].textureVertex2[1] = (float)(((index/numtrigperrow)*trigtexres)+1)/(float)TEXH;
						glTexCoord2fv( manu->trigList[i].textureVertex2 );
						glVertex3fv((float *)&tempParticle->pos);
						
						idx = manu->trigList[i].idx3;
						tempParticle = &m_CurrentSys[idx];
						manu->trigList[i].textureVertex3[0] = (float)(((index%numtrigperrow)*trigtexres)+1)/(float)TEXW;
						manu->trigList[i].textureVertex3[1] = (float)((((index/numtrigperrow)+1)*trigtexres)-1)/(float)TEXH;
						glTexCoord2fv( manu->trigList[i].textureVertex3 );
						glVertex3fv((float *)&tempParticle->pos);
						glEnd();
					}
					printf( "%d %d\n", numtrigperrow, trigtexres );
					manu->firstRun = false;
				}
				else // if manu->firstRun == false
				{
					int minusTriangles = 0;
					for (int i = 0; i < manu->nTrig; i++ )
					{
						if( ( i % manu->numberOfTrianglesInATexture ) == 0 )
						{
							manu->BindNextTexture();
							minusTriangles = manu->numberOfTrianglesInATexture * ( i / manu->numberOfTrianglesInATexture );
						}
						
					int idx;
					glBegin(GL_POLYGON);
						idx = manu->trigList[i].idx1;
						tempParticle = &m_CurrentSys[idx];
						glTexCoord2fv( manu->trigList[i].textureVertex1 );

						glVertex3fv((float *)&tempParticle->pos);
						
						idx = manu->trigList[i].idx2;
						tempParticle = &m_CurrentSys[idx];
						glTexCoord2fv( manu->trigList[i].textureVertex2 );

						glVertex3fv((float *)&tempParticle->pos);
						
						idx = manu->trigList[i].idx3;
						tempParticle = &m_CurrentSys[idx];
						glTexCoord2fv( manu->trigList[i].textureVertex3 );
						
						glVertex3fv((float *)&tempParticle->pos);
					glEnd();
					}
				}
				glDisable(GL_TEXTURE_2D);
		}
		else{
			// THIS
			if(manu->texArray == true) {
				// manu->BindArrTexture(0);
				for (int i = 0; i < manu->nTrig; i++ ){
					int idx, cur_x, cur_y;
					bindBestTextureSplit(manu->trigList[i].idx1,
							manu->trigList[i].idx2, manu->trigList[i].idx3,cur_x,cur_y);

					idx = manu->trigList[i].idx1;
					// bindTextureSplit(idx);	
					glBegin(GL_POLYGON);
					tempParticle = &m_CurrentSys[idx];
					// renderTextureSplit(idx);	
					renderBestTextureSplit(idx,cur_x,cur_y);	

					glVertex3fv((float *)&tempParticle->pos);
					idx = manu->trigList[i].idx2;
					tempParticle = &m_CurrentSys[idx];
					// renderTextureSplit(idx);	
					renderBestTextureSplit(idx,cur_x,cur_y);	

					glVertex3fv((float *)&tempParticle->pos);
					idx = manu->trigList[i].idx3;
					tempParticle = &m_CurrentSys[idx];
					// renderTextureSplit(idx);		
					renderBestTextureSplit(idx,cur_x,cur_y);	

					glVertex3fv((float *)&tempParticle->pos);
					glEnd();
				}
				glDisable(GL_TEXTURE_2D);

			}
			else {
				manu->BindNextTexture();
				int imaW = manu->imaW, imaH = manu->imaH;
				for (int i = 0; i < manu->nTrig; i++ ){
					glBegin(GL_POLYGON);
					int idx;
					idx = manu->trigList[i].idx1;
					tempParticle = &m_CurrentSys[idx];
					glTexCoord2f( manu->verList[idx].u1*imaW/TEXW, manu->verList[idx].v1*imaH/TEXH);
					glVertex3fv((float *)&tempParticle->pos);
					idx = manu->trigList[i].idx2;
					tempParticle = &m_CurrentSys[idx];
					glTexCoord2f( manu->verList[idx].u1*imaW/TEXW, manu->verList[idx].v1*imaH/TEXH);
					glVertex3fv((float *)&tempParticle->pos);
					idx = manu->trigList[i].idx3;
					tempParticle = &m_CurrentSys[idx];
					glTexCoord2f( manu->verList[idx].u1*imaW/TEXW, manu->verList[idx].v1*imaH/TEXH);
					glVertex3fv((float *)&tempParticle->pos);
					glEnd();
				}
				glDisable(GL_TEXTURE_2D);
			}
		}
	 } // end if (manu)

		if (m_DrawVertices)
		{
			glColor3f(1.0, 0.0, 0.0);
			glPointSize(vertexPointSize);
			glBegin(GL_POINTS);
			tempParticle = m_CurrentSys;
			int idx = 0;
			for (int loop = 0; loop < m_ParticleCnt; loop++)
			{
				if( loop == m_Pick[0] ){ glColor3f(0.0f,0.8f,0.0f); }
				else if( loop == m_Pick[1] ){ glColor3f(0.8f,0.0f,0.0f); }
				else if( m_LockParticles && lockedArray[loop] ){ glColor3f(1.0f,0.0f,0.0f); }
				else{ glColor3f(0.0f,0.4f,1.0f); }
				
				glVertex3fv((float *)&tempParticle->pos);

				float *vector = (float *)&tempParticle->pos;

				manu->verList[idx].x = vector[0];
				manu->verList[idx].y = vector[2];
				manu->verList[idx].z = vector[1];
				idx++;
				tempParticle++;
			}
			glEnd();
		}
			// try may texture if you can!
		
			
	}
	glFlush();

}

void CPhysEnv::GetNearestPoint(int x, int y)
{
/// Local Variables ///////////////////////////////////////////////////////////
	float *feedBuffer;
	int hitCount;
	tParticle *tempParticle;
	int loop;
///////////////////////////////////////////////////////////////////////////////
	// INITIALIZE A PLACE TO PUT ALL THE FEEDBACK INFO (3 DATA, 1 TAG, 2 TOKENS)
	feedBuffer = (float *)malloc(sizeof(GLfloat) * m_ParticleCnt * 6);
	// TELL OPENGL ABOUT THE BUFFER
	glFeedbackBuffer(m_ParticleCnt * 6,GL_3D,feedBuffer);
	(void)glRenderMode(GL_FEEDBACK);	// SET IT IN FEEDBACK MODE

	tempParticle = m_CurrentSys;
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		// PASS THROUGH A MARKET LETTING ME KNOW WHAT VERTEX IT WAS
		glPassThrough((float)loop);
		// SEND THE VERTEX
		glBegin(GL_POINTS);
		glVertex3fv((float *)&tempParticle->pos);
		glEnd();
		tempParticle++;
	}
	hitCount = glRenderMode(GL_RENDER); // HOW MANY HITS DID I GET
	CompareBuffer(hitCount,feedBuffer,(float)x,(float)y);		// CHECK THE HIT 
	free(feedBuffer);		// GET RID OF THE MEMORY
}

bool CPhysEnv::GetParticlePosition( int index, float position[] )
{
	if( index >= 0 && index < m_ParticleCnt )
	{
		position[0] = m_CurrentSys[index].pos.x;
		position[1] = m_CurrentSys[index].pos.y;
		position[2] = m_CurrentSys[index].pos.z;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	CompareBuffer
// Purpose:		Check the feedback buffer to see if anything is hit
// Arguments:	Number of hits, pointer to buffer, point to test
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::CompareBuffer(int size, float *buffer,float x, float y)
{
/// Local Variables ///////////////////////////////////////////////////////////
	GLint count;
	GLfloat token,point[3], glX, glY;
	int loop,currentVertex,result = -1;
	GLfloat nearest = -1, dist;
///////////////////////////////////////////////////////////////////////////////
	count = size;
	while (count)
	{
		token = buffer[size - count];	// CHECK THE TOKEN
		count--;
		if (token == GL_PASS_THROUGH_TOKEN)	// VERTEX MARKER
		{
			currentVertex = (int)buffer[size - count]; // WHAT VERTEX
			count--;
		}
		else if (token == GL_POINT_TOKEN)
		{
			// THERE ARE THREE ELEMENTS TO A POINT TOKEN
			for (loop = 0; loop < 3; loop++)
			{
				point[loop] = buffer[size - count];
				count--;
			}
			
			glX = (GLfloat)x; glY = (GLfloat)y;
			dist = ((x - point[0]) * (x - point[0])) + ((y - point[1]) * (y - point[1]));
			if (result == -1 || dist < nearest)
			{
				nearest = dist;
				result = currentVertex;
			}
		}
	}

	if (nearest < 50.0f)
	{
		if (m_Pick[0] == -1)
			m_Pick[0] = result;
		else if (m_Pick[1] == -1)
			m_Pick[1] = result;
		else
		{
			m_Pick[0] = result;
			m_Pick[1] = -1;
		}
	}
}
////// CompareBuffer //////////////////////////////////////////////////////////

void CPhysEnv::SetWorldParticles(tTexturedVertex *coords,int particleCnt)
{
	tParticle *tempParticle;

	if (m_ParticleSys[0])
		free(m_ParticleSys[0]);
	if (m_ParticleSys[1])
		free(m_ParticleSys[1]);
	if (m_ParticleSys[2])
		free(m_ParticleSys[2]);
	for (int i = 0; i < 5; i++)
	{
		if (m_TempSys[i])
			free(m_TempSys[i]);
	}
	if (m_Contact)
		free(m_Contact);
	// THE SYSTEM IS DOUBLE BUFFERED TO MAKE THINGS EASIER

	m_CurrentSys = (tParticle *)malloc(sizeof(tParticle) * particleCnt);
	m_TargetSys = (tParticle *)malloc(sizeof(tParticle) * particleCnt);
	m_ParticleSys[2] = (tParticle *)malloc(sizeof(tParticle) * particleCnt);

	for(int i = 0; i < 5; i++)
	{
		m_TempSys[i] = (tParticle *)malloc(sizeof(tParticle) * particleCnt);
	}
	m_ParticleCnt = particleCnt;

	// MULTIPLIED PARTICLE COUNT * 2 SINCE THEY CAN COLLIDE WITH MULTIPLE WALLS
	m_Contact = (tContact *)malloc(sizeof(tContact) * particleCnt * 2);
	m_ContactCnt = 0;

	tempParticle = m_CurrentSys;
	for (int loop = 0; loop < particleCnt; loop++)
	{
		MAKEVECTOR(tempParticle->pos, coords->x, coords->y, coords->z)
		MAKEVECTOR(tempParticle->v, 0.0f, 0.0f, 0.0f)
		MAKEVECTOR(tempParticle->f, 0.0f, 0.0f, 0.0f)
	//  tempParticle->oneOverM = 1.0f;							// MASS OF 1
	    tempParticle->oneOverM = 12.0f;
		tempParticle++;
		coords++;
	}

	// COPY THE SYSTEM TO THE SECOND ONE ALSO
	memcpy(m_TargetSys,m_CurrentSys,sizeof(tParticle) * particleCnt);
	// COPY THE SYSTEM TO THE RESET BUFFER ALSO
	memcpy(m_ParticleSys[2],m_CurrentSys,sizeof(tParticle) * particleCnt);

	m_ParticleSys[0] = m_CurrentSys;
	m_ParticleSys[1] = m_TargetSys;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	FreeSystem
// Purpose:		Remove all particles and clear it out
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::FreeSystem()
{
	m_Pick[0] = -1;
	m_Pick[1] = -1;
	if (m_ParticleSys[0])
	{
		m_ParticleSys[0] = NULL;
		free(m_ParticleSys[0]);
	}
	if (m_ParticleSys[1])
	{
		free(m_ParticleSys[1]);
		m_ParticleSys[1] = NULL;
	}
	if (m_ParticleSys[2])
	{
		free(m_ParticleSys[2]);
		m_ParticleSys[2] = NULL;	// RESET BUFFER
	}
	for (int i = 0; i < 5; i++)
	{
		if (m_TempSys[i])
		{
			free(m_TempSys[i]);
			m_TempSys[i] = NULL;	// RESET BUFFER
		}
	}
	if (m_Contact)
	{
		free(m_Contact);
		m_Contact = NULL;
	}
	if (m_Spring)
	{
		free(m_Spring);
		m_Spring = NULL;
	}
	m_SpringCnt = 0;	
	m_ParticleCnt = 0;
	
}
////// FreeSystem //////////////////////////////////////////////////////////////

void CPhysEnv::LoadData(FILE *fp)
{
	//fread(&m_UseGravity,sizeof(bool),1,fp);
	//fread(&m_UseDamping,sizeof(bool),1,fp);
	//fread(&m_UserForceActive,sizeof(bool),1,fp);
	int temp;
	fread(&temp,sizeof(int),1,fp);
	if( temp == 1 ) m_UseGravity = true;
	else m_UseGravity = false;
	
	fread(&temp,sizeof(int),1,fp);
	if( temp == 1 ) m_UseDamping = true;
	else m_UseDamping = false;
	
	fread(&temp,sizeof(int),1,fp);
	if( temp == 1 ) m_UserForceActive = true;
	else m_UserForceActive = false;
	
	fread(&m_Gravity,sizeof(tVector),1,fp);
	fread(&m_UserForce,sizeof(tVector),1,fp);
	fread(&m_UserForceMag,sizeof(float),1,fp);
	fread(&m_Kd,sizeof(float),1,fp);
	fread(&m_Kr,sizeof(float),1,fp);
	fread(&m_Ksh,sizeof(float),1,fp);
	fread(&m_Ksd,sizeof(float),1,fp);
	fread(&m_ParticleCnt,sizeof(int),1,fp);
	m_CurrentSys = (tParticle *)malloc(sizeof(tParticle) * m_ParticleCnt);
	m_TargetSys = (tParticle *)malloc(sizeof(tParticle) * m_ParticleCnt);
	m_ParticleSys[2] = (tParticle *)malloc(sizeof(tParticle) * m_ParticleCnt);
	for (int i = 0; i < 5; i++)
	{
		m_TempSys[i] = (tParticle *)malloc(sizeof(tParticle) * m_ParticleCnt);
	}
	m_ParticleSys[0] = m_CurrentSys;
	m_ParticleSys[1] = m_TargetSys;
	m_Contact = (tContact *)malloc(sizeof(tContact) * m_ParticleCnt);
	fread(m_ParticleSys[0],sizeof(tParticle),m_ParticleCnt,fp);
	fread(m_ParticleSys[1],sizeof(tParticle),m_ParticleCnt,fp);
	fread(m_ParticleSys[2],sizeof(tParticle),m_ParticleCnt,fp);
	fread(&m_SpringCnt,sizeof(int),1,fp);
	m_Spring = (tSpring *)malloc(sizeof(tSpring) * (m_SpringCnt));
	fread(m_Spring,sizeof(tSpring),m_SpringCnt,fp);
	fread(m_Pick,sizeof(int),2,fp);
}

void CPhysEnv::SaveData(FILE *fp)
{
	fwrite(&m_UseGravity,sizeof(bool),1,fp);
	fwrite(&m_UseDamping,sizeof(bool),1,fp);
	fwrite(&m_UserForceActive,sizeof(bool),1,fp);
	fwrite(&m_Gravity,sizeof(tVector),1,fp);
	fwrite(&m_UserForce,sizeof(tVector),1,fp);
	fwrite(&m_UserForceMag,sizeof(float),1,fp);
	fwrite(&m_Kd,sizeof(float),1,fp);
	fwrite(&m_Kr,sizeof(float),1,fp);
	fwrite(&m_Ksh,sizeof(float),1,fp);
	fwrite(&m_Ksd,sizeof(float),1,fp);
	fwrite(&m_ParticleCnt,sizeof(int),1,fp);
	fwrite(m_ParticleSys[0],sizeof(tParticle),m_ParticleCnt,fp);
	fwrite(m_ParticleSys[1],sizeof(tParticle),m_ParticleCnt,fp);
	fwrite(m_ParticleSys[2],sizeof(tParticle),m_ParticleCnt,fp);
	fwrite(&m_SpringCnt,sizeof(int),1,fp);
	fwrite(m_Spring,sizeof(tSpring),m_SpringCnt,fp);
	fwrite(m_Pick,sizeof(int),2,fp);
}

// RESET THE SIM TO INITIAL VALUES
void CPhysEnv::ResetWorld()
{
	memcpy(m_CurrentSys,m_ParticleSys[2],sizeof(tParticle) * m_ParticleCnt);
	memcpy(m_TargetSys,m_ParticleSys[2],sizeof(tParticle) * m_ParticleCnt);
}

void CPhysEnv::SetWorldProperties( float simProperties[] )
{
	m_Kr = simProperties[0];
	m_Kd = simProperties[1];
	m_Gravity.x = simProperties[2];
	m_Gravity.y = simProperties[3];
	m_Gravity.z = simProperties[4];
	m_Ksh = simProperties[5];
	m_Ksd = simProperties[6];
	m_UserForceMag = simProperties[7];
	
	for (int loop = 0; loop < m_SpringCnt; loop++)
	{
		m_Spring[loop].Ks = m_Ksh;
		m_Spring[loop].Kd = m_Ksd;
	}
}

void CPhysEnv::SetVertexProperties( float m_VertexMass )
{
	m_ParticleSys[0][m_Pick[0]].oneOverM = m_VertexMass;
	m_ParticleSys[0][m_Pick[1]].oneOverM = m_VertexMass;
	m_ParticleSys[1][m_Pick[0]].oneOverM = m_VertexMass;
	m_ParticleSys[1][m_Pick[1]].oneOverM = m_VertexMass;
	m_ParticleSys[2][m_Pick[0]].oneOverM = m_VertexMass;
	m_ParticleSys[2][m_Pick[1]].oneOverM = m_VertexMass;
}

void CPhysEnv::ApplyUserForce(tVector *force)
{
	ScaleVector(force, m_UserForceMag, &m_UserForce);
	m_UserForceActive = true;
}

void CPhysEnv::SetSelectedParticle( int number )
{
	m_Pick[0] = number;
}

///////////////////////////////////////////////////////////////////////////////
// Function:	SetMouseForce 
// Purpose:		Allows the user to interact with selected points by dragging
// Arguments:	Delta distance from clicked point, local x and y axes
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::SetMouseForce(int deltaX,int deltaY, tVector *localX, tVector *localY)
{
/// Local Variables ///////////////////////////////////////////////////////////
	tVector tempX,tempY;
///////////////////////////////////////////////////////////////////////////////
	ScaleVector(localX,  (float)deltaX * 0.03f, &tempX);
	ScaleVector(localY, -(float)deltaY * 0.03f, &tempY);
	if (m_Pick[0] > -1)
	{
		VectorSum(&m_CurrentSys[m_Pick[0]].pos,&tempX,&m_MouseDragPos[0]);
		VectorSum(&m_MouseDragPos[0],&tempY,&m_MouseDragPos[0]);
	}
	if (m_Pick[1] > -1)
	{
		VectorSum(&m_CurrentSys[m_Pick[1]].pos,&tempX,&m_MouseDragPos[1]);
		VectorSum(&m_MouseDragPos[1],&tempY,&m_MouseDragPos[1]);
	}
}
/// SetMouseForce /////////////////////////////////////////////////////////////

/*/
void CPhysEnv::AddSpring()
{
	tSpring	*spring;
	// MAKE SURE TWO PARTICLES ARE PICKED
	if (m_Pick[0] > -1 && m_Pick[1] > -1)
	{
		spring = (tSpring *)malloc(sizeof(tSpring) * (m_SpringCnt + 1));
		if (m_Spring != NULL)
			memcpy(spring,m_Spring,sizeof(tSpring) * m_SpringCnt);
		m_Spring = spring;
		spring = &m_Spring[m_SpringCnt++];
		spring->Ks = m_Ksh;
		spring->Kd = m_Ksd;
		spring->p1 = m_Pick[0];
		spring->p2 = m_Pick[1];
		spring->restLen = 
			sqrt(VectorSquaredDistance(&m_CurrentSys[m_Pick[0]].pos, 
									   &m_CurrentSys[m_Pick[1]].pos));
		spring->type = 	MANUAL_SPRING;
	}
}
/*/

void CPhysEnv::SmartAddSpringInit( int sizeOfArray )
{
	m_SpringCnt = sizeOfArray;
	if( m_Spring != NULL ) free(m_Spring);
	m_Spring = (tSpring *)malloc(sizeof(tSpring)*m_SpringCnt);
	
	smartAddSpringCurrentSpring = 0;
}

void CPhysEnv::SmartAddSpring( int v1, int v2, float Ksh, float Ksd, int type )
{
	tSpring	*spring;
	// MAKE SURE TWO PARTICLES ARE PICKED
	if (v1 > -1 && v2 > -1)
	{
		spring = &m_Spring[smartAddSpringCurrentSpring++];
		spring->type = type;
		spring->Ks = Ksh;
		spring->Kd = Ksd;
		spring->p1 = v1;
		spring->p2 = v2;
		spring->restLen = sqrt(VectorSquaredDistance(&m_CurrentSys[v1].pos, &m_CurrentSys[v2].pos));
	}
}

void CPhysEnv::AddSpring(int v1, int v2,float Ksh,float Ksd, int type)
{
	tSpring	*spring;
	// MAKE SURE TWO PARTICLES ARE PICKED
	if (v1 > -1 && v2 > -1)
	{
		spring = (tSpring *)malloc(sizeof(tSpring) * (m_SpringCnt + 1));
		if (m_Spring != NULL)
		{
			memcpy(spring,m_Spring,sizeof(tSpring) * m_SpringCnt);
			free(m_Spring);
		}
		m_Spring = spring;
		spring = &m_Spring[m_SpringCnt++];
		spring->type = type;
		spring->Ks = Ksh;
		spring->Kd = Ksd;
		spring->p1 = v1;
		spring->p2 = v2;
		spring->restLen = sqrt(VectorSquaredDistance(&m_CurrentSys[v1].pos, &m_CurrentSys[v2].pos));
	}
}

int CPhysEnv::GetNumberOfParticles()
{
	return m_ParticleCnt;
}

void CPhysEnv::ComputeForces( tParticle *system )
{
	int loop;
	tParticle	*curParticle,*p1, *p2;
	tSpring		*spring;
	float		dist, Hterm, Dterm;
	tVector		springForce,deltaV,deltaP;
	
	curParticle = system;
#pragma omp parallel for
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		curParticle = system + loop;
		MAKEVECTOR(curParticle->f,0.0f,0.0f,0.0f)		// CLEAR FORCE VECTOR

		if (m_UseGravity)
		{
			curParticle->f.x += (m_Gravity.x / curParticle->oneOverM);
			curParticle->f.y += (m_Gravity.y / curParticle->oneOverM);
			curParticle->f.z += (m_Gravity.z / curParticle->oneOverM);
		}

		if (m_UseDamping)
		{
			curParticle->f.x += (-m_Kd * curParticle->v.x);
			curParticle->f.y += (-m_Kd * curParticle->v.y);
			curParticle->f.z += (-m_Kd * curParticle->v.z);
		}
		else
		{
			curParticle->f.x += (-DEFAULT_DAMPING * curParticle->v.x);
			curParticle->f.y += (-DEFAULT_DAMPING * curParticle->v.y);
			curParticle->f.z += (-DEFAULT_DAMPING * curParticle->v.z);
		}
	}
	
	/*
	//My hack here
	int i;
	float vx,vy,vz,totalM;
	tVector     m_MyForce;
	totalM=0;
	curParticle = system;
    for (i = 0; i < m_ParticleCnt; i++){
		totalM += 1/curParticle->oneOverM;
		curParticle++;
	}
	MAKEVECTOR(m_MyForce,0.0f,0.0f,0.0f);
	if (m_Unscrolling) {
		MAKEVECTOR(m_MyForce,-m_Gravity.x*totalM,-m_Gravity.y*totalM,-m_Gravity.z*totalM);
		for (i = 0; i < manu->ySamples; i++){
			system[i*manu->xSamples].f.x += m_MyForce.x;
			system[i*manu->xSamples].f.y += m_MyForce.y;
			system[i*manu->xSamples].f.z += m_MyForce.z;

		}
	}
	else {
		MAKEVECTOR(m_MyForce,-m_MyForce.x,-m_MyForce.y,-m_MyForce.z);
		for (i = 0; i < manu->ySamples; i++){
			system[i*manu->xSamples].f.x += m_MyForce.x;
			system[i*manu->xSamples].f.y += m_MyForce.y;
			system[i*manu->xSamples].f.z += m_MyForce.z;
		}
	}
	*/

	// CHECK IF THERE IS A USER FORCE BEING APPLIED
	if (m_UserForceActive)
	{
		if( m_Pick[0] != -1 )
		{
			VectorSum(&system[m_Pick[0]].f,&m_UserForce,&system[m_Pick[0]].f);
		}
		if( m_Pick[1] != -1 )
		{
			VectorSum(&system[m_Pick[1]].f,&m_UserForce,&system[m_Pick[1]].f);
		}
		MAKEVECTOR(m_UserForce,0.0f,0.0f,0.0f);	// CLEAR USER FORCE
	}
	
	// NOW DO ALL THE SPRINGS
	spring = m_Spring;
#pragma omp parallel for
	for (loop = 0; loop < m_SpringCnt; loop++)
	{
		spring = m_Spring + loop;
		p1 = &system[spring->p1];
		p2 = &system[spring->p2];
		
		VectorDifference(&p1->pos,&p2->pos,&deltaP);	// Vector distance 
		dist = VectorLength(&deltaP);					// Magnitude of deltaP
		Hterm = (dist - spring->restLen) * spring->Ks;	// Ks * (dist - rest)
		
		VectorDifference(&p1->v,&p2->v,&deltaV);		// Delta Velocity Vector
		Dterm = (DotProduct(&deltaV,&deltaP) * spring->Kd) / dist; // Damping Term
		
		ScaleVector(&deltaP,1.0f / dist, &springForce);	// Normalize Distance Vector
		ScaleVector(&springForce,-(Hterm + Dterm),&springForce);	// Calc Force
		VectorSum(&p1->f,&springForce,&p1->f);			// Apply to Particle 1
		VectorDifference(&p2->f,&springForce,&p2->f);	// - Force on Particle 2
	}
	
	
	// APPLY THE MOUSE DRAG FORCES IF THEY ARE ACTIVE
	if (m_MouseForceActive)
	{
		// APPLY TO EACH PICKED PARTICLE
		if (m_Pick[0] > -1)
		{
			p1 = &system[m_Pick[0]];
			VectorDifference(&p1->pos,&m_MouseDragPos[0],&deltaP);	// Vector distance 
			dist = VectorLength(&deltaP);					// Magnitude of deltaP

			if (dist != 0.0f)
			{
				Hterm = (dist) * m_MouseForceKs;					// Ks * dist

				ScaleVector(&deltaP,1.0f / dist, &springForce);	// Normalize Distance Vector
				ScaleVector(&springForce,-(Hterm),&springForce);	// Calc Force
				VectorSum(&p1->f,&springForce,&p1->f);			// Apply to Particle 1
			}
		}
		if (m_Pick[1] > -1)
		{
			p1 = &system[m_Pick[1]];
			VectorDifference(&p1->pos,&m_MouseDragPos[1],&deltaP);	// Vector distance 
			dist = VectorLength(&deltaP);					// Magnitude of deltaP

			if (dist != 0.0f)
			{
				Hterm = (dist) * m_MouseForceKs;					// Ks * dist

				ScaleVector(&deltaP,1.0f / dist, &springForce);	// Normalize Distance Vector
				ScaleVector(&springForce,-(Hterm),&springForce);	// Calc Force
				VectorSum(&p1->f,&springForce,&p1->f);			// Apply to Particle 1
			}
		}
	}

	if( m_LockParticles && lockedArray != NULL )
	{
		for ( int i = 0; i < m_ParticleCnt; i++ )
		{
			if( lockedArray[i] )
			{
				MAKEVECTOR(system[i].f,0.0f,0.0f,0.0f);
				MAKEVECTOR(system[i].v,0.0f,0.0f,0.0f);
			}
		}

	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	IntegrateSysOverTime 
// Purpose:		Does the Integration for all the points in a system
// Arguments:	Initial Position, Source and Target Particle Systems and Time
// Notes:		Computes a single integration step
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::IntegrateSysOverTime(tParticle *initial,tParticle *source, tParticle *target, float deltaTime)
{
	float deltaTimeMass;
	
	for( int loop = 0; loop < m_ParticleCnt; loop++)
	{
		deltaTimeMass = deltaTime * initial->oneOverM;
		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE
		
		if( m_UseXAxis ) target->v.x = initial->v.x + (source->f.x * deltaTimeMass);
		if( m_UseYAxis ) target->v.y = initial->v.y + (source->f.y * deltaTimeMass);
		if( m_UseZAxis ) target->v.z = initial->v.z + (source->f.z * deltaTimeMass);
		target->oneOverM = initial->oneOverM;
		
		// SET THE NEW POSITION
		if( m_UseXAxis ) target->pos.x = initial->pos.x + (deltaTime * source->v.x);
		if( m_UseYAxis ) target->pos.y = initial->pos.y + (deltaTime * source->v.y);
		if( m_UseZAxis ) target->pos.z = initial->pos.z + (deltaTime * source->v.z);
		
		initial++;
		source++;
		target++;
	}
}

void CPhysEnv::RotateSystem( float degree, bool running )
{
	float cy, cz, radian = ( degree * M_PI / 180.0f );
	
	if( !running )
	{
		tParticle *current = m_CurrentSys;
		tParticle *target = m_TargetSys;
		
		for( int loop = 0; loop < m_ParticleCnt; loop++)
		{
			cy = current->pos.y;
			cz = current->pos.z;
			
			current->pos.y = ( cos(radian) * cy ) + ( sin(radian) * -cz );
			current->pos.z = ( sin(radian) * cy ) + ( cos(radian) * cz );

			target->pos.x = current->pos.x;
			target->pos.y = current->pos.y;
			target->pos.z = current->pos.z;
			
			current++;
			target++;
		}
	}
}

void CPhysEnv::TranslateSystem( float deltaX, float deltaY, float deltaZ, bool running )
{
	if( !running )
	{
		tParticle *current = m_CurrentSys;
		tParticle *target = m_TargetSys;
		
		for( int loop = 0; loop < m_ParticleCnt; loop++)
		{
			current->pos.x += deltaX;
			current->pos.y += deltaY;
			current->pos.z += deltaZ;
			
			target->pos.x = current->pos.x;
			target->pos.y = current->pos.y;
			target->pos.z = current->pos.z;
			
			current++;
			target++;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Function:	EulerIntegrate 
// Purpose:		Calculate new Positions and Velocities given a deltatime
// Arguments:	DeltaTime that has passed since last iteration
// Notes:		This integrator uses Euler's method
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::EulerIntegrate( float DeltaTime)
{
	// JUST TAKE A SINGLE STEP
	IntegrateSysOverTime( m_CurrentSys, m_CurrentSys, m_TargetSys, DeltaTime );
}

///////////////////////////////////////////////////////////////////////////////
// Function:	MidPointIntegrate 
// Purpose:		Calculate new Positions and Velocities given a deltatime
// Arguments:	DeltaTime that has passed since last iteration
// Notes:		This integrator uses the Midpoint method
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::MidPointIntegrate( float DeltaTime )
{
/// Local Variables ///////////////////////////////////////////////////////////
	float		halfDeltaT;
///////////////////////////////////////////////////////////////////////////////
	halfDeltaT = DeltaTime / 2.0f;

	// TAKE A HALF STEP AND UPDATE VELOCITY AND POSITION
	IntegrateSysOverTime(m_CurrentSys,m_CurrentSys,m_TempSys[0],halfDeltaT);
	
	// COMPUTE FORCES USING THESE NEW POSITIONS AND VELOCITIES
	ComputeForces(m_TempSys[0]);

	// TAKE THE FULL STEP WITH THIS NEW INFORMATION
	IntegrateSysOverTime(m_CurrentSys,m_TempSys[0],m_TargetSys,DeltaTime);
}

///////////////////////////////////////////////////////////////////////////////
// Function:	RK4Integrate 
// Purpose:		Calculate new Positions and Velocities given a deltatime
// Arguments:	DeltaTime that has passed since last iteration
// Notes:		This integrator uses the Runga-Kutta 4 method
//				This could use a generic function 4 times instead of unrolled
//				but it was easier for me to debug.  Fun for you to optimize.
///////////////////////////////////////////////////////////////////////////////
void CPhysEnv::RK4Integrate( float DeltaTime)
{
/// Local Variables ///////////////////////////////////////////////////////////
	int loop;
	float		halfDeltaT,sixthDeltaT;
	tParticle *source, *target, *accum1, *accum2, *accum3, *accum4;
///////////////////////////////////////////////////////////////////////////////
	halfDeltaT = DeltaTime / 2.0f;		// SOME TIME VALUES I WILL NEED
	sixthDeltaT = 1.0f / 6.0f;

	// FIRST STEP
	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TempSys[0];	// TEMP STORAGE FOR NEW POSITION
	accum1 = m_TempSys[1];	// ACCUMULATE THE INTEGRATED VALUES
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		accum1->f.x = halfDeltaT * source->f.x * source->oneOverM;
		accum1->f.y = halfDeltaT * source->f.y * source->oneOverM;
		accum1->f.z = halfDeltaT * source->f.z * source->oneOverM;

		accum1->v.x = halfDeltaT * source->v.x;
		accum1->v.y = halfDeltaT * source->v.y;
		accum1->v.z = halfDeltaT * source->v.z;
		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE OVER 1/2 TIME
		target->v.x = source->v.x + (accum1->f.x);
		target->v.y = source->v.y + (accum1->f.y);
		target->v.z = source->v.z + (accum1->f.z);

		target->oneOverM = source->oneOverM;

		// SET THE NEW POSITION
		target->pos.x = source->pos.x + (accum1->v.x);
		target->pos.y = source->pos.y + (accum1->v.y);
		target->pos.z = source->pos.z + (accum1->v.z);

		source++;
		target++;
		accum1++;
	}

	ComputeForces(m_TempSys[0]);  // COMPUTE THE NEW FORCES

	// SECOND STEP
	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TempSys[0];	// TEMP STORAGE FOR NEW POSITION
	accum1 = m_TempSys[2];	// ACCUMULATE THE INTEGRATED VALUES
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		accum1->f.x = halfDeltaT * target->f.x * source->oneOverM;
		accum1->f.y = halfDeltaT * target->f.y * source->oneOverM;
		accum1->f.z = halfDeltaT * target->f.z * source->oneOverM;
		accum1->v.x = halfDeltaT * target->v.x;
		accum1->v.y = halfDeltaT * target->v.y;
		accum1->v.z = halfDeltaT * target->v.z;

		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE
		target->v.x = source->v.x + (accum1->f.x);
		target->v.y = source->v.y + (accum1->f.y);
		target->v.z = source->v.z + (accum1->f.z);

		target->oneOverM = source->oneOverM;

		// SET THE NEW POSITION
		target->pos.x = source->pos.x + (accum1->v.x);
		target->pos.y = source->pos.y + (accum1->v.y);
		target->pos.z = source->pos.z + (accum1->v.z);

		source++;
		target++;
		accum1++;
	}

	ComputeForces(m_TempSys[0]);  // COMPUTE THE NEW FORCES

	// THIRD STEP
	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TempSys[0];	// TEMP STORAGE FOR NEW POSITION
	accum1 = m_TempSys[3];	// ACCUMULATE THE INTEGRATED VALUES
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		// NOTICE I USE THE FULL DELTATIME THIS STEP
		accum1->f.x = DeltaTime * target->f.x * source->oneOverM;
		accum1->f.y = DeltaTime * target->f.y * source->oneOverM;
		accum1->f.z = DeltaTime * target->f.z * source->oneOverM;
		accum1->v.x = DeltaTime * target->v.x;
		accum1->v.y = DeltaTime * target->v.y;
		accum1->v.z = DeltaTime * target->v.z;

		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE
		target->v.x = source->v.x + (accum1->f.x);
		target->v.y = source->v.y + (accum1->f.y);
		target->v.z = source->v.z + (accum1->f.z);

		target->oneOverM = source->oneOverM;

		// SET THE NEW POSITION
		target->pos.x = source->pos.x + (accum1->v.x);
		target->pos.y = source->pos.y + (accum1->v.y);
		target->pos.z = source->pos.z + (accum1->v.z);

		source++;
		target++;
		accum1++;
	}

	ComputeForces(m_TempSys[0]);  // COMPUTE THE NEW FORCES

	// FOURTH STEP
	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TempSys[0];	// TEMP STORAGE FOR NEW POSITION
	accum1 = m_TempSys[4];	// ACCUMULATE THE INTEGRATED VALUES
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		// NOTICE I USE THE FULL DELTATIME THIS STEP
		accum1->f.x = DeltaTime * target->f.x * source->oneOverM;
		accum1->f.y = DeltaTime * target->f.y * source->oneOverM;
		accum1->f.z = DeltaTime * target->f.z * source->oneOverM;

		accum1->v.x = DeltaTime * target->v.x;
		accum1->v.y = DeltaTime * target->v.y;
		accum1->v.z = DeltaTime * target->v.z;

		// THIS TIME I DON'T NEED TO COMPUTE THE TEMPORARY POSITIONS
		source++;
		target++;
		accum1++;
	}

	source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
	target = m_TargetSys;
	accum1 = m_TempSys[1];
	accum2 = m_TempSys[2];
	accum3 = m_TempSys[3];
	accum4 = m_TempSys[4];
	for (loop = 0; loop < m_ParticleCnt; loop++)
	{
		// DETERMINE THE NEW VELOCITY FOR THE PARTICLE USING RK4 FORMULA
		if( m_UseXAxis ) target->v.x = source->v.x + ((accum1->f.x + ((accum2->f.x + accum3->f.x) * 2.0f) + accum4->f.x) * sixthDeltaT);
		if( m_UseYAxis ) target->v.y = source->v.y + ((accum1->f.y + ((accum2->f.y + accum3->f.y) * 2.0f) + accum4->f.y) * sixthDeltaT);
		if( m_UseZAxis ) target->v.z = source->v.z + ((accum1->f.z + ((accum2->f.z + accum3->f.z) * 2.0f) + accum4->f.z) * sixthDeltaT);
		// DETERMINE THE NEW POSITION FOR THE PARTICLE USING RK4 FORMULA
		if( m_UseXAxis ) target->pos.x = source->pos.x + ((accum1->v.x + ((accum2->v.x + accum3->v.x) * 2.0f) + accum4->v.x) * sixthDeltaT);
		if( m_UseYAxis ) target->pos.y = source->pos.y + ((accum1->v.y + ((accum2->v.y + accum3->v.y) * 2.0f) + accum4->v.y) * sixthDeltaT);
		if( m_UseZAxis ) target->pos.z = source->pos.z + ((accum1->v.z + ((accum2->v.z + accum3->v.z) * 2.0f) + accum4->v.z) * sixthDeltaT);

		source++;
		target++;
		accum1++;
		accum2++;
		accum3++;
		accum4++;
	}

}

int CPhysEnv::CheckForCollisions( tParticle *system )
{
	// be optimistic!
	int collisionState = NOT_COLLIDING;
	float const depthEpsilon = 0.001f;
	
	tParticle *curParticle;
	
	m_ContactCnt = 0;		// THERE ARE CURRENTLY NO CONTACTS
	curParticle = system;
	
	for( int loop = 0; (loop < m_ParticleCnt) && (collisionState != PENETRATING); loop++, curParticle++ )
	{
		// CHECK THE MAIN BOUNDARY PLANES FIRST
		for(int planeIndex = 0;(planeIndex < m_CollisionPlaneCnt) && (collisionState != PENETRATING); planeIndex++)
		{
			tCollisionPlane *plane = &m_CollisionPlane[planeIndex];
			
			float axbyczd = DotProduct(&curParticle->pos, &plane->normal) + plane->d;
			
			if(axbyczd < -depthEpsilon)
			{
				// ONCE ANY PARTICLE PENETRATES, QUIT THE LOOP
				collisionState = PENETRATING;
			}
			else if(axbyczd < depthEpsilon)
			{
				float relativeVelocity = DotProduct(&plane->normal,&curParticle->v);
				
				if(relativeVelocity < 0.0f)
				{
					collisionState = COLLIDING;
					m_Contact[m_ContactCnt].particle = loop;
					memcpy(&m_Contact[m_ContactCnt].normal,&plane->normal,sizeof(tVector));
					m_ContactCnt++;
				}
			}
		}
	}
	
	if( !m_CollisionActive || collisionState == PENETRATING ) return collisionState;
	
	// Self-collision
	if( manu->YL_UseQuad )
	{
		Quad *quad = manu->quadList;
		int quads = manu->nQuad;
		int quadPoint1, quadPoint2, quadPoint3, quadPoint4;
		tParticle *p1, *p2, *p3, *p4;
		tVector quadPlane, quadPlane2, quadSide1, quadSide2, quadSide3, quadSide4, vector1, vector2;
		float axbycz, relativeVelocity;
		
		if( quad == NULL || quads <= 0 ) return collisionState;
		
		for( int j = 0; j < quads; j++, quad++ )
		{
			quadPoint1 = quad->v1;
			quadPoint2 = quad->v2;
			quadPoint3 = quad->v3;
			quadPoint4 = quad->v4;
			
			p1 = &system[quadPoint1];
			p2 = &system[quadPoint2];
			p3 = &system[quadPoint3];
			p4 = &system[quadPoint4];
			
			VectorDifference( &p2->pos, &p1->pos, &vector1 );
			VectorDifference( &p4->pos, &p1->pos, &vector2 );
			
			CrossProduct( &vector1, &vector2, &quadPlane );
			
			//VectorDifference( &p2->pos, &p1->pos, &vector1 );
			CrossProduct( &quadPlane, &vector1, &quadSide1 ); // quadPlane (x) ( P2 - P1 )
			VectorDifference( &p3->pos, &p2->pos, &vector2 );
			CrossProduct( &quadPlane, &vector2, &quadSide2 ); // quadPlane (x) ( P3 - P2 )
			VectorDifference( &p4->pos, &p3->pos, &vector2 );
			CrossProduct( &quadPlane, &vector2, &quadSide3 ); // quadPlane (x) ( P4 - P3 )
			VectorDifference( &p1->pos, &p4->pos, &vector2 );
			CrossProduct( &quadPlane, &vector2, &quadSide4 ); // quadPlane (x) ( P1 - P4 )
			
			NormalizeVector( &quadPlane );
			
			curParticle = system;
			for( int loop = 0; loop < m_ParticleCnt; loop++, curParticle++ )
			{
				if( loop != quadPoint1 && loop != quadPoint2 && loop != quadPoint3 && loop != quadPoint4 )
				{
					ScaleVector( &quadPlane, 1.0f, &quadPlane2 );
					VectorDifference( &curParticle->pos, &p1->pos, &vector2 );
					axbycz = DotProduct( &vector2, &quadPlane2 );
					
					if( axbycz < 0.0f )
					{
						ScaleVector( &quadPlane2, -1.0f, &quadPlane2 );
						axbycz = DotProduct( &vector2, &quadPlane2 );
					}
					
					if( axbycz < 0.11f )
					{
						//VectorDifference( &curParticle->pos, &p1->pos, &vector2 );
						float s1 = DotProduct(&vector2, &quadSide1);
						VectorDifference( &curParticle->pos, &p2->pos, &vector2 );
						float s2 = DotProduct(&vector2, &quadSide2);
						VectorDifference( &curParticle->pos, &p3->pos, &vector2 );
						float s3 = DotProduct(&vector2, &quadSide3);
						VectorDifference( &curParticle->pos, &p4->pos, &vector2 );
						float s4 = DotProduct(&vector2, &quadSide4);
						
						if( (s1 >= 0 && s2 >= 0 && s3 >= 0 && s4 >= 0) || (s1 <= 0 && s2 <= 0 && s3 <= 0 && s4 <= 0) )
						{
							relativeVelocity = DotProduct( &quadPlane2, &curParticle->v );
							
							if( relativeVelocity < 0.0f )
							{
								ScaleVector( &quadPlane2, relativeVelocity, &vector1 );
								VectorDifference( &curParticle->v, &vector1, &vector2 );
								ScaleVector( &vector1, 0.5f, &vector1 );
								VectorDifference( &vector2, &vector1, &curParticle->v );
							}
						}
					}				
				}
			}
		}
	}
	else
	{
		Triangle *triangle = manu->trigList;
		int triangles = manu->nTrig;
		int triPoint1, triPoint2, triPoint3;
		tParticle *p1, *p2, *p3;
		tVector trianglePlane, trianglePlane2, triSide1, triSide2, triSide3, vector1, vector2;
		float axbycz, relativeVelocity;
		
		if( triangle == NULL || triangles <= 0 ) return collisionState;
		
		for( int j = 0; j < triangles; j++, triangle++ )
		{
			triPoint1 = triangle->idx1;
			triPoint2 = triangle->idx2;
			triPoint3 = triangle->idx3;
			
			p1 = &system[triPoint1];
			p2 = &system[triPoint2];
			p3 = &system[triPoint3];
			
			VectorDifference( &p2->pos, &p1->pos, &vector1 );
			VectorDifference( &p3->pos, &p1->pos, &vector2 );
			
			CrossProduct( &vector1, &vector2, &trianglePlane );
			//VectorDifference( &p2->pos, &p1->pos, &vector1 );
			CrossProduct( &trianglePlane, &vector1, &triSide1 ); // trianglePlane (x) ( P2 - P1 )
			VectorDifference( &p3->pos, &p2->pos, &vector2 );
			CrossProduct( &trianglePlane, &vector2, &triSide2 ); // trianglePlane (x) ( P3 - P2 )
			VectorDifference( &p1->pos, &p3->pos, &vector2 );
			CrossProduct( &trianglePlane, &vector2, &triSide3 ); // trianglePlane (x) ( P1 - P3 )
			
			NormalizeVector( &trianglePlane );
			
			curParticle = system;
			for( int loop = 0; loop < m_ParticleCnt; loop++, curParticle++ )
			{
				if( loop != triPoint1 && loop != triPoint2 && loop != triPoint3 )
				{
					ScaleVector( &trianglePlane, 1.0f, &trianglePlane2 );
					VectorDifference( &curParticle->pos, &p1->pos, &vector2 );
					axbycz = DotProduct( &vector2, &trianglePlane2 );
					
					if( axbycz < 0.0f )
					{
						ScaleVector( &trianglePlane2, -1.0f, &trianglePlane2 );
						axbycz = DotProduct( &vector2, &trianglePlane2 );
					}
					
					if( axbycz < 0.11f )
					{
						//VectorDifference( &curParticle->pos, &p1->pos, &vector2 );
						float s1 = DotProduct(&vector2, &triSide1);
						VectorDifference( &curParticle->pos, &p2->pos, &vector2 );
						float s2 = DotProduct(&vector2, &triSide2);
						VectorDifference( &curParticle->pos, &p3->pos, &vector2 );
						float s3 = DotProduct(&vector2, &triSide3);
						
						if( (s1 >= 0 && s2 >= 0 && s3 >= 0) || (s1 <= 0 && s2 <= 0 && s3 <= 0) )
						{
							relativeVelocity = DotProduct( &trianglePlane2, &curParticle->v );
							
							if( relativeVelocity < 0.0f )
							{
								ScaleVector( &trianglePlane2, relativeVelocity, &vector1 );
								VectorDifference( &curParticle->v, &vector1, &vector2 );
								ScaleVector( &vector1, 0.5f, &vector1 );
								VectorDifference( &vector2, &vector1, &curParticle->v );
							}
						}
					}				
				}
			}
		}
	}
	return collisionState;
}

void CPhysEnv::ResolveCollisions( tParticle *system )
{
	tContact *contact;
	tParticle *particle; // THE PARTICLE COLLIDING
	float VdotN;
	tVector Vn, Vt; // CONTACT RESOLUTION IMPULSE
	contact = m_Contact;
	
	for( int loop = 0; loop < m_ContactCnt; loop++, contact++ )
	{
		particle = &system[contact->particle];
		// CALCULATE Vn
		VdotN = DotProduct( &contact->normal, &particle->v );
		ScaleVector( &contact->normal, VdotN, &Vn );
		// CALCULATE Vt
		VectorDifference( &particle->v, &Vn, &Vt );
		// SCALE Vn BY COEFFICIENT OF RESTITUTION
		ScaleVector( &Vn, m_Kr, &Vn );
		// SET THE VELOCITY TO BE THE NEW IMPULSE
		VectorDifference( &Vt, &Vn, &particle->v );
	}
}


void CPhysEnv::CheckDistance( float DeltaTime )
{
	float magx = 0.0f, magy = 0.0f, magz = 0.0f;

	extern bool screenshot;
	extern float TotalTime;
	extern float time_limit;
	
	TotalTime += DeltaTime;
	if( time_limit == 0.0f ) {
		tParticle *source = m_CurrentSys;	// CURRENT STATE OF PARTICLE
		tParticle *target = m_TargetSys;
		for (int loop = 0; loop < m_ParticleCnt; loop++)
		{
			magx += fabsf(target->pos.x - source->pos.x);
			magy += fabsf(target->pos.y - source->pos.y);
			magz += fabsf(target->pos.z - source->pos.z);
			
			source++;
			target++;
		}

		magx = (magx/(float) m_ParticleCnt)/DeltaTime;
		magy = (magy/(float) m_ParticleCnt)/DeltaTime;
		magz = (magz/(float) m_ParticleCnt)/DeltaTime;
		float dist = sqrtf(powf(magx,2.0)+powf(magy,4.0)+powf(magz,2.0))/DeltaTime;

		/*	
		printf("time:\t%1.20f\n",TotalTime);
		printf("magx:\t%1.20f\n",magx);
		printf("magy:\t%1.20f\n",magy);
		printf("magz:\t%1.20f\n",magz);
		printf("dist:\t%1.20f\n",dist);
		*/
		
		float comp = 0.001f;
		
		if( (magy < comp) && (magx < comp) && (magz < comp) ) {
			if( (dist < 0.1f) && (dist > 0.0f) ) {
				performAction( PERFORM_ACTION_SET_RUNNING, PERFORM_ACTION_FALSE );
				// printf("Done.\n\n");
				screenshot = true;
			}
		}
	}
}

void CPhysEnv::Simulate( float DeltaTime, bool running )
{
	float CurrentTime = 0.0f;
	float TargetTime = DeltaTime;
	tParticle *tempSys;
	int collisionState;
	m_Unscrolling = running;
	
	while( CurrentTime < DeltaTime )
	{
		if( running )
		{
			ComputeForces( m_CurrentSys );
				
			// IN ORDER TO MAKE THINGS RUN FASTER, I HAVE THIS LITTLE TRICK
			// IF THE SYSTEM IS DOING A BINARY SEARCH FOR THE COLLISION POINT,
			// I FORCE EULER'S METHOD ON IT. OTHERWISE, LET THE USER CHOOSE.
			// THIS DOESN'T SEEM TO EFFECT STABILITY EITHER WAY
			if( m_CollisionRootFinding )
			{
				EulerIntegrate( TargetTime - CurrentTime );
			}
			else
			{
				switch( m_IntegratorType )
				{
					case EULER_INTEGRATOR:
						EulerIntegrate( TargetTime - CurrentTime );
						break;
					case MIDPOINT_INTEGRATOR:
						MidPointIntegrate( TargetTime - CurrentTime );
						break;
					case RK4_INTEGRATOR:
						RK4Integrate( TargetTime - CurrentTime );
						break;
				}
			}	
		}
		
		collisionState = CheckForCollisions( m_TargetSys );
		
		if(running) {
			if( collisionState != PENETRATING ) {
				CheckDistance(TargetTime - CurrentTime);
			}
			else if( fabs( ( ( CurrentTime + TargetTime ) / 2.0f ) 
						- CurrentTime ) <= EPSILON ) {
				printf("Expanding world size...\n");
			
				m_WorldSizeY += 0.25f;

				// MAKE THE TOP PLANE (CEILING)
				m_CollisionPlane[0].d = m_WorldSizeY / 2.0f;

				// MAKE THE BOTTOM PLANE (FLOOR)
				m_CollisionPlane[1].d = m_WorldSizeY / 2.0f;
				break;
			}
		}
		
		if( collisionState == PENETRATING )
		{
			// TELL THE SYSTEM I AM LOOKING FOR A COLLISION SO IT WILL USE EULER
			m_CollisionRootFinding = true;
			// we simulated too far, so subdivide time and try again
			TargetTime = ( CurrentTime + TargetTime ) / 2.0f;
			
			// blow up if we aren't moving forward each step, which is
			// probably caused by interpenetration at the frame start
			assert( fabs( TargetTime - CurrentTime ) > EPSILON );
		}
		else
		{
			// either colliding or clear
			if( collisionState == COLLIDING )
			{
				int Counter = 0;
				do
				{
					ResolveCollisions( m_TargetSys );
					Counter++;
				} while( ( CheckForCollisions( m_TargetSys ) == COLLIDING ) && ( Counter < 100 ) );
				
				assert( Counter < 100 );
				m_CollisionRootFinding = false;  // FOUND THE COLLISION POINT
			}
			
			
			// we made a successful step, so swap configurations
			// to "save" the data for the next step
			
			CurrentTime = TargetTime;
			TargetTime = DeltaTime;
			
			// SWAP MY TWO PARTICLE SYSTEM BUFFERS SO I CAN DO IT AGAIN
			tempSys = m_CurrentSys;
			m_CurrentSys = m_TargetSys;
			m_TargetSys = tempSys;
		}
	}
}
