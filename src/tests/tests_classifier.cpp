#include "tests_classifier.hpp"
#include "testframework.hpp"

#include "classifier.hpp"
#include "lda_fisher.hpp"

namespace tests
{
    int testFisherLDA()
    {
        music::Classifier* c = new music::FisherLDAClassifier();
        CHECK(c != NULL);
        
        ERROR_OUT("this test is not finished yet.", 0);
        return EXIT_FAILURE;
        
        return EXIT_SUCCESS;
    }
}
