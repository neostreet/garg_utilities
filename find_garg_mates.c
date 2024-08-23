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
"usage: find_garg_mates (-mine) (-not_mine) filename\n";

int bHaveGame;
int afl_dbg;

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bMine;
  bool bNotMine;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;
  unsigned char board[CHARS_IN_BOARD];

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bMine = false;
  bNotMine = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-mine"))
      bMine = true;
    else if (!strcmp(argv[curr_arg],"-not_mine"))
      bNotMine = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bMine && bNotMine) {
    printf("can't specify both -mine and -not_mine\n");
    return 3;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
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

    if (curr_game.moves[curr_game.num_moves-1].special_move_info & SPECIAL_MOVE_MATE) {
      if (!bMine && !bNotMine) {
        printf("%s\n",filename);
      }
      else if (bMine) {
        if (curr_game.num_moves % 2) {
          if (!curr_game.orientation) {
            printf("%s\n",filename);
          }
        }
        else {
          if (curr_game.orientation) {
            printf("%s\n",filename);
          }
        }
      }
      else {
        if (curr_game.num_moves % 2) {
          if (curr_game.orientation) {
            printf("%s\n",filename);
          }
        }
        else {
          if (!curr_game.orientation) {
            printf("%s\n",filename);
          }
        }
      }
    }
  }

  return 0;
}
