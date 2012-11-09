#ifndef TESTS_FEATURE_EXTRACTION_HPP
#define TESTS_FEATURE_EXTRACTION_HPP

namespace tests
{
    /** @ingroup tests
     */
    int testEstimateBPM();
    /** @ingroup tests
     */
    int testEstimateChords();
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
