#include "include.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CHUNK_SIZE 1000

struct cdpk_package_path {
    char* rel;
    struct cdpk_package_path* next;
};

struct cdpk_package {
    char* res_dir;
    struct cdpk_package_path* first;
};

void cdpk_package_init(struct cdpk_package* pkg, const char* resource_dir) {
    char add_slash = resource_dir[strlen(resource_dir) - 1] != '/';
    pkg->res_dir = malloc(strlen(resource_dir) + 1 + add_slash);
    strcpy(pkg->res_dir, resource_dir);
    if (add_slash) strcat(pkg->res_dir, "/");
    pkg->first = 0;
}

void cdpk_package_cleanup(struct cdpk_package* pkg) {
    free(pkg->res_dir); 
    struct cdpk_package_path* p = pkg->first;
    while (p) {
        free(p->rel);
        struct cdpk_package_path* next = p->next;
        free(p);
        p = next;
    }
}

void cdpk_package_add(struct cdpk_package* pkg, const char* path) {
    struct cdpk_package_path* pkg_path = malloc(sizeof(struct cdpk_package_path));
    pkg_path->rel = malloc(strlen(path) + 1);
    strcpy(pkg_path->rel, path);
    pkg_path->next = 0;
    if (pkg->first == 0) pkg->first = pkg_path;
    else {
        struct cdpk_package_path* pkg_path_it = pkg->first;
        while (pkg_path_it->next) pkg_path_it = pkg_path_it->next;
        pkg_path_it->next = pkg_path;
    }
}

int cdpk_package_add_dir(struct cdpk_package* pkg, const char* dir) {

    WIN32_FIND_DATA win32_find_data;
    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s%s/*", pkg->res_dir, dir);

    HANDLE handle = FindFirstFile(search_path, &win32_find_data);

    if (handle == INVALID_HANDLE_VALUE) return GetLastError();

    do {
        char path[MAX_PATH + 1];
        size_t len = strlen(dir);
        if (len) {
            strcpy(path, dir);
            strcat(path, "/");
            len += 1;
        }
        for (int i = 0; i < wcslen(win32_find_data.cFileName); i++)
            path[len + i] = win32_find_data.cFileName[i];
        if (win32_find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (strcmp(win32_find_data.cFileName, ".") != 0 && strcmp(win32_find_data.cFileName, "..") != 0) {
                if (cdpk_package_add_dir(pkg, path)) return 1;
            }
        } else {
            cdpk_package_add(pkg, path);
        }
    } while (FindNextFile(handle, &win32_find_data) != 0);

    FindClose(handle);

    return 0;

}

unsigned int cdpk_package_path_count(struct cdpk_package* pkg) {
    struct cdpk_package_path* p = pkg->first;
    unsigned int c = 0;
    while (p) {
        c++;
        p = p->next;
    }
    return c;
}

#define write_uintN(N) \
void write_uint##N(FILE* file, uint##N##_t n) { /* big endian */ \
    unsigned char bytes[sizeof(n)]; \
    for (unsigned char i = 0; i < sizeof(n); i++) \
        bytes[i] = (n >> (8 * (sizeof(n) - 1 - i))) & 0xff; \
    fwrite(bytes, 1, sizeof(n), file); \
}
write_uintN(32)
write_uintN(64)

int cdpk_package_apply(struct cdpk_package* pkg, const char* apply_path) {

    FILE* out_file = fopen(apply_path, "ab");
    if (out_file == NULL) return 1;

    struct cdpk_package_path* path = pkg->first;
    if (!path) return 1;
    uint32_t path_count = (uint32_t)cdpk_package_path_count(pkg);
    uint64_t* data_sizes = malloc(sizeof(uint64_t) * path_count);
    unsigned int path_i = 0;
    while (path) {

        char* abs_path = malloc(strlen(pkg->res_dir) + strlen(path->rel) + 1);
        strcpy(abs_path, pkg->res_dir);
        strcat(abs_path, path->rel);
        FILE* in_file = fopen(abs_path, "rb");
        free(abs_path);
        if (in_file == NULL) return 1;
        uint64_t total_size = 0;
        while (1) {
            char chunk[CHUNK_SIZE];
            size_t size = fread(chunk, 1, CHUNK_SIZE, in_file);
            if (!size) break;
            total_size += size;
            fwrite(chunk, 1, size, out_file);
        }
        fclose(in_file);

        data_sizes[path_i] = total_size;

        path = path->next;
        path_i++;

    }

    path = pkg->first;
    path_i = 0;
    while (path) {

        write_uint64(out_file, data_sizes[path_i]);
        fputc(0, out_file);
        fwrite(path->rel, 1, strlen(path->rel), out_file);

        path = path->next;
        path_i++;

    }

    write_uint32(out_file, path_count);

    fclose(out_file);
    free(data_sizes);

    return 0;
    
}

int main(int argc, char* argv[]) {

    if (argc < 2) return 1;

    const char* target_file = argv[1];
    const char* resource_dir = argv[2];

    struct cdpk_package package;
    cdpk_package_init(&package, resource_dir);
    cdpk_package_add_dir(&package, "");
    cdpk_package_apply(&package, target_file);
    cdpk_package_cleanup(&package);

    return 0;

}