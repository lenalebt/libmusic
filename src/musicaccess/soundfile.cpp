#include "soundfile.hpp"
#include "stringhelper.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include "debug.hpp"

#ifdef HAVE_VORBISFILE
    #include "vorbisfile.h"
    #include "vorbis/codec.h"
#endif

namespace musicaccess
{
    SoundFile::SoundFile() :
        channelCount(0), sampleSize(0), sampleCount(0),
        sampleRate(0), position(0), fileOpen(false),
        dataType(DATATYPE_UNKNOWN),
        mpg123Handle(NULL), sndfileHandle(NULL),
        metadata(NULL)
    {
        //will init mpg123 and sndfile if necessary
        SingletonInitializer::initialize();
    }
    SoundFile::~SoundFile()
    {
        if (fileOpen)
            close();
        if (metadata != NULL)
        {
            delete metadata;
            metadata = NULL;
        }
        //will destroy mpg123 and sndfile if necessary
        SingletonInitializer::destroy();
    }

    bool SoundFile::open(const std::string& filename, bool decodeToFloat)
    {
        //first: close open files, if any.
        if (fileOpen)
        {
            if (!close())
                return false;
        }
        
        this->decodeToFloat = decodeToFloat;
        
        std::string loweredFilename(filename);
        tolower(loweredFilename);
        
        //std::cerr << loweredFilename << std::endl;
        
        //now open the file - take libmpg123 or libsndfile.
        if (endsWith(loweredFilename, ".mp3"))
        {   //use libmpg123
            dataType = DATATYPE_MPG123;
            int error;
            mpg123_id3v1* id3v1;
            mpg123_id3v2* id3v2;
            int meta;
            mpg123Handle = mpg123_new(NULL, &error);
            mpg123_param(mpg123Handle, MPG123_ADD_FLAGS, MPG123_QUIET, MPG123_QUIET);
            if (error != MPG123_OK)
            {
                std::cerr << "mpg123: setup failed, " << mpg123_plain_strerror(error) << std::endl;
                fileOpen = false;
                return false;
            }
            
            //force mpg123 to use float if chosen.
            if (decodeToFloat)
                mpg123_param(mpg123Handle, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT, 0.0);
            
            //open file
            error = mpg123_open(mpg123Handle, filename.c_str());
            if (error != MPG123_OK)
            {
                std::cerr << "mpg123: open failed, " << mpg123_plain_strerror(error) << std::endl;
                fileOpen = false;
                return false;
            }
            
            //need this to find id3 tags
            mpg123_scan(mpg123Handle);
            
            meta = mpg123_meta_check(mpg123Handle);
            if ((meta & MPG123_ID3) && (mpg123_id3(mpg123Handle, &id3v1, &id3v2) == MPG123_OK))
            {
                metadata = new SoundFileMetadata();
                metadata->setFilename(filename);
                if (id3v2 != NULL)
                {
                    metadata->setTitle(mpg123_stringToStdString(id3v2->title));
                    metadata->setArtist(mpg123_stringToStdString(id3v2->artist));
                    metadata->setAlbum(mpg123_stringToStdString(id3v2->album));
                    metadata->setYear(mpg123_stringToStdString(id3v2->year));
                    metadata->setGenre(mpg123_stringToStdString(id3v2->genre));
                }
                else
                {
                    if (id3v1 != NULL)
                    {
                        metadata->setTitle(std::string(id3v1->title));
                        metadata->setArtist(std::string(id3v1->artist));
                        metadata->setAlbum(std::string(id3v1->album));
                        metadata->setYear(std::string(id3v1->year));
                        metadata->setGenre("-not yet implemented-");    //TODO: Convert genre.
                    }
                }
            }
            
            int encoding;
            error = mpg123_getformat(mpg123Handle, &sampleRate, &channelCount, &encoding);
            if (error != MPG123_OK)
            {
                std::cerr << "mpg123: getting file info failed, " << mpg123_plain_strerror(error) << std::endl;
                fileOpen = false;
                return false;
            }
            
            if (decodeToFloat)
            {
                if (encoding != MPG123_ENC_FLOAT_32)
                {
                    std::cerr << "mpg123: wrong data encoding, requested float. got 0x" << std::hex << encoding << std::endl;
                    fileOpen = false;
                    return false;
                }
            }
            else
            {
                if (encoding != MPG123_ENC_SIGNED_16)
                {
                    std::cerr << "mpg123: wrong data encoding, requested 16bit signed integer. got 0x" << std::hex << encoding << std::endl;
                    fileOpen = false;
                    return false;
                }
            }

            
            //force mpg123 to not change the encoding or other things while decoding
            mpg123_format_none(mpg123Handle);
            mpg123_format(mpg123Handle, sampleRate, channelCount, encoding);
            
            //get the length of the file (if available!)
            sampleCount = mpg123_length(mpg123Handle);
            if (sampleCount == MPG123_ERR)
            {
                std::cerr << "mpg123: was not able to get the length of the file. aborting." << std::endl;
                fileOpen = false;
                return false;
            }
            sampleCount *= channelCount;    //fix the count. could not multiply before if().
            
            //get the size of one sample
            sampleSize = mpg123_encsize(encoding);
            
            position = 0;
            
            //okay, done now. file is open!
            
            fileOpen = true;
            return true;
        }
        else
        {   //use libsndfile
            dataType = DATATYPE_SNDFILE;
            
            #ifdef HAVE_VORBISFILE
            if (endsWith(loweredFilename, ".ogg"))
            {
                metadata = new SoundFileMetadata();
                metadata->setFilename(filename);
                
                OggVorbis_File vf;
                if (ov_fopen(filename.c_str(), &vf) != 0)
                    return false;
                
                vorbis_comment* comment = NULL;
                comment = ov_comment(&vf, -1);
                
                for (int i=0; i<comment->comments; i++)
                {
                    std::string actComment(comment->user_comments[i]);
                    DEBUG_OUT("vorbis_comment: " << actComment, 15);
                    
                    std::string loweredComment = actComment;
                    tolower(loweredComment);
                    
                    if (loweredComment.substr(0, 6) == "title=")
                    {
                        metadata->setTitle(actComment.substr(6, std::string::npos));
                        DEBUG_OUT("  title:  " << metadata->getTitle(), 15);
                    }
                    else if (loweredComment.substr(0, 7) == "artist=")
                    {
                        metadata->setArtist(actComment.substr(7, std::string::npos));
                        DEBUG_OUT("  artist: " << metadata->getArtist(), 15);
                    }
                    else if (loweredComment.substr(0, 6) == "album=")
                    {
                        metadata->setAlbum(actComment.substr(6, std::string::npos));
                        DEBUG_OUT("  album:  " << metadata->getAlbum(), 15);
                    }
                    else if (loweredComment.substr(0, 6) == "genre=")
                    {
                        metadata->setGenre(actComment.substr(6, std::string::npos));
                        DEBUG_OUT("  genre:  " << metadata->getGenre(), 15);
                    }
                    else if (loweredComment.substr(0, 6) == "track=")
                    {
                        metadata->setTrack(actComment.substr(6, std::string::npos));
                        DEBUG_OUT("  track: " << metadata->getTrack(), 15);
                    }
                }
                
                vorbis_comment_clear(comment);
                ov_clear(&vf);
            }
            #endif  //HAVE_VORBISFILE
            
            SF_INFO sfinfo;
            sfinfo.format = 0;  //documentation says I need to do so
            sndfileHandle = sf_open(filename.c_str(), SFM_READ, &sfinfo);
            
            position = 0;
            sampleRate = sfinfo.samplerate;
            channelCount = sfinfo.channels;
            sampleCount = sfinfo.frames * channelCount;
            
            if (decodeToFloat)
                sampleSize = 4;
            else
                sampleSize = 2;
            
            //okay, done now. file is open! somehow less work than with libmpg123.
            
            fileOpen = true;
            return true;
        }
        
        fileOpen = false;
        return false;
    }

    bool SoundFile::close()
    {
        if (metadata != NULL)
        {
            delete metadata;
            metadata = NULL;
        }
        
        if (fileOpen)
        {
            if (dataType == DATATYPE_MPG123)
            {
                mpg123_close(mpg123Handle);
                mpg123_delete(mpg123Handle);
                
                mpg123Handle = NULL;
                sndfileHandle = NULL;
                dataType = DATATYPE_UNKNOWN;
                fileOpen = false;
                
                return true;
            }
            else if (dataType == DATATYPE_SNDFILE)
            {
                sf_close(sndfileHandle);
                
                mpg123Handle = NULL;
                sndfileHandle = NULL;
                dataType = DATATYPE_UNKNOWN;
                fileOpen = false;
                
                return true;
            }
            else
            {
                //do nothing.
                mpg123Handle = NULL;
                sndfileHandle = NULL;
                dataType = DATATYPE_UNKNOWN;
                fileOpen = false;
                return true;
            }
        }
        else
            return true;
    }

    size_t SoundFile::readSamples(int16_t* buffer, unsigned int count)
    {
        if (!fileOpen)
        {
            std::cerr << "trying to read without opening a file!" << std::endl;
            return 0;
        }
        
        if (dataType == DATATYPE_MPG123)
        {   //return data from libmpg123
            int error;
            size_t bytesRead;
            size_t framesRead;
            
            if (!decodeToFloat)
            {
                error = mpg123_read( mpg123Handle, (unsigned char*)buffer, count*sampleSize, &bytesRead );
                framesRead = bytesRead / sampleSize;
            }
            else
            {
                //read as float and reformat to int16_t
                float* floatBuffer = new float[count];
                error = mpg123_read( mpg123Handle, (unsigned char*)floatBuffer, count*sizeof(float), &bytesRead );
                for (unsigned int i=0; i<count; i++)
                {
                    buffer[i] = 32768.0 * floatBuffer[i];
                }
                delete[] floatBuffer;
                framesRead = bytesRead / sizeof(float);
            }
            
            position += framesRead;
            
            if (error == MPG123_DONE)
            {   //okay, decoding finished
                return framesRead;
            }
            else if (error == MPG123_OK)
            {   //okay, more bytes follow
                return framesRead;
            }
            else if (error == MPG123_NEED_MORE)
            {   //okay, seems as if the stream is damaged, but stopping here should be okay.
                return framesRead;
            }
            else if (error == MPG123_ERR)
            {   //okay, seems as if the stream is severely damaged, but stopping here should be okay in many cases.
                return framesRead;
            }
            else
            {
                std::cerr << "mpg123: error while reading at position " << position << ": " << error << ", \"" << mpg123_plain_strerror(error) << "\"" << std::endl;
                return 0;
            }
        }
        else if (dataType == DATATYPE_SNDFILE)
        {   //return data from libsndfile
            int itemsRead = sf_read_short(sndfileHandle, buffer, count);
            position += itemsRead;
            return itemsRead;
        }
        else
        {
            std::cerr << "trying to read unknown datatype!" << std::endl;
            return 0;
        }
    }
    
    size_t SoundFile::readSamples(float* buffer, unsigned int count)
    {
        if (!fileOpen)
        {
            std::cerr << "trying to read without opening a file!" << std::endl;
            return 0;
        }
        
        if (dataType == DATATYPE_MPG123)
        {   //return data from libmpg123
            int error;
            size_t bytesRead;
            size_t framesRead;
            
            if (decodeToFloat)
            {
                error = mpg123_read( mpg123Handle, (unsigned char*)buffer, count*sampleSize, &bytesRead );
                framesRead = bytesRead / sampleSize;
            }
            else
            {
                //read as int16_t and reformat as float
                int16_t* intBuffer = new int16_t[count];
                error = mpg123_read( mpg123Handle, (unsigned char*)intBuffer, count*sizeof(int16_t), &bytesRead );
                for (unsigned int i=0; i<count; i++)
                {
                    buffer[i] = float(intBuffer[i]) / 32768.0;
                }
                delete[] intBuffer;
                framesRead = bytesRead / sizeof(int16_t);
            }
            
            position += framesRead;
            
            if (position >= count)
            {
                return framesRead;
            }
            else if (error == MPG123_DONE)
            {   //okay, decoding finished
                return framesRead;
            }
            else if (error == MPG123_OK)
            {   //okay, more bytes follow
                return framesRead;
            }
            else if (error == MPG123_NEED_MORE)
            {   //okay, seems as if the stream is damaged, but stopping here should be okay.
                return framesRead;
            }
            else if (error == MPG123_ERR)
            {   //okay, seems as if the stream is severely damaged, but stopping here should be okay in many cases.
                return framesRead;
            }
            else
            {
                std::cerr << "mpg123: error while reading at position " << position << ": " << error << ", \"" << mpg123_plain_strerror(error) << "\"" << std::endl;
                return 0;
            }
        }
        else if (dataType == DATATYPE_SNDFILE)
        {   //return data from libsndfile
            int itemsRead = sf_read_float(sndfileHandle, buffer, count);
            position += itemsRead;
            return itemsRead;
        }
        else
        {
            std::cerr << "trying to read unknown datatype!" << std::endl;
            return 0;
        }
    }



    SoundFile::SingletonInitializer* SoundFile::SingletonInitializer::instance = NULL;

    SoundFile::SingletonInitializer::SingletonInitializer() :
        counter(0)
    {
        
    }
    void SoundFile::SingletonInitializer::initialize()
    {
        if (instance == NULL)
            instance = new SingletonInitializer();
        
        if (instance->counter == 0)
        {
            mpg123_init();
            if(mpg123_init() != MPG123_OK)
                std::cerr << "mpg123: basic setup impossible." << std::endl;
        }
        instance->counter++;
    }
    void SoundFile::SingletonInitializer::destroy()
    {
        instance->counter--;
        if (instance->counter <= 0)
        {
            instance->counter = 0;
        }
    }
    SoundFile::SingletonInitializer::~SingletonInitializer()
    {
        mpg123_exit();
    }
    
    SoundFileMetadata* SoundFile::getMetadata()
    {
        return metadata;
    }
    
    std::string SoundFileMetadata::getTitle() const                     {return title;}
    std::string SoundFileMetadata::getArtist() const                    {return artist;}
    std::string SoundFileMetadata::getAlbum() const                     {return album;}
    std::string SoundFileMetadata::getGenre() const                     {return genre;}
    std::string SoundFileMetadata::getFilename() const                  {return filename;}
    std::string SoundFileMetadata::getYear() const                      {return year;}
    std::string SoundFileMetadata::getTrack() const                     {return track;}
    
    void SoundFileMetadata::setTitle(const std::string& title)          {this->title = title;}
    void SoundFileMetadata::setArtist(const std::string& artist)        {this->artist = artist;}
    void SoundFileMetadata::setAlbum(const std::string& album)          {this->album = album;}
    void SoundFileMetadata::setGenre(const std::string& genre)          {this->genre = genre;}
    void SoundFileMetadata::setFilename(const std::string& filename)    {this->filename = filename;}
    void SoundFileMetadata::setYear(const std::string& year)            {this->year = year;}
    void SoundFileMetadata::setTrack(const std::string& track)          {this->track = track;}
    
    std::string SoundFile::mpg123_stringToStdString(mpg123_string* str)
    {
        //TODO: This might be unsafe and a security risk.
        if (str != NULL)
        {
			if ((str->p != NULL) && (str->fill > 0))
				return std::string(str->p);
			else
				return std::string("--EMPTY--");
		}
		else
			return std::string("--NULL--");
    }
}
