#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

void processDirectory(const char *path, unsigned long long *totalSize);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <directory_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    unsigned long long totalSize = 0;
    processDirectory(argv[1], &totalSize);

    printf("Total size of the directory: %llu bytes\n", totalSize);

    return 0;
}

void processDirectory(const char *path, unsigned long long *totalSize) {
    DIR *dir = opendir(path);

    if (!dir) {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    struct stat fileStat;

    while ((entry = readdir(dir)) != NULL) {
        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);

        if (stat(filePath, &fileStat) < 0) {
            perror("Error getting file status");
            continue;
        }

        if (S_ISDIR(fileStat.st_mode)) {
            // Ignore '.' and '..' directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Recursively process subdirectory
            processDirectory(filePath, totalSize);
        } else {
            // Process file
            printf("File: %s\n", filePath);

            // Accumulate the size of the file
            *totalSize += fileStat.st_size;
        }
    }

    closedir(dir);
}


