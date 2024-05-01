# CPS2000 Coursework

This repository contains the source code for all the tasks which
had to be completed for the CPS2000 coursework. CPS2000 is a
compulsory unit at the University of Malta which has to be taken
by third year Computer Science and Mathematics undergraduates
during the second semester.

## Dependencies

Please ensure that the following are installed:

- pgkconfig ^0.29.2
- cmake ^0.22.1

## Build Instructions

`.gitignore` is being used to make sure that all the files
generated by `cmake` and `latex` during compilation are not
pushed to the repository.

Additionally, for resolving the `fmtlib` dependency `git` might
be required by `cmake`.

The minimum required version for `cmake` is `3.20` and the
minimum required version for `latex` is `LuaHBTeX, Version
1.17.0 (TeX Live 2023)`.

Additionally, a compilers with support for C++17 were used.

Built and run on:

- Arch Linux (Linux Kernel 6.8.4 x86\_64) using `gcc` (Version
13.2.1).
- macOS Sonoma (Version 14.4 Apple M1) using `clang` (Version
15).

Starting from the root of the directory, run the `build.sh`
script.

```
./build.sh
```

This generates the `parl` executable in the `/build` directory.
It can be run from using the `run.sh` script.

```
./run.sh
```

## Testing

`valgrind` was used for profiling to ensure that no
memory is leaked.

## References

`fmtlib` was heavily used for string manipulation. The
repository can be found at
[fmtlib/fmt](https://github.com/fmtlib/fmt).

Crafting Interpreters heavily inspired many aspect of the
compiler. The book can be found at
[https://craftinginterpreters.com/](https://craftinginterpreters.com/).
