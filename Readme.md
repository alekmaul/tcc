# TinyCC Compiler for the WDC 65816

- [TinyCC Compiler for the WDC 65816](#tinycc-compiler-for-the-wdc-65816)
  - [About](#about)
    - [Some considerations](#some-considerations)
  - [Getting Started](#getting-started)
    - [Build it](#build-it)
    - [Generate the documentation](#generate-the-documentation)
    - [Use it](#use-it)
  - [License](#license)
  - [Contributing](#contributing)
  - [Acknowledgements](#acknowledgements)


## About

TinyCC Compiler for the WDC 65816 for the [WDC 65816](https://en.wikipedia.org/wiki/WDC_65C816) processor.

This projet is a fork of the [TinyCC Compiler](https://github.com/TinyCC/tinycc).

It does not manage the linkage or the creation of binary but only produces the assembly code.

The produced code is:
* Strongly typed [Wla-dx](https://github.com/vhelin/wla-dx)
* Primarily intended for use with the [Pvsneslib](https://github.com/alekmaul/pvsneslib) SDK and for creating games on the [SNES console](https://en.wikipedia.org/wiki/Super_Nintendo_Entertainment_System).
* Not optimized and is best used in conjunction with the dedicated [816-opt](https://github.com/alekmaul/pvsneslib/tree/develop/tools/816-opt) and [constify](https://github.com/alekmaul/pvsneslib/tree/develop/tools/constify) optimizers.

### Some considerations

TCC is mostly implemented in C90. **Do not use any non-C90 features** that are not already in use.

Non-C90 features currently in use, as revealed by:

```bash
./configure --extra-cflags="-std=c90 -Wpedantic"
```
- long long (including "LL" constants)
- inline
- very long string constants
- assignment between function pointer and 'void *'
- "//" comments
- unnamed struct and union fields (in struct Sym), a C11 feature
  
For more informations, see [CodingStyle](CodingStyle)

## Getting Started

### Build it

Compile it using the `make` command.

```bash
make
```

### Generate the documentation

Generate the documentation in `docs/html` like this.

```bash
make docs
```

> The current documentation is still in progress and can be incomplete or inaccurate.

### Use it

See the help option.

```bash
./816-tcc -h

usage: 816-tcc [-v] [-c] [-o outfile] [-Idir] [-Wwarn] [infile1 infile2...]

General options:
  -v          display current version, increase verbosity
  -c          compile only - generate an object file
  -o outfile  set output filename
  -Wwarning   set or reset (with 'no-' prefix) 'warning' (see man page)
  -w          disable all warnings
Preprocessor options:
  -E          preprocess only
  -Idir       add include path 'dir'

```

## License

TCC is distributed under the GNU Lesser General Public License (see [COPYING](COPYING) and [RELICENSING](RELICENSING)).

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

## Contributing

See [CONTRIBUTING](CONTRIBUTING.md).

## Acknowledgements

The main contributors of this project:

- **All the community of the original project TinyCC**.
- **Ulrich Hecht**
- [Alekmaul](https://github.com/alekmaul)
- [Kobenairb](https://github.com/kobenairb)

And all the other contributors that I forget to name.
