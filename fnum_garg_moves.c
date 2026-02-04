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
"usage: fnum_garg_moves (-normalize) (-ge_num_movesnum_moves) (-eq_num_movesnum_moves)\n"
"  (-lt_num_movesnum_moves) (-terse_modemode) (-even) (-odd)\n"
"  (-only_wins) (-only_draws) (-only_losses) (-i_am_white) (-i_am_black) filename\n";
//"  (-only_wins) (-only_draws) (-only_losses) (-i_am_white) (-i_am_black) (-date) filename\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

int bHaveGame;
int afl_dbg;

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  bool bNormalize;
  int num_moves;
  int ge_num_moves;
  int eq_num_moves;
  int lt_num_moves;
  int terse_mode;
  bool bEven;
  bool bOdd;
  bool bOnlyWins;
  bool bOnlyDraws;
  bool bOnlyLosses;
  bool bIAmWhite;
  bool bIAmBlack;
  //bool bDate;
  int retval;
  FILE *fptr;
  int filename_len;
  struct game curr_game;

  //if ((argc < 2) || (argc > 15)) {
  if ((argc < 2) || (argc > 14)) {
    printf(usage);
    return 1;
  }

  terse_mode = 0;
  bNormalize = false;
  ge_num_moves = -1;
  eq_num_moves = -1;
  lt_num_moves = -1;
  bEven = false;
  bOdd = false;
  bOnlyWins = false;
  bOnlyDraws = false;
  bOnlyLosses = false;
  bIAmWhite = false;
  bIAmBlack = false;
  //bDate = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-normalize"))
      bNormalize = true;
    else if (!strncmp(argv[curr_arg],"-ge_num_moves",13))
      sscanf(&argv[curr_arg][13],"%d",&ge_num_moves);
    else if (!strncmp(argv[curr_arg],"-eq_num_moves",13))
      sscanf(&argv[curr_arg][13],"%d",&eq_num_moves);
    else if (!strncmp(argv[curr_arg],"-lt_num_moves",13))
      sscanf(&argv[curr_arg][13],"%d",&lt_num_moves);
    else if (!strncmp(argv[curr_arg],"-terse_mode",11))
      sscanf(&argv[curr_arg][11],"%d",&terse_mode);
    else if (!strcmp(argv[curr_arg],"-even"))
      bEven = true;
    else if (!strcmp(argv[curr_arg],"-odd"))
      bOdd = true;
    else if (!strcmp(argv[curr_arg],"-only_wins"))
      bOnlyWins = true;
    else if (!strcmp(argv[curr_arg],"-only_draws"))
      bOnlyDraws = true;
    else if (!strcmp(argv[curr_arg],"-only_losses"))
      bOnlyLosses = true;
    else if (!strcmp(argv[curr_arg],"-i_am_white"))
      bIAmWhite = true;
    else if (!strcmp(argv[curr_arg],"-i_am_black"))
      bIAmBlack = true;
    //else if (!strcmp(argv[curr_arg],"-date"))
      //bDate = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if (bOnlyWins and bOnlyDraws) {
    printf("can't specify both -only_wins and -only_draws\n");
    return 3;
  }

  if (bOnlyWins and bOnlyLosses) {
    printf("can't specify both -only_wins and -only_losses\n");
    return 4;
  }

  if (bOnlyDraws and bOnlyLosses) {
    printf("can't specify both -only_draws and -only_losses\n");
    return 5;
  }

  if ((ge_num_moves != -1) && (eq_num_moves != -1)) {
    printf("can't specify both -ge_num_moves and -eq_num_moves\n");
    return 6;
  }

  if ((ge_num_moves != -1) && (lt_num_moves != -1)) {
    printf("can't specify both -ge_num_moves and -lt_num_moves\n");
    return 7;
  }

  if ((eq_num_moves != -1) && (lt_num_moves != -1)) {
    printf("can't specify both -eq_num_moves and -lt_num_moves\n");
    return 8;
  }

  if (bEven && bOdd) {
    printf("can't specify both -even and -odd\n");
    return 9;
  }

  if (bIAmWhite and bIAmBlack) {
    printf("can't specify both -i_am_white and -i_am_black\n");
    return 10;
  }

  if ((fptr = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 11;
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

    if (bIAmWhite && curr_game.orientation)
      continue;

    if (bIAmBlack && !curr_game.orientation)
      continue;

#ifdef UNDEF
    if (bOnlyWins || bOnlyDraws || bOnlyLosses) {
      if (bOnlyWins) {
        if (curr_game.result != RESULT_WIN)
          continue;
      }
      else if (bOnlyDraws) {
        if (curr_game.result != RESULT_DRAW)
          continue;
      }
      else if (bOnlyLosses) {
        if (curr_game.result != RESULT_LOSS)
          continue;
      }
    }
#endif

    if (!bNormalize)
      num_moves = curr_game.num_moves;
    else
      num_moves = (curr_game.num_moves + 1) / 2;

    if (ge_num_moves != -1) {
      if (num_moves >= ge_num_moves) {
        if (!terse_mode) {
          //if (!bDate)
            printf("%d %s\n",num_moves,filename);
          //else
            //printf("%d %s %s\n",num_moves,filename,curr_game.date);
        }
        else if (terse_mode == 1)
          printf("%d\n",num_moves);
        else {
          //if (!bDate)
            printf("%s\n",filename);
          //else
            //printf("%s %s\n",filename,curr_game.date);
        }
      }
    }
    else if (eq_num_moves != -1) {
      if (num_moves == eq_num_moves) {
        if (!terse_mode) {
          //if (!bDate)
            printf("%d %s\n",num_moves,filename);
          //else
            //printf("%d %s %s\n",num_moves,filename,curr_game.date);
        }
        else if (terse_mode == 1)
          printf("%d\n",num_moves);
        else {
          //if (!bDate)
            printf("%s\n",filename);
          //else
            //printf("%s %s\n",filename,curr_game.date);
        }
      }
    }
    else if (lt_num_moves != -1) {
      if (num_moves < lt_num_moves) {
        if (!terse_mode) {
          //if (!bDate)
            printf("%d %s\n",num_moves,filename);
          //else
            //printf("%d %s %s\n",num_moves,filename,curr_game.date);
        }
        else if (terse_mode == 1)
          printf("%d\n",num_moves);
        else {
          //if (!bDate)
            printf("%s\n",filename);
          //else
            //printf("%s %s\n",filename,curr_game.date);
        }
      }
    }
    else {
      if (!terse_mode) {
        if (bEven) {
          if (!(num_moves % 2)) {
            //if (!bDate)
              printf("%d %s\n",num_moves,filename);
            //else
              //printf("%d %s %s\n",num_moves,filename,curr_game.date);
          }
        }
        else if (bOdd) {
          if (num_moves % 2) {
            //if (!bDate)
              printf("%d %s\n",num_moves,filename);
            //else
              //printf("%d %s %s\n",num_moves,filename,curr_game.date);
          }
        }
        else {
          //if (!bDate)
            printf("%d %s\n",num_moves,filename);
          //else
            //printf("%d %s %s\n",num_moves,filename,curr_game.date);
        }
      }
      else if (terse_mode == 1) {
        if (bEven) {
          if (!(num_moves % 2)) {
            //if (!bDate)
              printf("%d\n",num_moves);
            //else
              //printf("%d %s\n",num_moves,curr_game.date);
          }
        }
        else if (bOdd) {
          if (num_moves % 2) {
            //if (!bDate)
              printf("%d\n",num_moves);
            //else
              //printf("%d %s\n",num_moves,curr_game.date);
          }
        }
        else {
          //if (!bDate)
            printf("%d\n",num_moves);
          //else
            //printf("%d %s\n",num_moves,curr_game.date);
        }
      }
      else {
        if (bEven) {
          if (!(num_moves % 2)) {
            //if (!bDate)
              printf("%d\n",num_moves);
            //else
              //printf("%d %s\n",num_moves,curr_game.date);
          }
        }
        else if (bOdd) {
          if (num_moves % 2) {
            //if (!bDate)
              printf("%d\n",num_moves);
            //else
              //printf("%d %s\n",num_moves,curr_game.date);
          }
        }
        else {
          //if (!bDate)
            printf("%d\n",num_moves);
          //else
            //printf("%d %s\n",num_moves,curr_game.date);
        }
      }
    }
  }

  fclose(fptr);

  return 0;
}
