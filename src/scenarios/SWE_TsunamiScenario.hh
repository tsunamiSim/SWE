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
    int *initX, *initY, *dispX, *dispY;
    float **initZ, **dispZ;      
    size_t init_ylen, init_xlen, dis_ylen, dis_xlen;
    
    void ba(int lookUpX, int lookUpY, int index, float &buff,int &squareDis){
                /*float tmp = (lookUpX - initX[index]) * (lookUpX - initX[index]) + (lookUpY - initY[index]) * (lookUpY - initY[index]);
                if(squareDis < 0 || squareDis > tmp){
                    buff = initZ[index]; 
                    squareDis = tmp;
                }*/
    };

    void readBathymetry(){
        int retval, ncid, dim, countVar, gats, unldim, zid, yid, xid;
        char dimZ = 'z', dimY = 'y', dimX = 'x'; 

        if(retval = nc_open("NetCDF_Input/initBathymetry.nc", NC_NOWRITE, &ncid)) ERR(retval);
	
        if(retval = nc_inq(ncid, &dim, &countVar, &gats, &unldim)) ERR(retval);
        assert(dim == 2); 
        assert(countVar == 3); 
        if(retval = nc_inq_varid(ncid, &dimZ, &zid)) ERR(retval);
        if(retval = nc_inq_dimid(ncid, &dimY, &yid)) ERR(retval);
        if(retval = nc_inq_dimid(ncid, &dimX, &xid)) ERR(retval); 

        if(retval = nc_inq_dimlen(ncid, yid, &init_ylen)) ERR(retval);
        if(retval = nc_inq_dimlen(ncid, xid, &init_xlen)) ERR(retval);
	
        initZ = new float*[init_xlen];
        for(unsigned int i = 0; i < init_xlen; i++)
            initZ[i] = new float[init_ylen];

        initY = new int[init_ylen];
        initX = new int[init_xlen];
	
        if(retval = nc_get_var_float(ncid, zid, &initZ[0][0])) ERR(retval);
        if(retval = nc_get_var_int(ncid, yid, &initY[0])) ERR(retval);
        if(retval = nc_get_var_int(ncid, xid, &initX[0])) ERR(retval);
	   
        if(retval = nc_close(ncid));  
    };

    void readDisplacement(){
        
    };

  public:

    SWE_TsunamiScenario() : SWE_Scenario(){
        readBathymetry();
        readDisplacement();

        cout << "check" << " " << init_ylen << " " << init_xlen << " " << initZ[0][0] << " "<< initY[0] << " "<< initX[0] << " \n";
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
