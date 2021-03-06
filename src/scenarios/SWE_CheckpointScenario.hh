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
 * loads NetCDF-Defined crashed Scenario
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
  public:
  Float2D *bathymetry, *hu, *hv, *h;
  float endOfSimulation, *initX, *initY, initBt, initBb, initBr, initBl, startingTime;
  int  boundTop, boundBot, boundLeft, boundRight, boundary;
  size_t time;
  
  /**
   * Reads the file at the relative location "SWE_checkpoints.nc"
   */
  void readNcFile(const char *file){
		int retval, ncid, dim, countVar,
		    y_id,x_id,b_id,hu_id,hv_id, h_id, bound_id, bU_id, bD_id, bR_id, bL_id, eos_id, time_id;
		float *initB, *initHu, *initHv, *initH;      
		size_t init_ylen, init_xlen;

		if(retval = nc_open(file, NC_NOWRITE, &ncid)) ERR(retval);

		if(retval = nc_inq(ncid, &dim, &countVar, NULL, NULL)) ERR(retval);
		assert(dim == 3); 
		assert(countVar == 12);

        //TODO
		//char dimB, dimHu, dimHv, dimH, dimX, dimY ,dimTime;
		if(retval = nc_inq_dimid(ncid, "time", &time_id)) ERRM(retval, "get time id");
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
		size_t timeStart[1] = { time - 1 }, timeLength[1] = { 1 };
		if(retval = nc_get_vara(ncid, time_id, timeStart, timeLength, (&startingTime))) ERRM(retval, "get last time value");

		tools::Logger::logger.printString("Dimension length and last time successfully read");

		initB = new float[init_ylen * init_xlen];
		initHu = new float[init_ylen * init_xlen];
		initHv = new float[init_ylen * init_xlen];
		initH = new float[init_ylen * init_xlen];
		initY = new float[init_ylen];
		initX = new float[init_xlen];
		size_t start[3] = { time - 1, 0, 0 }, length[3] = { 1, init_xlen, init_ylen }; 

		if(retval = nc_get_var_float(ncid, b_id, initB)) ERRM(retval, "get b values");
		tools::Logger::logger.printString("Bathymetry values successfully read");
		if(retval = nc_get_vara_float(ncid, hu_id, start, length, initHu)) ERRM(retval, "get hu values");
		tools::Logger::logger.printString("HU values successfully read");
		if(retval = nc_get_vara_float(ncid, hv_id, start, length, initHv)) ERRM(retval, "get hv values");
		tools::Logger::logger.printString("HV values successfully read");
		if(retval = nc_get_vara_float(ncid, h_id, start, length, initH)) ERRM(retval, "get h values");
		tools::Logger::logger.printString("H values successfully read");
		if(retval = nc_get_var_float(ncid, y_id, initY)) ERRM(retval, "get y values");
		tools::Logger::logger.printString("Y values successfully read");
		if(retval = nc_get_var_float(ncid, x_id, initX)) ERRM(retval, "get x values");
		tools::Logger::logger.printString("X values successfully read");
		
		if(retval = nc_get_var_float(ncid, bU_id, &initBt)) ERR(retval);
		tools::Logger::logger.printString("Top boundary values successfully read");
		if(retval = nc_get_var_float(ncid, bD_id, &initBb)) ERR(retval);
		tools::Logger::logger.printString("Bottom boundary values successfully read");
		if(retval = nc_get_var_float(ncid, bR_id, &initBr)) ERR(retval);
		tools::Logger::logger.printString("Right boundary values successfully read");
		if(retval = nc_get_var_float(ncid, bL_id, &initBl)) ERR(retval);
		tools::Logger::logger.printString("Left boundary values successfully read");
		
		if(retval = nc_get_var_float(ncid, eos_id, &endOfSimulation)) ERR(retval);
		tools::Logger::logger.printString("End of Simulation values successfully read");

		if(retval = nc_close(ncid)) ERR(retval);
		tools::Logger::logger.printString("File closed");

/* Display sql: select DISTINCT value from initH
		float temporary[init_xlen * init_ylen];
		int j = 0;
		for(int i = 0; i < init_xlen * init_ylen; i++) {
			bool found = false;
			for(int j_l = 0; j_l < j && !found; j_l++) 
				if(temporary[j_l] == initH[i])
					found = true;
			if(!found)
				temporary[j++] = initH[i];
		}
		Array::print(temporary, j);
*/
		bathymetry = new Float2D(init_xlen, init_ylen, initB);
		hu = new Float2D(init_xlen, init_ylen, initHu);
		hv = new Float2D(init_xlen, init_ylen, initHv);
		h = new Float2D(init_xlen, init_ylen, initH);
		
#ifndef NDEBUG
		tools::Logger::logger.printString("File read");
#endif
  };

public:

  /**
   * Creates a new instance of the SWE_CheckpointScenario class
   */
  SWE_CheckpointScenario(const char *filename = "SWE_checkpoints.nc") : SWE_Scenario(0, 0){
	tools::Logger::logger.printString("Starting to read");
	readNcFile(filename);
	tools::Logger::logger.printString("Reading finished");
	
	//TODO check if necessary
	cells_x = h->getCols();
	cells_y = h->getRows();
	
	cout << Array::max(initX, bathymetry->getCols()) << "\n";
	boundRight = Array::max(initX, bathymetry->getCols())*cells_x/(cells_x-1);
	boundLeft = Array::min(initX, bathymetry->getCols())*cells_x/(cells_x-1);
	boundTop = Array::max(initY, bathymetry->getRows())*cells_y/(cells_y-1);
	boundBot = Array::min(initY, bathymetry->getRows())*cells_y/(cells_y-1);
	std::string comma = ", ";
	tools::Logger::logger.printString(toString("Set boundaries to: left, right, top, bottom: ") + toString(boundLeft) + comma + toString(boundRight) + comma + toString(boundTop) + comma + toString(boundBot));
  };
  
  /**
   * Returns the amount of checkpoints
   */
  size_t getCheckpointCount(){ return time; }

  /**
   * Returns the time, at which the last simulation crashed
   */
  float getLastTime() { return startingTime; };

  /**
   * The bathymetry at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getBathymetry(float x, float y) {
	int bestX, bestY;
	Array::lookUp(y, bathymetry->getRows(), initY, &bestY);
	Array::lookUp(x, bathymetry->getCols(), initX, &bestX);
	return (*bathymetry)[bestY][bestX];
  };

  /**
   * The water height at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getWaterHeight(float x, float y) { 
	int bestX, bestY;
	Array::lookUp(y, h->getRows(), initY, &bestY);
	Array::lookUp(x, h->getCols(), initX, &bestX);
	return (*h)[bestY][bestX];
  };
  
  /**
   * The velocity (not momentum!) at a requested location in u direction
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getVeloc_u(float x, float y){
    int bestX, bestY;
	Array::lookUp(y, hu->getRows(), initY, &bestY);
	Array::lookUp(x, hu->getCols(), initX, &bestX);
	float result = (*hu)[bestY][bestX] / (*h)[bestY][bestX];
	if(result != result)
		return 0;
	else
		return result;
  };
  
  /**
   * The velocity (not momentum!) at a requested location in v direction
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getVeloc_v(float x, float y){
    int bestX, bestY;
	Array::lookUp(y, hv->getRows(), initY, &bestY);
	Array::lookUp(x, hv->getCols(), initX, &bestX);
	float result = (*hv)[bestY][bestX] / (*h)[bestY][bestX];
	if(result != result)
		return 0;
	else
		return result;
  };  
  
  /**
   * Sets the boundaries
   */
  void getBoundaries(float* u, float* d, float* r, float* l){
    *u= initBt;
    *d= initBb;
    *r= initBr;
    *l= initBl;
  };

  virtual float endSimulation() { return (float) endOfSimulation; };

  virtual BoundaryType getBoundaryType(BoundaryEdge edge) {
		int tmp;
		switch(edge) {
			case BND_LEFT:
				tmp = initBl; break;
			case BND_RIGHT:
				tmp = initBr; break;
			case BND_TOP:	
				tmp = initBt; break;
			case BND_BOTTOM:
				tmp = initBb; break;
		}
		switch(tmp) {
			case 0:
				return OUTFLOW;
			case 1:
				return WALL;
		}
	};

  /** Get the boundary positions
   *
   * @param i_edge which edge
   * @return value in the corresponding dimension
   */
  float getBoundaryPos(BoundaryEdge i_edge) {
	if ( i_edge == BND_LEFT )
		return boundLeft;
	else if ( i_edge == BND_RIGHT)
		return boundRight;
	else if ( i_edge == BND_BOTTOM )
		return boundBot;
	else
		return boundTop;
  };
	

};

#endif
