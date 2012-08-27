#include "tests_classifier.hpp"
#include "testframework.hpp"

#include "classifier.hpp"
#include "lda_fisher.hpp"
#include "progress_callback.hpp"
#include "gmm.hpp"

#include <fstream>

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
        
        //create data. don't have a generator for multivariate gaussian values, just taking equally distributed data (uncorrelated)
        //set these values to change how much data will be generated, and how many dimensions you have
        int dimension = 8;
        int dataCount = 10000;
        
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
        DEBUG_OUT(dataCount / 4 << " for distribution 1...", 0);
        Eigen::VectorXd meanVec = Eigen::VectorXd::Zero(dimension);
        Eigen::VectorXd varVec = Eigen::VectorXd::Zero(dimension);
        for (int i=0; i<dataCount/4; i++)
        {
            for (int j=0; j<dimension; j++)
                dataVector[j] = -0.5 + double(rand() % 1000) / 1000.0;
            meanVec = meanVec + dataVector + mu1;
            varVec = varVec + (dataVector).array().pow(2.0).matrix();
            data.push_back(dataVector + mu1);
        }
        DEBUG_OUT("mean of distribution 1 = " << meanVec / (dataCount/4), 0);
        DEBUG_OUT("estimated variance of distribution 1 = " << varVec / (dataCount/4), 0);
        
        meanVec = Eigen::VectorXd::Zero(dimension);
        varVec = Eigen::VectorXd::Zero(dimension);
        //distribution 2
        DEBUG_OUT(3*dataCount / 4 << " for distribution 2...", 0);
        for (int i=dataCount/4; i<dataCount; i++)
        {
            for (int j=0; j<dimension; j++)
                dataVector[j] = -1.0 + double(rand() % 2000) / 1000.0;
            meanVec = meanVec + dataVector + mu2;
            varVec = varVec + (dataVector).array().pow(2.0).matrix();
            data.push_back(dataVector + mu2);
        }
        DEBUG_OUT("mean of distribution 2 = " << meanVec / (3*dataCount/4), 0);
        DEBUG_OUT("estimated variance of distribution 2 = " << varVec / (3*dataCount/4), 0);
        
        //train GMM with data
        DEBUG_OUT("training gmm...", 0);
        gmm.trainGMM(data, 2);
        DEBUG_OUT("training done.", 0);
        
        //test if the GMM converged the right way, or not. test data.
        std::vector<music::Gaussian*> gaussians = gmm.getGaussians();
        
        for (std::vector<music::Gaussian*>::iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            DEBUG_OUT("gaussian mean: " << (*it)->getMean(), 0);
            DEBUG_OUT("gaussian sigma: " << (*it)->getCovarianceMatrix(), 0);
            DEBUG_OUT("testing if the estimated mean fits to the input mean of the clusters...", 0);
            CHECK( (((**it).getMean() - mu1).norm() / mu1.norm() < 10e-1) || (((**it).getMean() - mu2).norm() / mu2.norm() < 10e-1));
        }
        
        DEBUG_OUT("testing generation of random vectors distributed like a given gaussian distribution...", 0);
        music::Gaussian* g = new music::GaussianFullCov(10);
        Eigen::VectorXd mean(2);
        mean.setZero();
        Eigen::MatrixXd cov(2, 2);// = Eigen::MatrixXd::Identity(10, 10);
        cov.setRandom();
        cov = (cov * cov.transpose()).eval();
        
        
        DEBUG_OUT("mean vector: " << mean, 0);
        DEBUG_OUT("covariance matrix: " << cov, 0);
        g->setMean(mean);
        g->setCovarianceMatrix(cov);
        
        DEBUG_OUT("saving random vectors to a file....", 0);
        std::ofstream outstr("random2x2.dat");
        
        for (int i=0; i<10000; i++)
            outstr << g->rand().transpose() << std::endl;
        
        //TODO: test is not ready yet.
        return EXIT_FAILURE;
        
        return EXIT_SUCCESS;
    }
    int testRNG()
    {
        music::UniformRNG<double>* urng = NULL;
        CHECK(urng == NULL);
        urng = new music::UniformRNG<double>(-1, 1);
        CHECK(urng != NULL);
        
        double mean=0.0;
        double variance=0.0;
        double number;
        int numValues=10000000;
        DEBUG_OUT("generating random numbers with uniform distribution: " << numValues, 0);
        for (int i=0; i<numValues; i++)
        {
            number = urng->rand();
            
            //make sure that all values are within the given interval
            assert(number >= urng->getA());
            assert(number < urng->getB());
            
            mean += number;
            variance += number*number;
        }
        mean /= numValues;
        variance /= numValues;
        
        CHECK_OP(mean, <, 0.02);
        CHECK_OP(mean, >,-0.02);
        CHECK_OP(variance, <, 0.36);
        CHECK_OP(variance, >, 0.30);
        
        DEBUG_VAR_OUT(mean, 0);
        DEBUG_VAR_OUT(variance, 0);
        
        music::NormalRNG<double>* grng = NULL;
        CHECK(grng == NULL);
        grng = new music::NormalRNG<double>();
        CHECK(grng != NULL);
        
        DEBUG_OUT("generating random numbers with normal distribution: " << numValues, 0);
        for (int i=0; i<numValues; i++)
        {
            number = grng->rand();
            //DEBUG_VAR_OUT(number, 0);
            mean += number;
            variance += number*number;
        }
        mean /= numValues;
        variance /= numValues;
        
        CHECK_OP(mean, <, 0.02);
        CHECK_OP(mean, >,-0.02);
        CHECK_OP(variance, <, 1.03);
        CHECK_OP(variance, >, 0.97);
        
        DEBUG_VAR_OUT(mean, 0);
        DEBUG_VAR_OUT(variance, 0);
        
        
        return EXIT_SUCCESS;
    }
}
