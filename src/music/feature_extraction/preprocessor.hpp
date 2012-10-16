#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <string>
#include "databaseconnection.hpp"
#include "progress_callback.hpp"
#include "constantq.hpp"

namespace music
{
    /**
     * @brief This class preprocesses files, extracts their features and
     *      adds them to the database.
     * 
     * Use this class to easily access the database to add files.
     * It will extract all the features from the files and save them directly to
     * the database, you don't have to worry about the details. The standard settings
     * of this class should be sufficient for most classification purposes.
     * You can change the most prominent settings in the constructor or via
     * the getter/setter functions, if you want to.
     * 
     * Example:
     * @code
     * #include "music.hpp"
     * using namespace music;
     * //...
     * DatabaseConnection* conn = new SQLiteDatabaseConnection();
     * conn->open("database.db");
     * FilePreprocessor processor(conn);
     * databaseentities::id_datatype recordingID;
     * if (!processor.preprocessFile("file.mp3", recordingID))
     *     std::cerr << "adding of file.mp3 failed!" << std::endl;
     * else
     *     std::cout << "file.mp3 added, recording id is " << recordingID << std::endl;
     * @endcode
     * 
     * If you need greater control
     * of the features, or need the features themselves,
     * you need to re-implement the behaviour of this class.
     * 
     * You may set the
     * dimension and time slice size of the timbre vectors, as well as
     * the size of the model that will be built. The other features do not
     * need fine-tuning.
     * 
     * @see TimbreModel
     * @see BPMEstimator
     * @see DynamicRangeCalculator
     * @see DatabaseConnection
     * 
     * @ingroup feature_extraction
     * @ingroup database
     * 
     * @author Lena Brueder
     * @date 2012-08-13
     */
    class FilePreprocessor
    {
    private:
        
    protected:
        musicaccess::IIRFilter* lowpassFilter;
        ConstantQTransform* cqt;
        DatabaseConnection* conn;
        
        unsigned int timbreModelSize;
        unsigned int timbreDimension;
        double timbreTimeSliceSize;
    public:
        /**
         * @brief Constructs a new FilePreprocessor object.
         * 
         * @param conn A pointer to the database connection. May not be null.
         * @param timbreModelSize The size of the timbre model, i.e. the
         *      number of normal distributions in the gaussian mixture model
         * @param timbreDimension The dimension of the timbre vectors.
         * @param timeSliceSize The size of the time slices for one timbre vector in seconds.
         * 
         * @see GaussianMixtureModel
         * @see TimbreEstimator
         */
        FilePreprocessor(DatabaseConnection* conn, unsigned int timbreModelSize = 20, unsigned int timbreDimension = 20, double timeSliceSize = 0.01);
        ~FilePreprocessor();
        /**
         * @brief Extracts the features from the file and adds them to the database.
         * 
         * This function first decodes the file and resamples it
         * to 22kHz/mono, then
         * calculates the constant Q transform and
         * extracts all supported features. For now, the features are:
         *  * Dynamic Range
         *  * Tempo in BPM
         *  * Timbre Vectors / Timbre Vector Model
         * 
         * The timbre features are more complicated then the other ones,
         * and they might need some fine-tuning, so their parameters
         * are accessible via the getter/setter functions of this class.
         * 
         * See the documentation of <code>libmusicaccess</code> for a list of
         * supported audio codecs.
         * 
         * @return <code>true</code> if the operation succeeded,
         *      <code>false</code> otherwise
         */
        bool preprocessFile(std::string filename, databaseentities::id_datatype& recordingID, ProgressCallbackCaller* callback = NULL);
        
        /**
         * @brief Sets the time slice size for timbre extraction.
         * 
         * Typical values are in the range of <code>0.005 - 0.03</code>.
         */
        void setTimbreTimeSliceSize(double timeSliceSize) {this->timbreTimeSliceSize = timeSliceSize;}
        /**
         * @brief Returns the time slice size for timbre extraction.
         * @return the time slice size for timbre extraction.
         */
        unsigned int getTimbreTimeSliceSize()             {return timbreTimeSliceSize;}
        
        /**
         * @brief Sets the dimension of the timbre vectors.
         * 
         * Typical values are in the range of <code>4-40</code>.
         */
        void setTimbreDimension(unsigned int dimension)   {this->timbreDimension = dimension;}
        /**
         * @brief Returns the dimension of the timbre vectors.
         * @return the dimension of the timbre vectors.
         */
        unsigned int getTimbreDimension()                 {return timbreDimension;}
        
        /**
         * @brief Sets the model size for the timbre vector model.
         * 
         * Typical values are in the range of <code>5-50</code>.
         * You may set larger values, but you risk overfitting.
         */
        void setTimbreModelSize(unsigned int modelSize)   {this->timbreModelSize = modelSize;}
        /**
         * @brief Returns the model size for the timbre vector model.
         * @return the model size for the timbre vector model.
         */
        unsigned int getTimbreModelSize()                 {return timbreModelSize;}
    };
}

#endif //PREPROCESSOR_HPP
