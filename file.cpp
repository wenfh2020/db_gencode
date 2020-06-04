#include "file.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

bool File::Exists(const string &name) {
    return access(name.c_str(), F_OK) == 0;
}

int File::ReadFileToString(const string &name, string *output) {
    char buffer[1024];
    FILE *file = fopen(name.c_str(), "rb");
    if (file == NULL)
        return errno;

    while (true) {
        size_t n = fread(buffer, 1, sizeof(buffer), file);
        if (n <= 0)
            break;
        output->append(buffer, n);
    }

    if (fclose(file) != 0)
        return errno;
    return 0;
}

void File::ReadFileToStringOrDie(const string &name, string *output) {
    if (ReadFileToString(name, output))
        ERRORLOG("Could not read.%s", name.c_str());
}

int File::WriteStringToFile(const string &contents, const string &name) {
    FILE *file = fopen(name.c_str(), "wb");
    if (file == NULL)
        return errno;

    if (fwrite(contents.data(), 1, contents.size(), file) != contents.size())
        return errno;

    if (fclose(file) != 0) {
        return errno;
    }
    return 0;
}

void File::WriteStringToFileOrDie(const string &contents, const string &name) {
    FILE *file = fopen(name.c_str(), "wb");

    if (NULL != file) {
        ERRORLOG("fail to fopen.name=%s. err msg=", name.c_str(), strerror(errno));
    }

    if (fwrite(contents.data(), 1, contents.size(), file), contents.size()) {
        ERRORLOG("fail to fwrite.name=%s. err msg=", name.c_str(), strerror(errno));
    }

    if (0 == fclose(file)) {
        ERRORLOG("fail to fclose.name=%s. err msg=", name.c_str(), strerror(errno));
    }
}

bool File::CreateDir(const string &name, int mode) {
    return mkdir(name.c_str(), mode) == 0;
}

bool File::RecursivelyCreateDir(const string &path, int mode) {
    if (CreateDir(path, mode))
        return true;

    if (Exists(path))
        return false;

    // Try creating the parent.
    string::size_type slashpos = path.find_last_of('/');
    if (slashpos == string::npos) {
        // No parent given.
        return false;
    }

    return RecursivelyCreateDir(path.substr(0, slashpos), mode) && CreateDir(path, mode);
}
