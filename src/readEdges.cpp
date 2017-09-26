#include "readEdges.h"

using namespace cv;
using namespace std;

int readEdges(vector< vector<Point2i> > &edges, string filename)
{
	 
	ifstream file (filename);
	
	if(!file.is_open())
    {
		cout<<"No such file or file broken! "<<filename<<endl;
		return -1;
    }

	vector<string> lines;
	string line;

	while (getline(file, line))
    {
		lines.push_back(line);
    }
	file.close();

	
	vector<Point2i> edge;
		
	
	for(int i=1; i<lines.size(); i++)
    {
		if(lines[i]=="Edge" && i<lines.size())
		{
			edges.push_back(edge);
			edge.clear();
			//cout << endl;
			i++;
		}
		
		
		string::size_type pos;
		
		Point2i temp;
		
		//cout << lines[i] <<":";
		
		temp.x = stoi(lines[i], &pos) - 1; //consider opencv handling edges generated by matlab. matlab, matrix starts at 1,1, opencv, matrix starts at 0,0
		temp.y = stoi(lines[i].substr(pos+1), &pos) -1 ;
		
		//cout <<temp.x<<","<<temp.y<<":";
 	
		//circle(designImg,temp,1,Scalar( 0, 0, 255 ),-1,8);
		
		edge.push_back(temp);
		
			
		if(i==lines.size()-1)
			edges.push_back(edge);
			
    }
    
    //cout << "Number of edges: "<<edges.size()<<endl;
    lines.clear();
	return edges.size();
}
	
	
	
	
	
