#ifndef FILESYSFUN_H
#define FILESYSFUN_H


/*
    OS independent functions for manipulating, changing etc. of directories are defined here

    Must have either 'Linux' or 'Windows' defined

*/
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>

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
//void backSlashToForwardSlash( std::string& str); // converts backslashes to fw slashes


bool setCurrentDirectory(const std::string& destdir);

std::string getCurrentDirectory();


bool dirExists(const std::string& dir);
bool fileExists(const std::string& file);       // check whether file exists already

bool createDirectory(const std::string& newdir);




#endif // FILESYSFUN_H