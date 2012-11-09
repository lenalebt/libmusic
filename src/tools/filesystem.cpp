#include "filesystem.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stringhelper.hpp"

void loadFilenames(const std::string& folder, std::vector<std::string>& files, const std::string& ending)
{
    files.clear();
    
    DIR* dir = NULL;        //POSIX standard calls
    struct dirent *ent;
    dir = opendir(folder.c_str());
    while ((ent = readdir (dir)) != NULL)
    {
        std::string filename(ent->d_name);
        if ((filename != ".") && (filename != "..") && endsWith(filename, ending))
            files.push_back(filename);
    }
    closedir(dir);
}
