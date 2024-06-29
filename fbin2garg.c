#include <stdio.h>
#include <string.h>

#define MAX_FILENAME_LEN 1024

static char filename[MAX_FILENAME_LEN];
static char garg_filename[MAX_FILENAME_LEN];

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] =
"usage: fbin2garg filename\n";

static struct game curr_game;

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

static int build_garg_filename(
  char *bin_fllename,
  int bin_fllename_len,
  char *garg_filename,
  int max_filename_len);

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int n;
  int retval;
  FILE *fptr;
  int filename_len;
  int garg_filename_len;
  int chessist_file_from;
  int chessist_file_to;
  int chessist_rank_from;
  int chessist_rank_to;
  int garg_from;
  int garg_to;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  for ( ; ; ) {
    GetLine(fptr,filename,&filename_len,MAX_FILENAME_LEN);

    if (feof(fptr))
      break;

    filename_len = strlen(filename);

    retval = build_garg_filename(filename,filename_len,garg_filename,MAX_FILENAME_LEN);

    if (retval) {
      printf("build_garg_filename failed on %s: %d\n",filename,retval);
      continue;
    }

    bzero(&curr_game,sizeof (struct game));

    retval = read_binary_game(filename,&curr_game);

    if (retval) {
      printf("read_binary_game of %s failed: %d\n",filename,retval);
      printf("curr_move = %d\n",curr_game.curr_move);

      continue;
    }

    retval = write_binary_game(garg_filename,&curr_game);

    if (retval) {
      printf("write_binary_game of %s failed: %d\n",garg_filename,retval);
      continue;
    }
  }

  fclose(fptr);

  return 0;
}

static int build_garg_filename(
  char *bin_fllename,
  int bin_fllename_len,
  char *garg_filename,
  int max_filename_len)
{
  int n;

  for (n = 0; n < bin_fllename_len; n++) {
    if (bin_fllename[n] == '.')
      break;
  }

  if (n == bin_fllename_len)
    return 1;

  if (n + 5 > max_filename_len - 1)
    return 2;

  strncpy(garg_filename,bin_fllename,n);
  strcpy(&garg_filename[n],".garg");

  return 0;
}
