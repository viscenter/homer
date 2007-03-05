linuxFlags = -lGL -lGLU -lglut -shared -D_UNIX -fPIC
linuxJdk = /usr/java/j2sdk1.4.1_04

flags = -lopengl32 -lglu32 -lglut32 -mno-cygwin
jdk = /cygdrive/c/j2sdk1.4.2_10

cygwin: JavaGL.dll smt.class
linux: JavaGL.so smt.class

JavaGL.dll: JavaGL.cpp Skeleton.o PhysEnv.o MathDefs.o manuModel.o MButils.o MBbitmap.o smc.o
	gcc -mno-cygwin -I$(jdk)/include -I$(jdk)/include/win32  -Wl,--add-stdcall-alias -shared -o JavaGL.dll JavaGL.cpp Skeleton.o PhysEnv.o MathDefs.o manuModel.o MButils.o MBbitmap.o smc.o $(flags) -lstdc++

JavaGL.so: JavaGL.class Skeleton.l PhysEnv.l MathDefs.l manuModel.l MButils.l MBbitmap.l JavaGL.l
	gcc -I$(linuxJdk)/include -I$(linuxJdk)/include/linux -Wl JavaGL.o Skeleton.o PhysEnv.o MathDefs.o manuModel.o MButils.o MBbitmap.o $(linuxFlags) -o JavaGL.so

Skeleton.o: Skeleton.cpp
	gcc -mno-cygwin -c Skeleton.cpp

PhysEnv.o: PhysEnv.cpp
	gcc -mno-cygwin -c PhysEnv.cpp

MathDefs.o: MathDefs.cpp
	gcc -mno-cygwin -c MathDefs.cpp

manuModel.o: manuModel.cpp
	gcc -mno-cygwin -c manuModel.cpp

MButils.o: MButils.cpp
	gcc -mno-cygwin -c MButils.cpp

MBbitmap.o: MBbitmap.cpp
	gcc -mno-cygwin -c MBbitmap.cpp

smc.o: smc.cpp
	gcc -mno-cygwin -c smc.cpp



Skeleton.l: Skeleton.cpp
	gcc -c Skeleton.cpp

PhysEnv.l: PhysEnv.cpp
	gcc -c PhysEnv.cpp

MathDefs.l: MathDefs.cpp
	gcc -c MathDefs.cpp

manuModel.l: manuModel.cpp
	gcc -c manuModel.cpp

MButils.l: MButils.cpp
	gcc -c MButils.cpp

MBbitmap.l: MBbitmap.cpp
	gcc -c MBbitmap.cpp

JavaGL.l: JavaGL.cpp
	gcc -c JavaGL.cpp


JavaGL.class: JavaGL.java
	javac JavaGL.java

CameraPanel.class: CameraPanel.java
	javac CameraPanel.java

DisplayPanel.class: DisplayPanel.java
	javac DisplayPanel.java

PropertiesPanel.class: PropertiesPanel.java
	javac PropertiesPanel.java

smt.class: smt.java JavaGL.class CameraPanel.class DisplayPanel.class PropertiesPanel.class
	javac smt.java

clean:
	rm *.o
	rm *.class
	rm JavaGL.dll
