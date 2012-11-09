#include "tests_classifier.hpp"
#include "testframework.hpp"

#include "classifier.hpp"
#include "lda_fisher.hpp"
#include "progress_callback.hpp"
#include "gmm.hpp"
#include "kmeans.hpp"
#include "randomnumbers.hpp"

#include <fstream>
#include <vector>

namespace tests
{
    //template <typename ScalarType>
    //int GMMHelper(const std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mu1, const Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mu2, GaussianMixtureModel<ScalarType>& gmm, GaussianMixtureModel<ScalarType>& gmm2, GaussianMixtureModel<ScalarType>& gmm3, GaussianMixtureModel<ScalarType>& gmm4);
    
    int testFisherLDA()
    {
        music::FisherLDAClassifier<double>* c = new music::FisherLDAClassifier<double>(true);
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
        
        int misclassificationCount=0;
        
        DEBUG_OUT("test classification. first: class 1.", 0);
        for (int i=0; i<1000; i++)
        {
            vec2.setRandom();
            if (c->classifyVector(vec2) >= 0.0)
                misclassificationCount++;
        }
        CHECK_OP(misclassificationCount, <, 5);
        misclassificationCount = 0;
        DEBUG_OUT("test classification. second: class 2.", 0);
        for (int i=0; i<1000; i++)
        {
            vec2.setRandom();
            vec2 += vec1;
            if (c->classifyVector(vec2) < 0.0)
                misclassificationCount++;
        }
        CHECK_OP(misclassificationCount, <, 5);
        
        return EXIT_SUCCESS;
    }
    
    template <typename ScalarType> 
    int GMMHelper(std::vector<Eigen::Matrix<ScalarType, Eigen::Dynamic, 1> >& data, Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mu1, Eigen::Matrix<ScalarType, Eigen::Dynamic, 1>& mu2, music::GaussianMixtureModel<ScalarType>& gmm, music::GaussianMixtureModel<ScalarType>& gmm2, music::GaussianMixtureModel<ScalarType>& gmm3, music::GaussianMixtureModel<ScalarType>& gmm4)
    {
        int dataCount = data.size();
        
        //train GMM with data
        DEBUG_OUT("training gmm...", 0);
        gmm.trainGMM(data, 2);
        DEBUG_OUT("training done.", 0);
        
        //test if the GMM converged the right way, or not. test data.
        std::vector<music::Gaussian<ScalarType>*> gaussians = gmm.getGaussians();
        
        for (typename std::vector<music::Gaussian<ScalarType>*>::iterator it = gaussians.begin(); it != gaussians.end(); it++)
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
        
        for (typename std::vector<music::Gaussian<ScalarType>*>::iterator it = gaussians.begin(); it != gaussians.end(); it++)
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
        gmm2.loadFromJSONString(gmmJSON);
        
        gaussians = gmm2.getGaussians();
        
        for (typename std::vector<music::Gaussian<ScalarType>*>::iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            DEBUG_OUT("gaussian mean: " << (*it)->getMean(), 0);
            DEBUG_OUT("gaussian sigma: " << (*it)->getCovarianceMatrix(), 0);
            DEBUG_OUT("testing if the estimated mean fits to the input mean of the clusters...", 0);
            CHECK( (((**it).getMean() - mu1).norm() / mu1.norm() < 10e-1) || (((**it).getMean() - mu2).norm() / mu2.norm() < 10e-1));
        }
        
        gmm3.trainGMM(data, 5);
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
    
    int testGMM()
    {
        int dimension=12;
        int dataCount=10000;
        int gaussianCount=3;
        
        music::GaussianMixtureModelDiagCov<kiss_fft_scalar> gmm1;
        music::GaussianMixtureModelDiagCov<kiss_fft_scalar> gmm2;
        music::KMeans<kiss_fft_scalar> kmeans;
        music::GaussianDiagCov<kiss_fft_scalar> gdc1(dimension);
        music::GaussianDiagCov<kiss_fft_scalar> gdc2(dimension);
        music::GaussianDiagCov<kiss_fft_scalar> gdc3(dimension);
        
        //generate data
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> mu1(dimension);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> mu2(dimension);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> mu3(dimension);
        
        mu1 << 500, -500, 200, 150,  100, 50, 1, 20, 50, -10, 10, 350;
        mu2 << 100,  100, 900, 190,  -60, 90, 8, 10, 20, -30, 70, 800;
        mu3 << 800,-1000,-300,  50, -100,-50, 5, 30, 80, -50, 30, 300;
        
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> cov1(dimension);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> cov2(dimension);
        Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> cov3(dimension);
        
        cov1 << 500,   2000, 200, 150,  100, 50, 1, 20, 50, 100, 8000, 550;
        cov2 << 1000,  1000, 900, 190,   60, 90, 8, 10, 50, 100, 8000, 550;
        cov3 << 10000, 1000, 300,  50,  100, 50, 5, 30, 50, 100, 8000, 550;
        
        gdc1.setMean(mu1);
        gdc1.setCovarianceMatrix(cov1.asDiagonal());
        gdc2.setMean(mu2);
        gdc2.setCovarianceMatrix(cov2.asDiagonal());
        gdc3.setMean(mu3);
        gdc3.setCovarianceMatrix(cov3.asDiagonal());
        
        
        std::vector<Eigen::Matrix<kiss_fft_scalar, Eigen::Dynamic, 1> > data;
        for (int i=0; i<dataCount/3; i++)
        {
            data.push_back(gdc1.rand());
            data.push_back(gdc2.rand());
            data.push_back(gdc3.rand());
        }
        
        
        /*
        for (typename std::vector<Eigen::VectorXd>::iterator it = data.begin(); it != data.end(); it++)
        {
            DEBUG_VAR_OUT((*it).transpose(), 0);
        }*/
        
        /*
        kmeans.trainKMeans(data, 10);
        std::vector<Eigen::VectorXd> means = kmeans.getMeans();
        for (typename std::vector<Eigen::VectorXd>::iterator it = means.begin(); it != means.end(); it++)
        {
            DEBUG_VAR_OUT((*it).transpose(), 0);
        }
        */
        
        DEBUG_OUT("training GMM 1...", 10);
        gmm1.trainGMM(data, gaussianCount);
        DEBUG_OUT("training GMM 2...", 10);
        gmm2.trainGMM(data, gaussianCount);
        DEBUG_OUT("results of GMM 1:", 10);
        std::vector<music::Gaussian<kiss_fft_scalar>*> gaussians = gmm1.getGaussians();
        for (typename std::vector<music::Gaussian<kiss_fft_scalar>*>::iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            DEBUG_OUT("gaussian mean: " << (*it)->getMean(), 0);
            DEBUG_OUT("gaussian sigma: " << (*it)->getCovarianceMatrix(), 0);
        }
        DEBUG_OUT("results of GMM 2:", 10);
        gaussians = gmm2.getGaussians();
        for (typename std::vector<music::Gaussian<kiss_fft_scalar>*>::iterator it = gaussians.begin(); it != gaussians.end(); it++)
        {
            DEBUG_OUT("gaussian mean: " << (*it)->getMean(), 0);
            DEBUG_OUT("gaussian sigma: " << (*it)->getCovarianceMatrix(), 0);
        }
        DEBUG_VAR_OUT(gmm1.compareTo(gmm2), 0);
        DEBUG_VAR_OUT(gmm2.compareTo(gmm1), 0);
        
        
        return EXIT_FAILURE;
        
        #if 0
        DEBUG_OUT("running test for gaussian mixture models...", 0);
        music::GaussianMixtureModelFullCov<double> fullgmmd;
        music::GaussianMixtureModelFullCov<double> fullgmmd2;
        music::GaussianMixtureModelFullCov<double> fullgmmd3;
        music::GaussianMixtureModelFullCov<double> fullgmmd4;
        music::GaussianMixtureModelFullCov<float>  fullgmmf;
        music::GaussianMixtureModelFullCov<float>  fullgmmf2;
        music::GaussianMixtureModelFullCov<float>  fullgmmf3;
        music::GaussianMixtureModelFullCov<float>  fullgmmf4;
        music::GaussianMixtureModelDiagCov<double> diaggmmd;
        music::GaussianMixtureModelDiagCov<double> diaggmmd2;
        music::GaussianMixtureModelDiagCov<double> diaggmmd3;
        music::GaussianMixtureModelDiagCov<double> diaggmmd4;
        music::GaussianMixtureModelDiagCov<float>  diaggmmf;
        music::GaussianMixtureModelDiagCov<float>  diaggmmf2;
        music::GaussianMixtureModelDiagCov<float>  diaggmmf3;
        music::GaussianMixtureModelDiagCov<float>  diaggmmf4;
        srand(time(NULL));
        
        //create data. don't have a generator for multivariate gaussian values, just taking equally distributed data (uncorrelated)
        //set these values to change how much data will be generated, and how many dimensions you have
        int dimension = 8;
        int dataCount = 10000;
        
        Eigen::Matrix<double, Eigen::Dynamic, 1> dataVectord(dimension);
        Eigen::Matrix<float , Eigen::Dynamic, 1> dataVectorf(dimension);
        std::vector<Eigen::Matrix<double, Eigen::Dynamic, 1> > datad;
        std::vector<Eigen::Matrix<float , Eigen::Dynamic, 1> > dataf;
        
        DEBUG_OUT("adding " << dataCount << " data vectors...", 0);
        //calc mu1 and mu2...
        Eigen::Matrix<double, Eigen::Dynamic, 1> mu1d(dimension);
        Eigen::Matrix<double, Eigen::Dynamic, 1> mu2d(dimension);
        Eigen::Matrix<float , Eigen::Dynamic, 1> mu1f(dimension);
        Eigen::Matrix<float , Eigen::Dynamic, 1> mu2f(dimension);
        for (int j=0; j<dimension; j++)
        {
            mu1d[j] = -0.5 + double(rand() % 1000) / 1000.0;
            mu2d[j] = 3.0 - 0.5 + double(rand() % 1000) / 1000.0;
            mu1f[j] = mu1d[j];
            mu2f[j] = mu2d[j];
        }
        DEBUG_OUT("mu1d = " << mu1d, 0);
        DEBUG_OUT("mu2d = " << mu2d, 0);
        DEBUG_OUT("mu1f = " << mu1f, 0);
        DEBUG_OUT("mu2fff " << mu2f, 0);
        
        //distribution 1
        DEBUG_OUT(dataCount / 4 << " for distribution 1...", 0);
        Eigen::Matrix<double, Eigen::Dynamic, 1> meanVecd = Eigen::Matrix<double, Eigen::Dynamic, 1>::Zero(dimension);
        Eigen::Matrix<double, Eigen::Dynamic, 1> varVecd  = Eigen::Matrix<double, Eigen::Dynamic, 1>::Zero(dimension);
        Eigen::Matrix<float , Eigen::Dynamic, 1> meanVecf = Eigen::Matrix<float , Eigen::Dynamic, 1>::Zero(dimension);
        Eigen::Matrix<float , Eigen::Dynamic, 1> varVecf  = Eigen::Matrix<float , Eigen::Dynamic, 1>::Zero(dimension);
        for (int i=0; i<dataCount/4; i++)
        {
            for (int j=0; j<dimension; j++)
            {
                dataVectord[j] = -0.5 + double(rand() % 1000) / 1000.0;
                dataVectorf[j] = dataVectord[j];
            }
            meanVecd = meanVecd + dataVectord + mu1d;
            varVecd = varVecd + (dataVectord).array().pow(2.0).matrix();
            datad.push_back(dataVectord + mu1d);
            
            meanVecf = meanVecf + dataVectorf + mu1f;
            varVecf = varVecf + (dataVectorf).array().pow(2.0).matrix();
            dataf.push_back(dataVectorf + mu1f);
        }
        DEBUG_OUT("mean of distribution 1d = " << meanVecd / (dataCount/4), 0);
        DEBUG_OUT("estimated variance of distribution 1d = " << varVecd / (dataCount/4), 0);
        DEBUG_OUT("mean of distribution 1f = " << meanVecf / (dataCount/4), 0);
        DEBUG_OUT("estimated variance of distribution 1f = " << varVecf / (dataCount/4), 0);
        
        meanVecd = Eigen::Matrix<double, Eigen::Dynamic, 1>::Zero(dimension);
        varVecd  = Eigen::Matrix<double, Eigen::Dynamic, 1>::Zero(dimension);
        meanVecf = Eigen::Matrix<float , Eigen::Dynamic, 1>::Zero(dimension);
        varVecf  = Eigen::Matrix<float , Eigen::Dynamic, 1>::Zero(dimension);
        //distribution 2
        DEBUG_OUT(3*dataCount / 4 << " for distribution 2...", 0);
        for (int i=dataCount/4; i<dataCount; i++)
        {
            for (int j=0; j<dimension; j++)
            {
                dataVectord[j] = -1.0 + double(rand() % 2000) / 1000.0;
                dataVectorf[j] = dataVectord[j];
            }
            meanVecd = meanVecd + dataVectord + mu2d;
            varVecd = varVecd + (dataVectord).array().pow(2.0).matrix();
            datad.push_back(dataVectord + mu2d);
            
            meanVecf = meanVecf + dataVectorf + mu2f;
            varVecf = varVecf + (dataVectorf).array().pow(2.0).matrix();
            dataf.push_back(dataVectorf + mu2f);
        }
        DEBUG_OUT("mean of distribution 2d = " << meanVecd / (3*dataCount/4), 0);
        DEBUG_OUT("estimated variance of distribution 2d = " << varVecd / (3*dataCount/4), 0);
        DEBUG_OUT("mean of distribution 2f = " << meanVecf / (3*dataCount/4), 0);
        DEBUG_OUT("estimated variance of distribution 2f = " << varVecf / (3*dataCount/4), 0);
        
        
        std::vector<Eigen::Matrix<double, Eigen::Dynamic, 1> > datadcopy = datad;
        std::vector<Eigen::Matrix<float , Eigen::Dynamic, 1> > datafcopy = dataf;
        
        Eigen::Matrix<double, Eigen::Dynamic, 1> mu1dcopy = mu1d;
        Eigen::Matrix<double, Eigen::Dynamic, 1> mu2dcopy = mu2d;
        Eigen::Matrix<float , Eigen::Dynamic, 1> mu1fcopy = mu1f;
        Eigen::Matrix<float , Eigen::Dynamic, 1> mu2fcopy = mu2f;
        
        int retVal = 0;
        DEBUG_OUT("running test for GMMs with double and full covariance matricies...", 10);
        retVal += GMMHelper<double>(datadcopy, mu1dcopy, mu2dcopy, fullgmmd, fullgmmd2, fullgmmd3, fullgmmd4);
        CHECK_EQ(retVal, 0);
        DEBUG_OUT("running test for GMMs with float and full covariance matricies...", 10);
        retVal += GMMHelper<float> (datafcopy, mu1fcopy, mu2fcopy, fullgmmf, fullgmmf2, fullgmmf3, fullgmmf4);
        CHECK_EQ(retVal, 0);
        
        datadcopy = datad;
        datafcopy = dataf;
        
        mu1dcopy = mu1d;
        mu2dcopy = mu2d;
        mu1fcopy = mu1f;
        mu2fcopy = mu2f;
        
        DEBUG_OUT("running test for GMMs with double and diagonal covariance matricies...", 10);
        retVal += GMMHelper<double>(datadcopy, mu1dcopy, mu2dcopy, diaggmmd, diaggmmd2, diaggmmd3, diaggmmd4);
        CHECK_EQ(retVal, 0);
        DEBUG_OUT("running test for GMMs with float and diagonal covariance matricies...", 10);
        retVal += GMMHelper<float> (datafcopy, mu1fcopy, mu2fcopy, diaggmmf, diaggmmf2, diaggmmf3, diaggmmf4);
        CHECK_EQ(retVal, 0);
        DEBUG_OUT("test finished.", 10);
        
        return retVal;
        #endif
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
        kmeans.calculateInitGuess(data, init, 2);
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
