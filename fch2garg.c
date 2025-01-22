#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LEN 1024

static char filename[MAX_FILENAME_LEN];
static char garg_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 8192
static char line[MAX_LINE_LEN];

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] =
"usage: fch2garg (-debug) (-ignore_read_errors) (-extext) filename\n";

static struct game curr_game;

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

static int build_garg_filename(
  char *ch_filename,
  int ch_filename_len,
  char *garg_filename,
  int max_filename_len,
  char *ext);

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  bool bIgnoreReadErrors;
  char *ext;
  FILE *fptr0;
  int file_len;
  int ch_filename_len;
  int retval;
  char *cpt;

  if ((argc < 2) || (argc > 5)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bIgnoreReadErrors = false;
  ext = NULL;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-ignore_read_errors"))
      bIgnoreReadErrors = true;
    else if (!strncmp(argv[curr_arg],"-ext",4))
      ext = &argv[curr_arg][4];
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  cpt = getenv("DEBUG_FCH2GARG");

  if (cpt != NULL) {
    debug_level = atoi(cpt);
    debug_fptr = fopen("fch2garg.dbg","w");
  }
  else {
    debug_level = 0;
    debug_fptr = NULL;
  }

  for ( ; ; ) {
    GetLine(fptr0,filename,&file_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    ch_filename_len = strlen(filename);

    retval = build_garg_filename(filename,ch_filename_len,garg_filename,MAX_FILENAME_LEN,ext);

    if (retval) {
      printf("build_garg_filename failed on %s: %d\n",filename,retval);
      continue;
    }

    bzero(&curr_game,sizeof (struct game));

    retval = read_game(filename,&curr_game,err_msg);

    if (retval) {
      if (!bIgnoreReadErrors) {
        printf("read_game of %s failed: %d, curr_move = %d\n",filename,retval,curr_game.curr_move);
        continue;
      }
      else
        printf("ignoring read error of %s at move %d\n",filename,curr_game.curr_move);
    }

    retval = write_binary_game(garg_filename,&curr_game);

    if (retval) {
      printf("write_binary_game of %s failed: %d\n",garg_filename,retval);
      continue;
    }
  }

  return 0;
}

static int build_garg_filename(
  char *ch_filename,
  int ch_filename_len,
  char *garg_filename,
  int max_filename_len,
  char *ext)
{
  int n;

  for (n = 0; n < ch_filename_len; n++) {
    if (ch_filename[n] == '.')
      break;
  }

  if (n == ch_filename_len)
    return 1;

  if (!ext) {
    if (n + 5 > max_filename_len - 1)
      return 2;
  }
  else {
    if (n + 1 + strlen(ext) > max_filename_len - 1)
      return 3;
  }

  strncpy(garg_filename,ch_filename,n);

  if (!ext)
    strcpy(&garg_filename[n],".garg");
  else {
    garg_filename[n] = '.';
    strcpy(&garg_filename[n+1],ext);
  }

  return 0;
}
