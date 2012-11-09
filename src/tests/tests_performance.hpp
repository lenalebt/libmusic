#ifndef TESTS_PERFORMANCE_HPP
#define TESTS_PERFORMANCE_HPP

/**
 * @defgroup performance_tests Performance tests
 * @brief Perfomance tests can be found in this module.
 * 
 * These tests create files and display data that tell the user something about the
 * performance of the library in terms of speed and reliability of the
 * classifiers.
 */

namespace performance_tests
{
    /** @ingroup performance_tests
     * @brief Tests the files found in <code>testdata/instruments/singlenotes</code>
     *      for similarity.
     * 
     * Calculates two similarity matricies for these files: one at a
     * file-per-file basis, and one on a instrument-per-instrument basis.
     * The matricies will be displayed on the screen and additionally
     * be written to the files <code>performance/instrument-single-{perfile,perinstrument}.dat</code>
     * 
     * @return <code>EXIT_SUCCESS</code> if no error occured, <code>EXIT_FAILURE</code> else.
     */
    int testInstrumentSimilarity();
}

#endif  //TESTS_PERFORMANCE_HPP
