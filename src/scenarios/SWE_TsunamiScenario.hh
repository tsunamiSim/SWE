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
#include <netcdf.h>

#include "tools/help.hh"
#include "SWE_Scenario.hh"

#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); assert(false);}

/**
 * Scenario "Artificial Tsunami Scenario":
 * displacement in the middle of the domain  
 */
class SWE_TsunamiScenario : public SWE_Scenario {
  private: 
  Float2D *bathymetry, *displacement;
  int *bathX, *bathY, *disX, *disY;	    

  void lookUp(float x, float y){
                /*float tmp = (lookUpX - initX[index]) * (lookUpX - initX[index]) + (lookUpY - initY[index]) * (lookUpY - initY[index]);
                if(squareDis < 0 || squareDis > tmp){
                    buff = initZ[index]; 
                    squareDis = tmp;
                }*/
  };

	void readNcFile(const char* fileDir, Float2D* buffZ, int* buffY, int* buffX){
		int retval, ncid, dim, countVar, 
		zid = 2, yid = 1, xid = 0;
		float **initZ;      
		size_t init_ylen, init_xlen;
		   
		if(retval = nc_open(fileDir, NC_NOWRITE, &ncid)) ERR(retval);

		if(retval = nc_inq(ncid, &dim, &countVar, NULL, NULL)) ERR(retval);
		assert(dim == 2); 
		assert(countVar == 3); 

#ifndef NDBUG
		char dimZ, dimY, dimX;
		nc_type nc;
		if(retval = nc_inq_dim(ncid, yid, &dimY, &init_ylen)) ERR(retval);
		if(retval = nc_inq_dim(ncid, xid, &dimX, &init_xlen)) ERR(retval);
		if(retval = nc_inq_var(ncid, zid, &dimZ, NULL, NULL, NULL, NULL)) ERR(retval);
		
		cout << "name dimZ: " << dimZ << endl;
		cout << "name/length dimY: " << dimY << init_ylen << endl;
		cout << "name/length dimX: " << dimX << init_xlen << endl;
#else 
		if(retval = nc_inq_dim(ncid, yid, NULL, &init_ylen)) ERR(retval);
		if(retval = nc_inq_dim(ncid, xid, NULL, &init_xlen)) ERR(retval);
#endif

		initZ = new float*[init_xlen];
    for(unsigned int i = 0; i < init_xlen; i++)
        initZ[i] = new float[init_ylen];
		cout << "1" <<endl;
    buffY = new int[init_ylen];
    buffX = new int[init_xlen];

		cout << "1" <<endl;
    if(retval = nc_get_var_float(ncid, zid, &initZ[0][0])) ERR(retval);
		cout << "1" <<endl;
    if(retval = nc_get_var_int(ncid, yid, &buffY[0])) ERR(retval);
		cout << "1" <<endl; 
		if(retval = nc_get_var_int(ncid, xid, &buffX[0])) ERR(retval);

 
		cout << "1" <<endl;
    if(retval = nc_close(ncid));

		cout << "1" <<endl;
		buffZ = new Float2D(init_ylen, init_xlen, initZ);
		assert(buffZ[10][15] = initZ[10][15]);

  };

public:

  SWE_TsunamiScenario() : SWE_Scenario(){
      readNcFile("NetCDF_Input/initBathymetry.nc", bathymetry, bathY, bathX);
			cout << "succesfully read initBathymetry" << endl;
			readNcFile("NetCDF_Input/displacement.nc", displacement, disY, disX);   
			cout << "succesfully read isplacement" << endl;     
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
     if ( i_edge == BND_LEFT )
       return (float)-5000;
     else if ( i_edge == BND_RIGHT)
       return (float)5000;
     else if ( i_edge == BND_BOTTOM )
       return (float)-5000;
     else
       return (float)5000;
  };


};

#endif
