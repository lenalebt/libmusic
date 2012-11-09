#ifndef DYNAMIC_RANGE_HPP
#define DYNAMIC_RANGE_HPP

#include "feature_extraction_helper.hpp"

class DynamicRangeEstimator
{
private:
    PerTimeSliceStatistics* perTimeSliceStatistics
protected:
    
public:
    //Benutze das PerTimeSliceStatistics wegen dem Summenvektor, dann muss man den evtl nur einmal ausrechnen
    DynamicRangeEstimator(PerTimeSliceStatistics* perTimeSliceStatistics)
};

#endif //DYNAMIC_RANGE_HPP
