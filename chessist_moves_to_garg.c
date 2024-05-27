#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

#define CHESSIST_NUM_FILES 8

#define CHESSIST_RANK_OF(pos) ((pos) / CHESSIST_NUM_FILES)
#define CHESSIST_FILE_OF(pos) ((pos) % CHESSIST_NUM_FILES)

static char usage[] =
"usage: chessist_moves_to_garg infile outfile\n";

int bHaveGame;
int afl_dbg;

static struct game curr_game;

int main(int argc,char **argv)
{
  int n;
  int retval;
  int chessist_file_from;
  int chessist_file_to;
  int chessist_rank_from;
  int chessist_rank_to;
  int garg_from;
  int garg_to;

  if (argc != 3) {
    printf(usage);
    return 1;
  }

  retval = read_binary_game(argv[1],&curr_game);

  if (retval) {
    printf("read_binary_game of %s failed: %d\n",argv[1],retval);
    printf("curr_move = %d\n",curr_game.curr_move);

    return 2;
  }

  for (n = 0; n < curr_game.num_moves; n++) {
    chessist_file_from = CHESSIST_FILE_OF(curr_game.moves[n].from);
    chessist_rank_from = CHESSIST_RANK_OF(curr_game.moves[n].from);
    chessist_file_to = CHESSIST_FILE_OF(curr_game.moves[n].to);
    chessist_rank_to = CHESSIST_RANK_OF(curr_game.moves[n].to);

    garg_from = POS_OF(chessist_rank_from,chessist_file_from + 1);
    garg_to = POS_OF(chessist_rank_to,chessist_file_to + 1);

    curr_game.moves[n].from = garg_from;
    curr_game.moves[n].to = garg_to;
  }

  write_binary_game(argv[2],&curr_game);

  return 0;
}
