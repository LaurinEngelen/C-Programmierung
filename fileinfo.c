#define _POSIX_C_SOURCE 200112L
#include "limits.h"
#include "fileinfo.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

/*Author Laurin Engelen
 * Datum 31.05.2024 */


typedef struct fileinfo fileinfo;

void list_directory(fileinfo *dir);
void print_regular(size_t size, const char *filename);
void print_directory(const char *path, const char *dirname, const fileinfo *dir);
void print_other(const char *filename);

fileinfo *fileinfo_create(const char *filename)
{
    if (strlen(filename) > NAME_MAX) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    fileinfo *f = malloc(sizeof(fileinfo));
    if (!f)
    {
        return NULL;
    }
    struct stat s;                      //prüft ob Datei existiert
    if (stat(filename, &s) == -1)
    {
        free(f);
        return NULL;
    }
    strncpy(f->filename, filename, NAME_MAX); // Copy filename to the fileinfo structure
    f->filename[NAME_MAX] = '\0';

    if(S_ISREG(s.st_mode))
    {
        f->type = filetype_regular;
        f->file_size = s.st_size; //größe der Datei in bytes speichern
    }
    else if(S_ISDIR(s.st_mode)){
        f->type = filetype_directory;
        list_directory(f);
    }
    else{
        f->type = filetype_other;
    }

    return f;
}

void list_directory(fileinfo *dir)
{
    if (chdir(dir->filename) == -1) // prüfen ob Ordner existiert
    {
        return;
    }

    DIR *d = opendir(".");
    if (d == NULL)
    {
        return;
    }

    // list files in directory and create fileinfo
    fileinfo *files = NULL;
    fileinfo *dirs = NULL;
    const struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        // skip "." and ".." dirs
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
            continue;
        }

        fileinfo *f = fileinfo_create(e->d_name);
        if (!f) {
            closedir(d);
            return;
        }
        // sort current file in directory or other category.
        if (f->type == filetype_directory)
        {
            f->next = dirs;
            dirs = f;
        }
        else
        {
            f->next = files;
            files = f;
        }
    }

    // if there are no files, append dirs to contents of directory
    if (!files) {
        dir->dir_files = dirs;
    }
        // otherwise start with files first and append dirs at last file
    else {
        dir->dir_files = files;
        while (files->next)
        {
            files = files->next;
        }
        files->next = dirs;
    }

    closedir(d);

    if (chdir("..") == -1) {
        printf("Error: %s\n", strerror(errno));
        fileinfo_destroy(dir);
        return;
    }

}

void fileinfo_print(const fileinfo *f)
{
    if(f->type == filetype_regular)
    {
        print_regular(f->file_size, f->filename);
        return;
    }
    else if(f->type == filetype_directory)
    {
        print_directory("", f->filename, f->dir_files);
        return;
    }
    else
    {
        print_other(f->filename);
        return;
    }
}


void print_regular(size_t size, const char *filename)
{
    printf("%s (regular, %zu Bytes)\n", filename, size);
}

void print_directory(const char *path, const char *dirname, const fileinfo *dir)
{

    printf("\n%s%s:\n", path, dirname);
    // print directory contents
    for (const fileinfo *e = dir; e; e = e->next)
    {
        switch (e->type)
        {
            case filetype_regular:
                print_regular(e->file_size, e->filename);
                break;
            case filetype_directory:
                printf("%s (directory)\n", e->filename);
                break;
            case filetype_other:
                print_other(e->filename);
                break;
        }
    }

    for (const fileinfo *e = dir; e; e = e->next)
    {
        if (e->type == filetype_directory)
        {

            char *newpath = malloc(sizeof (char) * (strlen(path) + strlen(dirname) + 3));
            if (newpath == NULL)
            {
                print_directory("PATHTOOLONG.../", e->filename, e->dir_files);
                continue;
            }

            sprintf(newpath, "%s%s", path, dirname);
            strcat(newpath, "/");
            print_directory(newpath, e->filename, e->dir_files);
            free(newpath);

        }

    }

}

void print_other(const char *filename)
{
    printf("%s (other)\n", filename);
}

void fileinfo_destroy(const fileinfo *f)
{
    if (f->type == filetype_directory)
    {
        fileinfo *head = f->dir_files;
        while (head) {
            fileinfo *next = head->next;
            fileinfo_destroy(head);
            head = next;
        }
    }
    free((fileinfo*)f);

}