#ifndef FILESYSFUN_H
#define FILESYSFUN_H
/*!
    OS independent functions for manipulating, changing etc.
    of directories are defined here. Must have either 'Linux' or
    'Windows' defined.  i.e. -DLinux or -DWindows is needed to compile
*/
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream>
#ifdef Linux
#include <unistd.h>
#include <sys/stat.h> // for making directories
#elif Windows
//#include <windows.h>
#include <direct.h>  // _getcwd
#include <windows.h> // SetCurrentDirectory
#else
#error "OS NOT DEFINED: Linux or Windows"
#endif
namespace FilesysFun {
bool setCurrentDirectory(const std::string &destdir);
std::string getCurrentDirectory();
bool dirExists(const std::string &dir);
bool createDirectory(const std::string &newdir);
}// end namespace FilesysFun

#endif // FILESYSFUN_H
