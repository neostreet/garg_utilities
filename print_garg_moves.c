#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] = "usage: print_garg_moves (-debug) (-hex) filename\n";

int bHaveGame;
int afl_dbg;

static struct game curr_game;

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bHex;
  int initial_move;
  int retval;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bHex = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-hex"))
      bHex = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  retval = read_binary_game(argv[curr_arg],&curr_game);

  if (retval) {
    printf("read_binary_game of %s failed: %d\n",argv[curr_arg],retval);
    printf("curr_move = %d\n",curr_game.curr_move);

    return 3;
  }

  printf("%s, num_moves = %d\n",argv[curr_arg],curr_game.num_moves);
  putchar(0x0a);
  print_bd(&curr_game);
  putchar(0x0a);

  print_moves(curr_game.moves,curr_game.num_moves,bHex);

  return 0;
}
