/**
 * @file
 * This file is part of SWE.
 *
 * @author Alexander Binsmaier, Ludwig Peuckert
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
 * Offers functionallity to read displacement that changes per timestep
 */
#ifndef __SWE_DISPLACEMENTREADER
#define __SWE_DISPLACEMENTREADER

#include <string>
#include <cstring>
#include <netcdf.h>
#include <vector>

#include "tools/help.hh"

#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); assert(false);}

using namespace std;
using namespace tools;

class SWE_DisplacementReader {

private:
  std::vector<vector<float> > m_bz;
  std::vector<vector<vector<float> > > m_dz;
  Float1D m_bx, m_by, m_dx, m_dy, m_dt;
  float *result;

public:
  SWE_DisplacementReader(std::string i_dispFile, std::string i_bathFile, const bool i_breakInitialization = false) :
    m_bx(NULL, 0), m_by(NULL, 0), m_dx(NULL, 0), m_dy(NULL, 0), m_dt(NULL, 0) {
    if(i_breakInitialization)
      return;

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
    if(retval = nc_open(i_bathFile.c_str(), NC_NOWRITE, &BFileID)) ERR(retval);
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
    if(retval = nc_open(i_dispFile.c_str(), NC_NOWRITE, &DFileID)) ERR(retval);
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

#ifndef NDEBUG
    tools::Logger::logger.printLine();
    tools::Logger::logger.printString("Seismological Data Reader read the following Data:");
    printBathymetryData();
    printDisplacementData();
    tools::Logger::logger.printString("Seismological Data Reader initialized");
    tools::Logger::logger.printLine();
#endif
  };

  float getBathymetry(float i_time, float i_x, float i_y) {
#ifndef NDEBUG
  cout << "requesting bathymetry at time " << i_time << ", location (" << i_x << ", " << i_y << ")" << endl;
#endif
    int interpolatedIndex;
    float interpolatedValue;

    // Look up timestep
    if(i_time <= m_dt[0]) {
      interpolatedIndex = 0;
      interpolatedValue = 0;
    } else if(i_time >= m_dt[m_dt.getSize() - 1]) {
      interpolatedIndex = m_dt.getSize() - 2;
      interpolatedValue = 1;
    } else {
      Array::lookUp(i_time, m_dt.getSize(), m_dt.elemVector(), &interpolatedIndex);
      if(interpolatedIndex > 0 && i_time <= m_dt[interpolatedIndex])
        interpolatedIndex--;
      interpolatedValue = (i_time - m_dt[interpolatedIndex]) / (m_dt[interpolatedIndex + 1] - m_dt[interpolatedIndex]);
    }
#ifndef NDEBUG
  cout << "time interpolation finished. Found left left timestep " << m_dt[interpolatedIndex] << "s and inteprolation factor " << interpolatedValue << endl;
#endif

    float result, dsp;
    int l_bxi, l_byi, l_dxi, l_dyi;

    // Look up bathymetry
    Array::lookUp(i_x, m_bx.getSize(), m_bx.elemVector(), &l_bxi);
    Array::lookUp(i_y, m_by.getSize(), m_by.elemVector(), &l_byi);
    result = m_bz[l_byi][l_bxi];

#ifndef NDEBUG
  cout << "bathymetry value at location. Found x index " << l_bxi << ", y index " << l_byi << endl << "Value: " << result << endl;
#endif
    
    // Look up displacement
    Array::lookUp(i_x, m_dx.getSize(), m_dx.elemVector(), &l_dxi);
    Array::lookUp(i_y, m_dy.getSize(), m_dy.elemVector(), &l_dyi);

    dsp = m_dz[interpolatedIndex][l_dyi][l_dxi] * (1 - interpolatedValue) + m_dz[interpolatedIndex + 1][l_dyi][l_dxi] * interpolatedValue;

#ifndef NDEBUG
  cout << "displacement value at location. Found x index " << l_dxi << ", y index " << l_dyi << endl << "Index: " << interpolatedIndex << ", Value: " << dsp << endl;
#endif

    // Apply displacement
    result += dsp;

#ifndef NDEBUG
  cout << "result: " << result << endl;
#endif
    return result;
  };

  void printBathymetryData() {
    printBathymetryXDimension();
    printBathymetryYDimension();
    printBathymetryValues();
  };
  void printBathymetryXDimension() {
    Array::print(m_bx.elemVector(), m_bx.getSize(), "Bathymetry x dimension");
  };
  void printBathymetryYDimension() {
    Array::print(m_by.elemVector(), m_by.getSize(), "Bathymetry y dimension");
  };
  void printBathymetryValues() {
    cout << "Bathymetry Values: " << endl;
    for(int i = 0; i < m_bz.size(); i++) {
      for(int j = 0; j < m_bz[i].size(); j++)
        cout << "[" << i << "][" << j << "]=" << m_bz[i][j] << ", ";
      cout << endl;
    }
  };

  void printDisplacementData() {
    printDisplacementXDimension();
    printDisplacementYDimension();
    printDisplacementTDimension();
    printDisplacementValues();
  };

  void printDisplacementXDimension() {
    Array::print(m_dx.elemVector(), m_dx.getSize(), "Displacement x dimension");
  };
  void printDisplacementYDimension() {
    Array::print(m_dy.elemVector(), m_dy.getSize(), "Displacement y dimension");
  };
  void printDisplacementTDimension() {
    Array::print(m_dt.elemVector(), m_dt.getSize(), "Displacement time dimension");
  };
  void printDisplacementValues() {
    cout << "Displacement Values: " << endl;
    for(int t = 0; t < m_dz.size(); t++) {
      for(int i = 0; i < m_dz[t].size(); i++) {
        for(int j = 0; j < m_dz[t][i].size(); j++) {
          cout << "[" << t << "][" << i << "][" << j << "]=" << m_dz[t][i][j] << ", ";
        }
        if( i < m_dz[t].size() - 1)
          cout << endl;
      }
      cout << endl;
    }
  };
};
#endif
