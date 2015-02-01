SWE
===

The Shallow Water Equations teaching code.

Documentation
-------------

The documentation is available in the [Wiki](https://github.com/TUM-I5/SWE/wiki)

License
-------

SWE is release unter GPLv3 (see [gpl.txt](gpl.txt))

-------
Extension

To use the real data, follow instructions in converter subfolder, then compile (scons with netcdf(and openmp)) and run with -h option.
Make sure that even though the file is not used, displacement.nc is still is available in the input folder (some fine-tuning not yet implemented)
