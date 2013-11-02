#ifndef SOUNDFILE_HPP
#define SOUNDFILE_HPP

#include <stdint.h>
#include <string>
#include <mpg123.h>
#include <sndfile.h>

namespace musicaccess
{
    /**
     * @brief Objects of this class hold metadata of a SoundFile.
     * 
     * @author Lena Brueder
     * @date 2012-07-19
     */
    class SoundFileMetadata
    {
    private:
        std::string title;
        std::string artist;
        std::string album;
        std::string genre;
        std::string filename;
        std::string year;
        std::string track;
    protected:
        
    public:
        std::string getTitle() const;
        std::string getArtist() const;
        std::string getAlbum() const;
        std::string getGenre() const;
        std::string getFilename() const;
        std::string getYear() const;
        std::string getTrack() const;
        
        void setTitle(const std::string& title);
        void setArtist(const std::string& artist);
        void setAlbum(const std::string& album);
        void setGenre(const std::string& genre);
        void setFilename(const std::string& filename);
        void setYear(const std::string& year);
        void setTrack(const std::string& track);
    };
    
    enum SOUNDFILE_DATATYPE
    {
        DATATYPE_MPG123,
        DATATYPE_SNDFILE,
        DATATYPE_UNKNOWN
    };

    /**
     * @brief This is a class for music files which allows to read
     *      data from a number of different music file formats.
     * 
     * Endianess if automatically converted to the correct format for the CPU.
     * 
     * @code
     * SoundFile soundfile;
     * 
     * soundfile.open("test.mp3");
     * int16_t* buffer = new int16_t[soundfile.getSampleCount()];
     * soundfile.readSamples(buffer, soundfile.getSampleCount());
     * 
     * //now, you can use buffer to access the samples.
     * //note that the samples of the different channels are interleaved.
     * 
     * //do some processing
     * 
     * delete[] buffer;
     * soundfile.close();
     * @endcode
     * 
     * 
     * @remarks Using this class, you will get the data in an integer format,
     * regardless of the possibilities in the underlying format.
     */
    class SoundFile
    {
    private:
        int channelCount;
        int sampleSize;   //may be any of 1, 2, 4 bytes.
        long sampleCount;
        long sampleRate;
        
        uint32_t position;
        
        bool fileOpen;
        
        bool decodeToFloat;
        
        SOUNDFILE_DATATYPE dataType;
        
        mpg123_handle* mpg123Handle;
        SNDFILE*       sndfileHandle;
        
        SoundFileMetadata* metadata;
        
        /**
         * @brief This class will be used to call to initialization functions of
         *      mpg123 and libsndfile if they haven't been called before and
         *      to call to destruction functions.
         * 
         * This class counts how often initialize() and destroy() have been called.
         * Every time the internal counter reaches zero, it will call the destruction
         * functions of the libraries. If the internal counter leaves zero, the
         * initialization functions will be called.
         * 
         */
        class SingletonInitializer
        {
        private:
            static SingletonInitializer* instance;
            SingletonInitializer();
            int counter;
        public:
            static void initialize();
            static void destroy();
            ~SingletonInitializer();
        };
        std::string mpg123_stringToStdString(mpg123_string* str);
    public:
        SoundFile();
        ~SoundFile();
        
        /**
         * @brief Opens a music file.
         * 
         * This function first tries to determine which underlying library it should
         * use to open the file dependend on the extension of the file.
         * if the extension is <code>.mp3</code>, it uses libmpg123, otherwise it
         * uses libsndfile.
         * 
         * @param filename The filename you want to open.
         * @param decodeToFloat Determines, wether you want to decode to float or to integer numbers.
         * @return <code>true</code>, if opening the file was successful, <code>false</code> otherwise.
         */
        bool open(const std::string&, bool decodeToFloat=false);
        
        /**
         * @brief Closes an opened music file.
         */
        bool close();
        
        /**
         * @brief Returns if a file has been opened, or not.
         * 
         * @return if a file has been opened
         */
        bool isFileOpen() {return fileOpen;}
        
        /**
         * @brief Returns the channel count of the music file.
         * 
         * @remarks If no file has been opened, the return value is undefined.
         * @return The channel count of the music file.
         */
        int getChannelCount() {return channelCount;}
        
        /**
         * @brief Retuns the sample count of the music file.
         * 
         * This value is the count of all samples together. A file
         * with 10 samples and 2 channels has 5 samples per channel.
         * 
         * @remarks If no file has been opened, the return value is undefined.
         * @return the sample count
         */
        unsigned long getSampleCount() {return sampleCount;}
        
        /**
         * @brief Returns the size of a sample, in bytes.
         * 
         * This effectively returns what the maximal and minimal values are
         * that you can expect from getSample(). You should be able to
         * choose the best data type depended on this value.
         * 
         * 1 means 8 bit samples, and so on.
         * 
         * @remarks If no file has been opened, the return value is undefined.
         * @return 1, 2 or 4 - dependend on the sample size in the file.
         */
        int getSampleSize() {return sampleSize;}
        
        /**
         * @brief Returns the sample rate of the file, in Hz.
         * 
         * 
         * 
         * 
         * @remarks If no file has been opened, the return value is undefined.
         * @remarks There are some file types out there which are able to change the
         *      sample rate over time. This function does not reflect this fact.
         * @return the sample rate in Hz.
         */
        long getSampleRate() {return sampleRate;}
        
        /**
         * @brief Returns the sample reader position within the file.
         * 
         * Use this to determine how far you have gotten while reading the file contents.
         * That position is an absolute position. It does not take care of
         * how many channels you have, and so on.
         * 
         * @remarks If no file has been opened, the return value is undefined.
         * @return The position within the file.
         */
        uint32_t getPosition() {return position;}
        
        /**
         * @brief Returns the next samples, formatted as int16_t.
         * 
         * This functions writes the next <code>count</code> samples to the
         * supplied blockAddress. it will not take care of how the samples are
         * arranged, so you have to take care of that for yourself.
         * 
         * @param buffer The buffer where to save the data. You have to make
         *      sure that there is enough memory to write to.
         * @param count The sample count that should be read.
         * @remarks If no file has been opened, the return value is undefined.
         * @remarks This function will return 16bit integers, regardless of the input
         *      format. Take a look at getSampleSize() to see what resolution the samples
         *      have.
         * @return the sample count that actually was read
         */
        size_t readSamples(int16_t* buffer, unsigned int count);
        
        /**
         * @brief Returns the next samples, formatted as float.
         * 
         * This functions writes the next <code>count</code> samples to the
         * supplied blockAddress. it will not take care of how the samples are
         * arranged, so you have to take care of that for yourself.
         * 
         * @param buffer The buffer where to save the data. You have to make
         *      sure that there is enough memory to write to.
         * @param count The sample count that should be read.
         * @remarks If no file has been opened, the return value is undefined.
         * @remarks This function will return 16bit integers, regardless of the input
         *      format. Take a look at getSampleSize() to see what resolution the samples
         *      have.
         * @return the sample count that actually was read
         */
        size_t readSamples(float* buffer, unsigned int count);
        
        /**
         * @brief Returns the metadata of the opened sound file (if any)
         * @remarks For now, metadata is only supported for mp3 files. id3v2
         *      is preferred if both id3v1 and id3v2 tags are available.
         * @return The metadata of the sound file, or NULL if there is no metadata.
         */
        SoundFileMetadata* getMetadata();
    };
}
#endif  //SOUNDFILE_HPP 
