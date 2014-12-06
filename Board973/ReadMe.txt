Place the Sencore Board973.dll and firmware in this directory and run the 
following command:

 LIB.EXE /DEF:Board973.def /MACHINE:X86
 
This will create the Board973.lib file needed by the HDTV998Server 
application. When compiling the project, it'll copy the content of this
directory into it's output directory in order to find the dependencies 
when running the program.

Note: The Board973.def does not contain all the exported function's 
__stdcall argument list (in bytes). I only populated those that I was
interested in.