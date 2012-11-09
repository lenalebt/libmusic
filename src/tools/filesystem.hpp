#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <vector>
#include <string>

/**
 * @brief Loads all file names from the given folder.
 * 
 * @param folder The folder from which the files will be loaded.
 * @param[out] files The list of files in the folder. The list will first be cleared.
 *      The file names will not contain the name of the folder.
 * 
 * @todo TEST MISSING
 * 
 */
void loadFilenames(const std::string& folder, std::vector<std::string>& files);

#endif  //FILESYSTEM_HPP
