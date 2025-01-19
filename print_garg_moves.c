#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] =
"usage: print_garg_moves (-debug) (-hex) (-move_numbers) (-binary_format) (-ignore_read_errors) filename\n";

int bHaveGame;
int afl_dbg;

static struct game curr_game;

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bHex;
  bool bMoveNumbers;
  bool bBinaryFormat;
  bool bIgnoreReadErrors;
  int initial_move;
  int retval;

  if ((argc < 2) || (argc > 7)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bHex = false;
  bMoveNumbers = false;
  bBinaryFormat = false;
  bIgnoreReadErrors = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-hex"))
      bHex = true;
    else if (!strcmp(argv[curr_arg],"-move_numbers"))
      bMoveNumbers = true;
    else if (!strcmp(argv[curr_arg],"-binary_format"))
      bBinaryFormat = true;
    else if (!strcmp(argv[curr_arg],"-ignore_read_errors"))
      bIgnoreReadErrors = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (!bBinaryFormat) {
    retval = read_game(argv[curr_arg],&curr_game,err_msg);

    if (retval) {
      if (!bIgnoreReadErrors) {
        printf("read_game of %s failed: %d\n",argv[curr_arg],retval);
        printf("curr_move = %d\n",curr_game.curr_move);
        return 3;
      }
      else
        printf("ignoring read error of %s at move %d\n",argv[curr_arg],curr_game.curr_move);
    }
  }
  else {
    retval = read_binary_game(argv[curr_arg],&curr_game);

    if (retval) {
      printf("read_binary_game of %s failed: %d\n",argv[curr_arg],retval);
      printf("curr_move = %d\n",curr_game.curr_move);
      return 4;
    }
  }

  printf("%s, num_moves = %d\n",argv[curr_arg],curr_game.num_moves);
  putchar(0x0a);
  print_bd(&curr_game);
  putchar(0x0a);

  print_moves(curr_game.moves,curr_game.num_moves,bHex,bMoveNumbers);

  return 0;
}
