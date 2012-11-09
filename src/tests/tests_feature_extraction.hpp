#ifndef TESTS_FEATURE_EXTRACTION_HPP
#define TESTS_FEATURE_EXTRACTION_HPP

#include <string>

namespace tests
{
    /** @ingroup tests
     */
    int testEstimateBPM();
    /** @ingroup tests
     */
    int testEstimateChroma();
    /** @ingroup tests
     */
    int applyTimbreEstimation(std::string filename);
    /** @ingroup tests
     */
    int testEstimateTimbre();
    /** @ingroup tests
     */
    int testCalculateDynamicRange();
    /** @ingroup tests
     */
    int testPerBinStatistics();
    /** @ingroup tests
     */
    int testPerTimeSliceStatistics();
}

#endif  //TESTS_FEATURE_EXTRACTION_HPP
