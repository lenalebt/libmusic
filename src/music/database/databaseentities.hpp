#ifndef DATABASEENTITIES_HPP
#define DATABASEENTITIES_HPP

#include <string>

namespace music
{
    /**
     * @brief This is a database value object for a song and its properties
     *      that are no features that were extracted from the signal.
     * 
     * @remarks Use this object to store data in a database.
     * 
     * @author Lena Brueder
     * @date 2012-07-16
     */
    class Song
    {
    private:
        long id;
        std::string filename;
        std::string artist;
        std::string title;
        std::string genre;
        int tracknr;
        
        SongFeatures* features;
    protected:
        
    public:
        Song();
        
        void setID(long id)                             {this->id = id;}
        long getID() const                              {return id;}
        
        void setFilename(std::string filename)          {this->filename = filename;}
        std::string getFilename() const                 {return filename;}
        
        void setArtist(std::string artist)              {this->artist = artist;}
        std::string getArtist() const                   {return artist;}
        
        void setTitle(std::string title)                {this->title = title;}
        std::string getTitle() const                    {return title;}
        
        void setGenre(std::string genre)                {this->genre = genre;}
        std::string getGenre() const                    {return genre;}
        
        void setTrackNumber(int trackNumber)            {this->tracknr = trackNumber;}
        int getTrackNumber() const                      {return tracknr;}
        
        void setSongFeatures(SongFeatures* features)    {this->features = features;}
        SongFeatures* getSongFeatures() const           {return songFeatures;}
        
    };
    
    class SongFeatures
    {
    private:
        
    protected:
        
    public:
        
    };
}
#endif  //DATABASEENTITIES_HPP
