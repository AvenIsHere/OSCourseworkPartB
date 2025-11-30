#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct List {
    char** list;
    int size;
} List;

List addList(const struct dirent* file, List list) {
    list.list = realloc(list.list, (list.size+1)*sizeof(char*));
    list.list[list.size] = strdup(file->d_name);
    list.size++;
    return list;
}

List browseDirectory(char* path, List execList) {
    struct dirent *file;
    struct stat fileStat;
    DIR *dir;
    if ((dir = opendir(path)) == NULL) {
        printf("Unable to open directory %s\n", path);
        return execList;
    }
    while ((file = readdir(dir)) != NULL) {
        if (file->d_name[0] == '.') {
            continue;
        }
        if (path[strlen(path)-1] != '/') {
            char* newPath = malloc((strlen(path)+strlen(file->d_name)+2)*sizeof(char));
            strcpy(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, file->d_name);
            if (stat(newPath, &fileStat) != 0) {
                free(newPath);
                continue;
            }
            free(newPath);
        }
        else {
            char* newPath = malloc((strlen(path)+strlen(file->d_name)+2)*sizeof(char));
            strcpy(newPath, path);
            strcat(newPath, file->d_name);
            if (stat(newPath, &fileStat) != 0) {
                free(newPath);
                continue;
            }
            free(newPath);
        }
        if (S_ISDIR(fileStat.st_mode)) {
            char* newPath = malloc((strlen(path)+strlen(file->d_name)+2)*sizeof(char));
            strcpy(newPath, path);
            strcat(newPath, "/");
            strcat(newPath, file->d_name);
            execList = browseDirectory(newPath, execList);
            free(newPath);
        }
        else if (S_ISREG(fileStat.st_mode) && (fileStat.st_mode & S_IXUSR)) {
            execList = addList(file, execList);
        }
    }
    closedir(dir);
    return execList;
}

int main(int argc, char *argv[]) {
    List list = {NULL, 0};
    if (argc == 1) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            printf("Could not get current directory\n");
            return 1;
        }
        list = browseDirectory(cwd, list);
    }
    for (int i = 1; i < argc; i++) {
        char* path = strtok(argv[i], ":");
        while (path != NULL) {
            list = browseDirectory(path, list);
            path = strtok(NULL, ":");
        }
    }
    if (list.list == NULL) {
        return 1;
    }
    for (int i = 0; i < list.size; i++) {
        printf("%s\n", list.list[i]);
    }
    return 0;
}