#ifndef COLOR_H // header guard
#define COLOR_H

#define COLOR_TEXT 1 // set to false to disable coloring the console output
#define NORMAL_TEXT printf(COLOR_TEXT?"\033[0m":"")
#define ERROR_TEXT printf(COLOR_TEXT?"\033[31m":"")
#define OOPS_TEXT printf(COLOR_TEXT?"\033[33m":"")
#define STRONG_TEXT printf(COLOR_TEXT?"\033[32m":"")

#endif // header guard
