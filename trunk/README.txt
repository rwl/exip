/**
 * @mainpage  EXIP - Embeddable EXI Processor in C
 * 
 * EXIP provides a general purpose EXI processing library that is efficient and
 * easy to use for both: embedded platforms and desktop/server applications.
 * The project strives to build large user community as well as committed
 * and eager development team. The purpose of this development documentation
 * is to help the C developers interested in the project to easy get the
 * knowledge they need to work and enhance the code of the library.
 * If you are only interested in using the library see the <em>EXIP
 * User Guide</em> available from the project web site.
 * <p>
 *   Please use the <a href="http://sourceforge.net/p/exip/discussion/" target="_blank">project forums</a>
 *   to ask questions, submit a proposal or search for help.
 * </p>
 * <p>
 *   For discussions and information connected to the code of the library and
 *   various development topics subscribe to the
 *   <a href="https://lists.sourceforge.net/lists/listinfo/exip-develop" target="_blank">
 *   exip-develop</a> mailing list. 
 * </p>
 * <p>
 *  Starting from the Alpha release the development of the library is entirely open -
 *  all the changes are committed to the public SVN repository and available to everyone.
 *  The goal is to make releases as often as possible - optimally every three months.
 *  </p>
 * <p>
 *  The main feature of the code is its modularity. Below are listed the main components
 *  of the codebase (each has a separate Makefile builds and use
 *  <a href="http://check.sourceforge.net/">Check Unit Testing Framework</a> for testing):
 *  <pre>
 *   Modules dependencies:
 *   ==========================================================================================
 *   |             Module              |                      Dependencies                    |
 *   ==========================================================================================
 *   |         @ref common           |                           N/A                         
 *   ------------------------------------------------------------------------------------------
 *   |      @ref streamIO          |                         common                       
 *   ------------------------------------------------------------------------------------------
 *   |     @ref stringTables        |                         common                       
 *   ------------------------------------------------------------------------------------------
 *   |        @ref grammar           |             common, streamIO, stringTables            
 *   ------------------------------------------------------------------------------------------
 *   |      @ref contentIO         |         common, streamIO, stringTables, grammar       
 *   ------------------------------------------------------------------------------------------
 *   |    @ref grammarGen     |   common, streamIO, stringTables, grammar, contentIO  
 *   ------------------------------------------------------------------------------------------
 *  </pre>
 * </p>
 * <p>
 *  More information about Efficient XML Interchange format can be found at - http://www.w3.org/TR/exi/
 * </p>
 *
 * @section get_involved How do I get involved in the EXIP development?
 * The code of the library is still in development stage and unless you use it
 * for trivial EXI processing the chances are that you will come across a bug.
 * Filing a detailed bug report is just one way to help yourself and
 * the project. The culture of the EXIP development is based on
 * <em>"Help yourself and others will help you"</em> principle so if you
 * need your problem fixed soon try trace the source of the error
 * yourself first before looking for help.
 * 
 * @subsection getting_started Getting started
 * Here are some basics steps for developers interested in contributing to the project:
 * <ol start="0">
 *   <li>Read the <em>EXIP User Guide</em> </li>
 *   <li>Start using the library in your applications </li>
 *   <li>Identify bugs, flaws in the design and improvements you would like to see in the library. </li>
 *   <li>Browse through the EXIP development documentation (i.e. the Doxygen pages) and
 *       try to get deeper knowledge on the details of the problem </li>
 *   <li>Implement the fix and create a patch </li>
 *	 <li>Email the patch to the project administrators. If the patch is following the
 *		 project coding rules and is approved by the EXIP administrators you will get a write access to
 *		 the EXIP SVN repository and become a part of the EXIP development team.
 *	     Note that your contribution will be distributed under the same BSD license 
 *	     used for EXIP (@ref copyright "see below")</li>
 *   <li>Being part of the project development will allow you to influence the direction of the
 *       project, set priorities on fixes and improvements and get answers to your
 *       code related questions on time. </li>
 * </ol>
 *
 * @subsection copyright Contributions, copyrights and licenses
 * All contributions are licensed under the same BSD license used by the EXIP project
 * although the project does not require explicitly signing Contributor License Agreement (CLA)
 * by each and every developer.
 * Instead a simple Developer Certificate Of Origin as the one used for the Linux
 * kernel development is applied.
 * See <a href="http://exip.sourceforge.net/license.html" target="_blank"><em>License</em></a>
 * page for more details.
 *  
 * @date Sep 14, 2010
 * @author Rumen Kyusakov
 * @version 0.5
 * @par[Revision] $Id$
 */