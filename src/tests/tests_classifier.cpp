#include "tests_classifier.hpp"
#include "testframework.hpp"

#include "classifier.hpp"
#include "lda_fisher.hpp"
#include "progress_callback.hpp"
#include "gmm.hpp"
#include "kmeans.hpp"
#include "randomnumbers.hpp"

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
        
        DEBUG_OUT("testing generation of random vectors distributed like a given gaussian mixture distribution...", 0);
        data.clear();
        
        DEBUG_OUT("reading old mean values...", 0);
        mu1 = gaussians[0]->getMean();
        mu2 = gaussians[1]->getMean();
        
        DEBUG_OUT("generating new data from the previously calculated gaussian mixture model (twice as many data points)...", 0);
        for (int i=0; i<2*dataCount; i++)
        {
            data.push_back(gmm.rand());
        }
        
        DEBUG_OUT("deleting old gaussians...", 0);
        for (unsigned int g=0; g<gaussians.size(); g++)
            delete gaussians[g];
        gaussians.clear();
        
        //train GMM with data
        DEBUG_OUT("training gmm...", 0);
        gmm.trainGMM(data, 2);
        DEBUG_OUT("training done.", 0);
        
        //test if the GMM converged the right way, or not. test data.
        gaussians = gmm.getGaussians();
        
        for (std::vector<music::Gaussian*>::iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            DEBUG_OUT("gaussian mean: " << (*it)->getMean(), 0);
            DEBUG_OUT("gaussian sigma: " << (*it)->getCovarianceMatrix(), 0);
            DEBUG_OUT("testing if the estimated mean fits to the input mean of the clusters...", 0);
            CHECK( (((**it).getMean() - mu1).norm() / mu1.norm() < 10e-1) || (((**it).getMean() - mu2).norm() / mu2.norm() < 10e-1));
        }
        
        DEBUG_OUT("Model as JSON: " << gmm, 0);
        std::string gmmJSON = gmm.toJSONString();
        DEBUG_OUT("Model as JSON from string: " << gmmJSON, 0);
        
        DEBUG_OUT("loading model from JSON string...", 0);
        music::GaussianMixtureModel gmm2;
        gmm2.loadFromJSONString(gmmJSON);
        
        gaussians = gmm2.getGaussians();
        
        for (std::vector<music::Gaussian*>::iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            DEBUG_OUT("gaussian mean: " << (*it)->getMean(), 0);
            DEBUG_OUT("gaussian sigma: " << (*it)->getCovarianceMatrix(), 0);
            DEBUG_OUT("testing if the estimated mean fits to the input mean of the clusters...", 0);
            CHECK( (((**it).getMean() - mu1).norm() / mu1.norm() < 10e-1) || (((**it).getMean() - mu2).norm() / mu2.norm() < 10e-1));
        }
        
        music::GaussianMixtureModel gmm3;
        gmm3.trainGMM(data, 5);
        music::GaussianMixtureModel gmm4;
        gmm4.trainGMM(data, 10);
        
        /*
        gaussians = gmm3.getGaussians();
        gaussians[0]->setMean(mu1 * 1.1);
        gaussians[1]->setMean(mu2 * 1.1);
        */
        
        DEBUG_VAR_OUT(gmm.compareTo(gmm2), 0);
        DEBUG_VAR_OUT(gmm2.compareTo(gmm), 0);
        DEBUG_VAR_OUT(gmm.compareTo(gmm3), 0);
        DEBUG_VAR_OUT(gmm3.compareTo(gmm), 0);
        DEBUG_VAR_OUT(gmm.compareTo(gmm4), 0);
        DEBUG_VAR_OUT(gmm4.compareTo(gmm), 0);
        
        return EXIT_SUCCESS;
    }
    int testKMeans()
    {
        DEBUG_OUT("running test for k-means...", 0);
        music::KMeans<double> kmeans;
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
        
        DEBUG_OUT("running k-means with generated data and 2 means...", 0);
        CHECK(kmeans.trainKMeans(data, 2, 100));
        std::vector<Eigen::VectorXd> means1 = kmeans.getMeans();
        DEBUG_VAR_OUT(means1[0], 0);
        DEBUG_VAR_OUT(means1[1], 0);
        
        CHECK( ((means1[0] - mu1).norm() / mu1.norm() < 10e-1) || ((means1[0] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means1[1] - mu1).norm() / mu1.norm() < 10e-1) || ((means1[1] - mu2).norm() / mu2.norm() < 10e-1));
        
        DEBUG_OUT("running k-means with generated data and 2 means, better initial guesses...", 0);
        std::vector<Eigen::VectorXd> init;
        kmeans.getInitGuess(data, init, 2);
        CHECK(kmeans.trainKMeans(data, 2, 100));
        std::vector<Eigen::VectorXd> means1x = kmeans.getMeans();
        DEBUG_VAR_OUT(means1x[0], 0);
        DEBUG_VAR_OUT(means1x[1], 0);
        
        CHECK( ((means1x[0] - mu1).norm() / mu1.norm() < 10e-1) || ((means1x[0] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means1x[1] - mu1).norm() / mu1.norm() < 10e-1) || ((means1x[1] - mu2).norm() / mu2.norm() < 10e-1));
        
        DEBUG_OUT("running k-means with generated data and 5 means...", 0);
        CHECK(kmeans.trainKMeans(data, 5, 100));
        std::vector<Eigen::VectorXd> means2 = kmeans.getMeans();
        DEBUG_VAR_OUT(means2[0], 0);
        DEBUG_VAR_OUT(means2[1], 0);
        DEBUG_VAR_OUT(means2[2], 0);
        DEBUG_VAR_OUT(means2[3], 0);
        DEBUG_VAR_OUT(means2[4], 0);
        
        CHECK( ((means2[0] - mu1).norm() / mu1.norm() < 10e-1) || ((means2[0] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means2[1] - mu1).norm() / mu1.norm() < 10e-1) || ((means2[1] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means2[2] - mu1).norm() / mu1.norm() < 10e-1) || ((means2[2] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means2[3] - mu1).norm() / mu1.norm() < 10e-1) || ((means2[3] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means2[4] - mu1).norm() / mu1.norm() < 10e-1) || ((means2[4] - mu2).norm() / mu2.norm() < 10e-1));
        
        DEBUG_OUT("running k-means with generated data and 15 means...", 0);
        CHECK(kmeans.trainKMeans(data, 15, 100));
        std::vector<Eigen::VectorXd> means3 = kmeans.getMeans();
        DEBUG_VAR_OUT(means3[0], 0);
        DEBUG_VAR_OUT(means3[1], 0);
        DEBUG_VAR_OUT(means3[2], 0);
        DEBUG_VAR_OUT(means3[3], 0);
        DEBUG_VAR_OUT(means3[4], 0);
        DEBUG_VAR_OUT(means3[5], 0);
        DEBUG_VAR_OUT(means3[6], 0);
        DEBUG_VAR_OUT(means3[7], 0);
        DEBUG_VAR_OUT(means3[8], 0);
        DEBUG_VAR_OUT(means3[9], 0);
        DEBUG_VAR_OUT(means3[10], 0);
        DEBUG_VAR_OUT(means3[11], 0);
        DEBUG_VAR_OUT(means3[12], 0);
        DEBUG_VAR_OUT(means3[13], 0);
        DEBUG_VAR_OUT(means3[14], 0);
        
        CHECK( ((means3[0] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[0] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[1] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[1] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[2] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[2] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[3] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[3] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[4] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[4] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[5] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[5] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[6] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[6] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[7] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[7] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[8] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[8] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[9] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[9] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[10] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[10] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[11] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[11] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[12] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[12] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[13] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[13] - mu2).norm() / mu2.norm() < 10e-1));
        CHECK( ((means3[14] - mu1).norm() / mu1.norm() < 10e-1) || ((means3[14] - mu2).norm() / mu2.norm() < 10e-1));
        
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
        DEBUG_OUT("generating random numbers from uniform distribution: " << numValues, 0);
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
        
        DEBUG_OUT("generating random numbers from normal distribution: " << numValues, 0);
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
        
        //TODO: Test of multivariate normal distribution is missing. how to test it!?
        
        return EXIT_SUCCESS;
    }
}
