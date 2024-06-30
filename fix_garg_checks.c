#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

#define MAX_FILENAME_LEN 256
static char filename[MAX_FILENAME_LEN];

static char usage[] =
"usage: fix_garg_checks (-verbose) filename\n";

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
  int filename_len;
  struct game curr_game;
  bool bBlack;
  int check_count;

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

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  for ( ; ; ) {
    GetLine(fptr,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr))
      break;

    bzero(&curr_game,sizeof (struct game));

    retval = read_binary_game(filename,&curr_game);

    if (retval) {
      printf("read_binary_game of %s failed: %d\n",filename,retval);
      printf("curr_move = %d\n",curr_game.curr_move);

      continue;
    }

    set_initial_board(&curr_game);
    check_count = 0;

    for (curr_game.curr_move = 0; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move++) {
      bBlack = curr_game.curr_move & 0x1;
      update_board(&curr_game,NULL,NULL);
      update_piece_info(&curr_game);

      if (player_is_in_check(!bBlack,curr_game.board)) {
        curr_game.moves[curr_game.curr_move].special_move_info |= SPECIAL_MOVE_CHECK;
        check_count++;
      }
    }

    if (check_count) {
      legal_moves_count = 0;
      get_legal_moves(&curr_game,&legal_moves[0],&legal_moves_count);

      if (!legal_moves_count) {
        if (curr_game.moves[curr_game.num_moves-1].special_move_info & SPECIAL_MOVE_CHECK)
          curr_game.moves[curr_game.num_moves-1].special_move_info |= SPECIAL_MOVE_MATE;
        else
          curr_game.moves[curr_game.num_moves-1].special_move_info |= SPECIAL_MOVE_STALEMATE;
      }

      retval = write_binary_game(filename,&curr_game);

      if (retval) {
        printf("write_binary_game of %s failed: %d\n",filename,retval);

        continue;
      }
    }
  }

  return 0;
}
