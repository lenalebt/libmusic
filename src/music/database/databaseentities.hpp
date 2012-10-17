#ifndef DATABASEENTITIES_HPP
#define DATABASEENTITIES_HPP

#include <string>
#include <Eigen/Dense>
#include <list>
#include "tuple.hpp"

namespace music
{
    namespace databaseentities
    {
        typedef long id_datatype;
        
        /**
         * @brief Objects of this class are database value objects for the
         *      features of a recording
         *      that were extracted from the signal,
         *      such as tempo, length, dynamic range, ...
         * 
         * @remarks Use this object to store data in a database.
         * 
         * @todo features are not ready yet. some are missing, formats are not clear, etc.
         * 
         * @ingroup database
         * @author Lena Brueder
         * @date 2012-07-16
         */
        class RecordingFeatures
        {
        private:
            id_datatype id;
            double length;
            double tempo;
            double dynamicRange;
            std::string timbreModel;
            std::string chordsModel;
        protected:
            
        public:
            /** @todo constructor not ready yet
             */
            RecordingFeatures();
            
            void setID(id_datatype id)                {this->id = id;}
            id_datatype getID() const                 {return id;}
            
            void setLength(double length)             {this->length = length;}
            double getLength() const                  {return length;}
            
            void setTempo(double tempo)               {this->tempo = tempo;}
            double getTempo() const                   {return tempo;}
            
            void setDynamicRange(double dynamicRange) {this->dynamicRange = dynamicRange;}
            double getDynamicRange() const            {return dynamicRange;}
            
            void setTimbreModel(const std::string& model) {this->timbreModel = model;}
            std::string getTimbreModel() const        {return timbreModel;}
            
            void setChordsModel(const std::string& model) {this->chordsModel = model;}
            std::string getChordsModel() const        {return chordsModel;}
        };
        
        /**
         * @brief Objects of this class are database value objects for
         *      a recording and its properties
         *      that are no features that were extracted from the signal.
         * 
         * @remarks Use this object to store data in a database.
         * 
         * @ingroup database
         * @author Lena Brueder
         * @date 2012-07-16
         */
        class Recording
        {
        private:
            id_datatype id;
            std::string filename;
            std::string artist;
            std::string title;
            std::string genre;
            std::string album;
            int tracknr;
            
            RecordingFeatures* features;
        protected:
            
        public:
            Recording();
            Recording(std::string filename);
            ~Recording();
            
            void setID(id_datatype id)                      {this->id = id;}
            id_datatype getID() const                       {return id;}
            
            void setFilename(std::string filename)          {this->filename = filename;}
            std::string getFilename() const                 {return filename;}
            
            void setArtist(std::string artist)              {this->artist = artist;}
            std::string getArtist() const                   {return artist;}
            
            void setTitle(std::string title)                {this->title = title;}
            std::string getTitle() const                    {return title;}
            
            void setGenre(std::string genre)                {this->genre = genre;}
            std::string getGenre() const                    {return genre;}
            
            void setAlbum(std::string album)                {this->album = album;}
            std::string getAlbum() const                    {return album;}
            
            void setTrackNumber(int trackNumber)            {this->tracknr = trackNumber;}
            int getTrackNumber() const                      {return tracknr;}
            
            void setRecordingFeatures(RecordingFeatures* features) {this->features = features;}
            RecordingFeatures* getRecordingFeatures() const        {return features;}
            
        };
        
        class CategoryDescription
        {
        private:
            id_datatype id;
            std::string timbreModel;
            std::string chordsModel;
            std::string classifierDescription;
        protected:
            
        public:
            CategoryDescription();
            void setID(id_datatype id)                      {this->id = id;}
            id_datatype getID() const                       {return id;}
            
            void setTimbreModel(const std::string& model)   {this->timbreModel = model;}
            std::string getTimbreModel() const              {return timbreModel;}
            
            void setChordsModel(const std::string& model) {this->chordsModel = model;}
            std::string getChordsModel() const        {return chordsModel;}
            
            void setClassifierDescription(const std::string& classifierDescription)
                                                            {this->classifierDescription = classifierDescription;}
            std::string getClassifierDescription() const    {return classifierDescription;}
        };
        
        class Category
        {
        private:
            id_datatype id;
            std::string categoryName;
            CategoryDescription* categoryDescription;
        protected:
            
        public:
            Category();
            ~Category();
            void setID(id_datatype id)                      {this->id = id;}
            id_datatype getID() const                       {return id;}
            
            void setCategoryName(std::string categoryName)  {this->categoryName = categoryName;}
            std::string getCategoryName() const             {return categoryName;}
            
            void setCategoryDescription(CategoryDescription* categoryDescription) {this->categoryDescription = categoryDescription;}
            CategoryDescription* getCategoryDescription() const                   {return categoryDescription;}
        };
        
    }
}
#endif  //DATABASEENTITIES_HPP
