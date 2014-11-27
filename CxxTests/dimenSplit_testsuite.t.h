#include <cxxtest/TestSuite.h>
#include "solvers/FWave.hpp"
#include "blocks/SWE_DimensionalSplitting.hpp"

class DimenSplitTest : public CxxTest::TestSuite
{
private:
	
	solver::FWave<float> *m_solver;

public:
	
	void testAddition() {
		TS_ASSERT(1 + 1 > 1);
	}


};
