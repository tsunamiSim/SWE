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
 * Updates the bathymetry of a SWE_Block object
 */
#ifndef __SWE_UPDATER
#define __SWE_UPDATER

#include "blocks/SWE_Block.hh"
#include "seismology/SWE_DisplacementReader.hh"

class SWE_Updater {

private:
  SWE_Block *m_block;
  SWE_DisplacementReader *m_reader;
  bool isActive;
  int m_xsize, m_ysize;
  float m_left, m_right, m_bot, m_top, m_xstep, m_ystep;
public:
  /** Default constructor for an inactive instance */
  SWE_Updater() :
    isActive(false),
    m_block(NULL),
    m_reader(NULL),
    m_xsize(0),
    m_ysize(0),
    m_left(0),
    m_right(0),
    m_bot(0),
    m_top(0),
    m_xstep(0),
    m_ystep(0) { };

  /**
    Creates an updater working with the given block
    @param i_block The SWE_Block instance to be updated
    @param i_reader The SWE_DisplacementReader to obtain the values from
  */
  SWE_Updater(SWE_Block *i_block, SWE_DisplacementReader *i_reader) :
    isActive(true),
    m_block(i_block),
    m_reader(i_reader) {
    // Set the cell sizes
    m_xstep = m_block->getDx();
    m_ystep = m_block->getDy();
    // Set the grid size
    m_xsize = m_block->getNx();
    m_ysize = m_block->getNy();
    // Set the grid boundaries
    m_left = m_block->getOffx() + m_xstep * .5f;
    m_right = m_left + (m_xsize - 1) * m_xstep;
    m_bot = m_block->getOffy() + m_ystep *.5f;
    m_top = m_bot + (m_ysize - 1) * m_ystep;

#ifndef NDEBUG
  cout << endl << "SWE_Updater initialized" << endl;
  cout << "Grid: Left, Right, Bot, Top (/1k): " << m_left / 1000 << ", " << m_right / 1000 << ", " << m_bot / 1000 << ", " << m_top / 1000 << endl;
  for(int i = 0; i < m_xsize; i++)
    cout << m_left + i * m_xstep << ", ";
cout << endl;
  cout << "X-Size, X-Step, Y-Size, Y-Step: " << m_xsize << ", " << m_xstep << ", " << m_ysize << ", " << m_ystep << endl << endl;
#endif
  };

  /** Sets the bathymetry of the block to the values that should be at the given time */
  void performUpdate(float i_time) {
    if(!isActive) {
      return;
#ifndef NDEBUG
      cout << "Updater inactive. Skipping update at " << i_time << "s" << endl;
#endif
    }
#ifndef NDEBUG
    cout << "Udater active. Performing update at " << i_time << "s" << endl;
#endif
    // c_x/y: counter in x and y direction
    float
      l_x = m_left,
      l_y = m_bot,
      tmp, old;
//    cout << endl << "Before" << endl;
//    cout << m_block->getBathymetry().ToString() << endl;
    #pragma omp parallel for private(c_x)
    for(int c_x = 0; c_x < m_xsize; c_x++)
      for(int c_y = 0; c_y < m_ysize; c_y++) {
        l_x = m_left + c_x * m_xstep;
        l_y = m_bot + c_y * m_ystep;

        old = m_block->getBathymetry()[c_x][c_y];
        tmp = m_reader->getBathymetry(i_time, l_x, l_y);
        m_block->setBathymetry(c_x, c_y, tmp);

      }
//    cout << "After" << endl;
//    cout << m_block->getBathymetry().ToString() << endl << endl;

    m_block->setGhostLayer();
  };
}; // class

#endif // __SWE_UPDATER
