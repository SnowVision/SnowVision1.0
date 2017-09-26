#ifndef _MATCHING_SCORE_
#define _MATCHING_SCORE_


#include <fstream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"


class matching_score
{
 public:
  std::vector<double> raw;
  double avg;
  double stddev;
};

#endif//_MATCHING_SCORE_
