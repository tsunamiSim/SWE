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
  int m_bnx, m_bny, m_dnx, m_dny, m_dnt;
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
        m_b[y_].push_back(BZVals[x_ + y_ * BLenX]);
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
    tools::Logger::logger.printString("Seismological data reader read the following data:");
    cout << "BX (" << m_bx.getSize() << "): ";
    for(int i = 0; i < m_bx.getSize(); i++)
      cout << m_bx[i] << ", ";
    cout << endl << "BY (" << m_by.getSize() << "): ";
    for(int i = 0; i < m_by.getSize(); i++)
      cout << m_by[i] << ", ";
    cout << endl << "BZ: ";
    for(int i = 0; i < m_bz.size(); i++) for(int j = 0; j < m_bz[i].size(); j++)
      cout << m_bz[i][j] << ", " <<;
    cout << endl;

    cout << endl << "DX (" << m_dx.getSize() << "): ";
    for(int i = 0; i < m_dx.getSize(); i++)
      cout << m_dx[i] << ", ";
    cout << endl << "DY (" << m_dy.getSize() << "): ";
    for(int i = 0; i < m_dy.getSize(); i++)
      cout << m_dy[i] << ", ";
    cout << endl << "DT (" << m_dt.getSize() << "): ";
    for(int i = 0; i < m_dt.getSize(); i++)
      cout << m_dt[i] << ", ";
    for(int i = 0; i < m_dz.size(); i++) {
      cout << endl << "DZ (" << m_dt[i] << "): ";
      for(int j = 0; j < m_dz[i].size(); j++) for(int k = 0; k < m_dz[i][j].size(); k++)
        cout << m_dz[i][j][k] << ", ";
    }
    cout << endl << "End Reader" << endl;
#endif
  };

  ~SWE_DisplacementReader() {
  };

  float getBathymetry(float i_time, float i_x, float i_y) {
    int interpolatedIndex;
    float interpolatedValue;
    if(i_time <= m_dt[0]) {
      interpolatedIndex = 0;
      interpolatedValue = 0;
    } else if(i_time >= m_dt[m_dt.getSize() - 1]) {
      interpolatedIndex = m_dt.getSize() - 2;
      interpolatedValue = 1;
    } else {
      for(int i = 0; i < m_dt.getSize(); i++) {
        if(m_dt[i] >= i_time) {
          interpolatedIndex = i - 1;
          interpolatedValue = (i_time - m_dt[interpolatedIndex]) / (m_dt[interpolatedIndex + 1] - m_dt[interpolatedIndex]);
          break;
        }
      }
    }


    
 /*
    Float2D l_result(m_bz, false);
    Float2D l_d(m_dz[0].getRows(), m_dz[0].getCols());

    int index;
    float interpolate;
    if(i_time <= m_dt[0]) {
      index = 0;
      interpolate = 0;
    }
    else if(i_time >= m_dt[m_dt.getSize() - 1]) {
      index = m_dt.getSize() - 2;
      interpolate = 1;
    }
    else
      for(int i = 0; i < m_dt.getSize(); i++)
        if(m_dt[i] >= i_time) {
          index = i - 1;
          interpolate = (i_time - m_dt[index]) / (m_dt[index + 1] - m_dt[index]);
          break;
        }

    int l_nx = m_dx.getSize(), l_ny = m_dy.getSize(), l_bnx = m_bx.getSize(), l_bny = m_by.getSize(),
      m_x, m_y;
    for(int x = 0; x < l_nx; x++) for(int y = 0; y < l_ny; y++) {
      Array::lookUp(m_dx[x], l_bnx, m_bx.elemVector(), &m_x);
      Array::lookUp(m_dy[y], l_bny, m_by.elemVector(), &m_y);
      l_result[m_x][m_y] += (1 - interpolate) * m_dz[index][x][y] + interpolate * m_dz[index + 1][x][y];
    }
    return l_result;***/
  };
};
#endif
