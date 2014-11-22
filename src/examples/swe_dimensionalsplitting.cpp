
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
	args.addOption("use_checkpoint_file", 'r', "Use this option to continue a previously failing run", tools::Args::No, false);
	args.addOption("end_of_simulation", 'e' , "Sets the end of the simulation", tools::Args::Required, false);
	
	// Parse them
	tools::Args::Result parseResult = args.parse(argc, argv);
	
	int test_cp = args.isSet("use_checkpoint_file"), test_size = args.isSet("size_x") && args.isSet("size_y"), test_eos = args.isSet("end_of_simulation");
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

	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Preparing scenario");
	//Prepare scenario
	SWE_Scenario* l_scenario;
	if(test_cp)
	{
		l_scenario = new SWE_CheckpointScenario();
		l_nx = l_scenario->getCellsX();
		l_ny = l_scenario->getCellsY();
		tools::Logger::logger.printString(toString("Read cell domain from file: (x, y) = ") + toString(l_nx) + toString(", ") + toString(l_ny));
	}
	else
	{
		l_nx = args.getArgument<int>("size_x");
		l_ny = args.getArgument<int>("size_y");	
		l_scenario = new SWE_TsunamiScenario(l_nx, l_ny);
		tools::Logger::logger.printString(toString("Read cell domain from command line: (x, y) = ") + toString(l_nx) + toString(", ") + toString(l_ny));
	}
    
    
	// Set step size
	float l_dx, l_dy;
  	l_dx = (l_scenario->getBoundaryPos(BND_RIGHT) - l_scenario->getBoundaryPos(BND_LEFT) )/l_nx;
  	l_dy = (l_scenario->getBoundaryPos(BND_TOP) - l_scenario->getBoundaryPos(BND_BOTTOM) )/l_ny;
	cout << "Calulated step size d_x: (" << l_scenario->getBoundaryPos(BND_RIGHT) << " - " << l_scenario->getBoundaryPos(BND_LEFT) << ") / " << l_nx << " = " << l_dx << endl;
	cout << "Calulated step size d_y: (" << l_scenario->getBoundaryPos(BND_TOP) << " - " << l_scenario->getBoundaryPos(BND_BOTTOM) << ") / " << l_ny << " = " << l_dy << endl;

	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Preparing simulation class");

	// Prepare simulation class
	SWE_DimensionalSplitting l_dimensionalSplitting(l_nx, l_ny, l_dx, l_dy);

	tools::Logger::logger.printString("Initializing simulation class");
	// Initialize the scenario
	l_dimensionalSplitting.initScenario(l_scenario->getBoundaryPos(BND_LEFT), l_scenario->getBoundaryPos(BND_BOTTOM), *l_scenario);

	tools::Logger::logger.printString("Reading time domain from scenario");
	// set time and end of simulation
	float l_time = l_scenario->getLastTime();
	float l_endOfSimulation;
	if(test_eos)
	    l_endOfSimulation = args.getArgument<float>("end_of_simulation");
	else
	    l_endOfSimulation = l_scenario->endSimulation();

	// configure Writer
	std::string basename = "SWE";
	std::string l_fileName = generateBaseFileName(basename,0,0);
	
 	io::BoundarySize l_boundarySize = {{1, 1, 1, 1}};

	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Preparing writer");

#ifdef WRITENETCDF
	
	float l_originx, l_originy;
	
	l_originx = l_scenario->getBoundaryPos(BND_LEFT);
	l_originy = l_scenario->getBoundaryPos(BND_BOTTOM);
	//set up NetCdfWriter
	io::NetCdfWriter l_writer( l_fileName,
			l_dimensionalSplitting.getBathymetry(),
			l_boundarySize,
			l_nx, l_ny,
			l_dx, l_dy,
			l_originx, l_originy,
			0,
			test_cp);
	
	// Set up Checkpoint writer
	std::string checkpointFile = "SWE_checkpoints";
	io::NetCdfWriter l_checkpointWriter(checkpointFile,
				l_dimensionalSplitting.getBathymetry(),
				l_boundarySize,
				l_scenario->getBoundaryType(BND_LEFT),
				l_scenario->getBoundaryType(BND_RIGHT),
				l_scenario->getBoundaryType(BND_TOP),
				l_scenario->getBoundaryType(BND_BOTTOM),
				l_endOfSimulation,
				l_nx, l_ny,
				l_dx, l_dy,
				l_originx, l_originy);

#else
	//set up VTKWriter
	io::VtkWriter l_writer( l_fileName,
		  	l_dimensionalSplitting.getBathymetry(),
			l_boundarySize,
			l_nx, l_ny,
			l_dx, l_dy,
			test_cp );
#endif
	//Print initial state
	l_writer.writeTimeStep( l_dimensionalSplitting.getWaterHeight(),
                        l_dimensionalSplitting.getDischarge_hu(),
                        l_dimensionalSplitting.getDischarge_hv(),
                        l_time);
	
#ifdef WRITENETCDF
	int l_timeStepsPerCheckpoint = 10, l_cpCounter = 0;
#endif
	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Starting simulation");

#ifndef NDBUG
	tools::Logger::logger.printString(toString("Start Time: ") + toString(l_time));
#endif
	std::string time = "Time: ";

	l_scenario->getBoundaryPos(BND_TOP);
	
	//
	//TODO remove
	if(1){
	cout << l_scenario->getBoundaryPos(BND_TOP);
	cout << l_scenario->getBoundaryPos(BND_LEFT);
	cout << l_scenario->getBoundaryPos(BND_RIGHT);
	cout << l_scenario->getBoundaryPos(BND_BOTTOM);
	cout << "\n";
	cout << l_scenario->getBathymetry(-100,-100) << "\n";
	cout << l_scenario->getBathymetry(100,-100) << "\n";
	cout << l_scenario->getBathymetry(100,100) << "\n";
	cout << l_scenario->getBathymetry(-100,100) << "\n";
	cout << l_scenario->getBathymetry(0,0) << "\n";
	}
	//
	
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
		
 		std::ostringstream buff;
    		buff << l_time;
		// write time to console
		tools::Logger::logger.printString(time + buff.str());

#ifdef WRITENETCDF	
		if(++l_cpCounter % l_timeStepsPerCheckpoint == 0)
			l_checkpointWriter.writeTimeStep( l_dimensionalSplitting.getWaterHeight(),
                	l_dimensionalSplitting.getDischarge_hu(),
			l_dimensionalSplitting.getDischarge_hv(),
                        l_time);
#endif
	}
	return 0;
}
