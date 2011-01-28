/**
 * @file README.txt
 * @date Jan 28, 2011
 * @author Samuel Guilbard
 * @version 0.1
 * @par[Revision] $Id: README.txt 35 2011-01-28 10:59:00Z sguilbard $
 */

Efficient XML Interchange Processor/Visual Studio 2008 Build

This build is implemented as a visual studio solution (exip.sln) with the following projects:

 - exip.vcproj
   The Efficient XML Interchange Processor library
   
 - exipe.vcproj
   A sample that demonstrates EXI encoding using EXIP
 
 - exipd.vcproj
   A sample that demonstrates EXI decoding using EXIP


Compatibility:

 - Compiler: Visual Studio 2008 C++ + Service Pack 1
 
 - Platform: Tested on Windows 7
 
 
 Notes:
 
 Visual Studio 2008 does not support C99 standard.
 
 Porting exip to VS2008 raised the following issues;
  - stdint.h is missing and was replaced by a specific implementation of stdint.h  from Alexander Chemeris.
  - inline function are not supported. Use macro instead
  - any local variables shall be declared before any other statement within a block or a function body.
  - binary literal values are not supported