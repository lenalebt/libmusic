#include "tests_classifier.hpp"
#include "testframework.hpp"

#include "classifier.hpp"
#include "lda_fisher.hpp"
#include "progress_callback.hpp"
#include "gmm.hpp"

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
    
    int testGMM()
    {
        DEBUG_OUT("running test for gaussian mixture models...", 0);
        music::GaussianMixtureModel gmm;
        srand(time(NULL));
        
        int dimension = 2;
        int dataCount = 1000;
        
        Eigen::VectorXd dataVector(dimension);
        std::vector<Eigen::VectorXd> data;
        
        DEBUG_OUT("adding " << dataCount << " data vectors...", 0);
        //calc mu1 and mu2...
        Eigen::VectorXd mu1(dimension);
        Eigen::VectorXd mu2(dimension);
        for (int j=0; j<dimension; j++)
        {
            mu1[j] = -0.5 + double(rand() % 1000) / 1000.0;
            mu2[j] = 3.0 - 0.5 + double(rand() % 1000) / 1000.0;
        }
        DEBUG_OUT("mu1 = " << mu1, 0);
        DEBUG_OUT("mu2 = " << mu2, 0);
        
        //distribution 1
        DEBUG_OUT(dataCount / 2 << " for distribution 1...", 0);
        Eigen::VectorXd meanVec = Eigen::VectorXd::Zero(dimension);
        for (int i=0; i<dataCount/2; i++)
        {
            for (int j=0; j<dimension; j++)
                dataVector[j] = -0.5 + double(rand() % 1000) / 1000.0;
            meanVec = meanVec + dataVector;
            data.push_back(dataVector + mu1);
        }
        DEBUG_OUT("mean of distribution 1 = " << meanVec / (dataCount/2) + mu1, 0);
        
        meanVec = Eigen::VectorXd::Zero(dimension);
        //distribution 2
        DEBUG_OUT(dataCount / 2 << " for distribution 2...", 0);
        for (int i=dataCount/2; i<dataCount; i++)
        {
            for (int j=0; j<dimension; j++)
                dataVector[j] = -1.0 + double(rand() % 2000) / 1000.0;
            meanVec = meanVec + dataVector;
            data.push_back(dataVector + mu2);
        }
        DEBUG_OUT("mean of distribution 2 = " << meanVec / (dataCount/2) + mu2, 0);
        
        DEBUG_OUT("training gmm...", 0);
        gmm.trainGMM(data, 2);
        DEBUG_OUT("training done.", 0);
        
        //TODO: create data
        //TODO: train data
        //TODO: test data
        
        //TODO: test is not ready yet.
        return EXIT_FAILURE;
        
        return EXIT_SUCCESS;
    }
}
