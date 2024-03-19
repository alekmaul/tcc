/*
 *  TCC - Tiny C Compiler
 *
 *  Copyright (c) 2001-2004 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef TCC_USE_LIBTCC
#include "tcc.h"
#else
#include "libtcc.c"
#endif

/**
 * @brief Display help information.
 *
 * The `help` function is responsible for displaying help information.
 * It provides guidance, instructions, or explanations about the usage and features of the program.
 *
 * This function does not take any parameters or return a value.
 * It typically prints the help text to the standard output or displays it in some user interface.
 */
void help(void)
{
    printf(
        "tcc version " TCC_VERSION " - Tiny C Compiler - Copyright (C) 2001-2006 Fabrice Bellard\n"
#ifdef TCC_TARGET_816
        "\n- WDC 65816 ASM code generator for Tiny C Compiler - Copyright (C) 2007 Ulrich Hecht\n"
        "- Modified for PVSneslib by Alekmaul in 2021\n"
        "- Updated by Kobenairb in 2022\n"
        "- Added support for Mode 21 (HiRom) Memory Mapping and FastRom by DigiDwrf in 2024\n\n"
        "usage: 816-tcc [-v] [-c] [-H] [-F] [-o outfile] [-Idir] [-Wwarn] [infile1 infile2...]\n"
        "\n"
        "General options:\n"
        "  -v          display current version, increase verbosity\n"
        "  -c          compile only - generate an object file\n"
        "  -H          hiRom (Mode 21) Memory Map compilation\n"
        "  -F          FastRom compilation\n"
        "  -o outfile  set output filename\n"
        "  -Wwarning   set or reset (with 'no-' prefix) 'warning' (see man page)\n"
        "  -w          disable all warnings\n"
        "Preprocessor options:\n"
        "  -E          preprocess only\n"
        "  -Idir       add include path 'dir'\n"
#else
        "usage: tcc [-v] [-c] [-H] [-F] [-o outfile] [-Bdir] [-bench] [-Idir] [-Dsym[=val]] "
        "[-Usym]\n"
        "           [-Wwarn] [-g] [-b] [-bt N] [-Ldir] [-llib] [-shared] [-soname name]\n"
        "           [-static] [infile1 infile2...] [-run infile args...]\n"
        "\n"
        "General options:\n"
        "  -v          display current version, increase verbosity\n"
        "  -c          compile only - generate an object file\n"
        "  -H          hiRom (Mode 21) Memory Map compilation\n"
        "  -F          FastRom compilation\n"
        "  -o outfile  set output filename\n"
        "  -Bdir       set tcc internal library path\n"
        "  -bench      output compilation statistics\n"
        "  -run        run compiled source\n"
        "  -fflag      set or reset (with 'no-' prefix) 'flag' (see man page)\n"
        "  -Wwarning   set or reset (with 'no-' prefix) 'warning' (see man page)\n"
        "  -w          disable all warnings\n"
        "Preprocessor options:\n"
        "  -E          preprocess only\n"
        "  -Idir       add include path 'dir'\n"
        "  -Dsym[=val] define 'sym' with value 'val'\n"
        "  -Usym       undefine 'sym'\n"
        "Linker options:\n"
        "  -Ldir       add library path 'dir'\n"
        "  -llib       link with dynamic or static library 'lib'\n"
        "  -shared     generate a shared library\n"
        "  -soname     set name for shared library to be used at runtime\n"
        "  -static     static linking\n"
        "  -rdynamic   export all global symbols to dynamic linker\n"
        "  -r          generate (relocatable) object file\n"
        "Debugger options:\n"
        "  -g          generate runtime debug info\n"
#ifdef CONFIG_TCC_BCHECK
        "  -b          compile with built-in memory and bounds checker (implies -g)\n"
#endif
#ifdef CONFIG_TCC_BACKTRACE
        "  -bt N       show N callers in stack traces\n"
#endif
#endif
    );
}

static char **files;
static int nb_files, nb_libraries;
static int multiple_files;
static int print_search_dirs;
static int output_type;
static int reloc_output;
static const char *outfile;
static int do_bench = 0;

#define TCC_OPTION_HAS_ARG 0x0001
#define TCC_OPTION_NOSEP 0x0002 /* cannot have space before option and arg */

/**
 * @struct TCCOption
 * @brief Structure representing a TCC option.
 */
typedef struct TCCOption
{
    const char *name; /**< Option name */
    uint16_t index;   /**< Option index */
    uint16_t flags;   /**< Option flags */
} TCCOption;

/* The above code is defining an enumeration in the C programming language. The enumeration consists of
various options that can be used with a program or library built with the Tiny C Compiler (TCC).
Each option is assigned a unique integer value starting from 0. */
enum {
    TCC_OPTION_HELP,
    TCC_OPTION_I,
    TCC_OPTION_D,
    TCC_OPTION_U,
    TCC_OPTION_L,
    TCC_OPTION_B,
    TCC_OPTION_l,
    TCC_OPTION_bench,
    TCC_OPTION_bt,
    TCC_OPTION_b,
    TCC_OPTION_g,
    TCC_OPTION_c,
    TCC_OPTION_static,
    TCC_OPTION_shared,
    TCC_OPTION_soname,
    TCC_OPTION_o,
    TCC_OPTION_r,
    TCC_OPTION_Wl,
    TCC_OPTION_W,
    TCC_OPTION_O,
    TCC_OPTION_m,
    TCC_OPTION_f,
    TCC_OPTION_nostdinc,
    TCC_OPTION_nostdlib,
    TCC_OPTION_print_search_dirs,
    TCC_OPTION_rdynamic,
    TCC_OPTION_run,
    TCC_OPTION_v,
    TCC_OPTION_w,
    TCC_OPTION_pipe,
    TCC_OPTION_E,
    TCC_OPTION_x,
    TCC_OPTION_H,
    TCC_OPTION_F,
};

/**
 * @brief Array of TCC options.
 */
static const TCCOption tcc_options[] = {
    {"h", TCC_OPTION_HELP, 0},                                  /**< Print help message */
    {"?", TCC_OPTION_HELP, 0},                                  /**< Print help message */
    {"I", TCC_OPTION_I, TCC_OPTION_HAS_ARG},                    /**< Include directory */
    {"D", TCC_OPTION_D, TCC_OPTION_HAS_ARG},                    /**< Define macro */
    {"U", TCC_OPTION_U, TCC_OPTION_HAS_ARG},                    /**< Undefine macro */
    {"L", TCC_OPTION_L, TCC_OPTION_HAS_ARG},                    /**< Library directory */
    {"B", TCC_OPTION_B, TCC_OPTION_HAS_ARG},                    /**< Linker option */
    {"l", TCC_OPTION_l, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP}, /**< Link library */
    {"bench", TCC_OPTION_bench, 0},                             /**< Run benchmark */
    {"bt", TCC_OPTION_bt, TCC_OPTION_HAS_ARG},                  /**< Set traceback level */
#ifdef CONFIG_TCC_BCHECK
    {"b", TCC_OPTION_b, 0}, /**< Perform bounds checking */
#endif
    {"g", TCC_OPTION_g, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP},     /**< Generate debug info */
    {"c", TCC_OPTION_c, 0},                                         /**< Compile only */
    {"static", TCC_OPTION_static, 0},                               /**< Generate static library */
    {"shared", TCC_OPTION_shared, 0},                               /**< Generate shared library */
    {"soname", TCC_OPTION_soname, TCC_OPTION_HAS_ARG},              /**< Set shared library name */
    {"o", TCC_OPTION_o, TCC_OPTION_HAS_ARG},                        /**< Output file */
    {"run", TCC_OPTION_run, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP}, /**< Run the compiled code */
    {"rdynamic", TCC_OPTION_rdynamic, 0}, /**< Export symbols to the dynamic symbol table */
    {"r", TCC_OPTION_r, 0},               /**< Generate relocatable output */
    {"Wl,", TCC_OPTION_Wl, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP}, /**< Pass option to linker */
    {"W", TCC_OPTION_W, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP},    /**< Warning level */
    {"O", TCC_OPTION_O, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP},    /**< Optimization level */
    {"m", TCC_OPTION_m, TCC_OPTION_HAS_ARG}, /**< Set architecture-specific options */
    {"f", TCC_OPTION_f, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP}, /**< Compiler flag */
    {"nostdinc", TCC_OPTION_nostdinc, 0}, /**< Do not search standard include directories */
    {"nostdlib", TCC_OPTION_nostdlib, 0}, /**< Do not use standard system libraries */
    {"print-search-dirs", TCC_OPTION_print_search_dirs, 0},     /**< Print search directories */
    {"v", TCC_OPTION_v, TCC_OPTION_HAS_ARG | TCC_OPTION_NOSEP}, /**< Verbose output */
    {"w", TCC_OPTION_w, 0},                                     /**< Suppress warning messages */
    {"pipe", TCC_OPTION_pipe, 0},            /**< Use pipes for intermediate output */
    {"E", TCC_OPTION_E, 0},                  /**< Preprocess only */
    {"x", TCC_OPTION_x, TCC_OPTION_HAS_ARG}, /**< Specify input language */
    {"H", TCC_OPTION_H, 0},                  /**< HiRom compiler */
    {"F", TCC_OPTION_F, 0},                  /**< FastRom compiler */
    {NULL},                                  /**< Null-terminated option */
};

/**
 * @brief Get the current time in microseconds.
 * @return The current time in microseconds.
 */
static int64_t getclock_us(void)
{
#ifdef _WIN32
    struct _timeb tb;
    _ftime(&tb);
    return (tb.time * 1000LL + tb.millitm) * 1000LL;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000LL + tv.tv_usec;
#endif
}

/**
 * @brief Check if a string starts with a given value.
 *
 * This function checks if the string pointed to by @p str starts with the
 * value pointed to by @p val. If the @p ptr parameter is not NULL, it can be
 * used to store a pointer to the character in @p str that follows the matched
 * value.
 *
 * @param str The string to check.
 * @param val The value to compare.
 * @param ptr Pointer to store the character after the matched value.
 *
 * @return 1 if the string starts with the value, 0 otherwise.
 */
static int strstart(const char *str, const char *val, const char **ptr)
{
    const char *p, *q;
    p = str;
    q = val;
    while (*q != '\0') {
        if (*p != *q)
            return 0;
        p++;
        q++;
    }
    if (ptr)
        *ptr = p;
    return 1;
}

/**
 * @brief Expand command-line arguments from a string.
 *
 * This function expands command-line arguments from a string by splitting it
 * into individual arguments based on whitespace. The expanded arguments are
 * stored in the dynamically allocated array `argv`, and the number of arguments
 * is returned as `argc`.
 *
 * @param pargv Pointer to store the expanded arguments.
 * @param str The input string containing the command-line arguments.
 *
 * @return The number of expanded arguments.
 */
static int expand_args(char ***pargv, const char *str)
{
    const char *s1;
    char **argv, *arg;
    int argc, len;

    argc = 0;
    argv = NULL;
    for (;;) {
        while (is_space(*str))
            str++;
        if (*str == '\0')
            break;
        s1 = str;
        while (*str != '\0' && !is_space(*str))
            str++;
        len = str - s1;
        arg = tcc_malloc(len + 1);
        memcpy(arg, s1, len);
        arg[len] = '\0';
        dynarray_add((void ***) &argv, &argc, arg);
    }
    *pargv = argv;
    return argc;
}

/**
 * @brief The function parses command line arguments and sets various options and flags accordingly.
 *
 * @param s A pointer to a TCCState struct, which contains the state of the Tiny C Compiler.
 * @param argc The number of command line arguments passed to the program, including the name of the
 * program itself.
 * @param argv `argv` is a pointer to an array of strings, where each string represents a command line
 * argument passed to the program.
 *
 * @return an integer value, which is the index of the next argument after the last one processed by
 * the function.
 */
int parse_args(TCCState *s, int argc, char **argv)
{
    int optind;
    const TCCOption *popt;
    const char *optarg, *p1, *r1;
    char *r;

#ifdef TCC_TARGET_816
    s->output_format = TCC_OUTPUT_FORMAT_BINARY;
#endif

    optind = 0;
    while (optind < argc) {
        r = argv[optind++];
        if (r[0] != '-' || r[1] == '\0') {
            /* add a new file */
            dynarray_add((void ***) &files, &nb_files, r);
            if (!multiple_files) {
                optind--;
                /* argv[0] will be this file */
                break;
            }
        } else {
            /* find option in table (match only the first chars */
            popt = tcc_options;
            for (;;) {
                p1 = popt->name;
                if (p1 == NULL)
                    error("invalid option -- '%s'", r);
                r1 = r + 1;
                for (;;) {
                    if (*p1 == '\0')
                        goto option_found;
                    if (*r1 != *p1)
                        break;
                    p1++;
                    r1++;
                }
                popt++;
            }
        option_found:
            if (popt->flags & TCC_OPTION_HAS_ARG) {
                if (*r1 != '\0' || (popt->flags & TCC_OPTION_NOSEP)) {
                    optarg = r1;
                } else {
                    if (optind >= argc)
                        error("argument to '%s' is missing", r);
                    optarg = argv[optind++];
                }
            } else {
                if (*r1 != '\0')
                    return 0;
                optarg = NULL;
            }

            switch (popt->index) {
            case TCC_OPTION_HELP:
                return 0;

            case TCC_OPTION_I:
                if (tcc_add_include_path(s, optarg) < 0)
                    error("too many include paths");
                break;
            case TCC_OPTION_D: {
                char *sym, *value;
                sym = (char *) optarg;
                value = strchr(sym, '=');
                if (value) {
                    *value = '\0';
                    value++;
                }
                tcc_define_symbol(s, sym, value);
            } break;
            case TCC_OPTION_U:
                tcc_undefine_symbol(s, optarg);
                break;
            case TCC_OPTION_L:
                tcc_add_library_path(s, optarg);
                break;
            case TCC_OPTION_B:
                /* set tcc utilities path (mainly for tcc development) */
                tcc_set_lib_path(s, optarg);
                break;
            case TCC_OPTION_l:
                dynarray_add((void ***) &files, &nb_files, r);
                nb_libraries++;
                break;
            case TCC_OPTION_bench:
                do_bench = 1;
                break;
#ifdef CONFIG_TCC_BACKTRACE
            case TCC_OPTION_bt:
                num_callers = atoi(optarg);
                break;
#endif
#ifdef CONFIG_TCC_BCHECK
            case TCC_OPTION_b:
                s->do_bounds_check = 1;
                s->do_debug = 1;
                break;
#endif
            case TCC_OPTION_g:
                s->do_debug = 1;
                break;
            case TCC_OPTION_c:
                multiple_files = 1;
                output_type = TCC_OUTPUT_OBJ;
                break;
            case TCC_OPTION_H:
                s->hirom_comp = 1;
                break;
            case TCC_OPTION_F:
                s->fastrom_comp = 1;
                break;
            case TCC_OPTION_static:
                s->static_link = 1;
                break;
            case TCC_OPTION_shared:
                output_type = TCC_OUTPUT_DLL;
                break;
            case TCC_OPTION_soname:
                s->soname = optarg;
                break;
            case TCC_OPTION_o:
                multiple_files = 1;
                outfile = optarg;
                break;
            case TCC_OPTION_r:
                /* generate a .o merging several output files */
                reloc_output = 1;
                output_type = TCC_OUTPUT_OBJ;
                break;
            case TCC_OPTION_nostdinc:
                s->nostdinc = 1;
                break;
            case TCC_OPTION_nostdlib:
                s->nostdlib = 1;
                break;
            case TCC_OPTION_print_search_dirs:
                print_search_dirs = 1;
                break;
            case TCC_OPTION_run: {
                int argc1;
                char **argv1;
                argc1 = expand_args(&argv1, optarg);
                if (argc1 > 0) {
                    parse_args(s, argc1, argv1);
                }
                multiple_files = 0;
                output_type = TCC_OUTPUT_MEMORY;
            } break;
            case TCC_OPTION_v:
                do {
                    if (0 == s->verbose++)
                        printf("tcc version %s\n", TCC_VERSION);
                } while (*optarg++ == 'v');
                break;
            case TCC_OPTION_f:
                if (tcc_set_flag(s, optarg, 1) < 0 && s->warn_unsupported)
                    goto unsupported_option;
                break;
            case TCC_OPTION_W:
                if (tcc_set_warning(s, optarg, 1) < 0 && s->warn_unsupported)
                    goto unsupported_option;
                break;
            case TCC_OPTION_w:
                s->warn_none = 1;
                break;
            case TCC_OPTION_rdynamic:
                s->rdynamic = 1;
                break;
            case TCC_OPTION_Wl: {
                const char *p;
                if (strstart(optarg, "-Ttext,", &p)) {
                    s->text_addr = strtoul(p, NULL, 16);
                    s->has_text_addr = 1;
                } else if (strstart(optarg, "--section-alignment,", &p)) {
                    s->section_align = strtoul(p, NULL, 16);
                } else if (strstart(optarg, "--image-base,", &p)) {
                    s->text_addr = strtoul(p, NULL, 16);
                    s->has_text_addr = 1;
#ifdef TCC_TARGET_PE
                } else if (strstart(optarg, "--file-alignment,", &p)) {
                    s->pe_file_align = strtoul(p, NULL, 16);
                } else if (strstart(optarg, "--subsystem,", &p)) {
#if defined(TCC_TARGET_I386) || defined(TCC_TARGET_X86_64)
                    if (!strcmp(p, "native"))
                        s->pe_subsystem = 1;
                    else if (!strcmp(p, "console"))
                        s->pe_subsystem = 3;
                    else if (!strcmp(p, "gui"))
                        s->pe_subsystem = 2;
                    else if (!strcmp(p, "posix"))
                        s->pe_subsystem = 7;
                    else if (!strcmp(p, "efiapp"))
                        s->pe_subsystem = 10;
                    else if (!strcmp(p, "efiboot"))
                        s->pe_subsystem = 11;
                    else if (!strcmp(p, "efiruntime"))
                        s->pe_subsystem = 12;
                    else if (!strcmp(p, "efirom"))
                        s->pe_subsystem = 13;
#elif defined(TCC_TARGET_ARM)
                    if (!strcmp(p, "wince"))
                        s->pe_subsystem = 9;
#endif
                    else {
                        error("invalid subsystem '%s'", p);
                    }
#endif
                } else if (strstart(optarg, "--oformat,", &p)) {
#if defined(TCC_TARGET_PE)
                    if (strstart(p, "pe-", NULL)) {
#else
#if defined(TCC_TARGET_X86_64)
                    if (strstart(p, "elf64-", NULL)) {
#else
                    if (strstart(p, "elf32-", NULL)) {
#endif
#endif
                        s->output_format = TCC_OUTPUT_FORMAT_ELF;
                    } else if (!strcmp(p, "binary")) {
                        s->output_format = TCC_OUTPUT_FORMAT_BINARY;
                    } else
#ifdef TCC_TARGET_COFF
                        if (!strcmp(p, "coff")) {
                        s->output_format = TCC_OUTPUT_FORMAT_COFF;
                    } else
#endif
                    {
                        error("target %s not found", p);
                    }
                } else if (strstart(optarg, "-rpath=", &p)) {
                    s->rpath = p;
                } else {
                    error("unsupported linker option '%s'", optarg);
                }
            } break;
            case TCC_OPTION_E:
                output_type = TCC_OUTPUT_PREPROCESS;
                break;
            case TCC_OPTION_x:
                break;
            default:
                if (s->warn_unsupported) {
                unsupported_option:
                    warning("unsupported option '%s'", r);
                }
                break;
            }
        }
    }
    return optind + 1;
}

/**
 * @brief The entry point of the program.
 *
 * This function is the entry point of the program. It parses the command-line arguments,
 * sets up the TCC state, processes the input files, and generates the output based on
 * the specified options.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line arguments.
 *
 * @return The exit status of the program.
 */
int main(int argc, char **argv)
{
    int i;
    TCCState *s;
    int nb_objfiles, ret, optind;
    char objfilename[1024];
    int64_t start_time = 0;

    s = tcc_new();

#ifdef _WIN32
    tcc_set_lib_path_w32(s);
#endif
    output_type = TCC_OUTPUT_EXE;
    outfile = NULL;
    multiple_files = 1;
    files = NULL;
    nb_files = 0;
    nb_libraries = 0;
    reloc_output = 0;
    print_search_dirs = 0;
    ret = 0;

    optind = parse_args(s, argc - 1, argv + 1);
    if (print_search_dirs) {
        /* enough for Linux kernel */
        printf("install: %s/\n", s->tcc_lib_path);
        return 0;
    }
    if (optind == 0 || nb_files == 0) {
        if (optind && s->verbose)
            return 0;
        help();
        return 1;
    }

    nb_objfiles = nb_files - nb_libraries;

    /* if outfile provided without other options, we output an
       executable */
    if (outfile && output_type == TCC_OUTPUT_MEMORY)
        output_type = TCC_OUTPUT_EXE;

    /* check -c consistency : only single file handled. XXX: checks file type */
    if (output_type == TCC_OUTPUT_OBJ && !reloc_output) {
        /* accepts only a single input file */
        if (nb_objfiles != 1)
            error("cannot specify multiple files with -c");
        if (nb_libraries != 0)
            error("cannot specify libraries with -c");
    }

    if (output_type == TCC_OUTPUT_PREPROCESS) {
        if (!outfile) {
            s->outfile = stdout;
        } else {
            s->outfile = fopen(outfile, "w");
            if (!s->outfile)
                error("could not open '%s", outfile);
        }
    } else if (output_type != TCC_OUTPUT_MEMORY) {
        if (!outfile) {
            /* compute default outfile name */
            char *ext;
            const char *name = strcmp(files[0], "-") == 0 ? "a" : tcc_basename(files[0]);
            pstrcpy(objfilename, sizeof(objfilename), name);
            ext = tcc_fileextension(objfilename);
#ifdef TCC_TARGET_PE
            if (output_type == TCC_OUTPUT_DLL)
                strcpy(ext, ".dll");
            else if (output_type == TCC_OUTPUT_EXE)
                strcpy(ext, ".exe");
            else
#endif
                if (output_type == TCC_OUTPUT_OBJ && !reloc_output && *ext)
                strcpy(ext, ".o");
            else
                pstrcpy(objfilename, sizeof(objfilename), "a.out");
            outfile = objfilename;
        }
    }

    if (do_bench) {
        start_time = getclock_us();
    }

    tcc_set_output_type(s, output_type);

    /* compile or add each files or library */
    for (i = 0; i < nb_files && ret == 0; i++) {
        const char *filename;

        filename = files[i];
        if (filename[0] == '-' && filename[1]) {
            if (tcc_add_library(s, filename + 2) < 0) {
                error_noabort("cannot find %s", filename);
                ret = 1;
            }
        } else {
            if (1 == s->verbose)
                printf("-> %s\n", filename);
            if (tcc_add_file(s, filename) < 0)
                ret = 1;
        }
    }

    /* free all files */
    tcc_free(files);

    if (0 == ret) {
        if (do_bench)
            tcc_print_stats(s, getclock_us() - start_time);

#ifndef TCC_TARGET_816
        if (s->output_type == TCC_OUTPUT_PREPROCESS) {
            if (outfile)
                fclose(s->outfile);
        } else if (s->output_type == TCC_OUTPUT_MEMORY)
            ret = tcc_run(s, argc - optind, argv + optind);
        else
#endif
            ret = tcc_output_file(s, outfile) ? 1 : 0;
    }

    tcc_delete(s);

#ifdef MEM_DEBUG
    if (do_bench) {
        printf("memory: %d bytes, max = %d bytes\n", mem_cur_size, mem_max_size);
    }
#endif
    return ret;
}
