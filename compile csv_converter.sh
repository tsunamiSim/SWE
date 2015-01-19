echo converter/Logger.o
g++ -o converter/Logger.o -c -Isrc -Isrc/include -I/usr/local/include src/tools/Logger.cpp
echo converter/csv_to_ndf_displacement_converter.o 
g++ -o converter/csv_to_ndf_displacement_converter.o -c -I/usr/local/include/ -Isrc/ -std=c++11 src/tools/csv_to_ndf_displacement_converter.cpp
echo csv_to_ndf_displacement_converter 
g++ -o csv_to_ndf_displacement_converter -Wl,-rpath=/usr/local/lib Logger.o converter/csv_to_ndf_displacement_converter.o -L/usr/local/lib -lnetcdf

