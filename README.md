# C Data Packer

## Function & Rationale

This project allows for packing file data from a single directory directly into an executable, where it can be read later at runtime. This is useful in any case where one wants to have access to data stored in a file without actually keeping track of the file. A particular instance that inspired this project was an attempt to access OpenGL shaders at runtime.

## `unpack` API

The primary function provided is `cdpk_get_data(const char* path, char** output_buf, size_t* output_size)`. Given the path to the data in `path`, if no error occurs, the data will be placed in `*output_buf` and the size of the data (in bytes) in `*output_size`. The return value is an integer error indicator. Note that, if `output_size` is `NULL`, it is assumed that you have already allocated `*output_buf` using the output from `cdpk_get_data_size`, so it will not be allocated for you (as it otherwise would be). This is particularly useful when the stored data is a non-null terminated string that needs to be stored with a null terminator.

See `src/test.c` for an example. The packed data is from the `res` directory.

## CMake API

The CMake API provides the `pack_data(target, resources)` function. `target` should be the executable target that resources are to be packed into. `resources` should be an absolute path to the directory containing the resources.

`packdata.exe`, which is provided in Releases or can be built manually from `src/pack.c`, must be present in your CMake source directory.

See the `test` target in `CMakeLists.txt` for an example.

## Current Limitations

* **Windows only.** I have access to a Linux system and intend to implement Linux functionality soon. I have no plans to implement MacOS functionality because I don't have access to MacOS.

* **Most likely cannot handle non-ASCII file names.**

## Details For Nerds

The data is packed into the end of the executable in this fashion (see `pack.c` for details, particularly the implementation of `cdpk_package_apply`):
1. Binary data of each file is appended
2. For each file:
    1. File size is appended as an 64-bit unsigned big endian integer
    2. A null byte is appended
    3. The file path is appended
6. Lastly, the number of file paths is appended as a 32-bit unsigned big endian integer

The data is read by opening the executable during runtime with `fopen` and reading it in reverse `fseek` and `fread`. See `unpack.c` for details. An initial call to `cdpk_init` caches data sizes and `fseek` offsets for easy later accessibility.
