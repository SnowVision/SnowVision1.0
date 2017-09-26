#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "opencv2/opencv.hpp"
#include "readEdges.h"
#include "matching_score.h"
#include "matched_position.h"


using namespace std;
using namespace cv;

Mat rotate(Mat src, double angle, double scale)
{
    Mat dst;
    Point2f pt(src.cols/2., src.rows/2.);    
    Mat r = getRotationMatrix2D(pt, angle, scale);
    warpAffine(src, dst, r, Size(src.cols, src.rows), cv::INTER_CUBIC,cv::BORDER_CONSTANT,255 );
    return dst;
}


int main ( int argc, char* argv[])
{
   
  
	Mat designImg, designImg_bw, patternImg;
	vector< vector<Point2i> > edgelist;
  
	designImg = imread(argv[1],CV_LOAD_IMAGE_GRAYSCALE);
  
	designImg_bw = designImg > 125;
	cout << argv[1] << ":";
	
	patternImg = imread(argv[2],1);
    	cout << argv[2] << ":";
  
	Mat distancemap(designImg.rows, designImg.cols, DataType<float>::type);
	distanceTransform(designImg_bw, distancemap, CV_DIST_L2, CV_DIST_MASK_PRECISE);
   

	if(!readEdges(edgelist, argv[3])) 
	{
		cout<<"\033[31m Error: Failed to read edge file! \033[0m  "<< argv[3]<<endl;
	}
    
  	
	
	ofstream mtfile_annotation(argv[4]);
	
	if (!mtfile_annotation.is_open())
	{
		cout << "Unable to open output file"<< argv[4] << endl;
		return 0;
	}
	

	//argv[5] is the matchpixels file, pass it.
	float disthresh = stof(argv[6]);
	
	float ratiothresh_matchpixels = stof(argv[7]);
	double scale = stof(argv[8]);
	int angle_start = stoi(argv[9]);
	int angle_end = stoi(argv[10]);

	
//	double t = (double)getTickCount();
	
	//Match variables
	Mat rt_designImg;
		
	float totalpixels = 0;
	float partialpixels = 0;
	float partialdistance = 0;
	
	
	int edgelistsize = edgelist.size();
	int edgesize[500];
	
	
	for(int k = 0; k < edgelistsize; k++)
	{
		edgesize[k] = edgelist[k].size();
		totalpixels = totalpixels+edgesize[k];
	}
	

	mtfile_annotation << totalpixels<<endl;
	
	//Chamfer Matching variables
	double min_distance = 9999;
	double cm_distance = 9999;
	int cm_theta = 0, cm_x = 0, cm_y = 0;
	float cm_scale = 1;
	int cm_partialpixels = 0;


	int match_distancemap_rows = distancemap.rows - patternImg.rows + 1;
	int match_distancemap_cols = distancemap.cols - patternImg.cols + 1;
	
	Mat match_distancemap(match_distancemap_rows, match_distancemap_cols, DataType<float>::type, Scalar::all(65535));
	Mat match_distancemap_theta(match_distancemap_rows, match_distancemap_cols, DataType<int>::type, Scalar::all(0));
	Mat match_distancemap_partialpixels(match_distancemap_rows, match_distancemap_cols, DataType<int>::type, Scalar::all(0));
	
	
	float distance;
	
	for(int theta = angle_start; theta < angle_end; theta++)
	{
		//cout << "Matching at theta "<<theta << endl;
		
		rt_designImg = rotate(designImg_bw, theta, scale);
		rt_designImg = rt_designImg > 125;
		distanceTransform(rt_designImg, distancemap, CV_DIST_L2, CV_DIST_MASK_PRECISE);
		
		for(int i = 0; i< match_distancemap_rows; i++) 
		
		{
		
			for(int j =0; j < match_distancemap_cols; j++)
			
			{
				
				float cur_distance = 0;
					
				partialpixels = 0;
				partialdistance = 0;
			
				for(int k=0; k < edgelistsize; k++) 
				{	
					for(int l=0; l< edgesize[k]; l++)
					{
							
						distance = distancemap.at<float>(edgelist[k][l].x + i,edgelist[k][l].y + j);
						
						if (distance < disthresh) //matched pixels
						{								
							partialpixels++;
	
							partialdistance = partialdistance + distance;
						}
							
						cur_distance = cur_distance + distance;
							
					
					}
						
				}
			
				if(cur_distance < min_distance) //Chamfer Matching result
				{
					min_distance = cur_distance;
					cm_theta = theta;
					cm_x = i;
					cm_y = j;
					cm_scale = scale;
						
				}	
			
				
				if(partialpixels > totalpixels * ratiothresh_matchpixels) //find a minimal distance at one location with all rotation.
				{
					
					
					if(match_distancemap.at<float>(i,j) > (partialdistance/partialpixels))
					{
						
						match_distancemap.at<float>(i,j) = partialdistance/partialpixels;
						match_distancemap_theta.at<int>(i,j) = theta;
						match_distancemap_partialpixels.at<int>(i,j) = partialpixels;
						
					}
					
				}
						
			}//each rotation: 
				
		}
			
	
	}
	
	
	Mat match_distancemap_temp((match_distancemap_rows + 2),(match_distancemap_cols + 2), DataType<float>::type, Scalar::all(65535));
	
	for(int i = 1; i < (match_distancemap_rows + 1); i++)
	{
		for(int j = 1; j < (match_distancemap_cols + 1); j++)
		{
			match_distancemap_temp.at<float>(i, j) = match_distancemap.at<float>((i-1), (j-1));
		}
	} //padding match_distancemap for 8-connected local minimum and copy it to match_distancemap_temp
	
	
	Mat match_distancemap_scratch(match_distancemap_rows, match_distancemap_cols, DataType<int>::type, Scalar::all(0));
	
	for(int i = 1; i < (match_distancemap_rows + 1); i++) // 8 connected local minimum record center
	{
		for(int j = 1; j < (match_distancemap_cols + 1); j++) 
		{
			float local_min = match_distancemap_temp.at<float>(i,j);
			int local_i = i, local_j = j;
			if(match_distancemap_temp.at<float>((i-1),(j-1)) < local_min)
			{
				local_min = match_distancemap_temp.at<float>((i-1),(j-1));
				local_i = i-1;
				local_j = j-1;
			}
			if(match_distancemap_temp.at<float>(i,(j-1)) < local_min)
			{
				local_min = match_distancemap_temp.at<float>(i,(j-1));
				local_i = i;
				local_j = j-1;
			}
			if(match_distancemap_temp.at<float>((i+1),(j-1)) < local_min)
			{
				local_min = match_distancemap_temp.at<float>((i+1),(j-1));
				local_i = i+1;
				local_j = j-1;
			}
			if(match_distancemap_temp.at<float>((i-1),j) < local_min)
			{
				local_min = match_distancemap_temp.at<float>((i-1),j);
				local_i = i-1;
				local_j = j;
			}
			if(match_distancemap_temp.at<float>((i+1),j) < local_min)
			{
				local_min = match_distancemap_temp.at<float>((i+1),j);
				local_i = i+1;
				local_j = j;
			}
			if(match_distancemap_temp.at<float>((i-1),(j+1)) < local_min)
			{
				local_min = match_distancemap_temp.at<float>((i-1),(j+1));
				local_i = i-1;
				local_j = j+1;
			}
			if(match_distancemap_temp.at<float>(i,(j+1)) < local_min)
			{
				local_min = match_distancemap_temp.at<float>(i,(j+1));
				local_i = i;
				local_j = j+1;
			}
			if(match_distancemap_temp.at<float>((i+1),(j+1)) < local_min)
			{
				local_min = match_distancemap_temp.at<float>((i+1),(j+1));
				local_i = i+1;
				local_j = j+1;
			}
			if((local_i == i)&&(local_j == j))
			{
				if(local_min < 65535)
				{
					match_distancemap_scratch.at<int>((i-1),(j-1)) = 1; //find the local minimum if in 8-connected matrix, the center is the minimum
					//totalmatches++;
				}
			}
		}
		
	}
	
	
	
	int totalmatches = 0;
	
	for(int i = 0; i< match_distancemap_rows; i++) //checking the matches
	{
		for(int j =0; j < match_distancemap_cols; j++)
		{
			if(match_distancemap_scratch.at<int>(i,j) == 1)
			{
				
				if ((match_distancemap.at<float>(i,j) <= disthresh)&&(match_distancemap.at<float>(i,j) > (1/totalpixels)))
				{
					if(match_distancemap_partialpixels.at<int>(i,j)> totalpixels * ratiothresh_matchpixels)
					{
						mtfile_annotation << i << "," << j << 
						","<< match_distancemap_theta.at<int>(i,j) <<
						","<< scale <<
						","<< match_distancemap_partialpixels.at<int>(i,j) <<
						","<<match_distancemap.at<float>(i,j) << endl;
						totalmatches++;
					}
					
				}
			}
		}
	}
  
    //t = ((double)getTickCount() - t)/getTickFrequency();

	cout << "Total matches: "<<totalmatches<<":"; 
	//cout << "Total pixels: "<<totalpixels << "min_distance: "<< min_distance<<endl;
	cout << "Chamfer Distance: theta: scale: x: y: score: "<<":";
	cout << cm_theta<<":"<<cm_scale<<":"<<cm_x<<":"<<cm_y<<":"<<100*min_distance/totalpixels <<endl;
	
    mtfile_annotation.close();
    //cout << "hello"<<endl;
    edgelist.clear();
    //cout << "edgelist hello"<< endl;
  
	return 0;
}
