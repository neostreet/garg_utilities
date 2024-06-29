#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

#define MAX_FILENAME_LEN 256
static char filename[MAX_FILENAME_LEN];

int bHaveGame;
int afl_dbg;

static char usage[] =
"usage: validate_garg_checks (-verbose) (-terse) filename\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  bool bTerse;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;
  bool bBlack;
  bool bPrintedFilename;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bTerse = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bVerbose && bTerse) {
    printf("can't specify both -verbose and -terse\n");
    return 3;
  }

  if ((fptr = fopen(argv[argc-1],"r")) == NULL) {
    printf(couldnt_open,argv[argc-1]);
    return 4;
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

    bPrintedFilename = false;

    for (curr_game.curr_move = 0; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move++) {
      bBlack = curr_game.curr_move & 0x1;
      update_board(&curr_game,NULL,NULL);

      if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK) {
        if (!player_is_in_check(!bBlack,curr_game.board)) {
          if (!bPrintedFilename) {
            printf("%s\n",filename);

            if (bTerse)
              break;

            bPrintedFilename = true;
          }

          printf("couldn't find a check on move %d\n",curr_game.curr_move);
          print_bd0(curr_game.board,curr_game.orientation);
        }
      }
      else {
        if (player_is_in_check(!bBlack,curr_game.board)) {
          if (!bPrintedFilename) {
            printf("%s\n",filename);

            if (bTerse)
              break;

            bPrintedFilename = true;
          }

          printf("found a spurious check on move %d\n",curr_game.curr_move);

          if (bVerbose)
            print_bd0(curr_game.board,curr_game.orientation);
        }
      }
    }
  }

  return 0;
}
