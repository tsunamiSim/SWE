/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/TestMain.h>
#include <cxxtest/ErrorPrinter.h>

int main( int argc, char *argv[] ) {
 int status;
    CxxTest::ErrorPrinter tmp;
    CxxTest::RealWorldDescription::_worldName = "cxxtest";
    status = CxxTest::Main< CxxTest::ErrorPrinter >( tmp, argc, argv );
    return status;
}
bool suite_DimenSplitTest_init = false;
#include "/home/sascha/Dokumente/Projects/TsunSim/SWE/src/CxxTests/dimenSplit_testsuite.t.h"

static DimenSplitTest suite_DimenSplitTest;

static CxxTest::List Tests_DimenSplitTest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_DimenSplitTest( "src/CxxTests/dimenSplit_testsuite.t.h", 8, "DimenSplitTest", suite_DimenSplitTest, Tests_DimenSplitTest );

static class TestDescription_suite_DimenSplitTest_test_tools_Array_min : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_DimenSplitTest_test_tools_Array_min() : CxxTest::RealTestDescription( Tests_DimenSplitTest, suiteDescription_DimenSplitTest, 15, "test_tools_Array_min" ) {}
 void runTest() { suite_DimenSplitTest.test_tools_Array_min(); }
} testDescription_suite_DimenSplitTest_test_tools_Array_min;

static class TestDescription_suite_DimenSplitTest_test_tools_Array_max : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_DimenSplitTest_test_tools_Array_max() : CxxTest::RealTestDescription( Tests_DimenSplitTest, suiteDescription_DimenSplitTest, 32, "test_tools_Array_max" ) {}
 void runTest() { suite_DimenSplitTest.test_tools_Array_max(); }
} testDescription_suite_DimenSplitTest_test_tools_Array_max;

static class TestDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_readNcFile : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_readNcFile() : CxxTest::RealTestDescription( Tests_DimenSplitTest, suiteDescription_DimenSplitTest, 49, "test_scenarios_SWE_TsunamiScenario_readNcFile" ) {}
 void runTest() { suite_DimenSplitTest.test_scenarios_SWE_TsunamiScenario_readNcFile(); }
} testDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_readNcFile;

static class TestDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getBoundaryPos : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getBoundaryPos() : CxxTest::RealTestDescription( Tests_DimenSplitTest, suiteDescription_DimenSplitTest, 67, "test_scenarios_SWE_TsunamiScenario_getBoundaryPos" ) {}
 void runTest() { suite_DimenSplitTest.test_scenarios_SWE_TsunamiScenario_getBoundaryPos(); }
} testDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getBoundaryPos;

static class TestDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getBathymetry : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getBathymetry() : CxxTest::RealTestDescription( Tests_DimenSplitTest, suiteDescription_DimenSplitTest, 136, "test_scenarios_SWE_TsunamiScenario_getBathymetry" ) {}
 void runTest() { suite_DimenSplitTest.test_scenarios_SWE_TsunamiScenario_getBathymetry(); }
} testDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getBathymetry;

static class TestDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getWaterHeight : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getWaterHeight() : CxxTest::RealTestDescription( Tests_DimenSplitTest, suiteDescription_DimenSplitTest, 213, "test_scenarios_SWE_TsunamiScenario_getWaterHeight" ) {}
 void runTest() { suite_DimenSplitTest.test_scenarios_SWE_TsunamiScenario_getWaterHeight(); }
} testDescription_suite_DimenSplitTest_test_scenarios_SWE_TsunamiScenario_getWaterHeight;

static class TestDescription_suite_DimenSplitTest_test_scenarios_SWE_CheckpointScenario : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_DimenSplitTest_test_scenarios_SWE_CheckpointScenario() : CxxTest::RealTestDescription( Tests_DimenSplitTest, suiteDescription_DimenSplitTest, 280, "test_scenarios_SWE_CheckpointScenario" ) {}
 void runTest() { suite_DimenSplitTest.test_scenarios_SWE_CheckpointScenario(); }
} testDescription_suite_DimenSplitTest_test_scenarios_SWE_CheckpointScenario;

#include <cxxtest/Root.cpp>
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";
