// PhysEnv.h : header file
//
// Updated July 30, 2003

#if !defined(_PhysEnv_H)
#define _PhysEnv_H

#include <stdio.h> // for FILE
#include "MathDefs.h"

#define EPSILON 0.00001f	// ERROR TERM
#define DEFAULT_DAMPING 0.002f

enum tCollisionTypes
{
	NOT_COLLIDING,
	PENETRATING,
	COLLIDING
};

enum tIntegratorTypes
{
	EULER_INTEGRATOR,
	MIDPOINT_INTEGRATOR,
	RK4_INTEGRATOR
};

// CLASSIFY THE SPRINGS SO I CAN HANDLE THEM SEPARATELY
enum tSpringTypes
{
	MANUAL_SPRING,
	STRUCTURAL_SPRING,
	SHEAR_SPRING,
	BEND_SPRING
};

// TYPE FOR A PLANE THAT THE SYSTEM WILL COLLIDE WITH
struct tCollisionPlane
{
	tVector normal;	// inward pointing
	float d;				// ax + by + cz + d = 0
};

// TYPE FOR A PHYSICAL PARTICLE IN THE SYSTEM
struct tParticle
{
	tVector pos;		// Position of Particle
	tVector v;			// Velocity of Particle
	tVector f;			// Force Acting on Particle
	float	oneOverM;	// 1 / Mass of Particle
	//bool locked;
};

// TYPE FOR CONTACTS THAT ARE FOUND DURING SIM
struct tContact
{
	int particle;		// Particle Index
	tVector normal;	// Normal of Collision plane
};

// TYPE FOR SPRINGS IN SYSTEM
struct tSpring
{
	int   p1,p2;		// PARTICLE INDEX FOR ENDS
	float restLen;		// LENGTH OF SPRING AT REST
	float Ks;			// SPRING CONSTANT
	float Kd;			// SPRING DAMPING
	int   type;			// SPRING TYPE - USED FOR SPECIAL FEATURES
};

class CPhysEnv
{
	public:
		CPhysEnv(); 	// Construction
		virtual ~CPhysEnv();
		void RenderWorld();
		void SetWorldParticles( tTexturedVertex *coords, int particleCnt );
		void ResetWorld();
		void Simulate( float DeltaTime, bool running );
		void ApplyUserForce( tVector *force );
		void SetMouseForce( int deltaX, int deltaY, tVector *localX, tVector *localY );
		void GetNearestPoint( int x, int y );
bool GetParticlePosition( int index, float position[] );
int GetNumberOfParticles();
void SetSelectedParticle( int number );
void RotateSystem( float degree, bool running );
void TranslateSystem( float deltaX, float deltaY, float deltaZ, bool running );

		//void AddSpring();
		void AddSpring( int v1, int v2, float Ksh, float Ksd, int type );
		void SetWorldProperties( float simProperties[] );
		void SetVertexProperties( float m_VertexMass ); // needed?
		void setWorldY( float Y )
			{
				m_WorldSizeY = Y;
				// MAKE THE BOTTOM PLANE (FLOOR)
				MAKEVECTOR( m_CollisionPlane[1].normal, 0.0f, 1.0f, 0.0f );
				m_CollisionPlane[1].d = m_WorldSizeY / 2.0f;
			}
void setWorldSize( float x, float y, float z );
		void FreeSystem();
		void LoadData( FILE *fp );
		void SaveData( FILE *fp );
		bool m_UseGravity;			// SHOULD GRAVITY BE ADDED IN
		bool m_UseDamping;			// SHOULD DAMPING BE ON
		bool m_UserForceActive;		// WHEN USER FORCE IS APPLIED
		bool m_DrawSprings;			// DRAW THE SPRING LINES
		bool m_DrawVertices;			// DRAW VERTICES
		bool m_MouseForceActive;		// MOUSE DRAG FORCE
		bool m_CollisionActive;		// COLLISION SPHERES ACTIVE
		bool m_CollisionRootFinding;	// AM I SEARCHING FOR A COLLISION
		bool m_DrawStructural;		// DRAW STRUCTURAL CLOTH SPRINGS
		//bool m_DrawShear;			// DRAW SHEAR CLOTH SPRINGS
		//bool m_DrawBend;				// DRAW BEND CLOTH SPRINGS
		int  m_IntegratorType;
		bool m_Unscrolling, m_LockParticles, *lockedArray;
		bool m_UseXAxis;            // Should X Axis be included when simulating
		bool m_UseYAxis;            // Should Y Axis be included when simulating
		bool m_UseZAxis;            // Should Z Axis be included when simulating
		// Improve LOAD time
		int smartAddSpringCurrentSpring;
		void SmartAddSpringInit( int sizeOfArray );
		void SmartAddSpring( int v1, int v2, float Ksh, float Ksd, int type );

	
	// Attributes
	private:
		float            m_WorldSizeX,m_WorldSizeY,m_WorldSizeZ;
		tVector          m_Gravity;				// GRAVITY FORCE VECTOR
		tVector          m_UserForce;				// USER FORCE VECTOR
		float            m_UserForceMag;			// MAGNITUDE OF USER FORCE
		float            m_Kd;						// DAMPING FACTOR
		float            m_Kr;						// COEFFICIENT OF RESTITUTION
		float            m_Ksh;						// HOOK'S SPRING CONSTANT
		float            m_Ksd;						// SPRING DAMPING
		float            m_MouseForceKs;			// MOUSE SPRING COEFFICIENT
		tCollisionPlane  *m_CollisionPlane;		// LIST OF COLLISION PLANES
		int              m_CollisionPlaneCnt;			
		tContact         *m_Contact;				// LIST OF POSSIBLE COLLISIONS
		int              m_ContactCnt;			// COLLISION COUNT
		tParticle        *m_ParticleSys[3];		// LIST OF PHYSICAL PARTICLES
		tParticle        *m_CurrentSys, *m_TargetSys;
		tParticle        *m_TempSys[5];			// SETUP FOR TEMP PARTICLES USED WHILE INTEGRATING
		int              m_ParticleCnt;
		tSpring          *m_Spring;				// VALID SPRINGS IN SYSTEM
		int              m_SpringCnt;		
		int              m_Pick[2];				// INDEX COUNTERS FOR SELECTING
		tVector          m_MouseDragPos[2];		// POSITION OF DRAGGED MOUSE VECTOR
		float            vertexPointSize;
	// Operations
		inline void	IntegrateSysOverTime( tParticle *initial, tParticle *source, tParticle *target, float deltaTime );
		void RK4Integrate( float DeltaTime );
		void MidPointIntegrate( float DeltaTime );
		void EulerIntegrate( float DeltaTime );
		void ComputeForces( tParticle	*system );
		int  CheckForCollisions( tParticle	*system );
		void ResolveCollisions( tParticle	*system );
		void CompareBuffer( int size, float *buffer,float x, float y );
};

#endif // !defined(_PhysEnv_H)
