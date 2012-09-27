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
 * @param ending The ending of the file name. Is case-sensitive. Example: <code>".mp3"</code>.
 * 
 * @todo TEST MISSING
 * 
 */
void loadFilenames(const std::string& folder, std::vector<std::string>& files, const std::string& ending = "");

#endif  //FILESYSTEM_HPP
