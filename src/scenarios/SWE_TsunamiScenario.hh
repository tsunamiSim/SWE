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
  private: 
  Float2D *bathymetry, *displacement;
  int *bathX, *bathY, *disX, *disY;
  float disTop, disBot, disLeft, disRight;
	float boundLeft, boundRight, boundTop, boundBot;

  void lookUp(int searchFor, int max, int* searchIn, int* best){
		int disBest = abs(searchFor-searchIn[0]);
		*best = 0;		
		for(int i = 1; i < max; i++){
			int dis = abs(searchFor-searchIn[i]);
			//cout << best << " " ;
			if(disBest > dis) {
				disBest = dis;
				*best = i;
			}				 
		} 
	};
	
/**
 * Opens a nc-file and reads the content. File must contain the dimension values for x on index 0, y on 1 and the bathymetry values as z on index 2
 *
 * @param fileDir: The path to the file (relative or absolute)
 * @param buffZ: The buffer for the bathymetry values
 * @param buffY: The buffer for the Y-Values
 * @param buffX: The buffer for the X-Values
 */
void readNcFile(const char* fileDir, Float2D** buffZ, int** buffY, int** buffX){
		int retval, ncid, dim, countVar, 
		zid = 2, yid = 1, xid = 0, *initX, *initY;
		float *initZ;      
		size_t init_ylen, init_xlen;

		if(retval = nc_open(fileDir, NC_NOWRITE, &ncid)) ERR(retval);

		if(retval = nc_inq(ncid, &dim, &countVar, NULL, NULL)) ERR(retval);
		assert(dim == 2); 
		assert(countVar == 3); 

#ifndef NDBUG
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
		initY = new int[init_ylen];
		initX = new int[init_xlen];
		memset(initX, 0, init_xlen);

		if(retval = nc_get_var_float(ncid, zid, initZ)) ERR(retval);
	    if(retval = nc_get_var_int(ncid, yid, initY)) ERR(retval);
		if(retval = nc_get_var_int(ncid, xid, initX)) ERR(retval);

	    if(retval = nc_close(ncid));

		*buffZ = new Float2D(init_ylen, init_xlen, initZ);
		*buffY = initY;
		*buffX = initX;

		assert(buffZ[10][15] == initZ[10][15]);
		
#ifndef NDBUG
		tools::Logger::logger.printString("File read");
#endif
  };


public:

  SWE_TsunamiScenario(int cellsX, int cellsY) : SWE_Scenario(cellsX, cellsY) {
	tools::Logger::logger.printLine();
	readNcFile("NetCDF_Input/initBathymetry.nc", &bathymetry, &bathY, &bathX);
	tools::Logger::logger.printString(toString("Succesfully read bathymetry. Rows: ") + toString(bathymetry->getRows()) + toString(", Cols: ") + toString(bathymetry->getCols()));
	readNcFile("NetCDF_Input/displacement.nc", &displacement, &disY, &disX);   
	tools::Logger::logger.printString("Succesfully read displacement. Rows: " + toString(displacement->getRows()) + toString(", Cols: ") + toString(displacement->getCols()));
	tools::Logger::logger.printString("Setting displacement boundaries");
	disTop = Array::max(disY, displacement->getRows());
	disBot = Array::min(disY, displacement->getRows());
	disLeft = Array::min(disX, displacement->getCols());
	disRight = Array::max(disY, displacement->getCols());
	std::string comma = ", ";
	tools::Logger::logger.printString(toString("Displacement boundaries set to: top, bot, left, right ") + toString(disTop) + comma + toString(disBot) + comma + toString(disLeft) + comma + toString(disRight));
	boundTop = Array::max(bathY, bathymetry->getRows());
	boundBot = Array::min(bathY, bathymetry->getRows());
	boundLeft = Array::min(bathX, bathymetry->getCols());
	boundRight = Array::max(bathX, bathymetry->getCols());
	tools::Logger::logger.printString(toString("Bathymetry boundaries set to: top, bot, left, right ") + toString(boundTop) + comma + toString(boundBot) + comma + toString(boundLeft) + comma + toString(boundRight));
	tools::Logger::logger.printLine();
  };

  float getBathymetry(float x, float y) {
	int bestXBath, bestYBath;
	lookUp(y, bathymetry->getRows(), bathY, &bestYBath);
	lookUp(x, bathymetry->getCols(), bathX, &bestXBath);
	if(x < disLeft || x > disRight || y < disBot || y > disTop)
		return (*bathymetry)[bestYBath][bestXBath];
	
	int bestXDis, bestYDis;
	lookUp(y, displacement->getRows(), disX, &bestYDis);
	lookUp(x, displacement->getCols(), disY, &bestXDis);
	return (*bathymetry)[bestYBath][bestXBath] + (*displacement)[bestYDis][bestXDis];
  };

  float getWaterHeight(float x, float y) { 
	int bestX, bestY;
	lookUp(y, bathymetry->getRows(), bathY, &bestY);
	lookUp(x, bathymetry->getCols(), bathX, &bestX);
	if((*bathymetry)[bestY][bestX] < 0)
		return -(*bathymetry)[bestY][bestX];
	else
		return 0;
  };

  virtual float endSimulation() { return (float) 100; };

  virtual BoundaryType getBoundaryType(BoundaryEdge edge) { return OUTFLOW; };

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
	

};

#endif
