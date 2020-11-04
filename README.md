# PACS

Official repository of the Programming and Architecture of Computing Systems
course of the MS in Robotics, Graphics, and Computer Vision

## Platform support

The code examples and some laboratories rely on
[cmake](https://cmake.org/overview/) to enable multi-platform building support,
so there is no need to manually write any Makefile or other build system. Linux
is the prefered operating system to work on the labs, but the binaries have
been also compiled on mac OS.

Although, we do not provide support for Windows. If you want to build and run
the test on Windows, please consider using [CMake with Visual
Studio](https://docs.microsoft.com/es-es/cpp/build/cmake-projects-in-visual-studio?view=vs-2019).

You can download the free community edition of [Visual Studio
2019](https://visualstudio.microsoft.com/downloads/), and then with you github
creentials download this repo and compile it.

One advantage of cmake is the separation between source and binary files. To
generate the programs, you always have to create a build directory first, then
invoke cmake to generate the build files, `Makefiles` for Linux, and, after
than, use the standard `make` tool for obtaining the binaries. By default,
cmake builds programs in release mode with optimizations enabled, if you want
to build them with debug support; e.g., to use them with `gdb`, you can set the
variable `CMAKE_BUILD_TYPE` to `Debug`.

## Code snippets from Class

The directory `code_examples` contains many of the small C++ programs and
fragments from the slides.

To compile them on linux or mac OS, you can use `cmake`. For example:

```bash
mkdir build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release ../ # generate the Makefile with cmake
make # compile the examples
ls # list the examples
```

If you want to compile for debugging, please note that at this point, we will
have two sets of binaries, set the variable `CMAKE_BUILD_TYPE` when calling
`cmake`:

```bash
mkdir build-debug
cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ../ # generate the Makefile with cmake
make -j4 # compile the examples in parallel with 4 jobs
```

Also if you want to see the actual build commands, there are several alternatives:

```bash
make VERBOSE=1
cmake --build . -- VERBOSE=1
```

## Laboratories

Each laboratory includes its own directory, but all of them can be built at the
same time if required. To build a single laboratory; e.g., the third laboratory,
please follow these steps:

```bash
mkdir -p build-release
cd build-release
rm -rf ./ # only run this command inside build-release directory
cmake ../
cd Laboratory-3
make -j2 # compile two jobs in parallel
```

_One workflow approach for the labs is to develop and test in debug mode,
inside a `build-debug` directory, and switch to release, in another
`build-release` directory, to run the final experiments and obtain execution
times or other metrics._
