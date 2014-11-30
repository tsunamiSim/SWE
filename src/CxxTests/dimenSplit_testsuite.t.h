#include <cxxtest/TestSuite.h>
#include "scenarios/SWE_TsunamiScenario.hh"
#include "scenarios/SWE_CheckpointScenario.hh"
#include "tools/help.hh"

using namespace tools;

class DimenSplitTest : public CxxTest::TestSuite
{
private:
	float eps;
public:
DimenSplitTest() : eps(0.001f) { };

void test_tools_Array_min() {
	int sortedAscI[] = { -3, 5, 10 }, sortedDescI[] = { 10, 5, -3 }, unsortedI[] = { -3, 10, 5 };
	float sortedAscF[] = { -2.5f, 0.f, 20.2f }, sortedDescF[] = { 20.2f, 0.f, -2.5f }, unsortedF[] = { -2.5f, 0.f, 20.2f };

	// Check sorted ascending
	TS_ASSERT_EQUALS(Array::min(sortedAscI, 3), -3);
	TS_ASSERT_EQUALS(Array::min(sortedAscF, 3), -2.5f);

	// Check sorted descending
	TS_ASSERT_EQUALS(Array::min(sortedDescI, 3), -3);
	TS_ASSERT_EQUALS(Array::min(sortedDescF, 3), -2.5f);

	// Check unsorted
	TS_ASSERT_EQUALS(Array::min(unsortedI, 3), -3);
	TS_ASSERT_EQUALS(Array::min(unsortedF, 3), -2.5f);
}

void test_tools_Array_max() {
	int sortedAscI[] = { -3, 5, 10 }, sortedDescI[] = { 10, 5, -3 }, unsortedI[] = { -3, 10, 5 };
	float sortedAscF[] = { -2.5f, 0.f, 20.2f }, sortedDescF[] = { 20.2f, 0.f, -2.5f }, unsortedF[] = { -2.5f, 0.f, 20.2f };

	// Check sorted ascending
	TS_ASSERT_EQUALS(Array::max(sortedAscI, 3), 10);
	TS_ASSERT_EQUALS(Array::max(sortedAscF, 3), 20.2f);

	// Check sorted descending
	TS_ASSERT_EQUALS(Array::max(sortedDescI, 3), 10);
	TS_ASSERT_EQUALS(Array::max(sortedDescF, 3), 20.2f);

	// Check unsorted
	TS_ASSERT_EQUALS(Array::max(unsortedI, 3), 10);
	TS_ASSERT_EQUALS(Array::max(unsortedF, 3), 20.2f);
}

void test_scenarios_SWE_TsunamiScenario_readNcFile() {
	Float2D *ZBuffer;
	float *xBuffer, *yBuffer;
	const char *name = "NetCDF_Input/testReadNcFile.nc";

	SWE_TsunamiScenario temp;
	temp.readNcFile(name, &ZBuffer, &xBuffer, &yBuffer);
	TS_ASSERT_EQUALS(ZBuffer->getRows(), 5);
	TS_ASSERT_EQUALS(ZBuffer->getCols(), 5);
	for(int i = 0; i < ZBuffer->getRows(); i++)
		for(int j = 0; j < ZBuffer->getCols(); j++)
			TS_ASSERT_EQUALS((*ZBuffer)[i][j], i * 10 + j);
	for(int i = 0; i < ZBuffer->getRows(); i++)
		TS_ASSERT_EQUALS(xBuffer[i], i - 2);
	for(int i = 0; i < ZBuffer->getCols(); i++)
		TS_ASSERT_EQUALS(yBuffer[i], i - 2);
}

void test_scenarios_SWE_TsunamiScenario_getBoundaryPos() {
	/* File for displacement
netcdf testDisplacement {
dimensions:
	x = 3 ;
	y = 3 ;
variables:
	float x(x) ;
	float y(y) ;
	float z(y, x) ;

// global attributes:
		:Conventions = "COARDS" ;
data:

 x = -1, 0, 1 ;

 y = -1, 0, 1 ;

 z =
  // z(0, 0-2)
    150, 150, 150,
  // z(1, 0-2)
    150, 0, 150,
  // z(2, 0-2)
    150, 150, 0 ;
}
	*/
	/* File for bathymetry
netcdf testBathymetry {
dimensions:
	x = 5 ;
	y = 5 ;
variables:
	float x(x) ;
	float y(y) ;
	float z(y, x) ;

// global attributes:
		:Conventions = "COARDS" ;
data:

 x = -2, -1, 0, 1, 2 ;

 y = -2, -1, 0, 1, 2 ;

 z =
  // z(0, 0-4)
    -100, -100, -100, -100, -100,
  // z(1, 0-4)
    -100, -100, -100, -100, -100,
  // z(2, 0-4)
    -100, -100, -100, -100, -100,
  // z(3, 0-4)
    -100, -100, -100, -100, -100,
  // z(4, 0-4)
    -100, -100, -100, -100, -10 ;
}	*/
	
	const char *bathFile = "NetCDF_Input/testBathymetry.nc", *dispFile = "NetCDF_Input/testDisplacement.nc";
	SWE_TsunamiScenario scenario(5, 5, bathFile, dispFile);
	
	// Check boundaries
	TS_ASSERT_EQUALS(scenario.getBoundaryPos(BND_LEFT), -2.f);
	TS_ASSERT_EQUALS(scenario.getBoundaryPos(BND_RIGHT), 2.f);
	TS_ASSERT_EQUALS(scenario.getBoundaryPos(BND_TOP), 2.f);
	TS_ASSERT_EQUALS(scenario.getBoundaryPos(BND_BOTTOM), -2.f);
}

void test_scenarios_SWE_TsunamiScenario_getBathymetry() {
	/* File for displacement
netcdf testDisplacement {
dimensions:
	x = 3 ;
	y = 3 ;
variables:
	float x(x) ;
	float y(y) ;
	float z(y, x) ;

// global attributes:
		:Conventions = "COARDS" ;
data:

 x = -1, 0, 1 ;

 y = -1, 0, 1 ;

 z =
  // z(0, 0-2)
    150, 150, 150,
  // z(1, 0-2)
    150, 0, 150,
  // z(2, 0-2)
    150, 150, 0 ;
}
	*/
	/* File for bathymetry
netcdf testBathymetry {
dimensions:
	x = 5 ;
	y = 5 ;
variables:
	float x(x) ;
	float y(y) ;
	float z(y, x) ;

// global attributes:
		:Conventions = "COARDS" ;
data:

 x = -2, -1, 0, 1, 2 ;

 y = -2, -1, 0, 1, 2 ;

 z =
  // z(0, 0-4)
    -100, -100, -100, -100, -100,
  // z(1, 0-4)
    -100, -100, -100, -100, -100,
  // z(2, 0-4)
    -100, -100, -100, -100, -100,
  // z(3, 0-4)
    -100, -100, -100, -100, -100,
  // z(4, 0-4)
    -100, -100, -100, -100, -10 ;
}	*/
	
	const char *bathFile = "NetCDF_Input/testBathymetry.nc", *dispFile = "NetCDF_Input/testDisplacement.nc";
	SWE_TsunamiScenario scenario(5, 5, bathFile, dispFile);

	// Check bathymetry AT cells
	float x[] = { -2, -1, 0, 1, 2 }, y[] = { -2, -1, 0, 1, 2 }, bath[5][5][3];
	for(int i = 0; i < 5; i++) for(int j = 0; j < 5; j++) {
		bath[i][j][0] = x[i];
		bath[i][j][1] = y[j];
		bath[i][j][2] = (((i == 1 || i == 3) && j > 0 && j < 4) ||
				(i == 2 && j % 2 == 1)) ? 50 : -100;
	}
	bath[4][4][2] = -20; // This is for bathymetry which is in the [-20,20] interval
	for(int i = 0; i < 5; i++) for(int j = 0; j < 5; j++)
		TS_ASSERT_EQUALS(scenario.getBathymetry(bath[i][j][0], bath[i][j][1]), bath[i][j][2]);
	// Check bathymetry BETWEEN cells
	TS_ASSERT_EQUALS(scenario.getBathymetry(-0.25f, 0.25f), -100);
}

void test_scenarios_SWE_TsunamiScenario_getWaterHeight() {
	/* File for displacement
netcdf testDisplacement {
dimensions:
	x = 3 ;
	y = 3 ;
variables:
	float x(x) ;
	float y(y) ;
	float z(y, x) ;

// global attributes:
		:Conventions = "COARDS" ;
data:

 x = -1, 0, 1 ;

 y = -1, 0, 1 ;

 z =
  // z(0, 0-2)
    150, 150, 150,
  // z(1, 0-2)
    150, 0, 150,
  // z(2, 0-2)
    150, 150, 0 ;
}
	*/
	/* File for bathymetry
netcdf testBathymetry {
dimensions:
	x = 5 ;
	y = 5 ;
variables:
	float x(x) ;
	float y(y) ;
	float z(y, x) ;

// global attributes:
		:Conventions = "COARDS" ;
data:
	
 x = -2, -1, 0, 1, 2 ;

 y = -2, -1, 0, 1, 2 ;

 z =
  // z(0, 0-4)
    -100, -100, -100, -100, -100,
  // z(1, 0-4)
    -100, -100, -100, -100, -100,
  // z(2, 0-4)
    -100, -100, -100, -100, -100,
  // z(3, 0-4)
    -100, -100, -100, -100, -100,
  // z(4, 0-4)
    -100, -100, -100, -100, -10 ;
}	*/
	
	const char *bathFile = "NetCDF_Input/testBathymetry.nc", *dispFile = "NetCDF_Input/testDisplacement.nc";
	SWE_TsunamiScenario scenario(5, 5, bathFile, dispFile);

	// Check water height
	for(int x = -2; x < 3; x++) for(int y = -2; y < 3; y++)
		TS_ASSERT_EQUALS(scenario.getWaterHeight(x, y), (x == 2 && y == 2) ? 20 : 100);
}

void test_scenarios_SWE_CheckpointScenario() {
	SWE_CheckpointScenario scenario("testCheckpoints.nc");

	int index; float array[] = { -2.f, 0.f, 1.f };
	float reference[] = { -2.f, -1.5f, -1.f, -0.5f, 0.5f };
	int results[] = { 0, 0, 0, 1, 1 };
	for(int i = 0; i < 5; i++) {
		scenario.lookUp(reference[i], 5, array, &index);
		TS_ASSERT_EQUALS(index, results[i]);
	}

	TS_ASSERT_EQUALS(scenario.endSimulation(), 100);
	TS_ASSERT_EQUALS(scenario.getCheckpointCount(), 2);
	TS_ASSERT_EQUALS(scenario.getLastTime(), 5.4f);

	for(int x = -2; x < 3; x++) for(int y = -2; y < 3; y++)
		TS_ASSERT_EQUALS(scenario.getBathymetry(x, y), -20);
	for(int x = -2; x < 3; x++) for(int y = -2; y < 3; y++)
		TS_ASSERT_EQUALS(scenario.getWaterHeight(x, y), (((x == -1 || x == 1) && y > -2 && y < 2) ||
				(x == 0 && (y + 2) % 2 == 1)) ? 0 : 20 );
	for(int x = -2; x < 3; x++) for(int y = -2; y < 3; y++)
		TS_ASSERT_EQUALS(scenario.getVeloc_u(x, y), (((x == -1 || x == 1) && y > -2 && y < 2) ||
				(x == 0 && (y + 2) % 2 == 1)) ? 0 : 1 );
	for(int x = -2; x < 3; x++) for(int y = -2; y < 3; y++)
		TS_ASSERT_EQUALS(scenario.getVeloc_v(x, y), (((x == -1 || x == 1) && y > -2 && y < 2) ||
				(x == 0 && (y + 2) % 2 == 1)) ? 0 : 1 );

	TS_ASSERT_EQUALS(scenario.getBoundaryType(BND_LEFT), OUTFLOW);
	TS_ASSERT_EQUALS(scenario.getBoundaryType(BND_RIGHT), WALL);
	TS_ASSERT_EQUALS(scenario.getBoundaryType(BND_TOP), OUTFLOW);
	TS_ASSERT_EQUALS(scenario.getBoundaryType(BND_BOTTOM), WALL);
	
	// Check boundaries
	TS_ASSERT_EQUALS(scenario.getBoundaryPos(BND_LEFT), -2.f);
	TS_ASSERT_EQUALS(scenario.getBoundaryPos(BND_RIGHT), 2.f);
	TS_ASSERT_EQUALS(scenario.getBoundaryPos(BND_TOP), 2.f);
	TS_ASSERT_EQUALS(scenario.getBoundaryPos(BND_BOTTOM), -2.f);
}
};
