/**
 * @file
 * This file is part of SWE.
 *
 * @author Michael Bader, Kaveh Rahnema
 * @author Sebastian Rettenberger
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
 * TODO
 */

#ifndef __HELP_HH
#define __HELP_HH

#include <math.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>

#ifdef __linux__
#include <unistd.h>
#elif __WINDOWS__
#include <windows.h>
#endif

#include <netcdf.h>
#include "tools/Logger.hh"
using namespace std;
#define ERR(e) {printf("Error: %s\n", nc_strerror(e)); assert(false);}

/**
 * Formats any input to string
 */
template<typename T>
inline std::string toString(T input) {
	std::ostringstream buff;
    	buff << input;
	return buff.str();
};

/**
 * class Float1D is a proxy class that can represent, for example, 
 * a column or row vector of a Float2D array, where row (sub-)arrays 
 * are stored with a respective stride. 
 * Besides constructor/deconstructor, the class provides overloading of 
 * the []-operator, such that elements can be accessed as v[i] 
 * (independent of the stride).
 * The class will never allocate separate memory for the vectors, 
 * but point to the interior data structure of Float2D (or other "host" 
 * data structures).
 */ 
class Float1D
{
  public:
	Float1D(float* _elem, int _rows, int _stride = 1) 
	: rows(_rows),stride(_stride),elem(_elem)
	{
	}

	~Float1D()
	{
	}

  std::string ToString() {
    std::string result = toString("Size: ") + toString(rows) + toString("\n");
    for(int i = 0; i < rows; i++)
      result += toString("[") + toString(i) + toString("]=") + toString(elem[i*stride]) + toString(", ");
  }

	inline float& operator[](int i) { 
		return elem[i*stride]; 
	}

	inline const float& operator[](int i) const {
		return elem[i*stride]; 
	}

	inline float* elemVector() {
		return elem;
	}

  void forceSize(int newSize) {
    rows = newSize;
  }

  inline int getSize() const { return rows; }; 

  private:
    int rows;
    int stride;
    float* elem;
};

/**
 * class Float2D is a very basic helper class to deal with 2D float arrays:
 * indices represent columns (1st index, "horizontal"/x-coordinate) and 
 * rows (2nd index, "vertical"/y-coordinate) of a 2D grid;
 * values are sequentially ordered in memory using "column major" order.
 * Besides constructor/deconstructor, the class provides overloading of 
 * the []-operator, such that elements can be accessed as a[i][j]. 
 */ 
class Float2D {
  public:
  	/**
     * Constructor:
	   * takes size of the 2D array as parameters and creates a respective Float2D object;
		 * allocates memory for the array, but does not initialise value.
     * @param _cols	number of columns (i.e., elements in horizontal direction)
     * @param _rows rumber of rows (i.e., elements in vertical directions)
     */
    Float2D(int _cols, int _rows, bool _allocateMemory = true):
      rows(_rows),
      cols(_cols),
      allocateMemory(_allocateMemory) {
      if (_allocateMemory) {
        elem = new float[rows*cols];
      }
	  }

    /**
     * Constructor:
		 * takes size of the 2D array as parameters and creates a respective Float2D object;
		 * this constructor does not allocate memory for the array, but uses the allocated memory 
		 * provided via the respective variable #_elem 
     * @param _cols	number of columns (i.e., elements in horizontal direction)
     * @param _rows rumber of rows (i.e., elements in vertical directions)
     * @param _elem pointer to a suitably allocated region of memory to be used for thew array elements
     */
    Float2D(int _cols, int _rows, float* _elem):
      rows(_rows),
      cols(_cols),
      allocateMemory(false) {
		  elem = _elem;
	  }


    /**
     * Constructor:
     * takes size of the 2D array as parameters and creates a respective Float2D object;
     * this constructor does not allocate memory for the array, but uses the allocated memory
     * provided via the respective variable #_elem
     * @param _cols number of columns (i.e., elements in horizontal direction)
     * @param _rows rumber of rows (i.e., elements in vertical directions)
     * @param _elem pointer to a suitably allocated region of memory to be used for thew array elements
     */
    Float2D(Float2D& _elem, bool shallowCopy):
      rows(_elem.rows),
      cols(_elem.cols),
      allocateMemory(!shallowCopy) {
      if (shallowCopy) {
        elem = _elem.elem;
        allocateMemory = false;
      }
      else {
        elem = new float[rows*cols];
        for (int i=0; i<rows*cols; i++) {
          elem[i] = _elem.elem[i];
        }
        allocateMemory = true;
      }
    }

	  ~Float2D() {
		  if (allocateMemory) {
		    delete[] elem;
		  }
  	}

	  inline float* operator[](int i) {
  		return (elem + (rows * i));
  	}

	  inline float const* operator[](int i) const {
  		return (elem + (rows * i));
  	}

	inline float* elemVector() {
		return elem;
	}

        inline int getRows() const { return rows; }; 
        inline int getCols() const { return cols; }; 

	inline Float1D getColProxy(int i) {
		// subarray elem[i][*]:
                // starting at elem[i][0] with rows elements and unit stride
		return Float1D(elem + (rows * i), rows);
	};
	
	inline Float1D getRowProxy(int j) {
		// subarray elem[*][j]
                // starting at elem[0][j] with cols elements and stride rows
		return Float1D(elem + j, cols, rows);
	};

	static Float2D compress(const Float2D &input, int compress, int cutColsLeft, int cutColsRight, int cutRowsBot, int cutRowsTop) {
	
		// Create temporary matrix to store compressed data to
//		std::cout << "writing with compression " << compress << std::endl;
		Float2D tmp((int) ceil((input.getCols() - cutColsLeft - cutColsRight) / (float) compress), (int) ceil((input.getRows() - cutRowsBot - cutRowsTop) / (float) compress));
//		std::cout << "matrix cols: " << input.getCols() << ", matrix rows: " << input.getRows() << ", compressed cols: " << tmp.getCols() << ", compressed rows: " << tmp.getRows() << std::endl;
		// Define variables for building average over one block average
		float currentValue;
		int div;
		// Fill each cell of the compressed matrix
		for(int _x = 0; _x < tmp.getCols(); _x++) {
			for(int _y = 0; _y < tmp.getRows(); _y++) {
//				std::cout << "writing at cell [" << _x << ", " << _y << "]: ";
				// Initialize average variables
				currentValue = 0;
				div = 0;
				// Iterate over the initial matrix' compression blocks
				for(int _orig_x = _x * compress;
					// compression boundary
					_orig_x < (_x + 1) * compress &&
					// for the last block make sure to not iterate out of the original matrix
					_orig_x < input.getCols() - cutColsLeft - cutColsRight;
					_orig_x++) {
					for(int _orig_y = _y * compress;
						// compression boundary
						_orig_y < (_y + 1) * compress &&
						// original boundary
						_orig_y < input.getRows() - cutRowsBot - cutRowsTop;
						_orig_y++) {
						// Add the original value to the currentValue buffer and increase the amount of values in this block by one
						currentValue += input[_orig_x + cutColsLeft][_orig_y + cutRowsBot];
						div++;
//						std::cout << "[" << _orig_x + cutColsLeft << ", " <<
//							_orig_y + cutRowsBot << "]: " <<
//							input[_orig_x + cutColsLeft][_orig_y + cutRowsBot] << ", ";
					}
				}
//				std::cout << "=> " << currentValue << "/" << div << "=" << (currentValue / div) << std::endl;
				// write calculated average to compressed matrix
				tmp[_x][_y] = currentValue / div;
			}
		}
		return tmp;
	}

  std::string ToString() const {
    std::string result = "Rows: " + toString(getRows()) + " Cols: " + toString(getCols()) + ": ";
    int cols = getCols(), rows = getRows();
    for(int c = 0; c < cols; c++) for(int r = 0; r < rows; r++)
      result += "[" + toString(c) + "," + toString(r) + "]=" + toString((*this)[c][r]) + ", ";
    return result;
  }

  private:
    int rows;
    int cols;
    float* elem;
	bool allocateMemory;
};

//-------- Methods for Visualistion of Results --------

/**
 * generate output filenames for the single-SWE_Block version
 * (for serial and OpenMP-parallelised versions that use only a 
 *  single SWE_Block - one output file is generated per checkpoint)
 *
 *  @deprecated
 */
inline std::string generateFileName(std::string baseName, int timeStep) {

	std::ostringstream FileName;
	FileName << baseName <<timeStep<<".vtk";
	return FileName.str();
};

/**
 * Generates an output file name for a multiple SWE_Block version based on the ordering of the blocks.
 *
 * @param i_baseName base name of the output.
 * @param i_blockPositionX position of the SWE_Block in x-direction.
 * @param i_blockPositionY position of the SWE_Block in y-direction.
 * @param i_fileExtension file extension of the output file.
 * @return
 *
 * @deprecated
 */
inline std::string generateFileName( std::string i_baseName,
                                     int i_blockPositionX, int i_blockPositionY,
                                     std::string i_fileExtension=".nc" ) {

  std::ostringstream l_fileName;

  l_fileName << i_baseName << "_" << i_blockPositionX << i_blockPositionY << i_fileExtension;
  return l_fileName.str();
};

/**
 * generate output filename for the multiple-SWE_Block version
 * (for serial and parallel (OpenMP and MPI) versions that use 
 *  multiple SWE_Blocks - for each block, one output file is 
 *  generated per checkpoint)
 *
 *  @deprecated
 */
inline std::string generateFileName(std::string baseName, int timeStep, int block_X, int block_Y, std::string i_fileExtension=".vts") {

	std::ostringstream FileName;
	FileName << baseName <<"_"<< block_X<<"_"<<block_Y<<"_"<<timeStep<<i_fileExtension;
	return FileName.str();
};

/**
 * Generates an output file name for a multiple SWE_Block version based on the ordering of the blocks.
 *
 * @param i_baseName base name of the output.
 * @param i_blockPositionX position of the SWE_Block in x-direction.
 * @param i_blockPositionY position of the SWE_Block in y-direction.
 *
 * @return the output filename <b>without</b> timestep information and file extension
 */
inline
std::string generateBaseFileName(std::string &i_baseName, int i_blockPositionX , int i_blockPositionY)
{
	  std::ostringstream l_fileName;

	  l_fileName << i_baseName << "_" << i_blockPositionX << i_blockPositionY;
	  return l_fileName.str();
}

/**
 * generate output filename for the ParaView-Container-File
 * (to visualize multiple SWE_Blocks per checkpoint)
 */
inline std::string generateContainerFileName(std::string baseName, int timeStep) {

	std::ostringstream FileName;
	FileName << baseName<<"_"<<timeStep<<".pvts";
	return FileName.str();
};
/**
 * Collection of helper methods for arrays
 */
class Array {
public:
	/**
	 * Finds the minimum of an array
	 * @param array The array to search in
	 * @param length The array's length
	 */
	template<typename T> static inline T min(T* array, int length) {
		T temp;
#ifndef NDEBUG /*
		std::cout << "Array with the size " << length << std::endl;
		for(int i = 0; i < length; i++)
			std::cout << array[i];
		std::cout << std::endl << std::endl;
		std::cout << std::endl << std::endl;*/
#endif
		if(length == 0) return 0; else temp = array[0];
		for(int i = 1; i < length; i++) {
			if(temp > array[i])
				temp = array[i];
			}
#ifndef NDEBUG // temp must not be NaN (NaN comparison ALWAYS is false)
		if(temp != temp)
			print(array, length);
#else
		assert(temp == temp);
#endif
		return temp;
	}
	/**
	 * Finds the maximum of an array
	 * @param array The array to search in
	 * @param length The array's length
	 */
	template<typename T> static inline T max(T* array, int length) {
		T temp;
#ifndef NDEBUG /*
		std::cout << "Array with the size " << length << std::endl;
		for(int i = 0; i < length; i++)
			std::cout << array[i];
		std::cout << std::endl << std::endl;
		std::cout << std::endl << std::endl; */
#endif
		if(length == 0) return 0; else temp = array[0];
		for(int i = 1; i < length; i++) {
			if(temp < array[i])
				temp = array[i];
			}
#ifndef NDEBUG // temp must not be NaN (NaN comparison ALWAYS is false)
		if(temp != temp)
			print(array, length);
#else
		assert(temp == temp);
#endif
		if(temp != temp)
			print(array, length);
		return temp;
	}
	
	/**
	 * Prints an array to the standard output
	 * @param array the array to be printed
	 * @param length The array's length
	 * @param name The array's name (by default "Array")
	 */
	template<typename T> static void print(T* array, int length, std::string name = "Array") {
		cout << name << ": ";
		for(int i = 0; i < length; i++)
			cout << array[i] << ", ";
		cout << endl;
	}


  	/**
	 * Looks up the closest value's index in a given array
	 * @param searchFor Reference value
	 * @param max The length of the array to search in
	 * @param searchIn The array to search in
	 * @param best The index of the closest element in the array
	 */
	static void lookUp(float searchFor, int max, float* searchIn, int* best){
//std::printf("Entering lookup with searched value=%f\n", searchFor);
		int min = 0;
		max = max - 1;
		int current;

		while(max - min > 1)
		{
			current = (max + min) / 2;
			if(searchIn[current] == searchFor) {
				*best = current;
				return;
			} else if(searchIn[current] > searchFor)
				max = current;
			else
				min = current;
		}
//std::printf("End of search. upper boundary: %f, lower boundary: %f\n", searchIn[max], searchIn[min]);
		if(searchIn[max] - searchFor < searchFor - searchIn[min])
			*best = max;
		else
			*best = min;
//std::printf("Return index %i (value: %f)\n", *best, searchIn[*best]);

		/* old implementation (way slower)

		float disBest = abs(searchFor-searchIn[0]);
		*best = 0;		
		for(int i = 1; i < max; i++){
			float dis = abs(searchFor-searchIn[i]);
			if(disBest > dis) {
				disBest = dis;
				*best = i;
			}				 
		} 

		*/
	};

  template<typename T> static void cut(T** inputArray, int startIndex, int length) {
    T* result = new T[length];
    for(int i = 0; i < length; i++) {
      result[i] = (*inputArray)[startIndex + i];
    }
    *inputArray = result;
  }

};

/**
 * Waits for a given amount of time
 *
 * @param seconds The amount of seconds to be waited
 */
inline void wait(unsigned int seconds) {
	tools::Logger::logger.printString(toString("Waiting for ") + toString(seconds) + toString(" seconds"));
#ifdef __linux__
	sleep(seconds); // Seconds for Linux
#elif __WINDOWS__
	sleep(seconds * 1000); // Milliseconds for windows
#endif
};


#endif

