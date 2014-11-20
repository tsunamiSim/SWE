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
#ifndef __SWE_CHECKPOINT_SCENARIO_H
#define __SWE_CHECKPOINT_SCENARIO_H

#include "SWE_Scenario.hh"

#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); assert(false);}

/**
 * Scenario "Checkpoint Scenario":
 * loads a checkkpoint of a simulation from a NC File    
 */
class SWE_CheckpointScenario : public SWE_Scenario {
  private: 
  Float2D *bathymetry, *hu, *hv, *h;
  float endOfSimulation, *initX, *initY, initBu, initBd, initBr, initBl;
  int  boundaryUp, boundaryDown, boundaryLeft, boundaryRight, boundary;
  
  void lookUp(float searchFor, int max, float* searchIn, int* best){
		float disBest = abs(searchFor-searchIn[0]);
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
  
  void readNcFile(){
		int retval, ncid, dim, countVar,
		    y_id,x_id,b_id,hu_id,hv_id, h_id, bound_id, bU_id, bD_id, bR_id, bL_id, eos_id;
		float *initB, *initHu, *initHv, *initH;      
		size_t init_ylen, init_xlen;

		if(retval = nc_open("checkpoints.nc", NC_NOWRITE, &ncid)) ERR(retval);

		if(retval = nc_inq(ncid, &dim, &countVar, NULL, NULL)) ERR(retval);
		assert(dim == 3); 
		assert(countVar == 12); 

        //TODO
		//char dimB, dimHu, dimHv, dimH, dimX, dimY;
		if(retval = nc_inq_dimid(ncid, "y" ,&y_id)) ERR(retval);
		if(retval = nc_inq_dimid(ncid, "x" ,&x_id)) ERR(retval);
		if(retval = nc_inq_varid(ncid, "b" ,&b_id)) ERR(retval);
		if(retval = nc_inq_varid(ncid, "hu" ,&hu_id)) ERR(retval);
		if(retval = nc_inq_varid(ncid, "hV" ,&hv_id)) ERR(retval);
		if(retval = nc_inq_varid(ncid, "h" ,&h_id)) ERR(retval);
		
		if(retval = nc_inq_varid(ncid, "h" ,&bound_id)) ERR(retval);
		if(retval = nc_inq_varid(ncid, "h" ,&bU_id)) ERR(retval);
		if(retval = nc_inq_varid(ncid, "h" ,&bD_id)) ERR(retval);
		if(retval = nc_inq_varid(ncid, "h" ,&bR_id)) ERR(retval);
		if(retval = nc_inq_varid(ncid, "h" ,&bL_id)) ERR(retval);
		
		if(retval = nc_inq_varid(ncid, "eos" ,&eos_id)) ERR(retval);

		if(retval = nc_inq_dimlen(ncid, y_id, &init_ylen)) ERR(retval);
		if(retval = nc_inq_dimlen(ncid, x_id, &init_xlen)) ERR(retval);
		
		initB = new float[init_ylen * init_xlen];
		initHu = new float[init_ylen * init_xlen];
		initHv = new float[init_ylen * init_xlen];
		initH = new float[init_ylen * init_xlen];
		initY = new float[init_ylen];
	    initX = new float[init_xlen];

	    if(retval = nc_get_var_float(ncid, b_id, initB)) ERR(retval);
	    if(retval = nc_get_var_float(ncid, hu_id, initHu)) ERR(retval);
	    if(retval = nc_get_var_float(ncid, hv_id, initHv)) ERR(retval);
	    if(retval = nc_get_var_float(ncid, h_id, initH)) ERR(retval);
	    if(retval = nc_get_var_float(ncid, y_id, initY)) ERR(retval);
		if(retval = nc_get_var_float(ncid, x_id, initX)) ERR(retval);
		
		if(retval = nc_get_var_int(ncid, bound_id, &boundary)) ERR(retval);
		if(retval = nc_get_var_float(ncid, bU_id, &initBu)) ERR(retval);
		if(retval = nc_get_var_float(ncid, bD_id, &initBd)) ERR(retval);
		if(retval = nc_get_var_float(ncid, bR_id, &initBr)) ERR(retval);
		if(retval = nc_get_var_float(ncid, bL_id, &initBl)) ERR(retval);
		
		if(retval = nc_get_var_float(ncid, eos_id, &endOfSimulation)) ERR(retval);

	    if(retval = nc_close(ncid));

		bathymetry = new Float2D(init_ylen, init_xlen, initB);
		hu = new Float2D(init_ylen, init_xlen, initHu);
		hv = new Float2D(init_ylen, init_xlen, initHv);
		h = new Float2D(init_ylen, init_xlen, initH);
		
		assert(bathymetry[10][15] == initZ[10][15]);
		
#ifndef NDBUG
		tools::Logger::logger.printString("File read");
#endif
  };

public:

  SWE_CheckpointScenario() : SWE_Scenario(){
	readNcFile();
  };

  float getBathymetry(float x, float y) {
	int bestX, bestY;
	lookUp(y, bathymetry->getRows(), initY, &bestY);
	lookUp(x, bathymetry->getCols(), initX, &bestX);
	return (*bathymetry)[bestY][bestX];
  };

  float getWaterHeight(float x, float y) { 
	int bestX, bestY;
	lookUp(y, h->getRows(), initY, &bestY);
	lookUp(x, h->getCols(), initX, &bestX);
	return (*h)[bestY][bestX];
  };
  
  float getHu(float x, float y){
    int bestX, bestY;
	lookUp(y, hu->getRows(), initY, &bestY);
	lookUp(x, hu->getCols(), initX, &bestX);
	return (*hu)[bestY][bestX];
  };
  
  float getHv(float x, float y){
    int bestX, bestY;
	lookUp(y, hv->getRows(), initY, &bestY);
	lookUp(x, hv->getCols(), initX, &bestX);
	return (*hv)[bestY][bestX];
  };  
  
  void getBoundaries(float* u, float* d, float* r, float* l){
    *u= initBu;
    *d= initBd;
    *r= initBr;
    *l= initBl;
  };

  virtual float endSimulation() { return (float) endOfSimulation; };

  virtual BoundaryType getBoundaryType(BoundaryEdge edge) { return boundary ? WALL : OUTFLOW; };

  /** Get the boundary positions
   *
   * @param i_edge which edge
   * @return value in the corresponding dimension
   */
  float getBoundaryPos(BoundaryEdge i_edge) {
	if ( i_edge == BND_LEFT )
		return Array::min(initX, bathymetry->getCols());
	else if ( i_edge == BND_RIGHT)
		return Array::max(initX, bathymetry->getCols());
	else if ( i_edge == BND_BOTTOM )
		return Array::min(initY, bathymetry->getRows());
	else
		return Array::max(initY, bathymetry->getRows());
  };
	

};

#endif
