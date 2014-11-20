/**
 * @file
 * This file is part of SWE.
 *
 * @author Alexander Breuer (breuera AT in.tum.de, http://www5.in.tum.de/wiki/index.php/Dipl.-Math._Alexander_Breuer)
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
 * A writer for the netCDF-format: http://www.unidata.ucar.edu/software/netcdf/
 */

#include "NetCdfWriter.hh"
#include <string>
#include <vector>
#include <iostream>
#include <cassert>


#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); assert(false);}

/**
 * Create a netCdf-file
 * Any existing file will be replaced.
 *
 * @param i_baseName base name of the netCDF-file to which the data will be written to.
 * @param i_nX number of cells in the horizontal direction.
 * @param i_nY number of cells in the vertical direction.
 * @param i_dX cell size in x-direction.
 * @param i_dY cell size in y-direction.
 * @param i_originX
 * @param i_originY
 * @param i_flush If > 0, flush data to disk every i_flush write operation
 * @param i_dynamicBathymetry
 */
io::NetCdfWriter::NetCdfWriter( const std::string &i_baseName,
		const Float2D &i_b,
		const BoundarySize &i_boundarySize,
		int i_nX, int i_nY,
		float i_dX, float i_dY,
		float i_originX, float i_originY,
		unsigned int i_flush,
		bool useCheckpoints) :
		//const bool  &i_dynamicBathymetry) : //!TODO
  io::Writer(i_baseName + ".nc", i_b, i_boundarySize, i_nX, i_nY),
  flush(i_flush)
{
	int status;
	if(!useCheckpoints)
		//create a netCDF-file, an existing file will be replaced
		status = nc_create(fileName.c_str(), NC_NETCDF4, &dataFile);
	else
		status = nc_open(fileName.c_str(), NC_WRITE, &dataFile);
	
  //check if the netCDF-file creation constructor succeeded.
	if (status != NC_NOERR) {
		assert(false);
		return;
	}

#ifdef PRINT_NETCDFWRITER_INFORMATION
	std::cout << "   *** io::NetCdfWriter::createNetCdfFile" << std::endl;
	std::cout << "     created/replaced: " << fileName << std::endl;
	std::cout << "     dimensions(nx, ny): " << nX << ", " << nY << std::endl;
	std::cout << "     cell width(dx,dy): " << i_dX << ", " << i_dY << std::endl;
	std::cout << "     origin(x,y): " << i_originX << ", " << i_originY << std::endl;
#endif

	//dimensions
	int l_timeDim, l_xDim, l_yDim;
	nc_def_dim(dataFile, "time", NC_UNLIMITED, &l_timeDim);
	nc_def_dim(dataFile, "x", nX, &l_xDim);
	nc_def_dim(dataFile, "y", nY, &l_yDim);

	//variables (TODO: add rest of CF-1.5)
	int l_xVar, l_yVar;

	nc_def_var(dataFile, "time", NC_FLOAT, 1, &l_timeDim, &timeVar);
	ncPutAttText(timeVar, "long_name", "Time");
	ncPutAttText(timeVar, "units", "seconds since simulation start"); // the word "since" is important for the paraview reader

	nc_def_var(dataFile, "x", NC_FLOAT, 1, &l_xDim, &l_xVar);
	nc_def_var(dataFile, "y", NC_FLOAT, 1, &l_yDim, &l_yVar);

	//variables, fastest changing index is on the right (C syntax), will be mirrored by the library
	int dims[] = {l_timeDim, l_yDim, l_xDim};
	nc_def_var(dataFile, "h",  NC_FLOAT, 3, dims, &hVar);
	nc_def_var(dataFile, "hu", NC_FLOAT, 3, dims, &huVar);
	nc_def_var(dataFile, "hv", NC_FLOAT, 3, dims, &hvVar);
	nc_def_var(dataFile, "b",  NC_FLOAT, 2, &dims[1], &bVar);

	//set attributes to match CF-1.5 convention
	ncPutAttText(NC_GLOBAL, "Conventions", "CF-1.5");
	ncPutAttText(NC_GLOBAL, "title", "Computed tsunami solution");
	ncPutAttText(NC_GLOBAL, "history", "SWE");
	ncPutAttText(NC_GLOBAL, "institution", "Technische Universitaet Muenchen, Department of Informatics, Chair of Scientific Computing");
	ncPutAttText(NC_GLOBAL, "source", "Bathymetry and displacement data.");
	ncPutAttText(NC_GLOBAL, "references", "http://www5.in.tum.de/SWE");
	ncPutAttText(NC_GLOBAL, "comment", "SWE is free software and licensed under the GNU General Public License. Remark: In general this does not hold for the used input data.");

	//setup grid size
	float gridPosition = i_originX + (float).5 * i_dX;
	for(size_t i = 0; i < nX; i++) {
		nc_put_var1_float(dataFile, l_xVar, &i, &gridPosition);

		gridPosition += i_dX;
	}

	gridPosition = i_originY + (float).5 * i_dY;
	for(size_t j = 0; j < nY; j++) {
		nc_put_var1_float(dataFile, l_yVar, &j, &gridPosition);

    	gridPosition += i_dY;
	}

	nc_sync(dataFile);
}


/**
 * Create a netCdf-file
 * Any existing file will be replaced.
 *
 * @param i_baseName base name of the netCDF-file to which the data will be written to.
 * @param i_boundaryTypeLeft The boundary type on the Left of the simulation domain
 * @param i_boundaryTypeRight The boundary type on the Right of the simulation domain
 * @param i_boundaryTypeTop The boundary type on the Top of the simulation domain
 * @param i_boundaryTypeBottom The boundary type on the Bottom of the simulation domain
 * @param i_endOfSimulation The time when the simulation should end
 * @param i_nX number of cells in the horizontal direction.
 * @param i_nY number of cells in the vertical direction.
 * @param i_dX cell size in x-direction.
 * @param i_dY cell size in y-direction.
 * @param i_originX
 * @param i_originY
 * @param i_flush If > 0, flush data to disk every i_flush write operation
 * @param i_dynamicBathymetry
 */
io::NetCdfWriter::NetCdfWriter( const std::string &i_baseName,
		const Float2D &i_b,
		const BoundarySize &i_boundarySize,
		const BoundaryType &i_boundaryTypeLeft,
		const BoundaryType &i_boundaryTypeRight,
		const BoundaryType &i_boundaryTypeTop,
		const BoundaryType &i_boundaryTypeBottom,
		const float i_endOfSimulation,			
		int i_nX, int i_nY,
		float i_dX, float i_dY,
		float i_originX, float i_originY,
		unsigned int i_flush,
		bool useCheckpoints) :
	io::Writer(i_baseName + ".nc", i_b, i_boundarySize, i_nX, i_nY),
	flush(i_flush) {

	int retVal;

	int status;
	if(!useCheckpoints)
		//create a netCDF-file, an existing file will be replaced
		status = nc_create(fileName.c_str(), NC_NETCDF4, &dataFile);
	else
		status = nc_open(fileName.c_str(), NC_WRITE, &dataFile);
	
	//check if the netCDF-file creation constructor succeeded.
	if (status != NC_NOERR) {
		assert(false);
		return;
	}

#ifdef PRINT_NETCDFWRITER_INFORMATION
	std::cout << "   *** io::NetCdfWriter::createNetCdfFile" << std::endl;
	std::cout << "     created/replaced: " << fileName << std::endl;
	std::cout << "     dimensions(nx, ny): " << nX << ", " << nY << std::endl;
	std::cout << "     cell width(dx,dy): " << i_dX << ", " << i_dY << std::endl;
	std::cout << "     origin(x,y): " << i_originX << ", " << i_originY << std::endl;
#endif

	//dimensions
	int l_timeDim, l_xDim, l_yDim;
	nc_def_dim(dataFile, "time", NC_UNLIMITED, &l_timeDim);
	nc_def_dim(dataFile, "x", nX, &l_xDim);
	nc_def_dim(dataFile, "y", nY, &l_yDim);

	//variables (TODO: add rest of CF-1.5)
	int l_xVar, l_yVar, l_eosVar;

	nc_def_var(dataFile, "time", NC_FLOAT, 1, &l_timeDim, &timeVar);
	ncPutAttText(timeVar, "long_name", "Time");
	ncPutAttText(timeVar, "units", "seconds since simulation start"); // the word "since" is important for the paraview reader

	nc_def_var(dataFile, "x", NC_FLOAT, 1, &l_xDim, &l_xVar);
	nc_def_var(dataFile, "y", NC_FLOAT, 1, &l_yDim, &l_yVar);

	// EndOfSimulation
	nc_def_var(dataFile, "eos", NC_FLOAT, 0, 0, &l_eosVar); // The end of simulation variable
	ncPutAttText(l_eosVar, "long_name", "End of Simulation");
	if(retVal = nc_put_var(dataFile, l_eosVar, &i_endOfSimulation)) ERR(retVal);
	
	// Boundary Types
	int l_currBoundary, l_currInt;
	nc_def_var(dataFile, "bL", NC_INT, 0, 0, &l_currBoundary); // Left boundadry
	ncPutAttText(l_currBoundary, "long_name", "Boundary Type left. 0: Outflow, 1: Wall");
	l_currInt = (int) i_boundaryTypeLeft;
	if(retVal = nc_put_var(dataFile, l_currBoundary, &l_currInt)) ERR(retVal);
	nc_def_var(dataFile, "bR", NC_INT, 0, 0, &l_currBoundary); // Right boundadry
	ncPutAttText(l_currBoundary, "long_name", "Boundary Type right. 0: Outflow, 1: Wall");
	l_currInt = (int) i_boundaryTypeRight;
	if(retVal = nc_put_var(dataFile, l_currBoundary, &l_currInt)) ERR(retVal);
	nc_def_var(dataFile, "bT", NC_INT, 0, 0, &l_currBoundary); // Top boundadry
	ncPutAttText(l_currBoundary, "long_name", "Boundary Type top. 0: Outflow, 1: Wall");
	l_currInt = (int) i_boundaryTypeTop;
	if(retVal = nc_put_var(dataFile, l_currBoundary, &l_currInt)) ERR(retVal);
	nc_def_var(dataFile, "bB", NC_INT, 0, 0, &l_currBoundary); // Bottom boundadry
	ncPutAttText(l_currBoundary, "long_name", "Boundary Type bottom. 0: Outflow, 1: Wall");
	l_currInt = (int) i_boundaryTypeBottom;
	if(retVal = nc_put_var(dataFile, l_currBoundary, &l_currInt)) ERR(retVal);

	//variables, fastest changing index is on the right (C syntax), will be mirrored by the library
	int dims[] = {l_timeDim, l_yDim, l_xDim};
	nc_def_var(dataFile, "h",  NC_FLOAT, 3, dims, &hVar);
	nc_def_var(dataFile, "hu", NC_FLOAT, 3, dims, &huVar);
	nc_def_var(dataFile, "hv", NC_FLOAT, 3, dims, &hvVar);
	nc_def_var(dataFile, "b",  NC_FLOAT, 2, &dims[1], &bVar);

	//set attributes to match CF-1.5 convention
	ncPutAttText(NC_GLOBAL, "Conventions", "CF-1.5");
	ncPutAttText(NC_GLOBAL, "title", "Computed tsunami solution");
	ncPutAttText(NC_GLOBAL, "history", "SWE");
	ncPutAttText(NC_GLOBAL, "institution", "Technische Universitaet Muenchen, Department of Informatics, Chair of Scientific Computing");
	ncPutAttText(NC_GLOBAL, "source", "Bathymetry and displacement data.");
	ncPutAttText(NC_GLOBAL, "references", "http://www5.in.tum.de/SWE");
	ncPutAttText(NC_GLOBAL, "comment", "SWE is free software and licensed under the GNU General Public License. Remark: In general this does not hold for the used input data.");

	//setup grid size
	float gridPosition = i_originX + (float).5 * i_dX;
	for(size_t i = 0; i < nX; i++) {
		nc_put_var1_float(dataFile, l_xVar, &i, &gridPosition);

		gridPosition += i_dX;
	}

	gridPosition = i_originY + (float).5 * i_dY;
	for(size_t j = 0; j < nY; j++) {
		nc_put_var1_float(dataFile, l_yVar, &j, &gridPosition);

    	gridPosition += i_dY;
	}

	nc_sync(dataFile);
}

/**
 * Destructor of a netCDF-writer.
 */
io::NetCdfWriter::~NetCdfWriter() {
	nc_close(dataFile);
}

/**
 * Writes time dependent data to a netCDF-file (-> constructor) with respect to the boundary sizes.
 *
 * boundarySize[0] == left
 * boundarySize[1] == right
 * boundarySize[2] == bottom
 * boundarySize[3] == top
 *
 * @param i_matrix array which contains time dependent data.
 * @param i_boundarySize size of the boundaries.
 * @param i_ncVariable time dependent netCDF-variable to which the output is written to.
 */
void io::NetCdfWriter::writeVarTimeDependent( const Float2D &i_matrix,
                                              int i_ncVariable ) {
	//write col wise, necessary to get rid of the boundary
	//storage in Float2D is col wise
	//read carefully, the dimensions are confusing
	size_t start[] = {timeStep, 0, 0};
	size_t count[] = {1, nY, 1};
	for(unsigned int col = 0; col < nX; col++) {
		start[2] = col; //select col (dim "x")
		nc_put_vara_float(dataFile, i_ncVariable, start, count,
				&i_matrix[col+boundarySize[0]][boundarySize[2]]); //write col
  }
}

/**
 * Write time independent data to a netCDF-file (-> constructor) with respect to the boundary sizes.
 * Variable is time-independent
 * boundarySize[0] == left
 * boundarySize[1] == right
 * boundarySize[2] == bottom
 * boundarySize[3] == top
 *
 * @param i_matrix array which contains time independent data.
 * @param i_boundarySize size of the boundaries.
 * @param i_ncVariable time independent netCDF-variable to which the output is written to.
 */
void io::NetCdfWriter::writeVarTimeIndependent( const Float2D &i_matrix,
                                                int i_ncVariable ) {
	//write col wise, necessary to get rid of the boundary
	//storage in Float2D is col wise
	//read carefully, the dimensions are confusing
	size_t start[] = {0, 0};
	size_t count[] = {nY, 1};
	for(unsigned int col = 0; col < nX; col++) {
		start[1] = col; //select col (dim "x")
		nc_put_vara_float(dataFile, i_ncVariable, start, count,
				&i_matrix[col+boundarySize[0]][boundarySize[2]]); //write col
  }
}

/**
 * Writes the unknwons to a netCDF-file (-> constructor) with respect to the boundary sizes.
 *
 * boundarySize[0] == left
 * boundarySize[1] == right
 * boundarySize[2] == bottom
 * boundarySize[3] == top
 *
 * @param i_h water heights at a given time step.
 * @param i_hu momentums in x-direction at a given time step.
 * @param i_hv momentums in y-direction at a given time step.
 * @param i_boundarySize size of the boundaries.
 * @param i_time simulation time of the time step.
 */
void io::NetCdfWriter::writeTimeStep( const Float2D &i_h,
                                      const Float2D &i_hu,
                                      const Float2D &i_hv,
                                      float i_time) {

	/*
	 * Prints the arrays
	for(int row = 0; row < i_h.getRows(); row++)
		for(int col = 0; col < i_h.getCols(); col++)
			{
				std::cout << "Row " << row << ", Column " << col << std::endl;
				std::cout << i_h[row][col] << " " << i_hu[row][col] << " " << i_hv[row][col] << std::endl;
			}
	*/

	if (timeStep == 0)
		// Write bathymetry
		writeVarTimeIndependent(b, bVar);

	//write i_time
	nc_put_var1_float(dataFile, timeVar, &timeStep, &i_time);

	//write water height
	writeVarTimeDependent(i_h, hVar);

	//write momentum in x-direction
	writeVarTimeDependent(i_hu, huVar);

	//write momentum in y-direction
	writeVarTimeDependent(i_hv, hvVar);

	// Increment timeStep for next call
	timeStep++;

	if (flush > 0 && timeStep % flush == 0)
		nc_sync(dataFile);

	nc_sync(dataFile);
	std:string text = "Wrote to file ";
	
	std::ostringstream buff;
   		buff << dataFile;
	tools::Logger::logger.printString(text + buff.str());
}
