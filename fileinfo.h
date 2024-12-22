#ifndef FILEINFO_H
#define FILEINFO_H
#include <limits.h>
#include <stddef.h>

/*Author Laurin Engelen
 * Datum 31.05.2024 */

enum fileType {filetype_regular, filetype_directory, filetype_other};
struct fileinfo
{
    struct fileinfo *next;

    enum fileType type;
    char filename[NAME_MAX + 1];
    union {
        size_t file_size;
        struct fileinfo *dir_files;
    };
};

typedef struct fileinfo fileinfo;

fileinfo *fileinfo_create(const char *filename);
void fileinfo_print(const fileinfo *f);
void fileinfo_destroy(const fileinfo *f);
#endif