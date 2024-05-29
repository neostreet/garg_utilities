#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] = "usage: garg_game_info (-debug) garg_game\n";

int bHaveGame;
int afl_dbg;

static struct game curr_game;

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  int retval;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDebug = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
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

  printf("%s\n",argv[curr_arg]);
  printf("title = %s\n",curr_game.title);
  printf("orientation = %d\n",curr_game.orientation);
  printf("num_moves = %d\n",curr_game.num_moves);
  printf("curr_move = %d\n",curr_game.curr_move);

  return 0;
}
