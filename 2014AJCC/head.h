#ifndef _INTTIALIZE_H_
#define _INTTIALIZE_H_

#define _CRT_SECURE_NO_WARNINGS //http://blog.csdn.net/xuleilx/article/details/7281499
#include <stdio.h>


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "cv.h"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

const int width  = 1920;
const int height = 1088;  
const int PICNUM = 65;
const long FRAMESIZE=width*height;

const int dusize=2;
const int s_width=width/dusize;
const int s_height=height/dusize;


#endif