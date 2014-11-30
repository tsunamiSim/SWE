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
class SWE_ArtificialTsunamiScenario : public SWE_Scenario {

public:

  /**
   * Creates a new instance of the TsunamiScenario Class
   * @param cellsX Cells in x dimension
   * @param cellsY Cells in y dimension
   */
  SWE_ArtificialTsunamiScenario(int cellsX, int cellsY) : SWE_Scenario(cellsX, cellsY, OUTFLOW) {
  };

  /**
   * The bathymetry at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getBathymetry(float x, float y) {
	if(x < -500 || x > 500 || y < -500 || y > 500)
		return -100;
	return  5.f * sin( (x / 500.f + 1) * M_PI) * (y * y / 250000 + 1) - 100;
  };

  /**
   * The water height at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getWaterHeight(float x, float y) {
	return 100;
  };

  /**
   * Returns the time of the end of the simulation
   */
  virtual float endSimulation() { return (float) 100; };

  /** Get the boundary positions
   *
   * @param i_edge which edge
   * @return value in the corresponding dimension
   */
  float getBoundaryPos(BoundaryEdge i_edge) {
	if ( i_edge == BND_LEFT ) {
		return -5000;
	}
	else if ( i_edge == BND_RIGHT) {
		return 5000;
	}
	else if ( i_edge == BND_BOTTOM ) {
		return -5000;
	}
	else {
		return 5000;
	}
  };
	

};

#endif
