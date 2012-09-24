#include "filesystem.hpp"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stringhelper.hpp"

void loadFilenames(const std::string& folder, std::vector<std::string>& files)
{
    files.clear();
    
    DIR* dir = NULL;        //POSIX standard calls
    struct dirent *ent;
    dir = opendir(folder.c_str());
    while ((ent = readdir (dir)) != NULL)
    {
        std::string filename(ent->d_name);
        if ((filename != ".") && (filename != ".."))
            files.push_back(filename);
    }
}
