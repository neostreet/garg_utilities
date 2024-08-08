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
"usage: find_mates_lite (-debug) (-no_preceding_mate_in_one) filename\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bNoPrecedingMateInOne;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bNoPrecedingMateInOne = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-no_preceding_mate_in_one"))
      bNoPrecedingMateInOne = true;
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
      if (!bNoPrecedingMateInOne) {
        printf("%s\n",filename);
      }
      else {
        if (!(curr_game.moves[curr_game.num_moves-2].special_move_info & SPECIAL_MOVE_MATE_IN_ONE)) {
          printf("%s\n",filename);
        }
      }
    }
  }

  return 0;
}
