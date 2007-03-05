
*notes
  Euler and Midpoint can cause earlier program termination when damping is off or is set to low.

to compile:
  in cygwin - make
  in linux - make linux


Usage: java smt -?, -h, --help


SHIFT + Button 1: Rotate Camera
SHIFT + Button 3: Adjust Zoom
SHIFT + Button 2 (1&3): Adjust Camera Height

CTRL + Button 1: Rotate Object around the X Axis

ALT + Button 1: Translate object X & Y  (X is correct when veiwed from the "front")
ALT + Button 2: Translate object Z



Demo Tips:
1. java smt canvas_scroll_10000.surf canvas_scroll.ppm
		spheretext.surf spheretext.ppm
1. When unrolling, Set "Lock X Axis" to avoid the cloth overlapping itself.
2. After unrolling, for flattening, change the gravity to "Gravity X"