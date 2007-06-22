# Set the JDK variable to point to your JDK path, then run
# make install as root

JDK	:=	/usr/lib/jvm/java-1.5.0-sun

install: gl4java.jar libGL4Java2.8.0.8-Linux-glibc2-xf86-x86.tar.gz 
	cp gl4java.jar $(JDK)/jre/lib/ext/
	tar xzvf libGL4Java2.8.0.8-Linux-glibc2-xf86-x86.tar.gz
	cp libGL4JavaJauGljJNI* $(JDK)/jre/lib/i386/

gl4java.jar: gl4java2.8.0.8-jar.zip
	unzip gl4java2.8.0.8-jar.zip

clean:
	rm -f gl4java.jar
	rm -f libGL4JavaJauGljJNI*
