
#include <iostream>
#include <string>
#include "tools/args.hh"
#include "blocks/SWE_DimensionalSplitting.hpp"
#include "scenarios/SWE_simple_scenarios.hh"
#ifdef WRITENETCDF
#include "writer/NetCdfWriter.hh"
#include "scenarios/SWE_TsunamiScenario.hh"
#include "scenarios/SWE_CheckpointScenario.hh"
#else
#include "writer/VtkWriter.hh"
#endif

/**
* Main program for the simulation using dimensional splitting
*/
int main(int argc, char** argv){
	tools::Args args;
	
	// Prepare command line arguments
 	args.addOption("size_x", 'x', "Number of cells in x direction", tools::Args::Required, false);
	args.addOption("size_y", 'y', "Number of cells in y direction", tools::Args::Required, false);
	args.addOption("checkpoint_file", 'f', "The relative/absolute path to the checkpoint file from where on to continue the iteration (default uses the default checkpoint file)", tools::Args::Optional, false);
	
	// Parse them
	tools::Args::Result parseResult = args.parse(argc, argv);
	
	int test_cp = args.isSet("checkpoint_file"), test_size = args.isSet("size_x") && args.isSet("size_y");
	if(!test_cp && !test_size)
		parseResult = tools::Args::Error;

	// If parsing failed, break the program (if help was asked for and granted, execution did not fail though)

	if(parseResult != tools::Args::Success)
	{
		if(parseResult == tools::Args::Help)
			return 0;
		else
		{
			tools::Logger::logger.printString("Something went wrong!");
			return 1;
		}
	}
	
	// Read simulation domain
	int l_nx, l_ny; 
	l_nx = args.getArgument<int>("size_x");
	l_ny = args.getArgument<int>("size_y");
	//Prepare dambreak scenario
	SWE_CheckpointScenario l_scenario;

	// Set step size
	float l_dx, l_dy;
  	l_dx = (l_scenario.getBoundaryPos(BND_RIGHT) - l_scenario.getBoundaryPos(BND_LEFT) )/l_nx;
  	l_dy = (l_scenario.getBoundaryPos(BND_TOP) - l_scenario.getBoundaryPos(BND_BOTTOM) )/l_ny;

	// Prepare simulation class
	SWE_DimensionalSplitting l_dimensionalSplitting(l_nx, l_ny, l_dx, l_dy);

	// Initialize the scenario
	l_dimensionalSplitting.initScenario(l_scenario.getBoundaryPos(BND_LEFT), l_scenario.getBoundaryPos(BND_BOTTOM), l_scenario);
	// set time and end of simulation
	float l_time = 0.f;
	float l_endOfSimulation = l_scenario.endSimulation();	

	// configure Writer
	std::string basename = "SWE";
	std::string l_fileName = generateBaseFileName(basename,0,0);
	
 	io::BoundarySize l_boundarySize = {{1, 1, 1, 1}};

#ifdef WRITENETCDF
	
	float l_originx, l_originy;
	
	l_originx = l_scenario.getBoundaryPos(BND_LEFT);
	l_originy = l_scenario.getBoundaryPos(BND_BOTTOM);
	//set up NetCdfWriter
	io::NetCdfWriter l_writer( l_fileName,
			l_dimensionalSplitting.getBathymetry(),
			l_boundarySize,
			l_nx, l_ny,
			l_dx, l_dy,
			l_originx, l_originy);

#else
	//set up VTKWriter
	io::VtkWriter l_writer( l_fileName,
		  	l_dimensionalSplitting.getBathymetry(),
			l_boundarySize,
			l_nx, l_ny,
			l_dx, l_dy );
#endif
	//Print initial state
	l_writer.writeTimeStep( l_dimensionalSplitting.getWaterHeight(),
                        l_dimensionalSplitting.getDischarge_hu(),
                        l_dimensionalSplitting.getDischarge_hv(),
                        l_time);
	
	// Loop over timesteps
	while(l_time < l_endOfSimulation)
	{
		l_dimensionalSplitting.setGhostLayer();
		
		// compute one timestep
		l_dimensionalSplitting.computeNumericalFluxes();
	
		// increment time
		l_time += l_dimensionalSplitting.getMaxTimestep();

		// write timestep
		l_writer.writeTimeStep( l_dimensionalSplitting.getWaterHeight(),
                	l_dimensionalSplitting.getDischarge_hu(),
			l_dimensionalSplitting.getDischarge_hv(),
                        l_time);
		
		// write time to console
	  	std::cout << l_time << std::endl << std::endl ;

	}
	return 0;
}
