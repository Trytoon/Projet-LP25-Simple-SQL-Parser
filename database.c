#include "database.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>


#include "utils.h"

void create_db_directory(char *name) {

    if (!name) return;

    if (!directory_exists(name)) {
        mkdir(name, S_IRWXU);
    }
}

void recursive_rmdir(char *dirname) {

    DIR *dir = opendir(dirname);
    if (!dir) {
        return;
    }
    struct dirent *entry = readdir(dir);
    while (entry) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            entry = readdir(dir);
            continue;
        }
        struct stat sb;
        if (stat(dirname, &sb) == -1) {
            perror("stat");
            exit(EXIT_SUCCESS);
        }

        char path[4096];
        switch (sb.st_mode & S_IFMT) {
            case S_IFDIR:
                strcpy(path, dirname);
                strcat(path, "/");
                strcat(path, entry->d_name);
                recursive_rmdir(path);
                break;

            default:
                strcpy(path, dirname);
                strcat(path, "/");
                strcat(path, entry->d_name);
                remove(path);
                break;
        }

        entry = readdir(dir);
    }
    rmdir(dirname);
}
