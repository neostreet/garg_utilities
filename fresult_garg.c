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
"usage: fresult_garg (-debug) (-my_wins) (-my_draws) (-my_losses) filename\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bMyWins;
  bool bMyDraws;
  bool bMyLosses;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;
  bool bPrintedFilename;

  if ((argc < 2) || (argc > 6)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bMyWins = false;
  bMyDraws = false;
  bMyLosses = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-my_wins"))
      bMyWins = true;
    else if (!strcmp(argv[curr_arg],"-my_draws"))
      bMyDraws = true;
    else if (!strcmp(argv[curr_arg],"-my_losses"))
      bMyLosses = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((bMyWins && bMyDraws) ||
      (bMyWins && bMyLosses) ||
      (bMyDraws && bMyLosses)) {

    printf("can only specify one of the -my_* options\n");
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

    if (bMyWins) {
      if ((!curr_game.orientation && (curr_game.result == WHITE_WIN)) ||
          (curr_game.orientation && (curr_game.result == BLACK_WIN))) {

        printf("%s\n",filename);
      }
    }
    else if (bMyDraws) {
      if (curr_game.result == DRAW) {
        printf("%s\n",filename);
      }
    }
    else if (bMyLosses) {
      if ((!curr_game.orientation && (curr_game.result == BLACK_WIN)) ||
          (curr_game.orientation && (curr_game.result == WHITE_WIN))) {

        printf("%s\n",filename);
      }
    }
    else {
      printf("%d %s\n",curr_game.result,filename);
    }
  }

  fclose(fptr);

  return 0;
}
