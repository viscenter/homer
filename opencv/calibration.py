#!/usr/bin/python

from opencv.cv import *
from opencv.highgui import *

import os
import sys
import threading
import time
import atexit
import math

cvStartWindowThread()
debugLock = threading.Lock()
debugEvent = threading.Event()
calibrated = False

def waitKey():
  while (debugEvent.isSet() == False):
    debugLock.acquire()
    cvWaitKey(10)
    debugLock.release()
    time.sleep(.1)

debugThread = threading.Thread(None,waitKey)

def debug():
  if (debugThread.isAlive()):
    return
  debugLock.acquire()
  cvNamedWindow("calibrate")
  debugLock.release()
  atexit.register(debugEvent.set)
  debugThread.start()

def findCorners(im, dim):
  found,corners = cvFindChessboardCorners(im,dim)
  subpix = cvSize(5,5)

  if found == False and im.width > 100:
    im2 = cvCreateImage( cvSize(im.width/2,im.height/2), \
      IPL_DEPTH_8U, 1 )
    cvPyrDown(im,im2)
    found,corners = findCorners(im2,dim)
    for i in corners:
      i.x *= 2
      i.y *= 2
    subpix = cvSize(11,11)

  if found:
    if corners[0].x + corners[0].y > \
       corners[dim.width*dim.height-1].x + corners[dim.width*dim.height-1].y:
      for i in range(dim.width*dim.height/2):
        tmp = corners[i]
        corners[i] = corners[dim.width*dim.height-1-i]
        corners[dim.width*dim.height-1-i] = tmp
    if corners[dim.width-1].x - corners[0].x < \
       corners[dim.width*(dim.height-1)].x - corners[0].x or \
       corners[dim.width-1].y - corners[0].y > \
       corners[dim.width*(dim.height-1)].y - corners[0].y:
      for x in range(dim.width):
        for y in range(x,dim.height):
          tmp = corners[x*dim.width+y]
          corners[x*dim.width+y] = corners[y*dim.width+x]
          corners[y*dim.width+x] = tmp
    if corners[0].x > corners[dim.width-1].x:
      for x in range(dim.width):
        for y in range(dim.height/2):
          tmp = corners[x*dim.width+y]
          corners[x*dim.width+y] = corners[(x+1)*dim.width-y-1]
          corners[(x+1)*dim.width-y-1] = tmp
    if corners[0].y > corners[dim.width*(dim.height-1)].y:
      for y in range(dim.height):
        for x in range(dim.width/2):
          tmp = corners[x*dim.width+y]
          corners[x*dim.width+y] = corners[(dim.height-1-x)*dim.width+y]
          corners[(dim.height-1-x)*dim.width+y] = tmp

  corners = cvFindCornerSubPix( im, corners, subpix, cvSize(-1,-1), \
    cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,100,0.001))

  return found,corners

def findAllCorners(files,dim):
  global camSize
  corners = []
  for f in files:
    im = cvLoadImage(f,CV_LOAD_IMAGE_GRAYSCALE)
    camSize = cvSize(im.width,im.height)
    t = cvLoadImage(f,CV_LOAD_IMAGE_COLOR)
    t2 = cvCreateImage(cvSize(im.width/2,im.height/2), IPL_DEPTH_8U, 3 )
    tmp = cvCreateImage(cvSize(im.width/4,im.height/4), IPL_DEPTH_8U, 3 )
    cvPyrDown(t,t2)
    cvPyrDown(t2,tmp)
    found,c = findCorners( im, dim )
    print 'Found ', len(c), ' corners'
    cvDrawChessboardCorners( t, dim, c, found )
    #cvSaveImage( f + 'detected.png', t )
    for i in c:
      i.x /= 4
      i.y /= 4
    cvDrawChessboardCorners( tmp, dim, c, found )
    for i in c:
      i.x *= 4
      i.y *= 4
    debugLock.acquire()
    cvShowImage("calibrate", tmp)
    cvWaitKey(100)
    debugLock.release()
    if found: corners.append(c)
  points = cvCreateMat( len(corners)*dim.width*dim.height, 2, CV_64FC1 )
  for i,r in enumerate(corners):
    for j,c in enumerate(r):
      points[i*dim.width*dim.height+j][0] = c.x
      points[i*dim.width*dim.height+j][1] = c.y
  return len(corners), points

def findImages(directory, prefix):
  imgs = [ directory+'/'+x for x in os.listdir(directory) if x.startswith(prefix) ]
  imgs.sort()
  return imgs

def calibrate( dir, dim=cvSize(7,7), size=25.4 ):
  count, points = findAllCorners( findImages(dir,'Image'), dim )
  world = cvCreateMat( count*dim.width*dim.height, 3, CV_64FC1 )
  worldp = [ [ x*size,y*size,0] for y in range(dim.height) for x in range(dim.width) ]
  for i in range(count):
    for j in range(dim.width*dim.height):
      world[i*dim.width*dim.height+j][0] = worldp[j][0]
      world[i*dim.width*dim.height+j][1] = worldp[j][1]
      world[i*dim.width*dim.height+j][2] = worldp[j][2]
  counts = cvCreateMat( 1, count, CV_32S )
  cvSet( counts, dim.width*dim.height )
  rot = cvCreateMat( 1, count, CV_64FC3 )
  trans = cvCreateMat( 1, count, CV_64FC3 )

  intr,dist = cvCalibrateCamera2( world, points, counts,\
    camSize, rot, trans )
  return intr,dist,rot,trans,points,world,counts

def reproj_error(pts,wld,cnt,intr,dist):
  total = 0
  r = cvCreateMat( 3, 1, CV_64FC1 )
  t = cvCreateMat( 3, 1, CV_64FC1 )
  err_x = cvCreateMat( pts.rows, 1, CV_64FC1 )
  err_y = cvCreateMat( pts.rows, 1, CV_64FC1 )
  err = cvCreateMat( pts.rows, 1, CV_64FC1 )
  for i in cnt:
    w = cvCreateMat( i, 3, CV_64FC1 )
    p = cvCreateMat( i, 2, CV_64FC1 )
    p2 = cvCreateMat( i, 2, CV_64FC1 )
    for j in range(i):
      w[j][0] = wld[total+j][0]
      w[j][1] = wld[total+j][1]
      w[j][2] = wld[total+j][2]
      p[j][0] = pts[total+j][0]
      p[j][1] = pts[total+j][1]
    cvFindExtrinsicCameraParams2( w, p, intr, dist, r, t )
    cvProjectPoints2( w, r, t, intr, dist, p2 )
    for j,x in enumerate(p2):
      err_x[total+j] = p[j][0] - x[0]
      err_y[total+j] = p[j][1] - x[1]
      err[total+j] = math.sqrt( \
        (p[j][0] - x[0])*(p[j][0] - x[0]) + \
        (p[j][1] - x[1])*(p[j][1] - x[1]) )
    total+=i
  mean = CvScalar()
  std_dev = CvScalar()
  cvAvgSdv( err_x, mean, std_dev )
  print std_dev[0],
  cvAvgSdv( err_y, mean, std_dev )
  print std_dev[0],
  cvAvgSdv( err, mean, std_dev )
  print std_dev[0]

def readCheckers(file, image):
  checkers = cvCreateMat( 64, 3, CV_64FC1 )
  f = open(file,'r')
  line = f.readline()
  i=0
  while line:
    if line[0] != '#':
      line = line.strip().split(' ')
      if line and line[0] == 'v':
        checkers[i][0] = float(line[1])
        checkers[i][1] = float(line[2])
        checkers[i][2] = float(line[3])
        i = i+1
    line = f.readline()
  f.close()
  size = math.sqrt(i)
  if size != int(size):
    print 'not square!'
  else:
    dist = (size-1)*4
    vect = cvPoint3D64f( \
        (checkers[i-1][0]-checkers[0][0])/dist, \
        (checkers[i-1][1]-checkers[0][1])/dist, \
        (checkers[i-1][2]-checkers[0][2])/dist )
    for j in range(i):
      checkers[j][0] += vect.x
      checkers[j][1] += vect.y+1
      checkers[j][2] += vect.z
  im = cvLoadImage( image, CV_LOAD_IMAGE_GRAYSCALE )
  dim = int(size*2-1)
  found,corners = findCorners( im, cvSize(dim,dim) )
  if found == False:
    return None,None
  img_checkers = cvCreateMat( i, 2, CV_64FC1 )
  j=0
  for r in range(0,dim,2):
    for c in range(0,dim,2):
      img_checkers[j][0] = corners[r*dim+c].x
      img_checkers[j][1] = corners[r*dim+c].y
      j = j+1
  return checkers[:i],img_checkers

def extrinsics(file,image,K,d):
  checkers,img_checkers = readCheckers(file,image)
  r = cvCreateMat( 3, 1, CV_64FC1 )
  t = cvCreateMat( 3, 1, CV_64FC1 )
  cvFindExtrinsicCameraParams2( checkers, img_checkers, K, d, r, t )
  im = cvLoadImage( image, CV_LOAD_IMAGE_COLOR )
  img_mat = cvCreateMat( 16, 2, CV_32FC1 )
  cvProjectPoints2( checkers, r, t, K, d, img_mat )
  img_proj = [ cvPoint2D32f( img_mat[0][0], img_mat[0][1] ) ]
  for i in range(1,16):
    img_proj.append( cvPoint2D32f( img_mat[i][0], img_mat[i][1] ) )
  cvDrawChessboardCorners( im, cvSize(4,4), img_proj, 1 )
  cvSaveImage('matched_checkers.jpg',im)
  debugLock.acquire()
  cvShowImage("calibrate", im)
  cvWaitKey(100)
  debugLock.release()
  return r,t

def mapTexture( file, K,d,r,t ):
  f = open(file,'r')
  o = open(file.split('.')[0]+'.surf','w')
  line = f.readline()
  line = f.readline()
  line = f.readline()
  line = line.strip().split(' ')
  verts = int(line[ len(line)-1 ])
  points = cvCreateMat( verts, 3, CV_64FC1 )
  texture = cvCreateMat( verts, 2, CV_64FC1 )
  i=0
  line = f.readline()
  while line and i < verts:
    if line[0] != '#':
      line = line.strip().split(' ')
      if line and line[0] == 'v':
        points[i][0] = float(line[1])
        points[i][1] = float(line[2])
        points[i][2] = float(line[3])
        i = i+1
    line = f.readline()
  cvProjectPoints2( points, r, t, K, d, texture )
  o.write( 'Vertices '+repr(verts)+'\n' )
  for i in range(verts):
    out = "%.6f %.6f %.6f %.6f %.6f\n" % \
           ( points[i][0], points[i][2], points[i][1],
             texture[i][0]/camSize.width, texture[i][1]/camSize.height )
    o.write( out )
  line = f.readline()
  line = f.readline()
  line = line.strip().split(' ')
  tris = int(line[ len(line)-1 ])
  o.write( 'Triangles '+repr(tris)+'\n' )
  i=0
  line = f.readline()
  while line and i < tris:
    if line[0] != '#':
      line = line.strip().split(' ')
      if line and line[0] == 'f':
        o.write( '  '+line[1]+' '+line[2]+' '+line[3]+'\n' )
        i = i+1
    line = f.readline()
  o.close()
  f.close()

def test(dir = 'calib2', im = 'Image_0267.JPG', check='checkers-transform.obj',
    obj_file = 'certificate-wrap-uniform2.obj' ):
  global results
  global calibrated
  if calibrated == False:
    results = calibrate(dir)
    calibrated = True
  K = results[0]
  d = results[1]
  reproj_error(results[4],results[5],results[6],K,d)
  print 'K', K
  print 'd', d
  r,t = extrinsics(check,dir+'/'+im, K,d)
  print 'R', r
  print 't', t
  mapTexture( obj_file, K,d,r,t )

#test()
