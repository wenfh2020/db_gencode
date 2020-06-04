#ifndef _FILE_H_
#define _FILE_H_

#include "common.h"

class File {
   public:
    // Check if the file exists.
    static bool Exists(const string& name);

    // Read an entire file to a string.  Return true if successful, false
    // otherwise.
    static int ReadFileToString(const string& name, string* output);

    // Same as above, but crash on failure.
    static void ReadFileToStringOrDie(const string& name, string* output);

    // Create a file and write a string to it.
    static int WriteStringToFile(const string& contents, const string& name);

    // Same as above, but crash on failure.
    static void WriteStringToFileOrDie(const string& contents, const string& name);

    // Create a directory.
    static bool CreateDir(const string& name, int mode);

    // Create a directory and all parent directories if necessary.
    static bool RecursivelyCreateDir(const string& path, int mode);

    static bool GetContents(const string& name, string* output) {
        return ReadFileToString(name, output);
    }

    static bool SetContents(const string& name, const string& contents) {
        return WriteStringToFile(contents, name);
    }
};

#endif
