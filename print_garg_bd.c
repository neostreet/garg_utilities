#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] = "usage: print_garg_bd (-debug) (-all_moves) (-crop) filename\n";

int bHaveGame;
int afl_dbg;

static struct game curr_game;

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bAllMoves;
  bool bCrop;
  int retval;
  bool bPrintedBoard;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bAllMoves = false;
  bCrop = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-all_moves"))
      bAllMoves = true;
    else if (!strcmp(argv[curr_arg],"-crop"))
      bCrop = true;
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
  curr_game.curr_move--;

  if (!bAllMoves) {
    printf("curr_move = %d\n",curr_game.curr_move);

    putchar(0x0a);

    if (!bCrop)
      print_bd(&curr_game);
    else
      print_bd_cropped(&curr_game);

    print_special_moves(&curr_game);
  }
  else {
    set_initial_board(&curr_game);
    curr_game.curr_move = 0;

    bPrintedBoard = false;

    for (curr_game.curr_move = 0; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move++) {
      update_board(&curr_game,NULL,NULL);

      if (bPrintedBoard)
        putchar(0x0a);

      printf("curr_move = %d\n",curr_game.curr_move);

      putchar(0x0a);

      if (!bCrop)
        print_bd(&curr_game);
      else
        print_bd_cropped(&curr_game);

      print_special_moves(&curr_game);
      bPrintedBoard = true;
    }
  }

  return 0;
}
