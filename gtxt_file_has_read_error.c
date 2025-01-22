#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] =
"usage: gtxt_file_has_read_error filename (filename ...)\n";

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  int retval;
  struct game curr_game;

  if (argc < 2) {
    printf(usage);
    return 1;
  }

  for (curr_arg = 0; curr_arg < argc; curr_arg++) {
    bzero(&curr_game,sizeof (struct game));

    retval = read_game(argv[curr_arg],&curr_game,err_msg);

    printf("%s: %s\n",argv[curr_arg],(retval ? "yes" : "no"));
  }

  return 0;
}
