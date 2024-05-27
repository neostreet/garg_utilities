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

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: chessist_moves_to_garg orientation infile outfile\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int bHaveGame;
int afl_dbg;

static struct game curr_game;

int main(int argc,char **argv)
{
  int orientation;
  FILE *fptr;
  int line_len;
  int line_no;
  int move_ix;
  int from;
  int to;
  int special_move_info;
  int chessist_file_from;
  int chessist_file_to;
  int chessist_rank_from;
  int chessist_rank_to;
  int garg_from;
  int garg_to;

  if (argc != 4) {
    printf(usage);
    return 1;
  }

  sscanf(argv[1],"%d",&orientation);

  if ((fptr = fopen(argv[2],"r")) == NULL) {
    printf(couldnt_open,argv[2]);
    return 2;
  }

  line_no = 0;
  move_ix = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %d %x",
      &from,
      &to,
      &special_move_info);

    chessist_file_from = CHESSIST_FILE_OF(from);
    chessist_rank_from = CHESSIST_RANK_OF(from);
    chessist_file_to = CHESSIST_FILE_OF(to);
    chessist_rank_to = CHESSIST_RANK_OF(to);

    garg_from = POS_OF(chessist_rank_from,chessist_file_from + 1);
    garg_to = POS_OF(chessist_rank_to,chessist_file_to + 1);

    curr_game.orientation = orientation;
    curr_game.moves[move_ix].from = garg_from;
    curr_game.moves[move_ix].to = garg_to;
    curr_game.moves[move_ix++].special_move_info = special_move_info;
  }

  fclose(fptr);

  curr_game.num_moves = move_ix;
  curr_game.curr_move =  move_ix;
  write_binary_game(argv[3],&curr_game);

  return 0;
}
