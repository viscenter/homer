#include "JavaGL.h"
#include "smc.h"

JNIEXPORT jint JNICALL Java_JavaGL_nativeInit( JNIEnv *env, jobject object )
{
	return (jint)Init();
}

JNIEXPORT jintArray JNICALL Java_JavaGL_getLockedArray( JNIEnv *env, jobject object )
{
	int *intLockedArray = NULL;
	int index = GetLockedArray( intLockedArray );
	jintArray lockArray = (jintArray)env->NewIntArray(index);
	env->SetIntArrayRegion( lockArray, 0, index, (jint *)intLockedArray );
	free( intLockedArray );
	return lockArray;
}

JNIEXPORT void JNICALL Java_JavaGL_reshapeCanvas( JNIEnv *env, jobject object, jint inWidth, jint inHeight )
{
	ReshapeCanvas( inWidth, inHeight );
}

JNIEXPORT void JNICALL Java_JavaGL_renderScene( JNIEnv *env, jobject object )
{
	RenderScene();
}

JNIEXPORT jint JNICALL Java_JavaGL_scriptIsDone( JNIEnv *env, jobject object )
{
	return (jint)ScriptIsDone();
}

JNIEXPORT void JNICALL Java_JavaGL_pickPoint( JNIEnv *env, jobject object, jint x, jint y )
{
	PickPoint( x, y );
}

JNIEXPORT void JNICALL Java_JavaGL_lockPoint( JNIEnv *env, jobject object, jint x, jint y )
{
	LockPoint( x, y );
}

JNIEXPORT void JNICALL Java_JavaGL_dragPoint( JNIEnv *env, jobject object, jint x, jint y )
{
	DragPoint( x, y );
}

JNIEXPORT void JNICALL Java_JavaGL_mouseRelease( JNIEnv *env, jobject object )
{
	MouseRelease();
}

JNIEXPORT void JNICALL Java_JavaGL_loadFilename( JNIEnv *env, jobject object, jstring inFilename, jint which )
{
	const char *str = env->GetStringUTFChars( inFilename, 0 );
	char filename[64];
	strcpy( filename, str );

	LoadFilename( filename, which );

	env->ReleaseStringUTFChars( inFilename, str );
}

JNIEXPORT void JNICALL Java_JavaGL_performAction( JNIEnv *env, jobject object, jint action, jint value )
{
	performAction( action, value );
}
