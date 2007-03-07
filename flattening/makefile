.SUFFIXES:

%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.class: %.java
	@echo $(notdir $<)
	$(JAVAC) $(JAVAFLAGS) $< -d .

export CC		:=	gcc
export CXX		:=	g++
export JAVAC	:=	javac

UNAME	:=	$(shell uname -s)

# General *nix flags (will be overwritten for Cygwin)
LIBS	:=	-lGL -lGLU -lglut -lstdc++ -lm -ljpeg
LDFLAGS	:=	-Wl -shared -D_UNIX -fPIC
CFLAGS	:=	-g

# Linux-specific flags
ifneq (,$(findstring Linux,$(UNAME)))	
	JDK		:=	/usr/lib/jvm/java-1.5.0-sun
	JAVAGL	:= JavaGL.so
	JAVAINCLUDE	:=	-I$(JDK)/include -I$(JDK)/include/linux
# Cygwin-specific flags
else ifneq (,$(findstring CYGWIN,$(UNAME)))
	LIBS	:=	-lopengl32 -lglu32 -lglut32 -lstdc++
	LDFLAGS	:=	-Wl,--add-stdcall-alias -shared
	CFLAGS	:=	-g -mno-cygwin
	JDK		:=	/cygdrive/c/j2sdk1.4.2_10
	JAVAGL	:= JavaGL.dll
	JAVAINCLUDE	:=	-I$(JDK)/include -I$(JDK)/include/win32
endif

BACKENDSOURCES	:=	source/backend
SMTSOURCES		:=	source/smt
SMTJAVASOURCES	:=	source/smtjava
INCLUDES		:=	$(BACKENDSOURCES) $(SMTSOURCES) $(SMTJAVASOURCES)

export VPATH	:=	$(foreach dir,$(BACKENDSOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SMTSOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(SMTJAVASOURCES),$(CURDIR)/$(dir))

BACKENDCPPFILES	:=	$(foreach dir,$(BACKENDSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SMTCPPFILES		:=	$(foreach dir,$(SMTSOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SMTJAVACPPFILES	:=	$(foreach dir,$(SMTJAVASOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

SMTJAVAJAVAILES	:=	$(foreach dir,$(SMTJAVASOURCES),$(notdir $(wildcard $(dir)/*.java)))

# Use CXX for linking
export LD	:=	$(CXX)

export BACKENDOFILES	:=	$(BACKENDCPPFILES:.cpp=.o)
export SMTOFILES		:=	$(SMTCPPFILES:.cpp=.o)
export SMTJAVAOFILES	:=	$(SMTJAVACPPFILES:.cpp=.o)
export SMTJAVACLASSFILES	:=	$(SMTJAVACPPFILES:.java=.class)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir))

CFLAGS		+= $(INCLUDE)
CXXFLAGS	:= $(CFLAGS)
JAVAFLAGS	:= -sourcepath $(SMTJAVASOURCES)

all: smt smtjava

smt: $(BACKENDOFILES) $(SMTOFILES)
	$(LD) $(SMTOFILES) $(BACKENDOFILES) $(LIBPATHS) $(LIBS) -o $@

.PHONY: smtjava
smtjava: $(JAVAGL) smt.class

$(JAVAGL): $(BACKENDOFILES) $(SMTJAVAOFILES)
	$(LD) $(JAVAINCLUDE) $(LDFLAGS) $(BACKENDOFILES) $(SMTJAVAOFILES) $(LIBS) -o $@

clean:
	rm -f smt
	rm -f *.o
	rm -f *.class
	rm -f $(JAVAGL)
