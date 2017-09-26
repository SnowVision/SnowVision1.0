#ifndef _MATCHED_POSITION_
#define _MATCHED_POSITION_


#include "matching_score.h"
#include <fstream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"


class Matched_position //position on design images
{
 public:
  int x; 
  int y;
  double angle;
  double scale;
  int pixels[10000];
  float tag;
  int singlepixels;
  int pixeldiff;
  
};

#endif//_MATCHED_POSITION_
