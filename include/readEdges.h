#ifndef _READEDGES_H_
#define _READEDGES_H_


#include <fstream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"

#include <iostream>


//load a text file and load the edges in to a vector
int readEdges(std::vector< std::vector<cv::Point2i> > &, std::string);

#endif//_READEDGES_
