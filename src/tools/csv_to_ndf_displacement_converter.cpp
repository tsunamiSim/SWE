#ifndef EXCLUDE_SCENARIO 
#define EXCLUDE_SCENARIO

#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "../writer/NetCdfWriter.cpp"

const int rows = 507020, cols = 6, 
    x = 200, y = 200, 
    files = 80;
const float stepSize = 3.5f;
const std::string filename = "seis";
float epiNS = 38.15, epiEW = 142.855, 
    lengthLat = 111.319;
float **coords;
int **valueCount;
Float2D* b;

//converts degree to radians
float degToRad(float deg){
    return deg * M_PI / 180;
}

//converts string to float
float sToF(std::string s){
	std::stringstream str;
	float val;
	str << s;
	str >> val;
	return val;
}

//converts integer to string
std::string iToS(int i){
	std::stringstream str;
	std::string val	;
	str << i;
	str >> val;
	return val;
}

//absolute function for floats
float absolute(float in){
	return (in > 0.f ? in : -in);
}

//returns distance (in kilometer) between to points (given in longitude and latitude)
float disForDeg(float EW1, float NS1, float EW2, float NS2){
    float R = 6371;
    float lat1 = degToRad(NS1);
    float lat2 = degToRad(NS2);
    float latD = degToRad(NS2-NS1);
    float lonD = degToRad(EW2-EW1);
    
    float tmp = sin(latD/2) * sin(latD/2) + cos(lat1) * cos(lat2) * sin(lonD/2) * sin(lonD/2);
    float atan = 2* atan2(sqrt(tmp), sqrt(1-tmp));
    return (EW2 > EW1 ? 1 : -1)* R * atan;
}

//overloads disForDeg to call with epicenter
float disForDeg(float EW, float NS){
	return disForDeg(epiEW,epiNS,EW,NS);
}

//converts latitude and longitude to kilometer relative to the epicenter
void degToRelEpi(float& ew, float& ns){
	float nsMid = (ns - epiNS)/2 + epiNS;
	ns = (ns-epiNS)*lengthLat;
	ew = disForDeg(epiEW, nsMid, ew, nsMid);
}

//read all lines of the csv file and stores the first secound and sixth element in coords
void getCoordsRelToEpi(std::string num){
	std::ifstream csv("sea_floor_disp_tohoku/SNAP_dis_floor_"+ num + ".csv");
	std::string ew,ns,b;
	
	//iterate over lines
	int r = 0;
	while(csv.good() && r < rows)
	{	
		std::getline(csv,ew,',');
		std::getline(csv,ns,',');
		std::getline(csv,b,',');
		std::getline(csv,b,',');
		std::getline(csv,b,',');
		std::getline(csv,b,'\n');
		coords[r][0]=sToF(ew);
		coords[r][1]=sToF(ns);
		coords[r][2]=sToF(b);
		degToRelEpi(coords[r][0],coords[r][1]);
		r++;	
	}
	csv.close();
}

//prints an array 
void print(int left, int rigth, int c, float** array, bool endl){
    std::cout << "-----------------------------------------------------" << std::endl;
    for(int i = left; i < rigth; i++){
        std::cout << array[i][c] << (endl ? "\n" : " ");
    }
    std::cout << (endl ? "-----------------------------------------------------" : "\n-----------------------------------------------------") << std::endl;
}

//returns the maximum of an array
float getMax(float** array, int left, int right, int depth = 0){
	float curr = array[left][depth];	
	for(int i = left+1; i < right; i++){
		if(curr < array[i][depth])
			curr = array[i][depth];
	}
	return curr;
}

//returns the minimum of an array
float getMin(float** array, int left, int right, int depth = 0){
	float curr = array[left][depth];	
	for(int i = left+1; i < right; i++){
		if(curr > array[i][depth])
			curr = array[i][depth];
	}
	return curr;
}

int main(){
    //init of coords, valueCount, b
	coords = new float*[rows];
    valueCount = new int*[x];
	for(int i = 0; i < rows; i++){
			coords[i] = new float[3];
	}
	for(int i = 0; i < x; i++){
	    valueCount[i] = new int[y];
	}
	b = new Float2D(x,y);
	
	//read first file (needed for following step)
	getCoordsRelToEpi(iToS(1));
	
	//calculate min/max and steps for x and y cooirdinates
	float minX = getMin(coords,0,rows), maxX = getMax(coords,0,rows), 
		minY = getMin(coords,0,rows,1),maxY = getMax(coords,0,rows,1),
		dX = (maxX-minX)/x,dY = (maxY-minY)/y;
	
	//init nc writer
	io:: NetCdfWriter writer(filename,*b,x,y,dX,dY,minX,minY);

    //iterate over all files
	for(int file = 1; file <= files; file++){
		std::cout << "-------------------------------------------\nreading file number " << file << std::endl;
		
		if(file > 1)
			getCoordsRelToEpi(iToS(file));
	    
	    //reset b and valueCount each iteration step
		for(int i = 0; i < x; i++){
			for(int j = 0; j < y; j++){
				(*b)[i][j] = 0;
			    valueCount[i][j] = 0;
			}
		}
		
		//iterate over all entries
		for(int i = 0; i < rows; i++){
		    //find matching grid
			int ns = (coords[i][1]-minY)/dY;
			int ew = (coords[i][0]-minX)/dX;
			if(ew >= x)
				ew = x-1;
			else if(ew < 0)
				ew = 0;
		    if(ns >= y)
				ns = y-1;
			else if(ns < 0)
				ns = 0;
			
			//add value
		    (*b)[ew][ns] += coords[i][2];
			valueCount[ns][ew]++;
   		}
   		
   		//divide all grid per valueCount to get the average
		for(int i = 0; i < x; i++){
			for(int j = 0; j < y; j++){
				if(valueCount[i][j] != 0){
	 		   	(*b)[i][j] = (*b)[i][j] / valueCount[i][j];
				}
			}
		}	
		
		//get values for 'empty' grids	
		//TODO ringe von innen nach außen
		for(int c = 0; c < 3; c++){
		for(int i = 1; i < x-1; i++){
			for(int j = 1; j < y-1; j++){
				if(valueCount[i][j] == 0){
	 		   	    (*b)[i][j] = ((*b)[i+1][j] + (*b)[i-1][j] + (*b)[i][j+1] + (*b)[i][j-1])/4;
				}
			}
		}
}
	std::cout << "writing to nc file" << std::endl;
	//write bathymetry to nc file
	writer.writeBathymetry(*b,stepSize*(file-1));
	std::cout << "Done!\n-------------------------------------------" << std::endl;
	}

    //delete 
	delete coords;
	delete b;
	delete valueCount;
  return 1;
}
#endif
