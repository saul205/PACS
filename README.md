# PACS

Official repository of the Programming and Architecture of Computing Systems
course of the MS in Robotics, Graphics, and Computer Vision

## Code snippets from Class

The directory `code_examples` contains many of the small C++ programs and
fragments from the slides.

To compile them, you can use `cmake`. For example:

```bash
cd code_examples
mkdir build
cd build
cmake ../ # generate the Makefile with cmake
make # compile the examples
ls # list the examples
```

If you want to build and run the test on Windows, please consider using [CMake
with Visual
Studio](https://docs.microsoft.com/es-es/cpp/build/cmake-projects-in-visual-studio?view=vs-2019)
