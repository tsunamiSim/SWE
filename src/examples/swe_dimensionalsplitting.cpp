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
	args.addOption("boundary_conditions", 'b', "Sets the boundary conditions, where 1 is Wall and 2 is Outflow", tools::Args::Required, false);
	args.addOption("input_folder", 'i', "Folder containing the input nc files", tools::Args::Required, false);
	args.addOption("compression", 'c', "Compression value. Simulation calculated with [x]x[y] domain and written [x/c]x[y/c] domain", tools::Args::Required, false);
	args.addOption("percentage_logging_steps", 'l', "Logs a message telling about the progress with the given stepsize", tools::Args::Required, false);
	
	// Parse them
	tools::Args::Result parseResult = args.parse(argc, argv);
	
	int test_cp = args.isSet("use_checkpoint_file"), test_size = args.isSet("size_x") && args.isSet("size_y"), test_eos = args.isSet("end_of_simulation"), test_boundary = args.isSet("boundary_condition");
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
		if(args.isSet("input_folder")) {
			std::string sfolder = args.getArgument<std::string>("input_folder");
			std::string sbath = sfolder + (std::string) "/initBathymetry.nc", sdisp = sfolder + (std::string) "/displacement.nc";
			l_scenario = new SWE_TsunamiScenario(l_nx, l_ny, sbath.c_str(), sdisp.c_str());
			
		}
		else
			l_scenario = new SWE_TsunamiScenario(l_nx, l_ny);
		tools::Logger::logger.printString(toString("Read cell domain from command line: (x, y) = ") + toString(l_nx) + toString(", ") + toString(l_ny));	
	}
   
	BoundaryType boundTypes[2];
	boundTypes[0] = WALL;
	boundTypes[1] = OUTFLOW;
	if(args.isSet("boundary_conditions")) {
		int l_boundIndex = args.getArgument<int>("boundary_conditions");
		if(l_boundIndex < 1 || l_boundIndex > 2) {
			cout << "Boundary conditions not set properly (see help)" << endl;
			return 1;
		}
		l_scenario->setBoundaryTypes(boundTypes[l_boundIndex - 1]);
	}
    
    
	// Set step size
	float l_dx, l_dy;
  	l_dx = (l_scenario->getBoundaryPos(BND_RIGHT) - l_scenario->getBoundaryPos(BND_LEFT))/l_nx;
  	l_dy = (l_scenario->getBoundaryPos(BND_TOP) - l_scenario->getBoundaryPos(BND_BOTTOM))/l_ny;

#ifndef NDEBUG
	cout << "Calulated step size d_x (divided by 1000): (" << (double)l_scenario->getBoundaryPos(BND_RIGHT)/1000 << " - " << (double)l_scenario->getBoundaryPos(BND_LEFT)/1000 << ") / " << (double)l_nx << " = " << (double)l_dx/1000 << endl;
	cout << "Calulated step size d_y (divided by 1000): (" << (double)l_scenario->getBoundaryPos(BND_TOP)/1000 << " - " << (double)l_scenario->getBoundaryPos(BND_BOTTOM)/1000 << ") / " << (double)l_ny << " = " << (double)l_dy/1000 << endl;
#endif

	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Preparing simulation class");

	// Prepare simulation class
	SWE_DimensionalSplitting l_dimensionalSplitting(l_nx, l_ny, l_dx, l_dy);

	tools::Logger::logger.printString("Initializing simulation class");
	// Initialize the scenario
	l_dimensionalSplitting.initScenario(l_scenario->getBoundaryPos(BND_LEFT), l_scenario->getBoundaryPos(BND_BOTTOM), *l_scenario);
	
	// set the type of all fout boundaries if set via comandline
	if(test_boundary){
	     switch(args.getArgument<int>("boundary_condition")){
	        case 1:
	            l_dimensionalSplitting.setBoundaryType(BND_LEFT, WALL);
                l_dimensionalSplitting.setBoundaryType(BND_RIGHT, WALL);
                l_dimensionalSplitting.setBoundaryType(BND_BOTTOM, WALL);
                l_dimensionalSplitting.setBoundaryType(BND_TOP, WALL);
	            tools::Logger::logger.printString("Setting boundary conditions: WALL");
	            break;
	        case 0:
	            l_dimensionalSplitting.setBoundaryType(BND_LEFT, OUTFLOW);
                l_dimensionalSplitting.setBoundaryType(BND_RIGHT, OUTFLOW);
                l_dimensionalSplitting.setBoundaryType(BND_BOTTOM, OUTFLOW);
                l_dimensionalSplitting.setBoundaryType(BND_TOP, OUTFLOW);
	            tools::Logger::logger.printString("Setting boundary conditions: OUTFLOW");
	            break;
	        default:  
	            tools::Logger::logger.printString("Unable to parse boundary condition from comandline, used default!");
	            break;
	     }
    }

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
	int l_timeStepsPerCheckpoint = 10, l_cpCounter = 0;
	size_t l_checkpoints = 0;
	l_checkpoints = l_scenario->getCheckpointCount();
	
	float l_originx, l_originy;
	int compression = 1;

	if(args.isSet("compression") && !args.isSet("use_checkpoints_file")) {
		compression = args.getArgument<int>("compression");
	}
	
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
			l_checkpoints * l_timeStepsPerCheckpoint,
			compression);
	
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
				l_originx, l_originy,
				0,
				test_cp,
				compression);
#else
	//set up VTKWriter
	io::VtkWriter l_writer( l_fileName,
		  	l_dimensionalSplitting.getBathymetry(),
			l_boundarySize,
			l_nx, l_ny,
			l_dx, l_dy,
			test_cp );
#endif

#ifndef NDEBUG
	tools::Logger::logger.printString(toString("Start Time: ") + toString(l_time));
#endif
	std::string time = "Time: ";
	float progress = 0; // progress in percent;
	float percStep = 0.1f;
	if(args.isSet("percentage_logging_steps"))
		percStep = args.getArgument<float>("percentage_logging_steps") / 100.0f;
	if(percStep < 0.01f)
		percStep = 0.01f;
	unsigned int loggedAmount = 1;

#ifdef _OPENMP
	tools::Logger::logger.printString(toString("Using openMP paralization with ") + toString(omp_get_max_threads()) + " threads");
#endif
	
	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Starting simulation");

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
		
 		std::ostringstream buff;
    		buff << l_time;
		// write time to console
//		tools::Logger::logger.printString(time + buff.str());        

		progress = l_time / l_endOfSimulation;
		if(progress > percStep * loggedAmount) {
			tools::Logger::logger.printString(toString("Progress: ") + toString((int) (100 * progress)) + toString("%"));
			loggedAmount = progress / percStep + 1;
		}
        
#ifdef WRITENETCDF	
		if(++l_cpCounter % l_timeStepsPerCheckpoint == 0)
			l_checkpointWriter.writeTimeStep( l_dimensionalSplitting.getWaterHeight(),
                	l_dimensionalSplitting.getDischarge_hu(),
			l_dimensionalSplitting.getDischarge_hv(),
                        l_time);
#endif
	}

	tools::Logger::logger.printString("End of simulation");
	delete l_scenario;
	return 0;
}
