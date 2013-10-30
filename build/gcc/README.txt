/**
 * @defgroup build_gcc Build using GCC
 *
 * @brief Information on how to build exip using GCC toolchain.
 *
 * Compiler options such as debugging support and optimization parameters
 * are located in a subfolder for each build target under build/gcc/[target]
 * The object files, dependencies and compiler output is stored in the /bin folder
 * <br/>
 * Make targets:
 *   <ul>
 *     <li>all - compiles the EXIP library to object files in the /bin folder and
 *               creates a static library. /bin/headers contains the public API of exip </li>
 *     <li>check - runs all unit tests</li>
 *     <li>examples - build samples' executables in /bin/examples</li>
 *     <li>clean - deletes the bin/ directory</li>
 *     <li>utils - builds exip utility applications</li>
 *     <li>doc - generates Doxygen documentation</li>
 *     <li>dynlib - generates a dynamic library libexip.so in bin/lib</li>
 *   </ul>
 * 
 * Additionally a command-line argument <em>TARGET</em> can be used to specify the target platform
 * (pc by default). For example:
 * <code>
 * 		make TARGET=contiki all
 * </code> 
 *
 * @date Jan 29, 2011
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id$
 */