to compile cxxtests, uncomment env initialisation for cxx tests in SConstruct (line ) and the cxxtest compile command (line )
To run, use <executable> -r | (-x <cellsX> -y <cellsY> | [-i <inputFolder (see specifications below)>] [-e <endOfsimulation in seconds>] [-l <value setting the progress information interval (%)>]) | -h (which is probably the best start)
inputFolder: By default this variable is set to "NetCDF_Input". Must contain "initBathymetry.nc" and "displacement.nc"
