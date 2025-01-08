#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] = "usage: print_bd0 filename orientation\n";

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int retval;
  unsigned char board[CHARS_IN_BOARD];
  int orientation;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  sscanf(argv[2],"%d",&orientation);

  retval = populate_board_from_bin_board_file(board,argv[1]);

  if (retval) {
    printf("populate_board_from_bin_board_file() failed on %s: %d\n",argv[1],retval);
    return 2;
  }

  print_bd0(board,orientation);

  return 0;
}
