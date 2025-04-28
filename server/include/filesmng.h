#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define MAX_PATH_LENGTH 1024

char* enter_dir(const char* parent_dir, const char* subdir_name) {
    size_t len = strlen(parent_dir) + strlen(subdir_name) + 2; // +1 for '/' +1 for '\0'
    char* new_path = malloc(len);
    if (!new_path) {
        perror("malloc failed");
        return NULL;
    }
    snprintf(new_path, len, "%s/%s", parent_dir, subdir_name);
    return new_path;
}

char* exit_dir(const char* path) {
    char* new_path = strdup(path);
    if (!new_path) {
        perror("strdup failed");
        return NULL;
    }

    // Find the last '/'
    char* last_slash = strrchr(new_path, '/');
    if (last_slash) {
        *last_slash = '\0'; // Cut off after the last '/'
    } else {
        // No slash found, make it empty
        new_path[0] = '\0';
    }

    return new_path;
}

int create_directory(const char* path) {
    if (mkdir(path, 0777) == 0) {
        printf("Directory created successfully: %s\n", path);
        return 0;
    } else {
        perror("mkdir");
        return -1;
    }
};

int remove_directory(const char* path) {
    if (rmdir(path) == 0) {
        printf("Directory removed successfully: %s\n", path);
        return 0;
    } else {
        perror("rmdir");
        return -1;
    }
}


void list_directory(const char* path, char* dirs[255][MAX_PATH_LENGTH], char* files[255][MAX_PATH_LENGTH], int* out_buf_len, char* out_buff) {
    struct dirent* entry;
    DIR* dir = opendir(path);

    if (dir == NULL) {
        perror("opendir");
        return;
    }

    //printf("Contents of directory: %s\n", path);
    *out_buf_len = 0;
    char fullpath[1024];
    struct stat path_stat;
    int files_pos = 0;
    int dirs_pos = 0;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (stat(fullpath, &path_stat) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            //printf("[DIR]\t%s\n", entry->d_name);

            int len = strlen(entry->d_name);
            memcpy(&dirs[dirs_pos], entry->d_name, len);

            memcpy(&out_buff[*out_buf_len], entry->d_name, len + 1);
            dirs_pos += len;
            *out_buf_len += len;
            out_buff[*out_buf_len] = '\n';
            *out_buf_len += 1;
            dirs[dirs_pos][len] = '\0'; // separate entries by '\0'
        }
        else if (S_ISREG(path_stat.st_mode)) {
            //printf("[FILE]\t%s\n", entry->d_name);

            int len = strlen(entry->d_name);
            memcpy(&files[files_pos], entry->d_name, len);
            memcpy(&out_buff[*out_buf_len], entry->d_name, len + 1);
            dirs_pos += len;
            *out_buf_len += len;
            out_buff[*out_buf_len] = '\n';
            *out_buf_len += 1;
            files_pos += len;
            files[files_pos][len] = '\0'; // separate entries by '\0'
        }
        else {
            printf("[OTHER]\t%s\n", entry->d_name);
        }
    }
    out_buff[*out_buf_len + 1] = '\0';
    closedir(dir);
}