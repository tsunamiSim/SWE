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

public:

  SWE_TsunamiScenario() : SWE_Scenario(){
	tools::Logger::logger.printLine();
	netCdfReader::readNcFile("NetCDF_Input/initBathymetry.nc", &bathymetry, &bathY, &bathX);
	tools::Logger::logger.printString("Succesfully read bathymetry");
	netCdfReader::readNcFile("NetCDF_Input/displacement.nc", &displacement, &disY, &disX);   
	tools::Logger::logger.printString("Succesfully read displacement");
	disTop = Array::max(disY, displacement->getRows());
	disBot = Array::min(disY, displacement->getRows());
	disLeft = Array::min(disX, displacement->getCols());
	disRight = Array::max(disY, displacement->getCols());
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

  virtual float endSimulation() { return (float) 30; };

  virtual BoundaryType getBoundaryType(BoundaryEdge edge) { return OUTFLOW; };

  /** Get the boundary positions
   *
   * @param i_edge which edge
   * @return value in the corresponding dimension
   */
  float getBoundaryPos(BoundaryEdge i_edge) {
	if ( i_edge == BND_LEFT )
		return Array::min(bathX, bathymetry->getCols());
	else if ( i_edge == BND_RIGHT)
		return Array::max(bathX, bathymetry->getCols());
	else if ( i_edge == BND_BOTTOM )
		return Array::min(bathY, bathymetry->getRows());
	else
		return Array::max(bathY, bathymetry->getRows());
  };
	

};

#endif
