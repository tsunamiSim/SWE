/**
 * @file
 * This file is part of SWE.
 *
 * @author Ludwig Peuckert, Alexander Binsmaier  
 *
 * @section LICENSE
 *
 * SWE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SWE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SWE.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 * loads NetCDF-Defined Scenario
 */
#ifndef __SWE_TSUNAMI_SCENARIO_H
#define __SWE_TSUNAMI_SCENARIO_H

#include <cmath>

#include "tools/help.hh"
#include "tools/Logger.hh"
#include "SWE_Scenario.hh"

/**
 * Scenario "Artificial Tsunami Scenario":
 * displacement in the middle of the domain  
 */
class SWE_TsunamiScenario : public SWE_Scenario {

public:

  Float2D *bathymetry, *displacement;
  float *bathX, *bathY, *disX, *disY;
  float disTop, disBot, disLeft, disRight;
  float boundLeft, boundRight, boundTop, boundBot;
	
/**
 * Opens a nc-file and reads the content. File must contain the dimension values for x on index 0, y on 1 and the bathymetry values as z on index 2
 *
 * @param fileDir: The path to the file (relative or absolute)
 * @param buffZ: The buffer for the bathymetry values
 * @param buffY: The buffer for the Y-Values
 * @param buffX: The buffer for the X-Values
 */
void readNcFile(const char* fileDir, Float2D** buffZ, float** buffY, float** buffX){
		int retval, ncid, dim, countVar, 
		zid = 2, yid = 1, xid = 0;
		float *initZ,*initX, *initY;      
		size_t init_ylen, init_xlen;

		if(retval = nc_open(fileDir, NC_NOWRITE, &ncid)) ERR(retval);

		if(retval = nc_inq(ncid, &dim, &countVar, NULL, NULL)) ERR(retval);
		assert(dim == 2); 
		assert(countVar == 3); 

#ifndef NDEBUG
		char dimZ, dimY, dimX;
		//nc_type nc;
		if(retval = nc_inq_dim(ncid, yid, &dimY, &init_ylen)) ERR(retval);
		if(retval = nc_inq_dim(ncid, xid, &dimX, &init_xlen)) ERR(retval);
		if(retval = nc_inq_var(ncid, zid, &dimZ, NULL, NULL, NULL, NULL)) ERR(retval);
		
		string text = "Name DimZ: ";
		text = text + dimZ;
		tools::Logger::logger.printString(text);
		text = "Name | Length DimY: ";
		text = text + dimY;
		text = text + " | ";
		text = text + static_cast<ostringstream*>( &(ostringstream() << init_ylen) )->str();
		tools::Logger::logger.printString(text);
		text = "Name | Length DimX: ";
		text = text + dimX;
		text = text + " | ";
		text = text + static_cast<ostringstream*>( &(ostringstream() << init_xlen) )->str();
		tools::Logger::logger.printString(text);
#else 
		if(retval = nc_inq_dim(ncid, yid, NULL, &init_ylen)) ERR(retval);
		if(retval = nc_inq_dim(ncid, xid, NULL, &init_xlen)) ERR(retval);
#endif
		initZ = new float[init_ylen * init_xlen];
		initY = new float[init_ylen];
		initX = new float[init_xlen];
		memset(initX, 0, init_xlen);
        
		if(retval = nc_get_var_float(ncid, zid, initZ)) ERR(retval);
		if(retval = nc_get_var_float(ncid, yid, initY)) ERR(retval);
		if(retval = nc_get_var_float(ncid, xid, initX)) ERR(retval);
        
		if(retval = nc_close(ncid)) ERR(retval);
        
		*buffZ = new Float2D(init_ylen, init_xlen, initZ);
		*buffY = initY;
		*buffX = initX;

#ifndef NDEBUG
		for(int i = 0; i < bathymetry->getRows(); i++){
		   assert(initX[i] == (*buffX)[i]);
		}
		for(int i = 0; i < bathymetry->getCols(); i++){
		   assert(initY[i] == (*buffY)[i]);
		}
		int rows = (*buffZ)->getRows(), cols = (*buffZ)->getCols();
		float a, b;
		for(int i = 0; i < rows; i++){
		   for(int j = 0; j < cols; j++){
			a = initZ[j * rows + i];
			b = (**buffZ)[j][i];
		    	assert(a == b);
		    }
		}
		
		tools::Logger::logger.printString("File read");
#endif
  };


public:
  SWE_TsunamiScenario(int cellsX, int cellsY, float minX, float maxX, float minY, float maxY, const char *bathymetryFile = "NetCDF_Input/initBathymetry.nc", const char *displacementFile = "NetCDF_Input/displacement.nc") : SWE_Scenario(cellsX, cellsY, OUTFLOW) {
	  tools::Logger::logger.printLine();
	  readNcFile(bathymetryFile, &bathymetry, &bathY, &bathX);
	  tools::Logger::logger.printString("Succesfully read bathymetry");
	  readNcFile(displacementFile, &displacement, &disY, &disX);   
	  tools::Logger::logger.printString("Succesfully read displacement");

    clip(minX, maxX, minY, maxY);

    applyBoundaries();
  }

  /**
   * Creates a new instance of the TsunamiScenario Class
   * @param cellsX Cells in x dimension
   * @param cellsY Cells in y dimension
   */
  SWE_TsunamiScenario(int cellsX, int cellsY, const char *bathymetryFile = "NetCDF_Input/initBathymetry.nc", const char *displacementFile = "NetCDF_Input/displacement.nc") : SWE_Scenario(cellsX, cellsY, OUTFLOW) {
	  tools::Logger::logger.printLine();
	  readNcFile(bathymetryFile, &bathymetry, &bathY, &bathX);
	  tools::Logger::logger.printString(toString("Succesfully read bathymetry. Rows: ") + toString(bathymetry->getRows()) + toString(", Cols: ") + toString(bathymetry->getCols()));
	  readNcFile(displacementFile, &displacement, &disY, &disX);   
	  tools::Logger::logger.printString("Succesfully read displacement. Rows: " + toString(displacement->getRows()) + toString(", Cols: ") + toString(displacement->getCols()));

	  applyBoundaries();
  };
  /** Dummy constructor for cxx tests only */
  SWE_TsunamiScenario() : SWE_Scenario(0, 0, OUTFLOW) { }


  /**
   * The bathymetry at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getBathymetry(float x, float y) {
	return getBathymetry(x, y, false);
  }

  /**
   * The bathymetry at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   * @param IgnoreDisplacement True if you just want to get the bathymetry value
   */
  float getBathymetry(float x, float y, bool IgnoreDisplacement) {
//	IgnoreDisplacement = true; // Uncomment that line to ignore displacement
	  int bestXBath, bestYBath;
	  float result;
	  Array::lookUp(y, bathymetry->getCols(), bathY, &bestYBath);
	  Array::lookUp(x, bathymetry->getRows(), bathX, &bestXBath);
	  if(x < disLeft || x > disRight || y < disBot || y > disTop || IgnoreDisplacement){
		  result = (*bathymetry)[bestYBath][bestXBath];
	  }
	  else{
		  int bestXDis, bestYDis;
		  Array::lookUp(y, displacement->getCols(), disY, &bestYDis);
		  Array::lookUp(x, displacement->getRows(), disX, &bestXDis);

		  result = (*bathymetry)[bestYBath][bestXBath] + (*displacement)[bestYDis][bestXDis];

//		std::printf("Given: (%i, %i) => Disp(%i, %i), Bath(%i, %i)\n", (int)x / 1000, (int)y / 1000, (int)disX[bestXDis] / 1000, (int)disY[bestYDis] / 1000, (int)bathX[bestXBath] / 1000, (int)bathY[bestYBath] / 1000);
		//result = (*displacement)[bestYDis][bestXDis];
	}
    if(result < 0.f && result > -20.f)
  	  return -20.f;
    else if(result > 0.f && result < 20.f)
  	  return 20.f;
    else
      return result;
  };

  void setBathymetry(float value) {
	int xMax = bathymetry->getCols(), yMax = bathymetry->getRows();
	#pragma omp parallel for
	for(int x = 0; x < xMax; x++) for(int y = 0; y < yMax; y++)
		(*bathymetry)[x][y] = value;
  };

  /**
   * The water height at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getWaterHeight(float x, float y) {
	return max(-getBathymetry(x, y, true), 0.f);
  };

  /**
   * Returns the time of the end of the simulation
   */
  virtual float endSimulation() { return 100.f; };

  /** Get the boundary positions
   *
   * @param i_edge which edge
   * @return value in the corresponding dimension
   */
  float getBoundaryPos(BoundaryEdge i_edge) {
	if ( i_edge == BND_LEFT ) {
		return boundLeft;
	}
	else if ( i_edge == BND_RIGHT) {
		return boundRight;
	}
	else if ( i_edge == BND_BOTTOM ) {
		return boundBot;
	}
	else {
		return boundTop;
	}
  };

//private:

  void applyBoundaries(){
  	tools::Logger::logger.printString("Setting displacement boundaries");
	  disTop = Array::max(disY, displacement->getCols());
	  disBot = Array::min(disY, displacement->getCols());
	  disLeft = Array::min(disX, displacement->getRows());
	  disRight = Array::max(disX, displacement->getRows());
	  std::string comma = ", ";
	  tools::Logger::logger.printString(toString("Displacement boundaries set to: left, right, bottom, top (divided by 1000): ") + toString(disLeft/1000) + comma + toString(disRight/1000) + comma + toString(disBot/1000) + comma + toString(disTop/1000));
  	tools::Logger::logger.printString("Setting bathymetry boundaries");
	  boundTop = Array::max(bathY, bathymetry->getCols());
	  boundBot = Array::min(bathY, bathymetry->getCols());
	  boundLeft = Array::min(bathX, bathymetry->getRows());
	  boundRight = Array::max(bathX, bathymetry->getRows());
	  tools::Logger::logger.printString(toString("Bathymetry boundaries set to: left, right, bottom, top (divided by 1000): ") + toString(boundLeft/1000) + comma + toString(boundRight/1000) + comma + toString(boundBot/1000) + comma + toString(boundTop/1000));
	  tools::Logger::logger.printLine();
  }
	
  void clip(float minX, float maxX, float minY, float maxY) {
    int minIndex_disp_x = 0, maxIndex_disp_x = 0,
      minIndex_disp_y = 0, maxIndex_disp_y = 0,
      minIndex_bath_x = 0, maxIndex_bath_x = 0,
      minIndex_bath_y = 0, maxIndex_bath_y = 0,
      cols = 0, rows = 0;

    // Displacement
    clip(minX, maxX, &disX, displacement->getRows(), &minIndex_disp_x, &maxIndex_disp_x);
//    tools::Logger::logger.printString("Displacement x values clipped: minIndex: " + toString(minIndex_disp_x) + " maxIndex: " + toString(maxIndex_disp_x));
//    Array::print(disX, maxIndex_disp_x - minIndex_disp_x + 1, "Displacement X values");
    clip(minY, maxY, &disY, displacement->getCols(), &minIndex_disp_y, &maxIndex_disp_y);
//    tools::Logger::logger.printString("Displacement y values clipped: minIndex: " + toString(minIndex_disp_y) + " maxIndex: " + toString(maxIndex_disp_y));
//    Array::print(disY, maxIndex_disp_y - minIndex_disp_y + 1, "Displacement Y values");

//    cout << "before" << endl;
//    cout << displacement->ToString() << endl;

    Float2D *newDisp = new Float2D(maxIndex_disp_y - minIndex_disp_y + 1, maxIndex_disp_x - minIndex_disp_x + 1);
//    cout << "New rows: " << newDisp->getRows() << ", new cols: " << newDisp->getCols() << endl;
    cols = newDisp->getCols();
    rows = newDisp->getRows();
    for(int col = 0; col < cols; col++) for(int row = 0; row < rows; row++)
      (*newDisp)[col][row] = (*displacement)[col + minIndex_disp_y][row + minIndex_disp_x];
    delete displacement;
    displacement = newDisp;
//    cout << "after" << endl;
//    cout << displacement->ToString() << endl;

    // Bathymetry
    clip(minX, maxX, &bathX, bathymetry->getRows(), &minIndex_bath_x, &maxIndex_bath_x);
    clip(minY, maxY, &bathY, bathymetry->getCols(), &minIndex_bath_y, &maxIndex_bath_y);
    Float2D *newBath = new Float2D(maxIndex_bath_y - minIndex_bath_y + 1, maxIndex_bath_x, minIndex_bath_x + 1);
    cols = newBath->getCols();
    rows = newBath->getRows();
    for(int col = 0; col < cols; col++) for(int row = 0; row < rows; row++)
      (*newBath)[col][row] = (*bathymetry)[col + minIndex_bath_y][row + minIndex_bath_x];
    delete bathymetry;
    bathymetry = newBath;
  }

  void clip(float min, float max, float** array, int length, int* minIndex, int* maxIndex) {
    Array::lookUp(min, length, *array, minIndex);
    Array::lookUp(max, length, *array, maxIndex);
    int mini = *minIndex, maxi = *maxIndex;
    Array::cut(&*array, mini, maxi - mini + 1);
  }
};

#endif
