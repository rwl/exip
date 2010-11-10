/**
 * @file README.txt
 * @date Sep 14, 2010
 * @author Rumen Kyusakov
 * @version 0.1
 * @par[Revision] $Id$
 */

Efficient XML Interchange Processor

The code is divided into several modules. Each module has a separate Makefile and use Check Unit Testing Framework
for testing - http://check.sourceforge.net/

More information about Efficient XML Interchange format can be found on - http://www.w3.org/TR/exi/

The platform-dependent code is separated in source files with prefix "p_"

The artifacts in the source code which need to be commented (some macro definitions for example)
are marked with #DOCUMENT# comment


Modules dependencies:
===========================================================
    Module    |                 References                |
===========================================================
    common    |                     N/A                   |
-----------------------------------------------------------
   streamIO   |                   common                  |
-----------------------------------------------------------
 stringTables |                   common                  |
-----------------------------------------------------------
    grammar   |       common, streamIO, stringTables      |
-----------------------------------------------------------
  contentIO   |  common, streamIO, stringTables, grammar  |
 ----------------------------------------------------------