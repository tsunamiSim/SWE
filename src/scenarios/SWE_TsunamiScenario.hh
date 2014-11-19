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
#include "SWE_Scenario.hh"

/**
 * Scenario "Artificial Tsunami Scenario":
 * displacement in the middle of the domain  
 */
class SWE_TsunamiScenario : public SWE_Scenario {
  private: 
  Float2D *bathymetry, *displacement;
  int *bathX, *bathY, *disX, *disY;

public:

  SWE_TsunamiScenario() : SWE_Scenario(){

	netCdfReader::readNcFile("NetCDF_Input/initBathymetry.nc", bathymetry, bathY, bathX);
			cout << "succesfully read bathymetry" << endl;
	netCdfReader::readNcFile("NetCDF_Input/displacement.nc", displacement, disY, disX);   
			cout << "succesfully read displacement" << endl;     
  };

  float getBathymetry(float x, float y) {
    return 0;
  };

  float getWaterHeight(float x, float y) { 
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
	int min;
	if ( i_edge == BND_LEFT ) {
		return (float)-5000;
	}
	else if ( i_edge == BND_RIGHT)
		return (float)5000;
	else if ( i_edge == BND_BOTTOM )
		return (float)-5000;
	else
		return (float)5000;
  };
	

};

#endif
