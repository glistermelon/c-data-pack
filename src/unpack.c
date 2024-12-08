#include "unpack.h"

const char* get_exe_path() {

#ifdef _WIN32
    char* path = malloc(MAX_PATH);
    if (GetModuleFileName(NULL, path, MAX_PATH) == 0) {
        free(path);
        return 0;
    }
#endif

#ifdef UNIX
    char *path = malloc(PATH_MAX + 1);
    ssize_t len = readlink("/proc/self/exe", path, PATH_MAX);
    if (len == -1) {
        free(path);
        return 0;
    }
    path[len] = 0;
#endif

    return path;

}

#define read_uintN(N)                                     \
uint##N##_t read_uint##N(FILE* file) { /* big endian */   \
    uint##N##_t n = 0;                                    \
    unsigned char c;                                      \
    for (unsigned char i = 0; i < sizeof(n); i++) {       \
        fread(&c, 1, 1, file);                            \
        n += ((uint##N##_t)c) << (sizeof(n) - 1 - i);     \
    }                                                     \
    return n;                                             \
}
read_uintN(32)
read_uintN(64)

static char** cdpk_paths;
static uint64_t* cdpk_data_sizes;
static uint64_t* cdpk_data_curs;
static int cdpk_path_count;
static char cdpk_initialized = 0;

int cdpk_init() {

    const char* exe_path = get_exe_path();
    if (!exe_path) return 1;
    FILE* file = fopen(exe_path, "rb");
    if (file == NULL) return 1;

    if (fseek(file, -4, SEEK_END)) return 1;
    uint32_t num_data = read_uint32(file);

    cdpk_paths = malloc(sizeof(char*) * num_data);
    cdpk_data_sizes = malloc(sizeof(uint64_t*) * num_data);
    cdpk_data_curs = malloc(sizeof(uint64_t*) * num_data);
    cdpk_path_count = num_data;

    uint32_t cur = 4;

    for (uint32_t i = 0; i < num_data; i++) {

        uint32_t len = 0;
        char c = 1;
        while (c) {
            if (fseek(file, -(++len) - cur, SEEK_END)) return 1;
            c = getc(file);
            ungetc(c, file);
        }
        len -= 1;

        if (fseek(file, -len - cur, SEEK_END)) return 1;
        char* path = malloc(len + 1);
        fread(path, 1, len, file);
        path[len] = 0;
        cdpk_paths[i] = path;

        if (fseek(file, -len - cur - 9, SEEK_END)) return 1;
        uint64_t size = read_uint64(file);
        cdpk_data_sizes[i] = size;

        cur += len + 9;

    }

    fclose(file);

    for (uint32_t i = 0; i < num_data; i++) {
        cur += cdpk_data_sizes[i];
        cdpk_data_curs[i] = cur;
    }

    return 0;

}

int cdpk_init_check() {
    return cdpk_initialized || !cdpk_init();
}

int64_t cdpk_find_path(const char* path) {
    if (!cdpk_init_check()) return -1;
    int i = 0;
    while (i < cdpk_path_count && strcmp(path, cdpk_paths[i]) != 0) i++;
    if (i == cdpk_path_count) return -1;
    return i; 
}

// I would prefer to return FILE* here, but Windows doesn't support fmemopen. Classic Microsoft
int cdpk_get_data(const char* path, char** output_buf, size_t* output_size) {
    if (!cdpk_init_check()) return 1;
    int64_t i = cdpk_find_path(path);
    if (i == -1) return 1;
    FILE* file = fopen(get_exe_path(), "rb");
    if (file == NULL) return 1;
    if (fseek(file, -cdpk_data_curs[i], SEEK_END)) return 1;
    if (output_size) *output_buf = malloc(cdpk_data_sizes[i]);
    fread(*output_buf, 1, cdpk_data_sizes[i], file);
    fclose(file);
    if (output_size) *output_size = cdpk_data_sizes[i];
    return 0;
}

int cdpk_get_data_size(const char* path, size_t* output_size) {
    if (!cdpk_init_check()) return 1;
    int64_t i = cdpk_find_path(path);
    if (i == -1) return 1;
    *output_size = cdpk_data_sizes[i];
    return 0;
}