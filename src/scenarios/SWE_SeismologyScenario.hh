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
 * loads NetCDF-Defined Scenario with dynamic displacement
 */
#ifndef __SWE_SEIS_SCENARIO_H
#define __SWE_SEIS_SCENARIO_H

#include <cmath>
#include <vector>

#include "tools/help.hh"
#include "tools/Logger.hh"
#include "SWE_Scenario.hh"

#define CERR(e) { if(e) printf("Error: %s\n", nc_strerror(e)); assert(false); }

/**
 * Scenario "Artificial Tsunami Scenario":
 * displacement in the middle of the domain  
 */
class SWE_SeismologyScenario : public SWE_Scenario {

private:

  float m_fixTime;
  std::vector<vector<float> > m_bz;
  std::vector<vector<vector<float> > > m_dz;
  Float1D m_bx, m_by, m_dx, m_dy, m_dt;
  float m_disTop, m_disBot, m_disLeft, m_disRight;
  float m_boundLeft, m_boundRight, m_boundTop, m_boundBot;

  void readFiles(const char* bathymetryFile, const char *seisFile) {
    int retval,
      DFileID, BFileID,
      DDimTID, DDimXID, DDimYID,
      BDimXID, BDimYID,
      DVarTID, DVarXID, DVarYID, DVarZID,
      BVarXID, BVarYID, BVarZID,
      DDimCount, DVarCount,
      BDimCount, BVarCount;
    size_t
      DLenX, DLenY, DLenT,
      BLenX, BLenY;
    float
      *DZVals, *DXVals, *DYVals, *DTVals,
      *BZVals, *BXVals, *BYVals;
    
    // Open bathymetry file
    if(retval = nc_open(bathymetryFile, NC_NOWRITE, &BFileID)) ERR(retval);
    // Inquire bathymetry file for dimension and var count
    if(retval = nc_inq(BFileID, &BDimCount, &BVarCount, NULL, NULL)) ERR(retval);
    assert(BDimCount >= 2);
    assert(BVarCount >= 3);
    // Inquire dimension IDs
    if(retval = nc_inq_dimid(BFileID, "x", &BDimXID)) ERR(retval);
    if(retval = nc_inq_dimid(BFileID, "y", &BDimYID)) ERR(retval);
    // Inquire dimension lengths
    if(retval = nc_inq_dimlen(BFileID, BDimXID, &BLenX)) ERR(retval);
    if(retval = nc_inq_dimlen(BFileID, BDimYID, &BLenY)) ERR(retval);
    // Inquire variable IDs
    if(retval = nc_inq_varid(BFileID, "x", &BVarXID)) ERR(retval);
    if(retval = nc_inq_varid(BFileID, "y", &BVarYID)) ERR(retval);
    if(retval = nc_inq_varid(BFileID, "z", &BVarZID)) ERR(retval);
    // Allocate variable memory
    BXVals = new float[BLenX];
    BYVals = new float[BLenY];
    BZVals = new float[BLenX * BLenY];
    // Get variable values
    if(retval = nc_get_var_float(BFileID, BVarXID, BXVals)) ERR(retval);
    if(retval = nc_get_var_float(BFileID, BVarYID, BYVals)) ERR(retval);
    if(retval = nc_get_var_float(BFileID, BVarZID, BZVals)) ERR(retval);
    // Save variables
    m_bx = Float1D(BXVals, BLenX);
    m_by = Float1D(BYVals, BLenY);
    m_bz = vector<vector<float> >();
    for(int y_ = 0; y_ < BLenY; y_++) {
      m_bz.push_back(vector<float>());
      for(int x_ = 0; x_ < BLenX; x_++)
        m_bz[y_].push_back(BZVals[x_ + y_ * BLenX]);
    }
    //Close bathymetry file
    nc_close(BFileID);
    
    // Open displacement file
    if(retval = nc_open(seisFile, NC_NOWRITE, &DFileID)) ERR(retval);
    // Inquire displacement file for dimension and var count
    if(retval = nc_inq(DFileID, &DDimCount, &DVarCount, NULL, NULL)) ERR(retval);
    assert(DDimCount >= 3);
    assert(DVarCount >= 3);
    // Inquire dimension IDs
    if(retval = nc_inq_dimid(DFileID, "x", &DDimXID)) ERR(retval);
    if(retval = nc_inq_dimid(DFileID, "y", &DDimYID)) ERR(retval);
    if(retval = nc_inq_dimid(DFileID, "time", &DDimTID)) ERR(retval);
    // Inquire dimension lengths
    if(retval = nc_inq_dimlen(DFileID, DDimXID, &DLenX)) ERR(retval);
    if(retval = nc_inq_dimlen(DFileID, DDimYID, &DLenY)) ERR(retval);
    if(retval = nc_inq_dimlen(DFileID, DDimTID, &DLenT)) ERR(retval);
    // Inquire variable IDs
    if(retval = nc_inq_varid(DFileID, "x", &DVarXID)) ERR(retval);
    if(retval = nc_inq_varid(DFileID, "y", &DVarYID)) ERR(retval);
    if(retval = nc_inq_varid(DFileID, "time", &DVarTID)) ERR(retval);
    if(retval = nc_inq_varid(DFileID, "z", &DVarZID)) ERR(retval);
    // Allocate variable memory
    DXVals = new float[DLenX];
    DYVals = new float[DLenY];
    DTVals = new float[DLenT];
    // Get variable values
    if(retval = nc_get_var_float(DFileID, DVarXID, DXVals)) ERR(retval);
    if(retval = nc_get_var_float(DFileID, DVarYID, DYVals)) ERR(retval);
    if(retval = nc_get_var_float(DFileID, DVarTID, DTVals)) ERR(retval);
    // Save variablest
    m_dx = Float1D(DXVals, DLenX);
    m_dy = Float1D(DYVals, DLenY);
    m_dt = Float1D(DTVals, DLenT);
    DZVals = new float[DLenT * DLenX * DLenY];
    if(retval = nc_get_var_float(DFileID, DVarZID, DZVals)) ERR(retval);
    for(int t_ = 0; t_ < DLenT; t_++) {
      m_dz.push_back(vector<vector<float> >());
      for(int y_ = 0; y_ < DLenY; y_++) {
        m_dz[t_].push_back(vector<float>());
        for(int x_ = 0; x_ < DLenX; x_++) {
          m_dz[t_][y_].push_back(DZVals[x_ + y_ * DLenX + t_ * DLenX * DLenY]);
        }
      }
    }
  };


public:

  void fixTime(float time)
  {
    m_fixTime = time;
  }

  /**
   * Creates a new instance of the SeismologyScenario Class (clipping does not work at the current state)
   * @param cellsX Cells in x dimension
   * @param cellsY Cells in y dimension
     @param minX clipping minimum in x direction
     @param maxX clipping maximum in x direction
     @param minY clipping minimum in y direction
     @param maxY clipping maximum in y direction
     @param bathymetryFile file with the initial bathymetry values
     @param seismologyFile file with the displacement values
   */
  SWE_SeismologyScenario(int cellsX, int cellsY, float minX = NAN, float maxX = NAN, float minY = NAN, float maxY = NAN, const char *bathymetryFile = "NetCDF_Input/initBathymetry.nc", const char *seismologyFile = "NetCDF_Input/seis.nc") : SWE_Scenario(cellsX, cellsY, OUTFLOW), m_fixTime(-1.f),
    m_bx(NULL, 0), m_by(NULL, 0), m_dx(NULL, 0), m_dy(NULL, 0), m_dt(NULL, 0)  {
    tools::Logger::logger.printString("Running with seismological data");

    tools::Logger::logger.printString("Reading files");
    readFiles(bathymetryFile, seismologyFile);

    if(!isnan(maxX) && !isnan(maxY) && !isnan(minX) && !isnan(minY)){
      tools::Logger::logger.printString("Clipping domain");
      cout << "Before" << endl << "dz(" << m_dz.size() << "," << m_dz[0].size() << "," << m_dz[0][0].size() << ")" << endl;
      cout << "bz(" << m_bz.size() << "," << m_bz[0].size() << ")" << endl;
      clip(minX, maxX, minY, maxY);
      cout << "After" << endl << "dz(" << m_dz.size() << "," << m_dz[0].size() << "," << m_dz[0][0].size() << ")" << endl;
      cout << "bz(" << m_bz.size() << "," << m_bz[0].size() << ")" << endl;
    }

    tools::Logger::logger.printString("Setting boundaries");
    applyBoundaries();
    tools::Logger::logger.printString("Scenario initialized");
    tools::Logger::logger.printLine();
  }

  /** Dummy constructor for cxx tests only */
  SWE_SeismologyScenario() : SWE_Scenario(0, 0, OUTFLOW), m_fixTime(-1.f), m_bx(NULL, 0), m_by(NULL, 0), m_dx(NULL, 0), m_dy(NULL, 0), m_dt(NULL, 0)  { }


  /**
   * The bathymetry at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getBathymetry(float x, float y) {
    return getBathymetry(0, x, y);
  }

  /**
   * The bathymetry at a requested location
   * @param i_time The time at which the bathymetry value should be given
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getBathymetry(float i_time, float x, float y) {
    if(m_fixTime > 0)
      i_time = m_fixTime;
	  int bestXBath, bestYBath;
	  float result;
	  Array::lookUp(y, m_by.getSize(), m_by.elemVector(), &bestYBath);
	  Array::lookUp(x, m_bx.getSize(), m_bx.elemVector(), &bestXBath);
    result = m_bz[bestYBath][bestXBath];
	  if(!(x < m_disLeft || x > m_disRight || y < m_disBot || y > m_disTop)) {
      int interIndex = m_dt.getSize() - 2;
      float interFactor = -1;

      for(int i = 0; i < m_dt.getSize() - 2; i++)
        if(m_dt[i + 1] >= i_time) {
          interIndex = i;
          break;
        }

      if(i_time <= 0)
        interFactor = 0;
      else if(i_time >= m_dt[m_dt.getSize() - 1])
        interFactor = 1;
      else
        interFactor = (i_time - m_dt[interIndex]) / (m_dt[interIndex + 1] - m_dt[interIndex]);

		  int bestXDis, bestYDis;
		  Array::lookUp(y, m_dy.getSize(), m_dy.elemVector(), &bestYDis);
		  Array::lookUp(x, m_dx.getSize(), m_dx.elemVector(), &bestXDis);

		  result += m_dz[interIndex][bestYDis][bestXDis] * (1 - interFactor) +
        m_dz[interIndex + 1][bestYDis][bestXDis] * interFactor;

	}
    if(result < 0.f && result > -20.f)
  	  return -20.f;
    else if(result > 0.f && result < 20.f)
  	  return 20.f;
    else
      return result;
  };

  /**
   * The water height at a requested location
   * @param x The x-Value of the location
   * @param y The y-Value of the location
   */
  float getWaterHeight(float x, float y) {
	return max(-getBathymetry(0, x, y), 0.f);
  };

  /**
   * Returns the time of the end of the simulation
   */
  virtual float endSimulation() { return 100.f; };

  /** Get the boundary positions
   *
   * @param i_edge which edge
   * @return value in the corresponding dimension
   */
  float getBoundaryPos(BoundaryEdge i_edge) {
    if ( i_edge == BND_LEFT ) {
      return m_boundLeft;
      }
    else if ( i_edge == BND_RIGHT) {
      return m_boundRight;
    }
    else if ( i_edge == BND_BOTTOM ) {
      return m_boundBot;
    }
    else {
      return m_boundTop;
    }
  };

//private:

  void applyBoundaries(){
  	tools::Logger::logger.printString("Setting displacement boundaries");
	  m_disTop = Array::max(m_dy.elemVector(), m_dy.getSize());
	  m_disBot = Array::min(m_dy.elemVector(), m_dy.getSize());
	  m_disLeft = Array::min(m_dx.elemVector(), m_dx.getSize());
	  m_disRight = Array::max(m_dx.elemVector(), m_dx.getSize());
	  std::string comma = ", ";
	  tools::Logger::logger.printString(toString("Displacement boundaries set to: left, right, bottom, top (divided by 1000): ") + toString(m_disLeft/1000) + comma + toString(m_disRight/1000) + comma + toString(m_disBot/1000) + comma + toString(m_disTop/1000));
  	tools::Logger::logger.printString("Setting bathymetry boundaries");
	  m_boundTop = Array::max(m_by.elemVector(), m_by.getSize());
	  m_boundBot = Array::min(m_by.elemVector(), m_by.getSize());
	  m_boundLeft = Array::min(m_bx.elemVector(), m_bx.getSize());
	  m_boundRight = Array::max(m_bx.elemVector(), m_bx.getSize());
	  tools::Logger::logger.printString(toString("Bathymetry boundaries set to: left, right, bottom, top (divided by 1000): ") + toString(m_boundLeft/1000) + comma + toString(m_boundRight/1000) + comma + toString(m_boundBot/1000) + comma + toString(m_boundTop/1000));
	  tools::Logger::logger.printLine();
  }
	
  void clip(float minX, float maxX, float minY, float maxY) {
    tools::Logger::logger.printLine();
    tools::Logger::logger.printString("Clipping not yet implemented for seismological data");
    tools::Logger::logger.printLine();
    return;

    tools::Logger::logger.printString(
      toString("Decreasing domain to x: ") +
      toString(minX) + toString("->") +
      toString(maxX) +
      toString(" and y: ") +
      toString(minY) +
      toString("->") +
      toString(maxY));

    int minIndex_disp_x = 0, maxIndex_disp_x = 0,
      minIndex_disp_y = 0, maxIndex_disp_y = 0,
      minIndex_bath_x = 0, maxIndex_bath_x = 0,
      minIndex_bath_y = 0, maxIndex_bath_y = 0,
      cols = 0, rows = 0;

    // Displacement
    clip(minX, maxX, &m_dx, &minIndex_disp_x, &maxIndex_disp_x);
//    tools::Logger::logger.printString("Displacement x values clipped: minIndex: " + toString(minIndex_disp_x) + " maxIndex: " + toString(maxIndex_disp_x));
//    Array::print(disX, maxIndex_disp_x - minIndex_disp_x + 1, "Displacement X values");
    clip(minY, maxY, &m_dy, &minIndex_disp_y, &maxIndex_disp_y);
//    tools::Logger::logger.printString("Displacement y values clipped: minIndex: " + toString(minIndex_disp_y) + " maxIndex: " + toString(maxIndex_disp_y));
//    Array::print(disY, maxIndex_disp_y - minIndex_disp_y + 1, "Displacement Y values");

    vector<vector<vector<float> > > newDisp;
    cols = maxIndex_disp_y - minIndex_disp_y + 1;
    rows = maxIndex_disp_x - minIndex_disp_x + 1;
    cout << "dsp" << endl;
    for(int time = 0; time < m_dt.getSize(); time++){
      newDisp.push_back(vector<vector<float> >());
      for(int col = 0; col < cols; col++){
        newDisp[time].push_back(vector<float>());
        for(int row = 0; row < rows; row++){
          newDisp[time][col].push_back(m_dz[time][col + minIndex_disp_x][row + minIndex_disp_y]);
          cout << "[" << time << "," << col << "," << row << "]=" << (int)(newDisp[time][col][row] * 100) / 100.0 << ", ";
    }}cout << endl;}
    
    m_dz = newDisp;
    // Bathymetry
    clip(minX, maxX, &m_bx, &minIndex_bath_x, &maxIndex_bath_x);
    clip(minY, maxY, &m_by, &minIndex_bath_y, &maxIndex_bath_y);
    vector<vector<float> > newBath;
    cols = maxIndex_bath_y - minIndex_bath_y + 1;
    rows = maxIndex_bath_x - minIndex_bath_x + 1;
    cout << "bath" << endl;
    for(int col = 0; col < cols; col++){
      newBath.push_back(vector<float>());
      for(int row = 0; row < rows; row++){
        newBath[col].push_back(m_bz[col + minIndex_bath_y][row + minIndex_bath_x]);
        cout << "[" << col << "," << row << "]=" << (int)(100 * newBath[col][row]) / 100.0 << ", ";
    }}
    cout << endl;
    m_bz = newBath;
  }

  void clip(float min, float max, Float1D* array, int* minIndex, int* maxIndex) {
    Array::lookUp(min, array->getSize(), array->elemVector(), minIndex);
    Array::lookUp(max, array->getSize(), array->elemVector(), maxIndex);
    int mini = *minIndex, maxi = *maxIndex;
    float *arr = array->elemVector();
    Array::cut(&arr, mini, maxi - mini + 1);
    array = new Float1D(arr, maxi - mini + 1);
  }

public:

  /** Provides information about the maximum timestep allowed with the current displacement and also the time for which the earthquake affects the simulation and bathymetry must be updated
    @param maxTime the last timestep will be stored here
    @param maxTimestep the maximum timestep will be stored here
  */
  void getTimestepInformation(float *maxTime, float *maxTimestep) {
    *maxTime = m_dt[m_dt.getSize() - 1];
    *maxTimestep = *maxTime;
    for(int i = m_dt.getSize() - 1; i > 0; i--)
      *maxTimestep = min(*maxTimestep, m_dt[i] - m_dt[i - 1]);
    if(m_fixTime > 0)
      *maxTime = 0;
  }
};

#endif
