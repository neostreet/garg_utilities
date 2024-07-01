#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] =
"usage: fix_broken_garg_games (-verbose) filename\n";

int bHaveGame;
int afl_dbg;

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  int retval;
  FILE *fptr;
  struct game curr_game;
  bool bBlack;
  int dbg;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  bzero(&curr_game,sizeof (struct game));

  retval = read_binary_game(argv[curr_arg],&curr_game);

  if (retval) {
    printf("read_binary_game of %s failed: %d\n",argv[curr_arg],retval);
    printf("curr_move = %d\n",curr_game.curr_move);

    return 3;
  }

  set_initial_board(&curr_game);

  for (curr_game.curr_move = 0; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move++) {
    bBlack = curr_game.curr_move & 0x1;
    update_board(&curr_game,NULL,NULL);
    update_piece_info(&curr_game);

    if (curr_game.curr_move == dbg_move)
      dbg = 1;

    if (player_is_in_check(bBlack,curr_game.board)) {
      if (bVerbose)
        print_bd0(curr_game.board,curr_game.orientation);

      // truncate the game at the point where it went south
      printf("fixing %s\n",argv[curr_arg]);
      curr_game.moves[curr_game.curr_move - 1].special_move_info |= SPECIAL_MOVE_CHECK;
      curr_game.num_moves = curr_game.curr_move;

      // return to the previous move
      set_initial_board(&curr_game);

      for (curr_game.curr_move = 0; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move++) {
        update_board(&curr_game,NULL,NULL);
        update_piece_info(&curr_game);
      }

      if (bVerbose)
        print_bd0(curr_game.board,curr_game.orientation);

      legal_moves_count = 0;
      get_legal_moves(&curr_game,&legal_moves[0],&legal_moves_count);

      if (bVerbose)
        printf("legal_moves_count = %d\n",legal_moves_count);

      if (!legal_moves_count) {
        curr_game.moves[curr_game.curr_move - 1].special_move_info |= SPECIAL_MOVE_MATE;
      }

      retval = write_binary_game(argv[curr_arg],&curr_game);

      if (retval)
        printf("write_binary_game of %s failed: %d\n",argv[curr_arg],retval);

      break;
    }
  }

  return 0;
}
