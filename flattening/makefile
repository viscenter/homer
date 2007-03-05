UNAME	:=	$(shell uname -s)

ifneq (,$(findstring Linux,$(UNAME)))
	LDFLAGS	:=	-lGL -lGLU -lglut -shared -D_UNIX -fPIC
	CFLAGS	:=	
	JDK		:=	/usr/lib/jvm/java-1.5.0-sun
else ifneq (,$(findstring CYGWIN,$(UNAME)))
	LDFLAGS	:=	-lopengl32 -lglu32 -lglut32 
	CFLAGS	:=	-g -mno-cygwin
	JDK		:=	/cygdrive/c/j2sdk1.4.2_10
endif

cygwin: JavaGL.dll smt.class
linux: JavaGL.so smt.class

JavaGL.dll: JavaGL.cpp Skeleton.o PhysEnv.o MathDefs.o manuModel.o MButils.o MBbitmap.o smc.o
	gcc $(CFLAGS) -I$(JDK)/include -I$(JDK)/include/win32  -Wl,--add-stdcall-alias -shared -o JavaGL.dll JavaGL.cpp Skeleton.o PhysEnv.o MathDefs.o manuModel.o MButils.o MBbitmap.o smc.o $(LDFLAGS) -lstdc++

JavaGL.so: JavaGL.class Skeleton.o PhysEnv.o MathDefs.o manuModel.o MButils.o MBbitmap.o JavaGL.o smc.o
	gcc -I$(JDK)/include -I$(JDK)/include/linux -Wl JavaGL.o Skeleton.o PhysEnv.o MathDefs.o manuModel.o MButils.o MBbitmap.o smc.o $(LDFLAGS) -o JavaGL.so

Skeleton.o: Skeleton.cpp
	gcc $(CFLAGS) -c Skeleton.cpp

PhysEnv.o: PhysEnv.cpp
	gcc $(CFLAGS) -c PhysEnv.cpp

MathDefs.o: MathDefs.cpp
	gcc $(CFLAGS) -c MathDefs.cpp

manuModel.o: manuModel.cpp
	gcc $(CFLAGS) -c manuModel.cpp

MButils.o: MButils.cpp
	gcc $(CFLAGS) -c MButils.cpp

MBbitmap.o: MBbitmap.cpp
	gcc $(CFLAGS) -c MBbitmap.cpp

smc.o: smc.cpp
	gcc $(CFLAGS) -c smc.cpp

JavaGL.o: JavaGL.cpp
	g++ $(CFLAGS) -c JavaGL.cpp

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
	rm -f *.o
	rm -f *.class
	rm -f JavaGL.dll JavaGL.so
