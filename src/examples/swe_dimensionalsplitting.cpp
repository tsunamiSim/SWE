#include <iostream>
#include <string>
#include "tools/args.hh"
#include "blocks/SWE_DimensionalSplitting.hpp"
#include "scenarios/SWE_simple_scenarios.hh"
#ifdef WRITENETCDF
#include "writer/NetCdfWriter.hh"
#include "scenarios/SWE_TsunamiScenario.hh"
#include "scenarios/SWE_CheckpointScenario.hh"
#include "scenarios/SWE_SeismologyScenario.hh"
#else
#include "writer/VtkWriter.hh"
#endif

#define ARG_SIZE_X "size_x"
#define ARG_SIZE_Y "size_y"
#define ARG_CP "use_checkpoint_file"
#define ARG_EOS "end_of_simulation"
#define ARG_BOUND "boundary_type"
#define ARG_INPUT "input_folder"
#define ARG_COMPRESSION "compression"
#define ARG_LOGGINGSTEPS "percentage_logging_steps"
#define ARG_LEFT "left"
#define ARG_RIGHT "right"
#define ARG_BOT "bottom"
#define ARG_TOP "top"
#define ARG_SEISMOLOGYPATH "seismological_data"
#define ARG_CONSTBATHYMETRY "constant_bathymetry"

/**
* Main program for the simulation using dimensional splitting
*/
int main(int argc, char** argv){
	tools::Args args;
	
	// Prepare command line arguments
 	args.addOption(ARG_SIZE_X, 'x', "Number of cells in x direction", tools::Args::Required, false);
	args.addOption(ARG_SIZE_Y, 'y', "Number of cells in y direction", tools::Args::Required, false);
	args.addOption(ARG_CP, 'r', "Use this option to continue a previously failing run", tools::Args::No, false);
	args.addOption(ARG_EOS, 'e' , "Sets the end of the simulation", tools::Args::Required, false);
	args.addOption(ARG_BOUND, 0, "Sets the boundary conditions, where 1 is Wall and 2 is Outflow", tools::Args::Required, false);
	args.addOption(ARG_INPUT, 'i', "Folder containing the input nc files", tools::Args::Required, false);
	args.addOption(ARG_COMPRESSION, 'c', "Compression value. Simulation calculated with [x]x[y] domain and written [x/c]x[y/c] domain", tools::Args::Required, false);
	args.addOption(ARG_LOGGINGSTEPS, 'l', "Logs a message telling about the progress with the given stepsize", tools::Args::Required, false);
  args.addOption(ARG_LEFT, 0, "Sets the minimum coordinate value in x direction. If one boundary is set, all others must be set too!", tools::Args::Required, false);
  args.addOption(ARG_RIGHT, 0, "Sets the maximum coordinate value in x direction. If one boundary is set, all others must be set too!", tools::Args::Required, false);
  args.addOption(ARG_BOT, 0, "Sets the minimum coordinate value in y direction. If one boundary is set, all others must be set too!", tools::Args::Required, false);
  args.addOption(ARG_TOP, 0, "Sets the maximum coordinate value in y direction. If one boundary is set, all others must be set too!", tools::Args::Required, false);
  args.addOption(ARG_SEISMOLOGYPATH, 's', "Path to the seismological data", tools::Args::Required, false);
  args.addOption(ARG_CONSTBATHYMETRY, 0, "Setting the bathymetry to the negative of the given value", tools::Args::Required, false);

	// Parse them
	tools::Args::Result parseResult = args.parse(argc, argv);
	
	bool test_cp = args.isSet(ARG_CP),
    test_size = args.isSet(ARG_SIZE_X) && args.isSet(ARG_SIZE_Y),
    test_eos = args.isSet(ARG_EOS),
    test_boundary = args.isSet(ARG_BOUND),
    test_seis = args.isSet(ARG_SEISMOLOGYPATH) && !test_cp;
  std::string l_seisPath = "";
  if(test_seis)
    l_seisPath = args.getArgument<std::string>(ARG_SEISMOLOGYPATH);
	if(!test_cp && !test_size)
		parseResult = tools::Args::Error;

	bool test_left = args.isSet(ARG_LEFT),
    test_right = args.isSet(ARG_RIGHT),
    test_top = args.isSet(ARG_TOP),
    test_bot = args.isSet(ARG_BOT);
  float l_left = 0,
    l_right = 0,
    l_bot = 0,
    l_top = 0;
  if(test_left != test_right || test_right != test_top || test_top != test_bot)
    parseResult = tools::Args::Error;
  else if(test_left) {
    l_left = args.getArgument<float>(ARG_LEFT);
    l_right = args.getArgument<float>(ARG_RIGHT);
    l_top = args.getArgument<float>(ARG_TOP);
    l_bot = args.getArgument<float>(ARG_BOT);
    tools::Logger::logger.printString("ARGS: Read boundaries: left="
      + toString(l_left)
      + ", right="
      + toString(l_right)
      + ", bottom="
      + toString(l_bot)
      + ", top="
      + toString(l_top));
  } // if(test_left)

	// If parsing failed, break the program (if help was asked for and granted, execution did not fail though)
	if(parseResult != tools::Args::Success)	{
		if(parseResult == tools::Args::Help)
			return 0;
		else {
			tools::Logger::logger.printString("Something went wrong!");
			return 1;
		} // else
	} // if(parseResult != tools::Args::Success)
	
	// Read simulation domain
	int l_nx, l_ny; 

  tools::Logger::logger.printLine();
  tools::Logger::logger.printString("Preparing scenario");
  //Prepare scenario
  SWE_Scenario* l_scenario;
	if(test_cp)	{
    l_scenario = new SWE_CheckpointScenario();
    l_nx = l_scenario->getCellsX();
    l_ny = l_scenario->getCellsY();
    tools::Logger::logger.printString(toString("Read cell domain from file: (x, y) = ")
      + toString(l_nx)
      + toString(", ")
      + toString(l_ny));
	} // if(test_cp)
	else {
		l_nx = args.getArgument<int>(ARG_SIZE_X);
		l_ny = args.getArgument<int>(ARG_SIZE_Y);	
		if(args.isSet(ARG_INPUT)) {
			std::string sfolder = args.getArgument<std::string>(ARG_INPUT);
			std::string sbath = sfolder + (std::string) "/initBathymetry.nc",
        sdisp = sfolder + (std::string) "/displacement.nc";
      if(test_left && !test_seis)
        l_scenario = new SWE_TsunamiScenario(l_nx, l_ny,
                      l_left,
                      l_right,
                      l_bot,
                      l_top,
                      sbath.c_str(), sdisp.c_str());
      else if(!test_seis)
  			l_scenario = new SWE_TsunamiScenario(l_nx, l_ny, sbath.c_str(), sdisp.c_str());
      else if(test_left)
        l_scenario = new SWE_SeismologyScenario(l_nx, l_ny, l_left, l_right, l_bot, l_top, sbath.c_str(), l_seisPath.c_str());
      else
        l_scenario = new SWE_SeismologyScenario(l_nx, l_ny, NAN, NAN, NAN, NAN, sbath.c_str(), l_seisPath.c_str());
			
		} // if(args.isSet(ARG_INPUT))
		else if(test_left && !test_seis)
        l_scenario = new SWE_TsunamiScenario(l_nx, l_ny,
                      l_left,
                      l_right,
                      l_bot,
                      l_top);
    else if(!test_seis)
  	  l_scenario = new SWE_TsunamiScenario(l_nx, l_ny);
    else if(test_left)
      l_scenario = new SWE_SeismologyScenario(l_nx, l_ny, l_left, l_right, l_bot, l_top);
    else
      l_scenario = new SWE_SeismologyScenario(l_nx, l_ny);

		tools::Logger::logger.printString(toString("Read cell domain from command line: (x, y) = ")
      + toString(l_nx)
      + toString(", ")
      + toString(l_ny));	
	} // else
   
	BoundaryType boundTypes[2];
	boundTypes[0] = WALL;
	boundTypes[1] = OUTFLOW;
	if(test_boundary) {
		int l_boundIndex = args.getArgument<int>(ARG_BOUND);
		if(l_boundIndex < 1 || l_boundIndex > 2) {
			cout << "Boundary conditions not set properly (see help)" << endl;
			return 1;
		} // if(l_boundIndex < 1 || l_boundIndex > 2)
		l_scenario->setBoundaryTypes(boundTypes[l_boundIndex - 1]);
	} // if(test_boundary)
    
	// Set step size
	float l_dx, l_dy;
  l_dx = (l_scenario->getBoundaryPos(BND_RIGHT) - l_scenario->getBoundaryPos(BND_LEFT))/l_nx;
  l_dy = (l_scenario->getBoundaryPos(BND_TOP) - l_scenario->getBoundaryPos(BND_BOTTOM))/l_ny;
  cout << l_dx << ", " << l_dy << endl;

#ifndef NDEBUG
	cout << "Calulated step size d_x (divided by 1000): ("
    << (double)l_scenario->getBoundaryPos(BND_RIGHT)/1000
    << " - "
    << (double)l_scenario->getBoundaryPos(BND_LEFT)/1000
    << ") / "
    << (double)l_nx
    << " = "
    << (double)l_dx/1000
    << endl;
	cout << "Calulated step size d_y (divided by 1000): ("
    << (double)l_scenario->getBoundaryPos(BND_TOP)/1000
    << " - "
    << (double)l_scenario->getBoundaryPos(BND_BOTTOM)/1000
    << ") / "
    << (double)l_ny
    << " = "
    << (double)l_dy/1000
    << endl;
#endif
	if(args.isSet(ARG_CONSTBATHYMETRY)) {
		tools::Logger::logger.printString(toString("Setting bathymetry values to -")
      + toString(args.getArgument<float>(ARG_CONSTBATHYMETRY)));
		l_scenario->setBathymetry(-args.getArgument<float>(ARG_CONSTBATHYMETRY));
	} // if(args.isSet(ARG_CONSTBATHYMETRY))

	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Preparing simulation class");

	// Prepare simulation class
	SWE_DimensionalSplitting l_dimensionalSplitting(l_nx, l_ny, l_dx, l_dy);

	tools::Logger::logger.printString("Initializing simulation class");
	// Initialize the scenario
	l_dimensionalSplitting.initScenario(l_scenario->getBoundaryPos(BND_LEFT),
    l_scenario->getBoundaryPos(BND_BOTTOM),
    *l_scenario);
	
	// set the type of all four boundaries if set via comandline
	if(test_boundary) {
	     switch(args.getArgument<int>(ARG_BOUND)){
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
	     } // switch(args.getArgument<int>(ARG_BOUND))
    } // if(test_boundary)

	tools::Logger::logger.printString("Reading time domain from scenario");
	// set time and end of simulation
	float l_time = l_scenario->getLastTime();
	float l_endOfSimulation;
	if(test_eos)
	    l_endOfSimulation = args.getArgument<float>(ARG_EOS);
	else
	    l_endOfSimulation = l_scenario->endSimulation();

	// configure Writer
	std::string basename = "SWE";
	if(args.isSet(ARG_INPUT))
		basename += toString("_") + args.getArgument<std::string>(ARG_INPUT);

	std::string l_fileName = basename
    + toString("_x")
    + toString(l_nx)
    + toString("_y")
    + toString(l_ny)
    + toString("_t")
    + toString(l_endOfSimulation);
  if(test_seis)
    l_fileName += "_seis";

	l_fileName.erase(
    std::remove(l_fileName.begin(),
      l_fileName.end(),
      '/'),
    l_fileName.end());
	
	tools::Logger::logger.printString(toString("Writing to file: ")
    + l_fileName);
 	io::BoundarySize l_boundarySize = {{1, 1, 1, 1}};

	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Preparing writer");

#ifdef WRITENETCDF
	int l_timeStepsPerCheckpoint = 10, l_cpCounter = 0;
	size_t l_checkpoints = 0;
	l_checkpoints = l_scenario->getCheckpointCount();
	
	float l_originx, l_originy;
	int compression = 1;

	if(args.isSet(ARG_COMPRESSION) && !args.isSet(ARG_CP))
		compression = args.getArgument<int>(ARG_COMPRESSION);
	
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
	tools::Logger::logger.printString(toString("Start Time: ")
    + toString(l_time));
#endif
	std::string time = "Time: ";
	float progress = 0; // progress in percent;
	float percStep = 0.1f;
	if(args.isSet(ARG_LOGGINGSTEPS))
		percStep = args.getArgument<float>(ARG_LOGGINGSTEPS) / 100.0f;

	if(percStep < 0.01f)
		percStep = 0.01f;

	unsigned int loggedAmount = 1;

#ifdef _OPENMP
	tools::Logger::logger.printString(toString("Using openMP paralization with ")
    + toString(omp_get_max_threads())
    + " threads");
#endif
	
	tools::Logger::logger.printLine();
	tools::Logger::logger.printString("Starting simulation");

	//Print initial state
	l_writer.writeTimeStep( l_dimensionalSplitting.getWaterHeight(),
                        l_dimensionalSplitting.getDischarge_hu(),
                        l_dimensionalSplitting.getDischarge_hv(),
                        l_dimensionalSplitting.getBathymetry(),
                        l_time);

  float fixTimestep = 3, actTimestep;
  bool useFix = false;

	// Loop over timesteps *************************************************************************************
	while(l_time < l_endOfSimulation)	{

#ifdef WRITENETCDF
    if(test_seis)
      useFix = l_dimensionalSplitting.updateBathymetry(l_time, (SWE_SeismologyScenario*)l_scenario) > 0.01;
#endif

		l_dimensionalSplitting.setGhostLayer();
		
		// compute one timestep
		l_dimensionalSplitting.computeNumericalFluxes();

		// increment time
    actTimestep = l_dimensionalSplitting.getMaxTimestep();
    if(useFix && fixTimestep < actTimestep)
      l_time += fixTimestep;
    else
      l_time += actTimestep;

		// write timestep
		l_writer.writeTimeStep( l_dimensionalSplitting.getWaterHeight(),
    	l_dimensionalSplitting.getDischarge_hu(),
      l_dimensionalSplitting.getDischarge_hv(),
      l_dimensionalSplitting.getBathymetry(),
      l_time);
		
 		std::ostringstream buff;
    		buff << l_time;
		// write time to console
    tools::Logger::logger.printString(time + buff.str());        

		progress = l_time / l_endOfSimulation;
		if(progress > percStep * loggedAmount) {
			tools::Logger::logger.printString(toString("Progress: ")
        + toString((int) (100 * progress))
        + toString("%"));
			loggedAmount = progress / percStep + 1;
		} // if(progress > percStep * loggedAmount)
        
#ifdef WRITENETCDF	
		if(++l_cpCounter % l_timeStepsPerCheckpoint == 0)
			l_checkpointWriter.writeTimeStep( l_dimensionalSplitting.getWaterHeight(),
        l_dimensionalSplitting.getDischarge_hu(),
        l_dimensionalSplitting.getDischarge_hv(),
        l_dimensionalSplitting.getBathymetry(),
        l_time);
#endif
	} // while(l_time < l_endOfSimulation)

	tools::Logger::logger.printString("End of simulation");
	delete l_scenario;
	return 0;
} // main
