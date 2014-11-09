/**
 * @file
 * This file is part of SWE.
 *
 * @author Michael Bader, Kaveh Rahnema, Tobias Schnabel
 * @author Sebastian Rettenberger (rettenbs AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Sebastian_Rettenberger,_M.Sc.)
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
 * TODO
 */

#ifndef __SWE_DIMSPLIT_HPP
#define __SWE_DIMSPLIT_HPP

#include "blocks/SWE_Block.hh"
#include "solvers/FWave.hpp"
#include "tools/help.hh"
#include <iostream>

#ifndef NDEBUG
	//#define NDEBUG
#endif

/**
* This class implements SWE_Block using the dimensional splitting approach and the F-Wave solver
*/
class SWE_DimensionalSplitting: public SWE_Block {

private : 
	solver::FWave<float> solver;
	
	//! Left net updates for height
	Float2D hNetUpdatesLeft;
	//! Right net updates for height
	Float2D hNetUpdatesRight;
	//! Left net updates for momentum
	Float2D huNetUpdatesLeft;
	//! Right net updates for momentum
	Float2D huNetUpdatesRight;
	//! Net updates above for height
	Float2D hNetUpdatesBelow;
	//! Net updates below for height
	Float2D hNetUpdatesAbove;
	//! Net updates for momentum below
	Float2D hvNetUpdatesBelow;
	//! Net updates for momentum above
	Float2D hvNetUpdatesAbove;

	/**
	* Sets all entries of an array to zero
	* @param array: the array itself
	* @param xSize: the array's size in the first dimension
	* @param ySize: the array's size in the second dimension
	*/
	void setZero(Float2D& array, int xSize, int ySize)
	{
		for(int i = 0; i < xSize-1; i++){
			for(int j = 0; j < ySize-1; j++){
				array[i][j] = 0;
			}		
		}
	}
public :
	/**
	* Constructor
	*
	* @param l_nx: x dimension of the domain
	* @param l_ny: y dimension of the domain
	* @param l_dx: cell size in x dimension
	* @param l_dy: cell size in y dimension
	*/
	SWE_DimensionalSplitting(int l_nx, int l_ny, float l_dx, float l_dy) :
		SWE_Block(l_nx, l_ny, l_dx, l_dy),
		hNetUpdatesLeft (l_nx + 1, l_ny),
		hNetUpdatesRight (l_nx + 1, l_ny),
		huNetUpdatesLeft (l_nx + 1, l_ny),
		huNetUpdatesRight (l_nx + 1, l_ny),

		hNetUpdatesBelow (l_nx, l_ny + 1),
		hNetUpdatesAbove (l_nx, l_ny + 1),
		hvNetUpdatesBelow (l_nx, l_ny + 1),
		hvNetUpdatesAbove (l_nx, l_ny + 1)
{
}
	
	/**
	* computing the net updates AND applying them already
	*/
	void computeNumericalFluxes()
	{
		for(unsigned int y = 0; y < ny; y++)
		{	
			for(unsigned int x = 0; x < nx+1; x++) 
			{
				float maxEdgeSpeed;
				solver.computeNetUpdates(h[x][y+1], h[x+1][y+1], hu[x][y+1], hu[x+1][y+1], b[x][y+1], b[x+1][y+1],
								hNetUpdatesLeft[x][y], hNetUpdatesRight[x][y],
								huNetUpdatesLeft[x][y], huNetUpdatesRight[x][y],
								maxEdgeSpeed
							);
				maxTimestep = std::max(maxEdgeSpeed, maxTimestep);

			}
		}

		setZero(hNetUpdatesBelow, nx, ny + 1);
		setZero(hNetUpdatesAbove, nx, ny + 1);
		setZero(hvNetUpdatesAbove, nx, ny + 1);
		setZero(hvNetUpdatesBelow, nx, ny + 1);
		
		maxTimestep = 0.4 * dx / maxTimestep;
		updateUnknowns(maxTimestep);
	
#ifndef NDEBUG
			maxTimestepY = 0.f;
#endif //NDEBUG

		for(unsigned int y = 0; y < ny+1; y++)
		{	
			for(unsigned int x = 0; x < nx; x++) 
			{
				float maxEdgeSpeed;
				solver.computeNetUpdates(h[x+1][y], h[x+1][y+1], hv[x+1][y], hv[x+1][y+1], b[x+1][y], b[x+1][y+1],
								hNetUpdatesBelow[x][y], hNetUpdatesAbove[x][y],
								hvNetUpdatesBelow[x][y], hvNetUpdatesAbove[x][y],
								maxEdgeSpeed
							);
//TODO DEBUG
#ifndef NDEBUG
					maxTimestepY = std::max(maxEdgeSpeed, maxTimestepY);
#endif //NDEBUG
						
			}
		}

#ifndef NDEBUG
			if(maxTimestep >= 0.5f * dy / maxTimestepY)
				std::cerr << "Used timestep was to big! Used/In X-DIR computed: " << maxTimestep << "; In Y-DIR computed: " << (0.5f * dy / maxTimestepY) << std::endl;			
#endif //NDEBUG

		setZero(hNetUpdatesLeft, nx + 1, ny);
		setZero(hNetUpdatesRight, nx + 1, ny);
		setZero(huNetUpdatesLeft, nx + 1, ny);
		setZero(huNetUpdatesRight, nx + 1, ny);

		updateUnknowns(maxTimestep);
			
	}

	/**
	* Applying the net updates (CARE: ComputeNumericalFluxes already calls this, don't use it again)
	*/
	void updateUnknowns(float dt)
	{
		for(unsigned int y = 1; y < ny+1; y++)
		{
			for(unsigned int x = 1; x < nx+1; x++)
			{
				h[x][y] -= dt / dx * (hNetUpdatesRight[x - 1][y - 1] + hNetUpdatesLeft[x][y - 1]) + 
						dt / dy * (hNetUpdatesAbove[x - 1][y - 1] + hNetUpdatesBelow[x - 1][y]);
				hu[x][y] -= dt / dx * (huNetUpdatesRight[x - 1][y - 1] + huNetUpdatesLeft[x][y - 1]);
				hv[x][y] -= dt / dy * (hvNetUpdatesAbove[x - 1][y - 1] + hvNetUpdatesBelow[x - 1][y]);
			}
		}
	}
};
#endif
