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
"usage: count_garg_checks (-debug) (-verbose) (-consecutive) (-game_ending) (-game_ending_countcount) (-by_player)\n"
" (-mate) (-mate_piecepiece) (-none) filename\n";

int bHaveGame;
int afl_dbg;

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bDebug;
  bool bVerbose;
  bool bConsecutive;
  bool bGameEnding;
  bool bByPlayer;
  bool bMate;
  bool bNone;
  int game_ending_count;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;
  bool bBlack;
  int num_checks;
  int total_num_checks;
  int num_checks1;
  int num_checks2;
  int total_num_checks1;
  int total_num_checks2;
  int max_consecutive_checks1;
  int max_consecutive_checks2;
  int check;
  bool bHaveCheck;
  char mate_piece;
  int last_piece;
  char last_piece_char;
  int dbg;

  if ((argc < 2) || (argc > 11)) {
    printf(usage);
    return 1;
  }

  bDebug = false;
  bVerbose = false;
  bConsecutive = false;
  bGameEnding = false;
  game_ending_count = 0;
  bByPlayer = false;
  bMate = false;
  mate_piece = ' ';
  bNone = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-verbose"))
      bVerbose = true;
    else if (!strcmp(argv[curr_arg],"-consecutive"))
      bConsecutive = true;
    else if (!strcmp(argv[curr_arg],"-game_ending"))
      bGameEnding = true;
    else if (!strncmp(argv[curr_arg],"-game_ending_count",18))
      sscanf(&argv[curr_arg][18],"%d",&game_ending_count);
    else if (!strcmp(argv[curr_arg],"-by_player"))
      bByPlayer = true;
    else if (!strcmp(argv[curr_arg],"-mate"))
      bMate = true;
    else if (!strncmp(argv[curr_arg],"-mate_piece",11))
      mate_piece = argv[curr_arg][11];
    else if (!strcmp(argv[curr_arg],"-none"))
      bNone = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bMate && bNone) {
    printf("can't specify both -mate and -none\n");
    return 3;
  }

  if ((fptr = fopen(argv[argc-1],"r")) == NULL) {
    printf(couldnt_open,argv[argc-1]);
    return 4;
  }

  if (!bVerbose)
    total_num_checks = 0;
  else {
    total_num_checks1 = 0;
    total_num_checks2 = 0;
  }

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

    if (bGameEnding) {
      num_checks = 0;

      for (curr_game.curr_move = curr_game.num_moves - 1; curr_game.curr_move >= 0; curr_game.curr_move -= 2) {
        if (curr_game.curr_move == dbg_move)
          dbg = 1;

        check = curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK;

        if (!check)
          break;
        else
          num_checks++;
      }

      if (num_checks) {
        if (!game_ending_count)
          printf("%d %s\n",num_checks,filename);
        else if (num_checks == game_ending_count)
          printf("%s\n",filename);
      }
    }
    else {
      num_checks1 = 0;
      num_checks2 = 0;

      if (!bConsecutive) {
        for (curr_game.curr_move = 0; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move++) {
          bBlack = curr_game.curr_move & 0x1;

          if (!bMate)
            bHaveCheck = curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK;
          else {
            bHaveCheck = curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_MATE;

            if (bHaveCheck) {
              if (mate_piece != ' ') {
                last_piece = get_piece1(curr_game.board,curr_game.moves[curr_game.num_moves-1].to);

                if (last_piece < 0)
                  last_piece *= -1;

                if (last_piece == 1)
                  last_piece_char = 'P';
                else
                  last_piece_char = piece_ids[last_piece - 2];

                if (last_piece_char != mate_piece)
                  bHaveCheck = false;
              }
            }
          }

          if (bHaveCheck) {
            if (!bByPlayer) {
              if (bBlack)
                num_checks2++;
              else
                num_checks1++;
            }
            else {
              if (!curr_game.orientation) {
                if (bBlack)
                  num_checks2++;
                else
                  num_checks1++;
              }
              else {
                if (bBlack)
                  num_checks1++;
                else
                  num_checks2++;
              }
            }
          }
        }
      }
      else {
        // calculate the number of checks for the first group

        num_checks = 0;
        max_consecutive_checks1 = 0;

        for (curr_game.curr_move = 0; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move += 2) {
          if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK)
            num_checks++;
          else {
            if (num_checks > max_consecutive_checks1)
              max_consecutive_checks1 = num_checks;

            num_checks = 0;
          }
        }

        if (num_checks > max_consecutive_checks1)
          max_consecutive_checks1 = num_checks;

        // calculate the number of checks for the second group

        num_checks = 0;
        max_consecutive_checks2 = 0;

        for (curr_game.curr_move = 1; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move += 2) {
          if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK)
            num_checks++;
          else {
            if (num_checks > max_consecutive_checks2)
              max_consecutive_checks2 = num_checks;

            num_checks = 0;
          }
        }

        if (num_checks > max_consecutive_checks2)
          max_consecutive_checks2 = num_checks;
      }

      if (!bConsecutive) {
        if ((!bNone && (num_checks1 + num_checks2)) || (bNone && !(num_checks1 + num_checks2))) {
          if (!bVerbose) {
            if (!bMate)
              printf("%d %s\n",num_checks1 + num_checks2,filename);
            else {
              if (!bDebug)
                printf("%s\n",filename);
              else
                printf("%s %d %x\n",filename,curr_game.num_moves,curr_game.moves[curr_game.num_moves-1].special_move_info); // for now
            }

            total_num_checks += num_checks1 + num_checks2;
          }
          else {
            if (!bByPlayer) {
              printf("%3d white, %3d black, %3d total %s\n",
                num_checks1,num_checks2,num_checks1 + num_checks2,filename);
            }
            else {
              printf("%3d me, %3d opponent, %3d total %s\n",
                num_checks1,num_checks2,num_checks1 + num_checks2,filename);
            }

            total_num_checks1 += num_checks1;
            total_num_checks2 += num_checks2;
          }
        }
      }
      else {
        if (!bByPlayer)
          printf("%d white, %d black %s\n",max_consecutive_checks1,max_consecutive_checks2,filename);
        else {
          if (!curr_game.orientation)
            printf("%d me, %d opponent %s\n",max_consecutive_checks1,max_consecutive_checks2,filename);
          else
            printf("%d me, %d opponent %s\n",max_consecutive_checks2,max_consecutive_checks1,filename);
        }
      }
    }
  }

  fclose(fptr);

  if (!bConsecutive && !bGameEnding) {
    if (!bVerbose) {
      if (!bMate)
        printf("\n%d\n",total_num_checks);
    }
    else {
      if (!bByPlayer)
        printf("\n%d white, %d black, %d total\n",total_num_checks1,total_num_checks2,total_num_checks1 + total_num_checks2);
      else
        printf("\n%d me, %d opponent, %d total\n",total_num_checks1,total_num_checks2,total_num_checks1 + total_num_checks2);
    }
  }

  return 0;
}
