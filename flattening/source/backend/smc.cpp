#include "smc.h"

GLdouble modelMatrix[16], projMatrix[16];
GLint width, height, viewport[4];

// Camera Information
GLfloat lookAtX = 0.0, lookAtY = 0.0, lookAtZ = 0.0;
GLfloat axisAngleX = 0.0, axisAngleY = 0.0, distance = 25.0, fov = 60.0;

// Selction information
GLint selectedPoint = -1, pixelRange = 5;
int drawSelectionBox = 0;
GLfloat selectionBox[2][2] = { { 0,  0 }, { 0, 0 } };

bool SMT_DEBUG = false, m_SimRunning = false, m_LockParticle, YL_UseQuad, lockTopRow = false;
int m_PickX, m_PickY, m_TimeIterations;
float m_MaxTimeStep, m_LastTime;

char meshFilename[64] = "", textureFilename[64] = "", lockedFilename[64] = "", saveMeshFilename[64] = "";

t_Bone *m_Skeleton, *m_CurBone;
CPhysEnv *m_PhysEnv;

manuModel *manu = NULL;
int manuSizeX = -1, manuSizeY = -1;

float simProperties[8], m_VertexMass;


// For "movie" mode
bool scriptRecord = false, scriptPlay = false, catchUp = false;
char scriptFilename[64] = "";
FILE *scriptFile = NULL;
int currentFrameCount, currentScriptCommand, currentScriptValue, currentScriptFrame;

int GetCurrentFrameCount()
{
	return currentFrameCount;
}

int Init()
{
	//lookAtX = 0.0;
	//lookAtY = 0.0;
	//lookAtZ = 0.0;
	//axisAngleX = 0.0;
	//axisAngleY = 0.0;
	//distance = 25.0;

	m_PhysEnv = new CPhysEnv();
	m_PhysEnv->lockedArray = NULL;
	m_Skeleton = new t_Bone();
	
	// INITIALIZE THE MODE KEYS
	m_SimRunning = false;
	m_CurBone = NULL;
	ResetBone( m_Skeleton, NULL );
	m_Skeleton->id = -1;
	strcpy( m_Skeleton->name, "Skeleton" );
	
	m_TimeIterations = 10;
	m_MaxTimeStep = 0.01f;
	m_LastTime = 0.0f;
	
	m_PickX = -1;
	m_PickY = -1;
	
	m_LockParticle = true;
	m_PhysEnv->m_LockParticles = true;
	
/*	simProperties[0] = 0.1f; // float m_CoefRest;
	simProperties[1] = 0.04f; // float m_Damping;
	simProperties[2] = 0.0f; // float m_GravX;
	simProperties[3] = -0.2f; // float m_GravY;
	simProperties[4] = 0.0f; // float m_GravZ;
	simProperties[5] = 2.5f; // float m_SpringConst;
	simProperties[6] = 1.0f; // float m_SpringDamp;
	simProperties[7] = 100.0f; // float m_UserForceMag;
	*/
	
	m_VertexMass = -4.22017e+37; // Vertex mass option for selected point -- needed?
	
	int loadedSuccessfully = 0;
	
	if( strlen( meshFilename ) > 0 && strlen( textureFilename ) > 0 )
		loadedSuccessfully = CreateClothPatch( meshFilename, textureFilename );

	//m_PhysEnv->lockedArray = NULL;
	CreateSelectionArray();
	
	performAction( PERFORM_ACTION_COMMIT_SIM_PROPERTIES, 0 );
	return loadedSuccessfully;
}

void CreateSelectionArray()
{
	if( m_PhysEnv->lockedArray == NULL )
	{
		int particles = m_PhysEnv->GetNumberOfParticles();
		m_PhysEnv->lockedArray = (bool *)malloc(sizeof(bool) * particles);
		for( int i = 0; i < m_PhysEnv->GetNumberOfParticles(); i++ )
		{
			m_PhysEnv->lockedArray[i] = false;
		}
		
		LoadLockPoints();
	}
}

void LoadLockPoints()
{
	if( strlen( lockedFilename ) > 0 )
	{
		if( SMT_DEBUG ) printf( "Loading Locked File\n" );

		FILE *lockedFile;
		
		lockedFile = fopen( lockedFilename, "r" );
		
		if( lockedFile != NULL && manu != NULL && m_PhysEnv->lockedArray != NULL )
		{
			char *string = (char *)malloc(sizeof(char) * 50);
			int max, lock;
			
			do
			{
				string = fgets( string, 50, lockedFile );
			} while( string != NULL && strncmp( string, "**Locked Points", 15 ) != 0 );
			
			if( string != NULL && strncmp( string, "***", 3 ) != 0 )
			{
				string = fgets( string, 50, lockedFile );
				max = atoi( string );
				
				if( max != m_PhysEnv->GetNumberOfParticles() )
				{
					if( SMT_DEBUG ) printf( "Wrong lock file for this mesh\n" );
					fclose( lockedFile );
					free( string );
					return;
				}
			}
			
			while( string != NULL && strncmp( string, "***", 3 ) != 0 )
			{
				string = fgets( string, 50, lockedFile );
				
				lock = atoi( string );
				
				if( lock >= 0 && lock < max )
					m_PhysEnv->lockedArray[lock] = true;
			}
			
			fclose( lockedFile );
			free( string );
		}
		else if( SMT_DEBUG ) printf( "Unable to read: \"%s\"", lockedFilename );
	}
}

int GetLockedArray( int *intLockedArray )
{
	int index = 0, elements = m_PhysEnv->GetNumberOfParticles();
	intLockedArray = (int*)malloc(sizeof(int)*elements+1);
	
	// Need to include the total number of elements at the beginning of the list
	intLockedArray[index] = elements; index++;
	
	for( int i = 0; i < elements; i++ )
	{
		if( m_PhysEnv->lockedArray[i] )
		{
			intLockedArray[index] = i;
			index++;
		}
	}
	return index;
} // Need to free intLockedArray if called

void ClearSelectionArray()
{
	for( int i = 0; i < m_PhysEnv->GetNumberOfParticles(); i++ )
	{
		m_PhysEnv->lockedArray[i] = false;
	}
}

void AddToSelectionArray( int number )
{
	if( m_LockParticle && number >= 0 && number < m_PhysEnv->GetNumberOfParticles() )
		m_PhysEnv->lockedArray[number] = !m_PhysEnv->lockedArray[number];
}

void changeAngle()
{
   glViewport(0, 0, width, height); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(fov, (GLfloat) width/(GLfloat) height, 1.0, 2000.0);
	glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   GLfloat cameraX, cameraY, cameraZ;
   cameraX = ( sinf(axisAngleX) * ( cosf(axisAngleY) * distance ) ) + lookAtX;
   cameraY = ( sinf(axisAngleY) * distance ) + lookAtY;
   cameraZ = ( cosf(axisAngleX) * ( cosf(axisAngleY) * distance ) ) + lookAtZ;
   
   gluLookAt ( cameraX, cameraY, cameraZ, lookAtX, lookAtY, lookAtZ, 0.0, 1.0, 0.0);
   
	glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
	viewport[0] = 0; viewport[1] = 0; viewport[2] = width; viewport[3] = height;
}

void ReshapeCanvas( int inWidth, int inHeight )
{
	width = inWidth;
	height = inHeight;
	
	changeAngle();
}

void selectionBoxWindow()
{
   glViewport(0, 0, width, height); 
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height );
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	
	glLineStipple(1, 0x80FF);
	glEnable(GL_LINE_STIPPLE);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_LINE_LOOP);
		glVertex2f( selectionBox[0][0], height - selectionBox[0][1] );
		glVertex2f( selectionBox[0][0], height - selectionBox[1][1] );
		glVertex2f( selectionBox[1][0], height - selectionBox[1][1] );
		glVertex2f( selectionBox[1][0], height - selectionBox[0][1] );
	glEnd();
	glDisable(GL_LINE_STIPPLE);
	
	// Pop the matrices back on
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

// Formerly GLvoid COGLView::drawScene(GLvoid)
void RenderScene()
{
	glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT );
	
	glPushMatrix();
	
	glLoadIdentity();
	changeAngle();
	
	if( m_PhysEnv != NULL && m_Skeleton != NULL )
	{
		//if( m_Skeleton->rot.y > 360.0f ) m_Skeleton->rot.y -= 360.0f;
		//if( m_Skeleton->rot.x > 360.0f ) m_Skeleton->rot.x -= 360.0f;
		//if( m_Skeleton->rot.z > 360.0f ) m_Skeleton->rot.z -= 360.0f;
		
		// Set root skeleton's orientation and position
		//glTranslatef(m_Skeleton->trans.x, m_Skeleton->trans.y, m_Skeleton->trans.z);
		
		// ROTATE THE ROOT
		//glRotatef( m_Skeleton->rot.z, 1.0f, 0.0f, 0.0f );
		//glRotatef( m_Skeleton->rot.y, 0.0f, 1.0f, 0.0f );
		//glRotatef( m_Skeleton->rot.x, 0.0f, 0.0f, 1.0f );
		
		// GRAB THE MATRIX AT THIS POINT SO I CAN USE IT FOR THE DEFORMATION
		glGetFloatv( GL_MODELVIEW_MATRIX, m_Skeleton->matrix.m );
		
		RunSim();
		
		m_PhysEnv->RenderWorld();
	}
	glPopMatrix();
		
	if( drawSelectionBox == 1 )
	{
		selectionBoxWindow();
	}
}

void PickPoint( int x, int y )
{
	GLdouble windowX, windowY, windowZ;
	GLfloat position[3];
	float deltaX, deltaY, oldDistance;
	selectedPoint = -1;
	
	int i = 0;
	while( m_PhysEnv->GetParticlePosition( i, position ) )
	{
		gluProject( position[0], position[1], position[2], modelMatrix, projMatrix, viewport, &windowX, &windowY, &windowZ );
		
		deltaX = windowX - (GLfloat)x;
		deltaY = height - windowY - (GLfloat)y;
		
		if( deltaX > -pixelRange && deltaX < pixelRange && deltaY > -pixelRange && deltaY < pixelRange )
		{
			if( selectedPoint == -1 || windowZ < oldDistance )
			{
				selectedPoint = i;
				m_PickX = (int)windowX;
				m_PickY = height - (int)windowY;
				oldDistance = windowZ;
			}
		}
		i++;
	}
	
	m_PhysEnv->SetSelectedParticle( selectedPoint );
}

void LockPoint( int x, int y )
{
	GLdouble windowX, windowY, windowZ;
	GLfloat position[3];
	float deltaX, deltaY, oldDistance;
	selectedPoint = -1;
	
	int i = 0;
	while( m_PhysEnv->GetParticlePosition( i, position ) )
	{
		gluProject( position[0], position[1], position[2], modelMatrix, projMatrix, viewport, &windowX, &windowY, &windowZ );
		
		deltaX = windowX - (GLfloat)x;
		deltaY = height - windowY - (GLfloat)y;
		
		if( deltaX > -pixelRange && deltaX < pixelRange && deltaY > -pixelRange && deltaY < pixelRange )
		{
			if( selectedPoint == -1 || windowZ < oldDistance )
			{
				selectedPoint = i;
				m_PickX = (int)windowX;
				m_PickY = height - (int)windowY;
				
				oldDistance = windowZ;
			}
		}
		i++;
	}
	
	performAction( PERFORM_ACTION_LOCK_POINT, selectedPoint );
	
	if( selectedPoint == -1 )
	{
		selectionBox[0][0] = x;
		selectionBox[0][1] = y;
		selectionBox[1][0] = x;
		selectionBox[1][1] = y;
		
		drawSelectionBox = 1;
	}
}

void boxPick()
{
	GLdouble windowX, windowY, windowZ;
	GLfloat position[3];
	float deltaX, deltaY, oldDeltas;
	GLfloat swap;
	
	if( selectionBox[0][0] > selectionBox[1][0] )
	{
		swap = selectionBox[0][0];
		selectionBox[0][0] = selectionBox[1][0];
		selectionBox[1][0] = swap;
	}
	
	if( selectionBox[0][1] > selectionBox[1][1] )
	{
		swap = selectionBox[0][1];
		selectionBox[0][1] = selectionBox[1][1];
		selectionBox[1][1] = swap;
	}
	
	int i = 0;
	while( m_PhysEnv->GetParticlePosition( i, position ) )
	{
		gluProject( position[0], position[1], position[2], modelMatrix, projMatrix, viewport, &windowX, &windowY, &windowZ );
		windowY = height - windowY;
		
		if( windowX > selectionBox[0][0] && windowX < selectionBox[1][0] && windowY > selectionBox[0][1] && windowY < selectionBox[1][1] )
		{
			performAction( PERFORM_ACTION_LOCK_POINT, i );
		}
		i++;
	}
	
	selectionBox[0][0] = 0;
	selectionBox[0][1] = 0;
	selectionBox[1][0] = 0;
	selectionBox[1][1] = 0;
	drawSelectionBox = 0;
}

void DragPoint( int x, int y )
{
	tVector	localX,localY;
	
	if( selectedPoint >= 0 )
	{
		// NEED TO GET THE VECTORS FOR THE LOCAL X AND Y AXES
		localY.x = m_Skeleton->matrix.m[1];
		localY.y = m_Skeleton->matrix.m[5];
		localY.z = m_Skeleton->matrix.m[9];
		localX.x = m_Skeleton->matrix.m[0];
		localX.y = m_Skeleton->matrix.m[4];
		localX.z = m_Skeleton->matrix.m[8];
		
		m_PhysEnv->SetMouseForce( x - m_PickX, y - m_PickY, &localX, &localY );
		m_PhysEnv->m_MouseForceActive = true;
	}
	else
	{
		selectionBox[1][0] = x;
		selectionBox[1][1] = y;
	}
}

void MouseRelease()
{
	selectedPoint = -1;
	
	m_PhysEnv->m_MouseForceActive = false;
	
	if( drawSelectionBox == 1 )
	{
		boxPick();
	}
}

/*
GLvoid drawModel(t_Bone *curBone)
{
	if (curBone->visualCnt > 0 && curBone->visuals[0].vertexData != NULL)
	{
		glColor3f(1.0f,1.0f,1.0f);	
		// Declare the Array of Data
		glInterleavedArrays(curBone->visuals[0].dataFormat,0,(GLvoid *)curBone->visuals[0].vertexData);
		if (curBone->visuals[0].reuseVertices)
		{
			// HANDLE EITHER QUADS OR TRIS
			if (curBone->visuals[0].vPerFace == 3)
				glDrawElements(GL_TRIANGLES,curBone->visuals[0].faceCnt * 3,GL_UNSIGNED_SHORT,curBone->visuals[0].faceIndex);
			else
				glDrawElements(GL_QUADS,curBone->visuals[0].faceCnt * 4,GL_UNSIGNED_SHORT,curBone->visuals[0].faceIndex);
		}
		else
		{
			// HANDLE EITHER QUADS OR TRIS
			if (curBone->visuals[0].vPerFace == 3)
				glDrawArrays(GL_TRIANGLES,0,curBone->visuals[0].faceCnt * 3);
			else
				glDrawArrays(GL_QUADS,0,curBone->visuals[0].faceCnt * 4);
		}
	}
}
*/

void NewSystem()
{
	manu = NULL;
	m_PhysEnv->FreeSystem();
	m_SimRunning = false;
	if (m_Skeleton->childCnt > 0)
	{
		if (m_Skeleton->children->visuals->vertexData)
			free(m_Skeleton->children->visuals->vertexData);
		if (m_Skeleton->children->visuals->faceIndex)
			free(m_Skeleton->children->visuals->faceIndex);
		free(m_Skeleton->children->visuals);
		free(m_Skeleton->children);
		m_Skeleton->childCnt = 0;
	}
}

void DeleteSystem()
{
	delete manu;
	manu = NULL;
	m_PhysEnv->FreeSystem();
	m_SimRunning = false;
	if (m_Skeleton->childCnt > 0)
	{
		if (m_Skeleton->children->visuals->vertexData)
			free(m_Skeleton->children->visuals->vertexData);
		if (m_Skeleton->children->visuals->faceIndex)
			free(m_Skeleton->children->visuals->faceIndex);
		free(m_Skeleton->children->visuals);
		free(m_Skeleton->children);
		m_Skeleton->childCnt = 0;
	}
	delete m_PhysEnv;
	delete m_Skeleton;
	m_PhysEnv = NULL;
	m_Skeleton = NULL;

	if( scriptFile != NULL ) fclose( scriptFile );
}

void RunSim()
{
	float Time = 0.0f;
	float DeltaTime = 0.0f;
	
	Time = m_LastTime + ( m_MaxTimeStep * (float)m_TimeIterations );
	
	bool simulatedFrame = true;
	if( scriptPlay && currentFrameCount >= currentScriptFrame )
	{
		if( catchUp )
		{
			performAction( currentScriptCommand, currentScriptValue );
			catchUp = false;
		}
		else
		{
			char scriptCommand[25];
			fgets( scriptCommand, 25, scriptFile );
			sscanf( scriptCommand, "%d %d %d\n", &currentScriptFrame, &currentScriptCommand, &currentScriptValue );
			
			if( currentFrameCount > currentScriptFrame )
			{
				currentFrameCount = currentScriptFrame;
				simulatedFrame = false;
			}
			
			if( currentFrameCount < currentScriptFrame )
			{
				catchUp = true;
			}
			else performAction( currentScriptCommand, currentScriptValue );
			
			if( feof( scriptFile ) )
			{
				fclose( scriptFile );
				scriptFile = NULL;
				scriptPlay = false;
			}
		}
	}
	
	if( m_SimRunning && simulatedFrame )
	{
		while(m_LastTime < Time)
		{
			DeltaTime = Time - m_LastTime;
			if(DeltaTime > m_MaxTimeStep)
			{
				DeltaTime = m_MaxTimeStep;
			}
	 		m_PhysEnv->Simulate(DeltaTime,m_SimRunning);
			m_LastTime += DeltaTime;
			currentFrameCount++;
		}
		m_LastTime = Time;
	}
	else
	{
		m_PhysEnv->Simulate(DeltaTime,m_SimRunning);
		currentFrameCount++;
	}
}

int ScriptIsDone()
{
	if( scriptPlay ) return 0;
	return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	LoadFiles
// Purpose:		Loads the OBJ files into memory
///////////////////////////////////////////////////////////////////////////////		
void LoadFile( char file1[] )//,CString baseName,CString ext) 
{
	t_Bone	*children;
	t_Visual *visual;
	FILE	*fp;
	
/*
	ext.MakeUpper();
	if (ext == "OBJ")
	{
		visual = (t_Visual *)malloc(sizeof(t_Visual));
		NewSystem();	// CLEAR WHAT DATA IS THERE
		// I WANT TO LOAD JUST THE VERTICES AND PUT THEM IN A INDEXED FORMAT
		if (file1.GetLength() > 0 && LoadOBJ((char *)(LPCTSTR)file1 ,visual,
			LOADOBJ_VERTEXONLY | LOADOBJ_REUSEVERTICES))
		{
			// INFORM THE PHYSICAL SIMULATION OF THE PARTICLES
			m_PhysEnv->SetWorldParticles((tTexturedVertex *)visual->vertexData,visual->vertexCnt);
			if (m_Skeleton->childCnt > 0)
			{
				if (m_Skeleton->children->visuals->faceIndex != NULL)
					free(m_Skeleton->children->visuals->faceIndex);
				free(m_Skeleton->children->visuals);
				free(m_Skeleton->children->visuals->vertexData);
				free(m_Skeleton->children);
				m_Skeleton->childCnt = 0;
			}
			children = (t_Bone *)malloc(sizeof(t_Bone));
			m_CurBone = &children[m_Skeleton->childCnt];
			ResetBone(m_CurBone,m_Skeleton);
			strcpy(m_CurBone->name,(LPCTSTR)baseName);
			m_CurBone->visuals = visual;
			m_CurBone->visualCnt = 1;
			m_Skeleton->childCnt = 1;
			m_Skeleton->children = children;
		}
		else
		{
			MessageBox("Must Be A Valid OBJ File","Error",MB_OK);
			free(visual);
		}
	}
	else	// LOAD SIM SYSTEM
	{
	*/
		if( strlen(file1) ) //file1.GetLength() )
		{
			fp = fopen(file1,"rb");
			if (fp != NULL)
			{
				NewSystem();	// CLEAR WHAT DATA IS THERE
				fread(m_Skeleton,sizeof(t_Bone),1,fp);
				if (m_Skeleton->childCnt > 0)
				{
					m_Skeleton->children = (t_Bone *)malloc(sizeof(t_Bone));
					fread(m_Skeleton->children,sizeof(t_Bone),1,fp);
					if (m_Skeleton->children->visualCnt > 0)
					{
						m_Skeleton->children->visuals = (t_Visual *)malloc(sizeof(t_Visual));
						visual = m_Skeleton->children->visuals;
						fread(visual,sizeof(t_Visual),1,fp);
						if (visual->reuseVertices)
						{
							visual->vertexData = (float *)malloc(sizeof(float) * visual->vSize * visual->vertexCnt);
							visual->faceIndex = (unsigned short *)malloc(sizeof(unsigned short) * visual->faceCnt * visual->vPerFace);
							fread(visual->vertexData,sizeof(float),visual->vSize * visual->vertexCnt,fp);
							fread(visual->faceIndex,sizeof(unsigned short),visual->faceCnt * visual->vPerFace,fp);
						}
						// SAVE THE PHYSICAL SIMULATION OF THE PARTICLES
						m_PhysEnv->LoadData(fp);
					}
				}
				fclose(fp);
			}
		}
	//}
}

///////////////////////////////////////////////////////////////////////////////
// Procedure:	CreateClothPatch
// Purpose:		Creates a System to Represent a Cloth Patch
// Arguments:	Number of Segments in U and V to build
///////////////////////////////////////////////////////////////////////////////		
int CreateClothPatch( char *meshfile, char *texturefile )//, float parameters[] )
{
/// Local Variables ///////////////////////////////////////////////////////////
	t_Bone	*children;
	t_Visual *visual;
	int fPos,vPos, u, v, l1, l2;
	tTexturedVertex *vertex;
	float	w = 8.0f, h = 8.0;
	
	float	SstK = 2.5f, SstD = 1.2f;
	float	SshK = 4.0f, SshD = 0.6f;
	float	SflK = 2.4f, SflD = 0.8f;

manuSizeX = -1;
manuSizeY = -1;
	
	//NewSystem();	// CLEAR WHAT DATA IS THERE
	
	if (YL_UseQuad)
	{
		// Insert my HACK HERE
		// Read in my manuscript format.
		manu = new manuModel();
		manu->SMT_DEBUG = SMT_DEBUG;
		manu->YL_UseQuad = YL_UseQuad;
		manu->readMesh(meshfile);
		manu->readTexture(texturefile);
		
		m_PhysEnv->setWorldY( manu->maxz * 15 );
		m_PhysEnv->setWorldSize( manu->maxz * 15, manu->maxz * 15, manu->maxz * 15 );
		
		if (manu->nVer == 0) exit(0);
		
		u = manu->xSamples;
		v = manu->ySamples;
		
		visual = (t_Visual *)malloc(sizeof(t_Visual));
		
		visual->reuseVertices = true;
		visual->dataFormat = GL_T2F_V3F;
		visual->vPerFace = 4;
		visual->vSize = 6; // 3 floats for vertex
		visual->vertexData = (float *)malloc(sizeof(float) * visual->vSize * manu->nVer);
		visual->vertexCnt = manu->nVer;
		visual->faceIndex = (unsigned short *)malloc(sizeof(unsigned short) * manu->nQuad * visual->vPerFace);
		visual->faceCnt = manu->nQuad;
		
		// SET THE VERTICES
		vertex = (tTexturedVertex *)visual->vertexData;
		
		int idx = 0;
		for(int i=0; i < manu->nVer; i++)
		{
			// my hack
			vertex->u = manu->verList[idx].u1= manu->verList[idx].u1/TEXW;
			vertex->v = manu->verList[idx].v1= manu->verList[idx].v1/TEXH ;
			
			vertex->x = manu->verList[idx].x;
			vertex->z = manu->verList[idx].y;
			vertex->y = manu->verList[idx].z;
			//
			vertex++;
			idx++;
		}
		
		// INFORM THE PHYSICAL SIMULATION OF THE PARTICLES
		m_PhysEnv->SetWorldParticles((tTexturedVertex *)visual->vertexData,visual->vertexCnt);
		
		if (m_Skeleton->childCnt > 0)
		{
			if (m_Skeleton->children->visuals->faceIndex != NULL)
				free(m_Skeleton->children->visuals->faceIndex);
			free(m_Skeleton->children->visuals);
			free(m_Skeleton->children->visuals->vertexData);
			free(m_Skeleton->children);
			m_Skeleton->childCnt = 0;
		}
		children = (t_Bone *)malloc(sizeof(t_Bone));
		m_CurBone = &children[m_Skeleton->childCnt];
		ResetBone(m_CurBone,m_Skeleton);
		strcpy(m_CurBone->name,"Cloth");
		m_CurBone->visuals = visual;
		m_CurBone->visualCnt = 1;
		m_Skeleton->childCnt = 1;
		m_Skeleton->children = children;

		#define FACTOR 1       
		
		if (true)//dialog.m_UseStruct)
		{
			// Horizontal
			for (l1 = 0; l1 < v; l1++)	// v
				for (l2 = 0; l2 < (u-1); l2++)
				{
					m_PhysEnv->AddSpring((l1 * u) + l2,(l1 * u) + l2 + 1,SstK/FACTOR,SstD/FACTOR,STRUCTURAL_SPRING);
				}
			
			for (l1 = 0; l1 < (u); l1++)	
				for (l2 = 0; l2 < (v - 1); l2++)
				{
					m_PhysEnv->AddSpring((l2 * u) + l1,((l2 +1)* u) + l1, SstK/FACTOR,SstD/FACTOR,STRUCTURAL_SPRING);
				}
		}

		if (false)
		{ //shear
			for (l1 = 0; l1 < (v - 1); l1++)
				for (l2 = 0; l2 < (u - 2); l2++)
				{
					m_PhysEnv->AddSpring((l1 * u) + l2,((l1 + 1) * u) + l2 + 1,SshK/FACTOR,SshD/FACTOR,SHEAR_SPRING);
					m_PhysEnv->AddSpring(((l1 + 1) * u) + l2,(l1 * u) + l2 + 1,SshK/FACTOR,SshD/FACTOR,SHEAR_SPRING);
				}
		}
	} // End for the Simple Quad Mesh
	else
	{ // For the Regular Triangular Mesh
		// Insert my HACK HERE
		// Read in my manuscript format.
		manu = new manuModel();
		manu->SMT_DEBUG = SMT_DEBUG;
		manu->YL_UseQuad = YL_UseQuad;
		manu->readMesh(meshfile);
		manu->readTexture(texturefile);
		//	  m_PhysEnv->setWorldY( manu->maxz * 6 );
		m_PhysEnv->setWorldY( manu->maxz * 20 );
		m_PhysEnv->setWorldSize( manu->maxz * 20, manu->maxz * 30, manu->maxz * 20 );
		
		if (manu->nVer == 0) exit(0);
		
		visual = (t_Visual *)malloc(sizeof(t_Visual));
		visual->reuseVertices = true;
		visual->dataFormat = GL_T2F_V3F;
		visual->vPerFace = 3;
		visual->vSize = 6; // 3 floats for vertex
		visual->vertexData = (float *)malloc(sizeof(float) * visual->vSize * manu->nVer);
		visual->vertexCnt = manu->nVer;
		visual->faceIndex = (unsigned short *)malloc(sizeof(unsigned short) * manu->nTrig * visual->vPerFace);
		visual->faceCnt = manu->nTrig;
		
		// SET THE VERTICES
		vertex = (tTexturedVertex *)visual->vertexData;
		int idx = 0;
		int i;
		for( i = 0; i < manu->nVer; i++ )
		{
			vertex->x = manu->verList[idx].x;
			vertex->z = manu->verList[idx].y;
			vertex->y = manu->verList[idx].z;
			vertex++;
			idx++;
		}
		
		// INFORM THE PHYSICAL SIMULATION OF THE PARTICLES
		m_PhysEnv->SetWorldParticles((tTexturedVertex *)visual->vertexData,visual->vertexCnt);
		
		if (m_Skeleton->childCnt > 0)
		{
			if (m_Skeleton->children->visuals->faceIndex != NULL)
				free(m_Skeleton->children->visuals->faceIndex);
			free(m_Skeleton->children->visuals->vertexData);
			free(m_Skeleton->children->visuals);
			free(m_Skeleton->children);
			m_Skeleton->childCnt = 0;
		}
		children = (t_Bone *)malloc(sizeof(t_Bone));
		m_CurBone = &children[m_Skeleton->childCnt];
		ResetBone(m_CurBone,m_Skeleton);
		strcpy(m_CurBone->name,"Cloth");
		m_CurBone->visuals = visual;
		m_CurBone->visualCnt = 1;
		m_Skeleton->childCnt = 1;
		m_Skeleton->children = children;
		
		#define FACTOR 1       
		
		m_PhysEnv->SmartAddSpringInit( visual->faceCnt * 3 );
		for( i = 0; i < visual->faceCnt; i++ )
		{
			m_PhysEnv->SmartAddSpring(manu->trigList[i].idx1, manu->trigList[i].idx2,SstK/FACTOR,SstD/FACTOR,STRUCTURAL_SPRING);
			m_PhysEnv->SmartAddSpring(manu->trigList[i].idx2, manu->trigList[i].idx3,SstK/FACTOR,SstD/FACTOR,STRUCTURAL_SPRING);
			m_PhysEnv->SmartAddSpring(manu->trigList[i].idx1, manu->trigList[i].idx3,SstK/FACTOR,SstD/FACTOR,STRUCTURAL_SPRING);
		}
	} // end else

manuSizeX = u;
manuSizeY = v;

	return 1;
}


/*
// MainFrm.cpp : Implementation of OpenGL Window of 3D Cloth Simulation

void CMainFrame::OnFileOpen()
	char szFilter[] = "DPS files (*.dps)|*.dps|OBJ files (*.obj)|*.obj||";
	m_OGLView.LoadFile(dialog.GetFileName( ),dialog.GetFileTitle( ),dialog.GetFileExt()  );

void CMainFrame::OnFileSave()
	char szFilter[] = "DPS files (*.dps)|*.dps||";
	m_OGLView.SaveFile(dialog.GetFileName( ),dialog.GetFileTitle( ));

*/

int SaveMeshFile()
{
	if( !(m_SimRunning) && strlen(saveMeshFilename) > 2 )
	{
		FILE *fp = fopen( saveMeshFilename, "w" );
		int i;
		
		if( fp >= 0 )
		{
			fprintf(fp, "# Deformation output \n");
			fprintf(fp, "# Vertices NumofVertices \n");
			fprintf(fp, "# x y z ox oy oz\n");
			fprintf(fp, "# Triangles NumofTriangles \n");
			fprintf(fp, "# index1 index2 index3 \n");
			
	
			fprintf(fp, "Vertices %i\n", manu->nVer );
			for(i=0; i < manu->nVer; i++){
				fprintf(fp, "%f %f %f (%f %f %f)\n",
					manu->verList[i].z*manu->scaleFactor + manu->cz, 
					manu->verList[i].y*manu->scaleFactor + manu->cy, 
					manu->verList[i].x*manu->scaleFactor + manu->cx,
					manu->originalList[i].z, 
					manu->originalList[i].y, 
					manu->originalList[i].x
					);
			}
			
			fprintf(fp, "Triangles %i\n", manu->nTrig);
			for(i=0; i < manu->nTrig; i++)
			{
				fprintf(fp, "%i %i %i\n", 
					manu->trigList[i].idx1 +1, 
					manu->trigList[i].idx2 +1,
					manu->trigList[i].idx3 +1);
			}
			fclose(fp);
			return 1;
		}
	}
	return 0;
}

void LoadFilename( char inFilename[], int which )
{
	switch( which )
	{
		case 0:{ strcpy( meshFilename, inFilename ); } break;
		case 1:{ strcpy( textureFilename, inFilename ); } break;
		case 2:{ strcpy( lockedFilename, inFilename ); } break;
		case 3:{ strcpy( scriptFilename, inFilename ); } break;
		case 4:{ strcpy( saveMeshFilename, inFilename ); } break;
	}
}

void performAction( int action, int value )
{
	if( scriptRecord && action != PERFORM_ACTION_DEFINE_MESH_TYPE && action != PERFORM_ACTION_RECORD_SCRIPT_FILE &&
		action != PERFORM_ACTION_PLAY_SCRIPT_FILE && action != PERFORM_ACTION_DEBUG && action != PERFORM_ACTION_QUIT
		&& action != PERFORM_ACTION_SAVE_MESH_FILE )
	{
		fprintf( scriptFile, "%d %d %d\n", currentFrameCount, action, value );
	}

	switch( action )
	{
		case PERFORM_ACTION_DEFINE_MESH_TYPE:
		{
			if( value == PERFORM_ACTION_TRUE ){ YL_UseQuad = true; }
			else{ YL_UseQuad = false; }
		} break;
		
		case PERFORM_ACTION_RECORD_SCRIPT_FILE:
		{
			if( value == PERFORM_ACTION_TRUE && !scriptPlay )
			{
				if( scriptFile == NULL )
				{
					scriptFile = fopen( scriptFilename, "w" );
					if( scriptFile < 0 && SMT_DEBUG ) printf( "Unable to create script file.\n" );
					else if( SMT_DEBUG ) printf( "Recording script file.\n" );
					fprintf( scriptFile, "***SMT SCRIPT FILE\n" );
					currentFrameCount = 0;
				}
				scriptRecord = true;
			}
			else
			{
				scriptRecord = false;
				if( scriptFile != NULL ){ fclose( scriptFile ); scriptFile = NULL; }
			}
		} break;
		
		case PERFORM_ACTION_PLAY_SCRIPT_FILE:
		{
			if( value == PERFORM_ACTION_TRUE && !scriptRecord )
			{
				if( scriptFile == NULL )
				{
					scriptFile = fopen( scriptFilename, "r" );
					if( scriptFile < 0 && SMT_DEBUG ) printf( "Unable to read script file.\n" );
					else
					{
						scriptPlay = true;
						currentScriptFrame = 0;
						currentFrameCount = 0;
						char intro[50];
						fgets( intro, 50, scriptFile );
						if( strncmp( intro, "***SMT SCRIPT FILE", 18 ) != 0 )
						{
							scriptPlay = false;
							scriptFile = NULL;
							if( SMT_DEBUG ) printf( "Incorrect script file format.\n" );
						}
						else if( SMT_DEBUG ) printf( "Playing script file.\n" );
					}
				}
			}
			else
			{
				scriptPlay = false;
				if( scriptFile != NULL ){ fclose( scriptFile ); scriptFile = NULL; }
			}
		} break;
		
		case PERFORM_ACTION_LOCK_POINT:
		{
			AddToSelectionArray( value );
		} break;
		
		case PERFORM_ACTION_ROTATE:
		{
			m_PhysEnv->RotateSystem( (float)value, m_SimRunning );
		} break;
		
		case PERFORM_ACTION_TRANSLATE_X:
		{
			m_PhysEnv->TranslateSystem( (float)value / 20.0f, 0.0f, 0.0f, m_SimRunning );
		} break;
		
		case PERFORM_ACTION_TRANSLATE_Y:
		{
			m_PhysEnv->TranslateSystem( 0.0f, (float)value / 20.0f, 0.0f, m_SimRunning );
		} break;
		
		case PERFORM_ACTION_TRANSLATE_Z:
		{
			m_PhysEnv->TranslateSystem( 0.0f, 0.0f, -(float)value / 20.0f, m_SimRunning );
		} break;
		
		case PERFORM_ACTION_ADJUST_COEF_REST:
		{
			simProperties[0] = (float) value / 1000.0;
		} break;
		
		case PERFORM_ACTION_ADJUST_DAMPING:
		{
			simProperties[1] = (float) value / 1000.0;
		} break;
		
		case PERFORM_ACTION_ADJUST_GRAVITY_X:
		{
			simProperties[2] = (float) value / 1000.0;
		} break;
		
		case PERFORM_ACTION_ADJUST_GRAVITY_Y:
		{
			simProperties[3] = (float) value / 1000.0;
		} break;
		
		case PERFORM_ACTION_ADJUST_GRAVITY_Z:
		{
			simProperties[4] = (float) value / 1000.0;
		} break;
		
		case PERFORM_ACTION_ADJUST_SPRING_CONSTANT:
		{
			simProperties[5] = (float) value / 1000.0;
		} break;
		
		case PERFORM_ACTION_ADJUST_SPRING_DAMPING:
		{
			simProperties[6] = (float) value / 1000.0;
		} break;
		
		case PERFORM_ACTION_ADJUST_USER_FORCE_MAG:
		{
			simProperties[7] = (float) value;
		} break;
		
		case PERFORM_ACTION_COMMIT_SIM_PROPERTIES:
		{
			m_PhysEnv->SetWorldProperties( simProperties );
		} break;
		
		case PERFORM_ACTION_SET_INTEGRATOR_TYPE:
		{
			if( value > 0 ){ m_PhysEnv->m_IntegratorType = MIDPOINT_INTEGRATOR; }
			else if( value < 0 ){ m_PhysEnv->m_IntegratorType = RK4_INTEGRATOR; }
			else{ m_PhysEnv->m_IntegratorType = EULER_INTEGRATOR; }
		} break;
		
		case PERFORM_ACTION_RESET_WORLD:
		{
			m_PhysEnv->ResetWorld();
		} break;
		
		case PERFORM_ACTION_SET_RUNNING:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_SimRunning = true; }
			else{ m_SimRunning = false; }
		} break;
		
		case PERFORM_ACTION_LOCK_PARTICLES:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_LockParticles = true; m_LockParticle = true; }
			else{ m_PhysEnv->m_LockParticles = false; m_LockParticle = false; }
		} break;
		
		case PERFORM_ACTION_CLEAR_PARTICLE_LOCK:
		{
			ClearSelectionArray();
		} break;
		
		case PERFORM_ACTION_SET_COLLISION_ACTIVE:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_CollisionActive = true; }
			else{ m_PhysEnv->m_CollisionActive = false; }
		} break;
		
		case PERFORM_ACTION_SET_USE_GRAVITY:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_UseGravity = true; }
			else{ m_PhysEnv->m_UseGravity = false; }
		} break;
		
		case PERFORM_ACTION_SET_USE_DAMPING:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_UseDamping = true; }
			else{ m_PhysEnv->m_UseDamping = false; }
		} break;
		
		case PERFORM_ACTION_DISPLAY_SPRINGS:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_DrawSprings = true; }
			else{ m_PhysEnv->m_DrawSprings = false; }
		} break;
		
		case PERFORM_ACTION_DISPLAY_VERTICES:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_DrawVertices = true; }
			else{ m_PhysEnv->m_DrawVertices = false; }
		} break;
		
		case PERFORM_ACTION_ADJUST_LOCK_X_AXIS:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_UseXAxis = true; }
			else{ m_PhysEnv->m_UseXAxis = false; }
		} break;
		
		case PERFORM_ACTION_ADJUST_LOCK_Y_AXIS:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_UseYAxis = true; }
			else{ m_PhysEnv->m_UseYAxis = false; }
		} break;
		
		case PERFORM_ACTION_ADJUST_LOCK_Z_AXIS:
		{
			if( value == PERFORM_ACTION_TRUE ){ m_PhysEnv->m_UseZAxis = true; }
			else{ m_PhysEnv->m_UseZAxis = false; }
		} break;
		
		case PERFORM_ACTION_RESET_CAMERA:
		{
			lookAtX = 0.0;     // X Coordinate of camera sphere
			lookAtY = 0.0;     // Y Coordinate of camera sphere
			lookAtZ = 0.0;     // Z Coordinate of camera sphere
			distance = 25.0;   // Distance from the center of the camera's sphere
			axisAngleX = 0.0;  // Angle left/right of "Prime Meridian"
			axisAngleY = 0.0;  // Angle above/below of "Equater"
			fov = 60.0;        // Field of view
		} break;
		
		case PERFORM_ACTION_ADJUST_AXIS_X:
		{
			lookAtX = ( (double)value / 10.0 ); // ( -50 to 50 by .1 )
		} break;
		
		case PERFORM_ACTION_ADJUST_AXIS_Y:
		{
			lookAtY = ( (double)value / 10.0 ); // ( -50 to 50 by .1 )
		} break;
		
		case PERFORM_ACTION_ADJUST_AXIS_Z:
		{
			lookAtZ = ( (double)value / 10.0 ); // ( -50 to 50 by .1 )
		} break;
		
		case PERFORM_ACTION_ADJUST_DISTANCE:
		{
			distance = ( (double)value / 2.0 ); // ( .5 to 100.5 by .5 )
		} break;
		
		case PERFORM_ACTION_ROTATE_ANGLE_X:
		{
			axisAngleX = ( (double)value * PI ) / 180.0; // Convert to radians ( -180 to 180 by 1 )
		} break;
		
		case PERFORM_ACTION_ROTATE_ANGLE_Y:
		{
			axisAngleY = ( (double)value * PI ) / 180.0; // Convert to radians ( -89 to 89 by 1 )
		} break;
		
		case PERFORM_ACTION_ADJUST_FIELD_OF_VIEW:
		{
			fov = (double)value; // Not needed, but implemented.
		} break;
		
		case PERFORM_ACTION_DEBUG:
		{
			if( value == PERFORM_ACTION_TRUE ){ SMT_DEBUG = true; }
			else{ SMT_DEBUG = false; }
		} break;
		
		case PERFORM_ACTION_SAVE_MESH_FILE:
		{
			SaveMeshFile(); // ignoring return value
		} break;

		case PERFORM_ACTION_QUIT:
		{
			m_SimRunning = false;
			DeleteSystem();
		} break;
	}
}

// Part of original windows version - source?
void MBglDrawString( int x, int y, char *string )
{
	while( *string )
	{
		glRasterPos2i( x, y );
		glutBitmapCharacter( GLUT_BITMAP_9_BY_15, *string );
		x += glutBitmapWidth( GLUT_BITMAP_9_BY_15, *string );
		string++;
	}
}
