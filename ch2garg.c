#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LEN 1024

static char garg_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 8192
static char line[MAX_LINE_LEN];

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] =
"usage: ch2garg (-debug) (-ignore_read_errors) filename\n";

static struct game curr_game;

static int build_garg_filename(
  char *ch_filename,
  int ch_filename_len,
  char *garg_filename,
  int max_filename_len);

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bIgnoreReadErrors;
  int ch_filename_len;
  int retval;

  if ((argc < 2) || (argc > 4)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bIgnoreReadErrors = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-ignore_read_errors"))
      bIgnoreReadErrors = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }


  ch_filename_len = strlen(argv[curr_arg]);

  retval = build_garg_filename(argv[curr_arg],ch_filename_len,garg_filename,MAX_FILENAME_LEN);

  if (retval) {
    printf("build_garg_filename failed on %s: %d\n",argv[curr_arg],retval);
    return 3;
  }

  bzero(&curr_game,sizeof (struct game));

  retval = read_game(argv[curr_arg],&curr_game,err_msg);

  if (retval) {
    if (!bIgnoreReadErrors) {
      printf("read_game of %s failed: %d\n",argv[curr_arg],retval);
      printf("curr_move = %d\n",curr_game.curr_move);
      return 4;
    }
    else
      printf("ignoring read error of %s at move %d\n",argv[curr_arg],curr_game.curr_move);
  }

  retval = write_binary_game(garg_filename,&curr_game);

  if (retval) {
    printf("write_binary_game of %s failed: %d\n",garg_filename,retval);
    return 5;
  }

  return 0;
}

static int build_garg_filename(
  char *ch_filename,
  int ch_filename_len,
  char *garg_filename,
  int max_filename_len)
{
  int n;

  for (n = 0; n < ch_filename_len; n++) {
    if (ch_filename[n] == '.')
      break;
  }

  if (n == ch_filename_len)
    return 1;

  if (n + 5 > max_filename_len - 1)
    return 2;

  strncpy(garg_filename,ch_filename,n);
  strcpy(&garg_filename[n],".garg");

  return 0;
}
