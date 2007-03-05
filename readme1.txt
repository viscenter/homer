cd.nnnnnn
to compile:
  in cygwin - make
  in linux - make linux


Usage: java smt -? or -h :for help
       java smt *.surf *.ppm


SHIFT + Button 1: Rotate Camera
SHIFT + Button 3: Adjust Zoom
SHIFT + Button 2 (1&3): Adjust Camera Height

CTRL + Button 1: Rotate Object around the X Axis

ALT + Button 1: Translate object X & Y  (X is correct when veiwed from the "front")
ALT + Button 2: Translate object Z


**Using Regular Texture Map:
1. For flattening, change the gravity to "Gravity X" (or other direction based on the orientation of the document model)
2. Don't use texture image with size larger than 2048*1024. 


**Using Irregular(Trianglular) Texture Map:
1. 
manuModel.cpp:
YL_UseTriangularTextureMap = true;

manuModel.h
#define TEXW 1024
#define TEXH 1024

2. compile 
- make

3. java smt canvas_scroll_10000.surf canvas_scroll.ppm
		spheretext.surf spheretext.ppm
4. When unrolling, Set "Lock X Axis" to avoid the cloth overlapping itself.
5. After unrolling, for flattening, change the gravity to "Gravity X"

