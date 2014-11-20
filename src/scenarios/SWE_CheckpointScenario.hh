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
#define ERRM(e, msg) {printf("Error: %s\n%s\n", nc_strerror(e), msg); assert(false);}

/**
 * Scenario "Checkpoint Scenario":
 * loads a checkkpoint of a simulation from a NC File    
 */
class SWE_CheckpointScenario : public SWE_Scenario {
  private: 
  Float2D *bathymetry, *hu, *hv, *h;
  float endOfSimulation, *initX, *initY, initBu, initBd, initBr, initBl, startingTime;
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
		    y_id,x_id,b_id,hu_id,hv_id, h_id, bound_id, bU_id, bD_id, bR_id, bL_id, eos_id, time_id;
		float *initB, *initHu, *initHv, *initH;      
		size_t init_ylen, init_xlen, time;

		if(retval = nc_open("SWE_checkpoints.nc", NC_NOWRITE, &ncid)) ERR(retval);

		if(retval = nc_inq(ncid, &dim, &countVar, NULL, NULL)) ERR(retval);
		assert(dim == 3); 
		assert(countVar == 12); 

        //TODO
		//char dimB, dimHu, dimHv, dimH, dimX, dimY;
		if(retval = nc_inq_dimid(ncid, "y" ,&y_id)) ERRM(retval, "get y id");
		if(retval = nc_inq_dimid(ncid, "x" ,&x_id)) ERRM(retval, "get x id");
		if(retval = nc_inq_varid(ncid, "b" ,&b_id)) ERRM(retval, "get b id");
		if(retval = nc_inq_varid(ncid, "hu" ,&hu_id)) ERRM(retval, "get hu id");
		if(retval = nc_inq_varid(ncid, "hv" ,&hv_id)) ERRM(retval, "get hv id");
		if(retval = nc_inq_varid(ncid, "h" ,&h_id)) ERRM(retval, "get h id");
		
		if(retval = nc_inq_varid(ncid, "bT" ,&bU_id)) ERRM(retval, "get bT id");
		if(retval = nc_inq_varid(ncid, "bB" ,&bD_id)) ERRM(retval, "get bB id");
		if(retval = nc_inq_varid(ncid, "bR" ,&bR_id)) ERRM(retval, "get bR id");
		if(retval = nc_inq_varid(ncid, "bL" ,&bL_id)) ERRM(retval, "get bL id");
		
		if(retval = nc_inq_varid(ncid, "eos" ,&eos_id)) ERRM(retval, "get eos id");

		tools::Logger::logger.printString("Ids successfully read");

		if(retval = nc_inq_dimlen(ncid, y_id, &init_ylen)) ERRM(retval, "get dimension length y");
		cells_y = init_ylen;
		if(retval = nc_inq_dimlen(ncid, x_id, &init_xlen)) ERRM(retval, "get dimension length x");
		cells_x = init_xlen;
		if(retval = nc_inq_dimlen(ncid, time_id, &time)) ERRM(retval, "get dimension length time");
		if(retval = nc_get_vara(ncid, time_id, new size_t[1] { time - 1 }, new size_t[1] { 1 }, (&startingTime))) ERRM(retval, "get last time value");

		tools::Logger::logger.printString("Dimension length and last time successfully read");

		initB = new float[init_ylen * init_xlen];
		initHu = new float[init_ylen * init_xlen];
		initHv = new float[init_ylen * init_xlen];
		initH = new float[init_ylen * init_xlen];
		initY = new float[init_ylen];
		initX = new float[init_xlen];
		size_t start[3] = { time - 1, 0, 0 }, length[3] = { 1, 0, 0 }; 

		if(retval = nc_get_var_float(ncid, b_id, initB)) ERRM(retval, "get b values");
		tools::Logger::logger.printString("Bathymetry values successfully read");
		if(retval = nc_get_var_float(ncid, hu_id, initHu)) ERRM(retval, "get hu values");
		tools::Logger::logger.printString("HU values successfully read");
		if(retval = nc_get_var_float(ncid, hv_id, initHv)) ERRM(retval, "get hv values");
		tools::Logger::logger.printString("HV values successfully read");
		if(retval = nc_get_vara_float(ncid, h_id, start, length, initH)) ERRM(retval, "get h values");
		tools::Logger::logger.printString("H values successfully read");
		if(retval = nc_get_var_float(ncid, y_id, initY)) ERRM(retval, "get y values");
		tools::Logger::logger.printString("Y values successfully read");
		if(retval = nc_get_var_float(ncid, x_id, initX)) ERRM(retval, "get x values");
		tools::Logger::logger.printString("X values successfully read");
		
		//if(retval = nc_get_var_int(ncid, bound_id, &boundary)) ERR(retval);
		if(retval = nc_get_var_float(ncid, bU_id, &initBu)) ERR(retval);
		tools::Logger::logger.printString("Top boundary values successfully read");
		if(retval = nc_get_var_float(ncid, bD_id, &initBd)) ERR(retval);
		tools::Logger::logger.printString("Bottom boundary values successfully read");
		if(retval = nc_get_var_float(ncid, bR_id, &initBr)) ERR(retval);
		tools::Logger::logger.printString("Right boundary values successfully read");
		if(retval = nc_get_var_float(ncid, bL_id, &initBl)) ERR(retval);
		tools::Logger::logger.printString("Left boundary values successfully read");
		
		if(retval = nc_get_var_float(ncid, eos_id, &endOfSimulation)) ERR(retval);
		tools::Logger::logger.printString("End of Simulation values successfully read");

		if(retval = nc_close(ncid)) ERR(retval);
		tools::Logger::logger.printString("File closed");

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

  float getLastTime() { return startingTime; }

  SWE_CheckpointScenario() : SWE_Scenario(0, 0){
	readNcFile();
	cells_x = h->getCols();
	cells_y = h->getRows();
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
