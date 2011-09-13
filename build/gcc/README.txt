/**
 * @file README.txt
 * @date Jan 29, 2011
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

Efficient XML Interchange Processor - gcc build

Compiler options such as debugging support and optimization parameters
are located in the main Makefile.

<module_name>.mk - help makefiles used by the main Makefile

Make targets:

all - creates static EXIP library libexip.a in /bin folder

check - runs all unit tests

examples - build samples' executables in /bin/examples

<module_name> check - runs the unit test for module <module_name>

clean - deletes the bin/ directory