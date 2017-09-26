//main test program

#include <iostream>
#include <vector>
#include <string>
#include "opencv2/opencv.hpp"
//#include "readDistancemap.h"
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
    warpAffine(src, dst, r, Size(src.cols, src.rows), cv::INTER_CUBIC,cv::BORDER_CONSTANT,cv::Scalar(255,255,255) );
    return dst;
};

class MatchResult
{
	//vector <int> edgeindex;
	public:
		int match1,match2;
		//double score;
		int pixeldiff;
	static bool before( const MatchResult& c1, const MatchResult& c2 ) { return c1.pixeldiff < c2.pixeldiff; }
	public:
		//MatchResult(int m1, int m2, int ind, double s)
		MatchResult(int m1, int m2, int ind)
		{
			match1 = m1;
			match2 = m2;
			//score = s;
			pixeldiff = ind;
		}
		int getpixeldiff()
		{
			return pixeldiff;
		}
};


int readMatchedPos(vector<Matched_position> &dest, string filename_annotation)
{


	ifstream mpfile_annotation(filename_annotation);
	
	if(!mpfile_annotation.is_open())
    {
		cout<<"No such file or file broken! "<<filename_annotation<<endl;
		return -1;
    }
    
    
    vector<string> mpalines;
	string mpaline;
	
	while (getline(mpfile_annotation, mpaline))
    {
		mpalines.push_back(mpaline);
		
    }
    
   
	mpfile_annotation.close();
	//mpfile_matchedpixels.close();
	
	Matched_position mppos;
	
	for(int i=0; i<mpalines.size(); i++)
    {
		string::size_type pos;
		if (i == 0 )
		{
			std::stringstream ss(mpalines[0]);
			ss >> mppos.tag; //total matches
						
		}
		else
		{
			
			std::stringstream ss(mpalines[i]);
			ss >> mppos.x;
			if (ss.peek()==',')
				ss.ignore();
			ss >> mppos.y;
			if (ss.peek()==',')
				ss.ignore();
			ss >> mppos.angle;
			if (ss.peek()==',')
				ss.ignore();
				
			ss >> mppos.scale;
			if (ss.peek()==',')
				ss.ignore();
			
			ss >> mppos.pixeldiff;
			if (ss.peek()==',')
				ss.ignore();
			
			
			
			//mpfile_matchedpixels.get(&outchar, sizeof(char)); //read endl.
			dest.push_back(mppos);
		}	
	}	
	
	mpalines.clear();	
	
	
	return 1;
	
}

int main ( int argc, char* argv[])
{
 
  
	//Mat distance_map_display;
	Mat designImg, designImg_bw, patternImg;
	vector< vector<Point2i> > edgelist;
	vector<Matched_position > matchedpos;
	vector <MatchResult> matchedresult;
    
	designImg = imread(argv[1],1);
	patternImg = imread(argv[2], 1);
	
	cout << argv[2] << endl;
	Mat distancemap(designImg.rows, designImg.cols, DataType<float>::type);
	
	if(!readEdges(edgelist, argv[3]))
 
    {
		cout<<"\033[31m Error: Failed to read edge file! \033[0m"<<endl;
    }
    
    int totalpixels = 0;
    int edgelistsize = edgelist.size();
    int edgesize[500];
    for(int k = 0; k < edgelistsize; k++)
    {
		edgesize[k] = edgelist[k].size();
		totalpixels = totalpixels + edgesize[k];
	}
    
	if(!readMatchedPos(matchedpos, argv[4]))

	{
		cout <<"Error: Failed to load matched edges! "<<endl;
	}
	float disthresh = stof(argv[5]);
	
	int x;
	int y;
		
	double scale;
	double angle;
	Mat rt_designImg;
	double distance;
	
	for(int i = 0; i < matchedpos.size(); i++) //get the actual matched pixels
	{
		x = matchedpos[i].x;
		y = matchedpos[i].y;
		
		scale = matchedpos[i].scale;
		angle = matchedpos[i].angle;
		rt_designImg = rotate(designImg_bw, angle, scale);
		rt_designImg = rt_designImg > 125;
		distanceTransform(rt_designImg, distancemap, CV_DIST_L2, CV_DIST_MASK_PRECISE);
		for(int k = 0; k < edgelist.size(); k++)
		{
			for(int l = 0; l < edgesize[k]; l++)
			{
				distance = distancemap.at<float>(edgelist[k][l].x + x, edgelist[k][l].y + y);
				if(distance < disthresh)
				{
					matchedpos[i].pixels[k*edgelist.size() + l] = 1;
				}
				else matchedpos[i].pixels[k*edgelist.size() + l] = 0;
			}
		}
		
	}
	
	
	
	
	
	
	namedWindow( "Display window", CV_WINDOW_AUTOSIZE );
	
	
	imshow("Display window",patternImg);
	
	namedWindow( "Edges window", CV_WINDOW_AUTOSIZE );
	//waitKey(0);
	
	cout << matchedpos.size() << endl;
	
	cout << "totalpixels: "<<totalpixels<<endl;
	
	int pixeldiff = 0;
	
	pixeldiff = matchedpos[0].pixeldiff;
	for(int k = 1; k < matchedpos.size(); k++)
	{
		if (matchedpos[k].pixeldiff > pixeldiff)
		{
			pixeldiff = matchedpos[k].pixeldiff;
		}
	}
	
	cout << "single score: 	"<< 100*pixeldiff/totalpixels<<endl;
	if(matchedpos.size() == 1)
	{
		edgelist.clear();
		matchedpos.clear();
		matchedresult.clear();
		return 1;
	}
	
	
	for(int i = 0; i < matchedpos.size(); i++)
	{
		
		x = matchedpos[i].x;
		y = matchedpos[i].y;
		
		scale = matchedpos[i].scale;
		angle = matchedpos[i].angle;
		
		
		for(int j = i+1; j < matchedpos.size(); j++)
		{
			pixeldiff = 0;
			for(int k = 0; k < totalpixels; k++)
			{
				if (matchedpos[i].pixels[k] != matchedpos[j].pixels[k])
				{
					pixeldiff++;
				}
			}
		
			matchedresult.push_back(MatchResult(i,j,pixeldiff));
				
		}
		
	
		
	}
	std::sort(matchedresult.begin(), matchedresult.end(), MatchResult::before);
	
    cout << "Sorted matches "<<endl;
	for (int i = 0; i < matchedresult.size(); i++)
	{
		//cout << "matche "<<i<<": i: "<<matchedresult[i].match1<<" j: "<<matchedresult[i].match2<<" diff: "<<matchedresult[i].pixeldiff<<endl;
	}
    cout << "The best match is "<<matchedresult[matchedresult.size()-1].match1 << "  and "<<matchedresult[matchedresult.size()-1].match2<<" diff: "<<matchedresult[matchedresult.size()-1].pixeldiff<<endl;
	cout << "Score: 	"<<100*matchedresult[matchedresult.size()-1].pixeldiff/totalpixels<<endl;
	
	
	Mat plotImg;
	for(int i = 0; i < matchedpos.size(); i++)
	//for(int i=65; i<118; i = i+52)
	//for(int i = matchedresult[matchedresult.size()-1].match1; i <= matchedresult[matchedresult.size()-1].match2; i = i + matchedresult[matchedresult.size()-1].match2 - matchedresult[matchedresult.size()-1].match1 )
	{
		
		x = matchedpos[i].x;
		y = matchedpos[i].y;
		//cout <<x<<","<<y<<endl;
		scale = matchedpos[i].scale;
		angle = matchedpos[i].angle;
		
		cout << "--------"<<endl<<"Angle: " <<angle <<endl<<"Scale: "<<scale<< endl;
		//plotImg = rotate(distancemap, angle, scale);
		
		plotImg = rotate(designImg, angle, scale);
		
		//imshow("Display window", plotImg);
		
	
	
		cout <<"Edges: ";
		
		int pixels_ind = 0;
		for(int j = 0; j < edgelist.size(); j++)
		
		{
			for(int l = 0; l < edgelist[j].size(); l++)
			{
				Point2i temp = edgelist[j][l];
				int newX = temp.y + y;
				int newY = temp.x + x;
				
				temp.x = newX;
				
				temp.y =  newY;
				
				//if (matchedpos[i].pixels[pixels_ind] == 1)
					circle(plotImg,temp,1,Scalar( 0, 0, 255 ),-1);
				//else circle(plotImg,temp,1,Scalar( 0, 255, 0 ),-1);
				pixels_ind++;
			}
			
		}
		
		cout << endl ;
		cout <<"Shifting- x: "<<x<<" y: "<<y<<endl;
	    //imshow( "Edges window", plotEdges ); 
	    imshow("Edges window", plotImg);
	  
		waitKey(0);
		//waitKey(200);
	
	}
	
    
	
    //waitKey(0);
    
    edgelist.clear();
    matchedpos.clear();
    matchedresult.clear();
    
   
  
	return 0;

}
