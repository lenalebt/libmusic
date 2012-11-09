#include "tests_classifier.hpp"
#include "testframework.hpp"

#include "classifier.hpp"
#include "lda_fisher.hpp"
#include "progress_callback.hpp"

namespace tests
{
    int testFisherLDA()
    {
        music::FisherLDAClassifier* c = new music::FisherLDAClassifier(true);
        CHECK(c != NULL);
        
        DEBUG_OUT("testing with very simple vectors (covariance 0)...", 0);
        
        std::vector<std::pair<Eigen::VectorXd, double> > trainingData;
        
        Eigen::VectorXd vec1(3);
        vec1[0]=1.0;
        vec1[1]=1.0;
        vec1[2]=1.0;
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec1, 0.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec1, 0.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec1, 0.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec1, 0.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec1, 0.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec1, 0.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec1, 0.0));
        
        Eigen::VectorXd vec2(3);
        vec2[0]=1.0;
        vec2[1]=0.0;
        vec2[2]=1.0;
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 1.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 1.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 1.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 1.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 1.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 1.0));
        trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 1.0));
        
        music::OutputStreamCallback osc(std::cout);
        c->learnModel(trainingData, &osc);
        
        //TODO: Test model.
        CHECK_OP(c->classifyVector(vec1), <, 0.0);
        CHECK_OP(c->classifyVector(vec2), >=, 0.0);
        
        vec1[1] = 2.0;
        CHECK_OP(c->classifyVector(vec1), <, 0.0);
        vec2[1] = -1.0;
        CHECK_OP(c->classifyVector(vec2), >=, 0.0);
        
        DEBUG_OUT("testing with slightly more complicated vectors (covariance != 0)...", 0);
        
        vec1[0] = 2.0;
        vec1[1] = 2.0;
        vec1[2] = 2.0;
        
        trainingData.clear();
        for (int i=0; i<40; i++)
        {
            vec2.setRandom();
            trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 0.0));
            vec2.setRandom();
            vec2 += vec1;
            trainingData.push_back(std::pair<Eigen::VectorXd, double>(vec2, 1.0));
        }
        
        c->learnModel(trainingData, &osc);
        
        for (std::vector<std::pair<Eigen::VectorXd, double> >::iterator it = trainingData.begin(); it != trainingData.end(); it++)
        {
            if (it->second < 0.5)
                {CHECK_OP(c->classifyVector(it->first), <=, 0);}
            else
                {CHECK_OP(c->classifyVector(it->first), >, 0);}
                
        }
        
        DEBUG_OUT("test classification. first: class 1.", 0);
        for (int i=0; i<1000; i++)
        {
            vec2.setRandom();
            CHECK_OP(c->classifyVector(vec2), <, 0.0);
        }
        DEBUG_OUT("test classification. second: class 2.", 0);
        for (int i=0; i<1000; i++)
        {
            vec2.setRandom();
            vec2 += vec1;
            CHECK_OP(c->classifyVector(vec2), >=, 0.0);
        }
        
        ERROR_OUT("this test is not finished yet.", 0);
        return EXIT_FAILURE;
        
        return EXIT_SUCCESS;
    }
}
