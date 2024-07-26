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
"usage: find_garg_missed_mates (-terse) (-all) (in_a_loss) (-mine) (-opponent)\n"
"  (-count) filename\n";

int bHaveGame;
int afl_dbg;

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

static struct move work_legal_moves[MAX_LEGAL_MOVES];

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bTerse;
  bool bAll;
  bool bInALoss;
  bool bMine;
  bool bOpponent;
  bool bCount;
  bool bLoss;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;
  bool bBlack;
  struct game work_game;
  int work_legal_moves_count;
  int dbg;
  int count;

  if ((argc < 2) || (argc > 8)) {
    printf(usage);
    return 1;
  }

  bTerse = false;
  bAll = false;
  bInALoss = false;
  bMine = false;
  bOpponent = false;
  bCount = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-all"))
      bAll = true;
    else if (!strcmp(argv[curr_arg],"-in_a_loss"))
      bInALoss = true;
    else if (!strcmp(argv[curr_arg],"-mine"))
      bMine = true;
    else if (!strcmp(argv[curr_arg],"-opponent"))
      bOpponent = true;
    else if (!strcmp(argv[curr_arg],"-count"))
      bCount = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bMine && bOpponent) {
    printf("can't specify both -mine and -opponent\n");
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

    set_initial_board(&curr_game);

    if (bCount)
      count = 0;

    for (curr_game.curr_move = 0;
         curr_game.curr_move < curr_game.num_moves;
         update_board(&curr_game,NULL,NULL,false),update_piece_info(&curr_game),curr_game.curr_move++) {

      if (curr_game.curr_move == dbg_move)
        dbg = 1;

      if (bMine) {
        if (!curr_game.orientation) {
          if (curr_game.curr_move % 2)
            continue;
        }
        else {
          if (!(curr_game.curr_move % 2))
            continue;
        }
      }
      else if (bOpponent) {
        if (!curr_game.orientation) {
          if (!(curr_game.curr_move % 2))
            continue;
        }
        else {
          if (curr_game.curr_move % 2)
            continue;
        }
      }

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

        bBlack = work_game.curr_move & 0x1;

        if (player_is_in_check(bBlack,work_game.board,work_game.curr_move)) {
          // don't report alternative mates if there was a mate in the game at the same move number
          if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_MATE)
            ;
          else {
            work_legal_moves_count = 0;
            get_legal_moves(&work_game,work_legal_moves,&work_legal_moves_count);

            if (!work_legal_moves_count) {
              if (!bInALoss) {
                if (bCount) {
                  count++;
                }
                else {
                  if (bTerse) {
                    printf("%s\n",filename);
                  }
                  else {
                    printf("%s: a mate was missed on move %d, from = %c%c, to = %c%c:\n",
                      filename,curr_game.curr_move,
                      'a' + FILE_OF(legal_moves[n].from),'1' + RANK_OF(legal_moves[n].from),
                      'a' + FILE_OF(legal_moves[n].to),'1' + RANK_OF(legal_moves[n].to));
                    print_bd(&work_game);
                  }
                }

                break;
              }
              else {
                bLoss = false;

                if (!curr_game.orientation) {
                  if (curr_game.result == BLACK_WIN)
                    bLoss = true;
                }
                else {
                  if (curr_game.result == WHITE_WIN)
                    bLoss = true;
                }

                if (bLoss) {
                  if (bCount) {
                    count++;
                  }
                  else {
                    if (bTerse) {
                      printf("%s\n",filename);
                    }
                    else {
                      printf("%s: a mate was missed on move %d, from = %c%c, to = %c%c, in a loss:\n",
                        filename,curr_game.curr_move,
                        'a' + FILE_OF(legal_moves[n].from),'1' + RANK_OF(legal_moves[n].from),
                        'a' + FILE_OF(legal_moves[n].to),'1' + RANK_OF(legal_moves[n].to));
                      print_bd(&work_game);
                    }
                  }

                  break;
                }
              }
            }
          }
        }
      }

      if (!bAll && !bCount && (n < legal_moves_count))
        break;
    }

    if (bCount && count)
      printf("%d missed mates found in %s\n",count,filename);
  }

  fclose(fptr);

  return 0;
}
