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
"usage: orient_garg_mates (-verbose) filename\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;
  int new_orientation;

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

    if (curr_game.moves[curr_game.num_moves-1].special_move_info & SPECIAL_MOVE_MATE) {
      new_orientation = -1;

      if (curr_game.num_moves % 2) {
        if (curr_game.orientation) {
          new_orientation = 0;
        }
      }
      else {
        if (!curr_game.orientation) {
          new_orientation = 1;
        }
      }

      if (new_orientation != -1) {
        if (bVerbose)
          printf("%s\n",filename);

        curr_game.orientation = new_orientation;

        retval = write_binary_game(filename,&curr_game);

        if (retval) {
          printf("write_binary_game of %s failed: %d\n",filename,retval);
          continue;
        }
      }
    }
  }

  return 0;
}
