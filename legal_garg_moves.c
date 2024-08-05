#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] =
"usage: legal_garg_moves (-debug) (-hex) filename\n";

int bHaveGame;
int afl_dbg;

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bHex;
  int retval;
  struct game curr_game;

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

  if (!(curr_game.num_moves % 2)) {
    printf("White to move\n");
    print_piece_info2(curr_game.white_pieces);
  }
  else {
    printf("Black to move\n");
    print_piece_info2(curr_game.black_pieces);
  }

  putchar(0x0a);

  legal_moves_count =  0;

  get_legal_moves(&curr_game,legal_moves,&legal_moves_count);

  printf("%d legal_moves:\n\n",legal_moves_count);

  print_moves(legal_moves,legal_moves_count,bHex);

  return 0;
}
