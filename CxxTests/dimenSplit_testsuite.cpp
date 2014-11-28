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
#include "/home/ludwig/SWE/CxxTests/dimenSplit_testsuite.t.h"

static DimenSplitTest suite_DimenSplitTest;

static CxxTest::List Tests_DimenSplitTest = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_DimenSplitTest( "CxxTests/dimenSplit_testsuite.t.h", 5, "DimenSplitTest", suite_DimenSplitTest, Tests_DimenSplitTest );

static class TestDescription_suite_DimenSplitTest_testAddition : public CxxTest::RealTestDescription {
public:
 TestDescription_suite_DimenSplitTest_testAddition() : CxxTest::RealTestDescription( Tests_DimenSplitTest, suiteDescription_DimenSplitTest, 13, "testAddition" ) {}
 void runTest() { suite_DimenSplitTest.testAddition(); }
} testDescription_suite_DimenSplitTest_testAddition;

#include <cxxtest/Root.cpp>
const char* CxxTest::RealWorldDescription::_worldName = "cxxtest";
