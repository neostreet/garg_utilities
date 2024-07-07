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

int mate_counts[NUM_PIECE_TYPES_0];

static char usage[] =
"usage: count_garg_mate_pieces (-verbose) (-sort) filename\n";

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
  int last_piece;
  int ixs[NUM_PIECE_TYPES_0];
  int ix;
  int total_counts;

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

  for (n = 0; n < NUM_PIECE_TYPES_0; n++)
    mate_counts[n] = 0;

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
      last_piece = get_piece1(curr_game.board,curr_game.moves[curr_game.num_moves-1].to);

      if (last_piece < 0)
        last_piece *= -1;

      if (last_piece == 1) {
        if (bVerbose)
          printf("%s P\n",filename);

         mate_counts[0]++;
      }
      else {
        ix = last_piece - 2;

        if ((ix < 0) || (ix >= NUM_PIECE_TYPES)) {
          printf("%s: invalid last piece\n",filename);
          continue;
        }

        if (bVerbose)
          printf("%s %c\n",filename,piece_ids[ix]);

        mate_counts[ix + 1]++;
      }
    }
  }

  fclose(fptr);

  if (bVerbose)
    putchar(0x0a);

  total_counts = 0;

  for (n = 0; n < NUM_PIECE_TYPES_0; n++)
    ixs[n] = n;

  if (bSort)
    qsort(ixs,NUM_PIECE_TYPES_0,sizeof (int),elem_compare);

  for (n = 0; n < NUM_PIECE_TYPES_0; n++) {
    printf("%5d %c\n",mate_counts[ixs[n]],((ixs[n] == 0) ? 'P' : piece_ids[ixs[n]-1]));
    total_counts += mate_counts[ixs[n]];
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

  return mate_counts[ix2] - mate_counts[ix1];
}
