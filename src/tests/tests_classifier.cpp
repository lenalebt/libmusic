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
        
        Eigen::MatrixXd matrix(3,3);
        matrix.setZero();
        
        //Eigen::VectorXd suppVec = (vec1 + vec2) / 2;
        
        music::OutputStreamCallback osc(std::cout);
        c->learnModel(trainingData, &osc);
        
        //TODO: Test model.
        
        DEBUG_OUT("testing with slightly more complicated vectors (covariance != 0)...", 0);
        
        vec1[0] = 2.0;
        vec1[1] = 2.0;
        vec1[2] = 2.0;
        
        trainingData[0].first.setRandom();
        trainingData[0].first += vec1;
        trainingData[1].first.setRandom();
        trainingData[1].first += vec1;
        trainingData[2].first.setRandom();
        trainingData[2].first += vec1;
        trainingData[3].first.setRandom();
        trainingData[3].first += vec1;
        trainingData[4].first.setRandom();
        trainingData[4].first += vec1;
        trainingData[5].first.setRandom();
        trainingData[5].first += vec1;
        trainingData[6].first.setRandom();
        trainingData[6].first += vec1;
        
        trainingData[7].first.setRandom();
        trainingData[8].first.setRandom();
        trainingData[9].first.setRandom();
        trainingData[10].first.setRandom();
        trainingData[11].first.setRandom();
        trainingData[12].first.setRandom();
        trainingData[13].first.setRandom();
        
        c->learnModel(trainingData, &osc);
        
        DEBUG_VAR_OUT(c->classifyVector(trainingData[0].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[1].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[2].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[3].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[4].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[5].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[6].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[7].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[8].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[9].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[10].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[11].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[12].first), 0);
        DEBUG_VAR_OUT(c->classifyVector(trainingData[13].first), 0);
        
        CHECK_OP(c->classifyVector(trainingData[0].first), <, 0.0);
        CHECK_OP(c->classifyVector(trainingData[1].first), <, 0.0);
        CHECK_OP(c->classifyVector(trainingData[2].first), <, 0.0);
        CHECK_OP(c->classifyVector(trainingData[3].first), <, 0.0);
        CHECK_OP(c->classifyVector(trainingData[4].first), <, 0.0);
        CHECK_OP(c->classifyVector(trainingData[5].first), <, 0.0);
        CHECK_OP(c->classifyVector(trainingData[6].first), <, 0.0);
        CHECK_OP(c->classifyVector(trainingData[7].first), >=, 0.0);
        CHECK_OP(c->classifyVector(trainingData[8].first), >=, 0.0);
        CHECK_OP(c->classifyVector(trainingData[9].first), >=, 0.0);
        CHECK_OP(c->classifyVector(trainingData[10].first), >=, 0.0);
        CHECK_OP(c->classifyVector(trainingData[11].first), >=, 0.0);
        CHECK_OP(c->classifyVector(trainingData[12].first), >=, 0.0);
        CHECK_OP(c->classifyVector(trainingData[13].first), >=, 0.0);
        
        DEBUG_OUT("test classification. first: class 1.", 0);
        for (int i=0; i<1000; i++)
        {
            vec2.setRandom();
            vec2 += vec1;
            CHECK_OP(c->classifyVector(vec2), <, 0.0);
        }
        DEBUG_OUT("test classification. second: class 2.", 0);
        for (int i=0; i<1000; i++)
        {
            vec2.setRandom();
            CHECK_OP(c->classifyVector(vec2), >=, 0.0);
        }
        
        ERROR_OUT("this test is not finished yet.", 0);
        return EXIT_FAILURE;
        
        return EXIT_SUCCESS;
    }
}
