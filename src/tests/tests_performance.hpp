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

#include <string>

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
     * @param folder The folder containing the test files.
     * 
     * @return <code>EXIT_SUCCESS</code> if no error occured, otherwise <code>EXIT_FAILURE</code>.
     */
    int testInstrumentSimilarity(const std::string& folder = std::string("./testdata/instrument/singlenotes/"));
    
    /**
     * @brief This test helps in finding the best parameters for timbre models.
     * 
     * The main parameters that will be tested are the model size and the model dimensionality.
     * The timbre vector time length will remain fixed, but can be chosen.
     * 
     * @param timbreVectorTimeLength The time slice length with which the timbre vectors will be calculated.
     * @param folder The folder containing the test files.
     * 
     * @return <code>EXIT_SUCCESS</code> if no error occured, otherwise <code>EXIT_FAILURE</code>.
     * @todo 
     */
    int testTimbreParameters(const std::string& timbreVectorTimeLength = std::string("0.01"),
        const std::string& folder = std::string("./testdata/instrument/singlenotes/"));
    
    int testGMMRand();
}

#endif  //TESTS_PERFORMANCE_HPP
