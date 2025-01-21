#include <stdio.h>
#include <string.h>

#include "garg.h"
#define MAKE_GLOBALS_HERE
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"

static char usage[] =
"usage: garg_flie_has_missed_mate (-binary_format) filename (filename ...)\n";

int bHaveGame;
int afl_dbg;

static struct move work_legal_moves[MAX_LEGAL_MOVES];

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bBinaryFormat;
  int retval;
  struct game curr_game;
  bool bBlacksMove;
  bool bMissedMate;
  struct game work_game;
  int work_legal_moves_count;

  if (argc < 2) {
    printf(usage);
    return 1;
  }

  bBinaryFormat = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-binary_format"))
      bBinaryFormat = true;
    else
      break;
  }

  if (argc - curr_arg < 1) {
    printf(usage);
    return 2;
  }

  for ( ; curr_arg < argc; curr_arg++) {
    bzero(&curr_game,sizeof (struct game));

    if (!bBinaryFormat) {
      retval = read_game(argv[curr_arg],&curr_game,err_msg);

      if (retval) {
        printf("read_game of %s failed: %d\n",argv[curr_arg],retval);
        printf("curr_move = %d\n",curr_game.curr_move);

        continue;
      }
    }
    else {
      retval = read_binary_game(argv[curr_arg],&curr_game);

      if (retval) {
        printf("read_binary_game of %s failed: %d\n",argv[curr_arg],retval);
        printf("curr_move = %d\n",curr_game.curr_move);

        continue;
      }
    }

    set_initial_board(&curr_game);
    bMissedMate = false;

    for (curr_game.curr_move = 0;
         curr_game.curr_move < curr_game.num_moves;
         update_board(&curr_game,NULL,NULL,false),update_piece_info(&curr_game),curr_game.curr_move++) {

      legal_moves_count = 0;
      get_legal_moves(&curr_game,legal_moves,&legal_moves_count);

      for (n = 0; n < legal_moves_count; n++) {
        // only search for alternative moves to what was actually played
        if ((legal_moves[n].from == curr_game.moves[curr_game.curr_move].from) &&
            (legal_moves[n].to == curr_game.moves[curr_game.curr_move].to)) {
          continue;
        }

        copy_game(&work_game,&curr_game);
        work_game.moves[work_game.curr_move].from = legal_moves[n].from;
        work_game.moves[work_game.curr_move].to = legal_moves[n].to;
        work_game.moves[work_game.curr_move].special_move_info = 0;
        update_board(&work_game,NULL,NULL,true);
        work_game.curr_move++;

        bBlacksMove = work_game.curr_move & 0x1;

        if (player_is_in_check(bBlacksMove,work_game.board,work_game.curr_move)) {
          // don't report alternative mates if there was a mate in the game at the same move number
          if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_MATE)
            ;
          else {
            work_legal_moves_count = 0;
            get_legal_moves(&work_game,work_legal_moves,&work_legal_moves_count);

            if (!work_legal_moves_count) {
              bMissedMate = true;
              break;
            }
          }
        }
      }
    }

    printf("%s: %s\n",argv[curr_arg],(bMissedMate ? "yes" : "no"));
  }

  return 0;
}
