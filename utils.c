#include "utils.h"

/*!
 *  \brief make_full_path concatenates path and basename and returns the result
 *  \param path the path to the database directory basename, can be NULL (i.e. path is current directory)
 *  Path may end with '/' but it is not required.
 *  \param basename the database name.
 *
 *  \return a pointer to the full path. Its content must be freed by make_full_path caller.
 */
char *make_full_path(char *path, char *basename) {

    char temp_path[1000] = "";

    if (basename != NULL && path == NULL) {
        return basename;
    } else if (basename) {
        strcat(temp_path, path);
        char *full_path = strcat(temp_path, basename);
        return full_path;
    }

    return NULL;
}

bool directory_exists(char *path) {

    if(!path) return false;

    DIR *my_dir = opendir(path);
    if (my_dir) {
        closedir(my_dir);
        return true;
    }
    return false;
}