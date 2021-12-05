# Find

POSIX - compatible utility for finding desired files in your filesystem. This utility can be accessed via command line.

## Compilation

To compile the utility, you will need a POSIX compliant system (such as GNU/Linux, MacOS, FreeBSD etc.) and a C compiler.
Compiling this utility is easy, please make sure you have the 'make' and 'gcc' utilities installed on your system:

```bash
# compile the project and clean the object files generated
make

# alternatively to see object files from our submodules
make find

# then to clean them
make clean

# to remove all files created by the compiler
make remove
```

## Usage

To find out how the utility works, simply run it in a following manner:
(Only short versions of options are available at the moment, the support might get added in a future patch)

```bash
./find -h
```
