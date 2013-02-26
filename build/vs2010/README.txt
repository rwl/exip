/**
 * @defgroup build_msvs Build using MSVS
 *
 * @brief Information on how to build exip using MS Visual Studio 2010.
 * @warning { The MSVS build is out-of-date since version 0.4.1 of exip! }
 * 
 * This build is implemented as a visual studio solution (exip.sln) with the following projects:
 *   <ul>
 *     <li>exip.vxcproj - The EXIP library</li>
 *     <li>exipe.vxcproj - A sample that demonstrates EXI encoding using EXIP</li>
 *     <li>exipd.vxcproj - A sample that demonstrates EXI decoding using EXIP</li>
 *     <li>exipg.vxcproj - The exipg utility</li>
 *     <li>exipsep2.vcxproj - A sample application for processing SEP2 messages</li>
 *   </ul>
 * Compatibility tested with Visual Studio 2010 C++ compiler on Windows 7 platforms
 * @note Visual Studio 2010 does not support C99 standard.
 * 
 * Porting exip to VS2010 raised the following issues:
 *   <ul>
 *     <li>stdint.h is missing and was replaced by a specific implementation of stdint.h  from Alexander Chemeris.</li>
 *     <li>inline function are not supported. Use macro instead</li>
 *     <li>any local variables shall be declared before any other statement within a block or a function body.</li>
 *     <li>binary literal values are not supported.</li>
 *   </ul>
 *
 * @date Feb 03, 2011
 * @author Robert Cragie
 * @author Samuel Guilbard
 * @version 0.4
 * @par[Revision] $Id: README.txt 176 2012-08-31 16:02:37Z rumen $
 *
 */