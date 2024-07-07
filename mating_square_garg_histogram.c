#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

#define MAX_FILENAME_LEN 256
static char filename[MAX_FILENAME_LEN];

int mating_square_counts[NUM_BOARD_SQUARES];

static char usage[] =
"usage: mating_square_garg_histogram (-verbose) (-sort) filename\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int elem_compare(const void *elem1,const void *elem2);

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bVerbose;
  bool bSort;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;
  int mating_square;
  int total_counts;
  char algebraic[3];
  int ixs[NUM_BOARD_SQUARES];

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bVerbose = false;
  bSort = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-sort"))
      bSort = true;
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

  for (n = 0; n < NUM_BOARD_SQUARES; n++)
    mating_square_counts[n] = 0;

  algebraic[2] = 0;

  for ( ; ; ) {
    GetLine(fptr,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr))
      break;

    retval = read_binary_game(filename,&curr_game);

    if (retval) {
      printf("read_binary_game of %s failed: %d\n",filename,retval);
      printf("curr_move = %d\n",curr_game.curr_move);

      continue;
    }

    if (curr_game.moves[curr_game.num_moves-1].special_move_info & SPECIAL_MOVE_MATE) {
      mating_square = curr_game.moves[curr_game.num_moves-1].to;
      mating_square_counts[mating_square]++;
    }
  }

  fclose(fptr);

  total_counts = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++)
    ixs[n] = n;

  if (bSort)
    qsort(ixs,NUM_BOARD_SQUARES,sizeof (int),elem_compare);

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    if (bVerbose || mating_square_counts[ixs[n]]) {
      algebraic[0] = 'a' + FILE_OF(ixs[n]);
      algebraic[1] = '1' + RANK_OF(ixs[n]);
      printf("%5d %s\n",mating_square_counts[ixs[n]],algebraic);
      total_counts += mating_square_counts[ixs[n]];
    }
  }

  printf("\n%5d total\n",total_counts);

  return 0;
}

int elem_compare(const void *elem1,const void *elem2)
{
  int ix1;
  int ix2;

  ix1 = *(int *)elem1;
  ix2 = *(int *)elem2;

  return mating_square_counts[ix2] - mating_square_counts[ix1];
}
