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
"usage: fprint_bd (-debug) (-terse) (-toggle) (-initial_boardfilename) (-i_am_white) (-i_am_black)\n"
"  (-init_bin_boardfilename) (-board_binfilename) (-num_white_piecesnum) (-num_black_piecesnum)\n"
"  (-match_boardfilename) (-match_forcefilename) (-match_force2filename) (-only_checks) (-only_mates) (-only_castles)\n"
"  (-only_promotions) (-only_captures) (-only_en_passants) (-multiple_queens) (-move_number_only)\n"
"  (-mine) (-not_mine) (-search_all_moves) (-exact_match) (-only_no_promotions) (-only_underpromotions)\n"
"  (-print_piece_counts) (-only_no_checks) (-only_no_mates) (-opposite_colored_bishops) (-same_colored_bishops)\n"
"  (-two_bishops) (-opposite_side_castling) (-same_side_castling) (-less_than_2_castles) (-qnn) [white | black]\n"
"  filename\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

static int afl_dbg;

int main(int argc,char **argv)
{
  int n;
  int curr_arg;
  int orientation;
  bool bDebug;
  bool bTerse;
  bool bSearchAllMoves;
  bool bExactMatch;
  bool bToggle;
  bool bIAmWhite;
  bool bIAmBlack;
  bool bBoardBin;
  int num_white_pieces;
  int num_black_pieces;
  bool bOnlyChecks;
  bool bOnlyNoChecks;
  bool bOnlyMates;
  bool bOnlyNoMates;
  bool bOnlyCastles;
  bool bOnlyPromotions;
  bool bOnlyUnderpromotions;
  bool bOnlyNoPromotions;
  bool bOnlyCaptures;
  bool bOnlyEnPassants;
  bool bMultipleQueens;
  bool bMoveNumberOnly;
  bool bMine;
  bool bNotMine;
  bool bHaveMatchBoard;
  bool bHaveMatchForce;
  bool bPrintPieceCounts;
  bool bOppositeColoredBishops;
  bool bSameColoredBishops;
  bool bTwoBishops;
  bool bOppositeSideCastling;
  bool bSameSideCastling;
  bool bLessThan2Castles;
  bool bPrintedFilename;
  bool bPrintedBoard;
  bool bSkip;
  unsigned char match_board1[CHARS_IN_BOARD];
  unsigned char force_board1[CHARS_IN_BOARD];
  int match_piece_counts[NUM_PIECE_TYPES_0 * 2];
  int curr_piece_counts[NUM_PIECE_TYPES_0 * 2];
  int board_bin_arg;
  int quiz_number;
  bool bBlack;
  int initial_move;
  int retval;
  struct game curr_game;
  int match;
  FILE *fptr;
  int filename_len;
  int num_pieces;

  if ((argc < 2) || (argc > 39)) {
    printf(usage);
    return 1;
  }

  dbg_move = -1;
  bDebug = false;
  bTerse = false;
  bSearchAllMoves = false;
  bExactMatch = false;
  bToggle = false;
  bIAmWhite = false;
  bIAmBlack = false;
  quiz_number = -1;
  bBoardBin = false;
  num_white_pieces = -1;
  num_black_pieces = -1;
  bOnlyChecks = false;
  bOnlyNoChecks = false;
  bOnlyMates = false;
  bOnlyNoMates = false;
  bOnlyCastles = false;
  bOnlyPromotions = false;
  bOnlyUnderpromotions = false;
  bOnlyNoPromotions = false;
  bOnlyCaptures = false;
  bOnlyEnPassants = false;
  bMultipleQueens = false;
  bMoveNumberOnly = false;
  bMine = false;
  bNotMine = false;
  bHaveMatchBoard = false;
  bHaveMatchForce = false;
  bPrintPieceCounts = false;
  bOppositeColoredBishops = false;
  bSameColoredBishops = false;
  bTwoBishops = false;
  bOppositeSideCastling = false;
  bSameSideCastling = false;
  bLessThan2Castles = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else if (!strcmp(argv[curr_arg],"-terse"))
      bTerse = true;
    else if (!strcmp(argv[curr_arg],"-search_all_moves"))
      bSearchAllMoves = true;
    else if (!strcmp(argv[curr_arg],"-toggle"))
      bToggle = true;
    else if (!strcmp(argv[curr_arg],"-i_am_white"))
      bIAmWhite = true;
    else if (!strcmp(argv[curr_arg],"-i_am_black"))
      bIAmBlack = true;
    else if (!strncmp(argv[curr_arg],"-initial_board",14)) {
      retval = populate_initial_board_from_board_file(&argv[curr_arg][14]);

      if (retval) {
        printf("populate_initial_board_from_board_file() failed on 5s: %d\n",
          &argv[curr_arg][14],retval);
        return 2;
      }
    }
    else if (!strncmp(argv[curr_arg],"-init_bin_board",15)) {
      retval = populate_initial_board_from_bin_board_file(&argv[curr_arg][15]);

      if (retval) {
        printf("populate_initial_board_from_bin_board_file() failed on %s: %d\n",
          &argv[curr_arg][15],retval);
        return 3;
      }
    }
    else if (!strncmp(argv[curr_arg],"-num_white_pieces",17))
      sscanf(&argv[curr_arg][17],"%d",&num_white_pieces);
    else if (!strncmp(argv[curr_arg],"-num_black_pieces",17))
      sscanf(&argv[curr_arg][17],"%d",&num_black_pieces);
    else if (!strncmp(argv[curr_arg],"-board_bin",10)) {
      bBoardBin = true;
      board_bin_arg = curr_arg;
    }
    else if (!strncmp(argv[curr_arg],"-qn",3))
      sscanf(&argv[curr_arg][3],"%d",&quiz_number);
    else if (!strncmp(argv[curr_arg],"-match_board",12)) {
      bHaveMatchBoard = true;
      retval = populate_board_from_bin_board_file(match_board1,&argv[curr_arg][12]);

      if (retval) {
        printf("populate_board_from_board_file() failed on %s: %d\n",
          &argv[curr_arg][12],retval);
        return 4;
      }
    }
    else if (!strncmp(argv[curr_arg],"-match_force2",13)) {
      bHaveMatchForce = true;
      retval = populate_piece_counts_from_piece_count_file(match_piece_counts,&argv[curr_arg][13]);

      if (retval) {
        printf("populate_piece_counts_from_piece_count_file() failed on %s: %d\n",
          &argv[curr_arg][13],retval);
        return 5;
      }
    }
    else if (!strncmp(argv[curr_arg],"-match_force",12)) {
      bHaveMatchForce = true;
      retval = populate_board_from_bin_board_file(force_board1,&argv[curr_arg][12]);

      if (retval) {
        printf("populate_board_from_bin_board_file() failed on %s: %d\n",
          &argv[curr_arg][12],retval);
        return 6;
      }

      get_piece_counts(force_board1,match_piece_counts);
    }
    else if (!strcmp(argv[curr_arg],"-only_checks"))
      bOnlyChecks = true;
    else if (!strcmp(argv[curr_arg],"-only_no_checks"))
      bOnlyNoChecks = true;
    else if (!strcmp(argv[curr_arg],"-only_mates"))
      bOnlyMates = true;
    else if (!strcmp(argv[curr_arg],"-only_no_mates"))
      bOnlyNoMates = true;
    else if (!strcmp(argv[curr_arg],"-only_castles"))
      bOnlyCastles = true;
    else if (!strcmp(argv[curr_arg],"-only_promotions"))
      bOnlyPromotions = true;
    else if (!strcmp(argv[curr_arg],"-only_underpromotions"))
      bOnlyUnderpromotions = true;
    else if (!strcmp(argv[curr_arg],"-only_no_promotions"))
      bOnlyNoPromotions = true;
    else if (!strcmp(argv[curr_arg],"-only_captures"))
      bOnlyCaptures = true;
    else if (!strcmp(argv[curr_arg],"-only_en_passants"))
      bOnlyEnPassants = true;
    else if (!strcmp(argv[curr_arg],"-multiple_queens"))
      bMultipleQueens = true;
    else if (!strcmp(argv[curr_arg],"-move_number_only"))
      bMoveNumberOnly = true;
    else if (!strcmp(argv[curr_arg],"-mine"))
      bMine = true;
    else if (!strcmp(argv[curr_arg],"-not_mine"))
      bNotMine = true;
    else if (!strcmp(argv[curr_arg],"-exact_match"))
      bExactMatch = true;
    else if (!strcmp(argv[curr_arg],"-print_piece_counts"))
      bPrintPieceCounts = true;
    else if (!strcmp(argv[curr_arg],"-opposite_colored_bishops"))
      bOppositeColoredBishops = true;
    else if (!strcmp(argv[curr_arg],"-same_colored_bishops"))
      bSameColoredBishops = true;
    else if (!strcmp(argv[curr_arg],"-two_bishops"))
      bTwoBishops = true;
    else if (!strcmp(argv[curr_arg],"-opposite_side_castling"))
      bOppositeSideCastling = true;
    else if (!strcmp(argv[curr_arg],"-same_side_castling"))
      bSameSideCastling = true;
    else if (!strcmp(argv[curr_arg],"-less_than_2_castles"))
      bLessThan2Castles = true;
    else
      break;
  }

  if (bOnlyChecks && bOnlyCastles) {
    printf("can't specify both -only_checks and -only_castles\n");
    return 7;
  }

  if (bOnlyChecks && bOnlyNoChecks) {
    printf("can't specify both -only_checks and -only_no_checks\n");
    return 8;
  }

  if (bOnlyMates && bOnlyNoMates) {
    printf("can't specify both -only_mates and -only_no_mates\n");
    return 9;
  }

  if (bMine && bNotMine) {
    printf("can't specify both -mine and -not_mine\n");
    return 10;
  }

  if (bOnlyPromotions && bOnlyNoPromotions) {
    printf("can't specify both -only_promotions and -only_no_promotions\n");
    return 11;
  }

  if (bOnlyPromotions && bOnlyUnderpromotions) {
    printf("can't specify both -only_promotions and -only_underpromotions\n");
    return 12;
  }

  if (bOnlyNoPromotions && bOnlyUnderpromotions) {
    printf("can't specify both -only_no_promotions and -only_underpromotions\n");
    return 13;
  }

  if (bIAmWhite and bIAmBlack) {
    printf("can't specify both -i_am_white and -i_am_black\n");
    return 14;
  }

  if (quiz_number != -1) {
    if (argc - curr_arg != 2) {
      printf(usage);
      return 15;
    }

    if (!strcmp(argv[curr_arg],"white"))
      bBlack = false;
    else if (!strcmp(argv[curr_arg],"black"))
      bBlack = true;
    else {
      printf(usage);
      return 16;
    }
  }
  else {
    if (argc - curr_arg != 1) {
      printf(usage);
      return 17;
    }
  }

  if ((fptr = fopen(argv[argc-1],"r")) == NULL) {
    printf(couldnt_open,argv[argc-1]);
    return 18;
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

  if (!bOnlyChecks && !bOnlyNoChecks && !bOnlyMates && !bOnlyNoMates && !bOnlyCastles && !bOnlyCaptures &&
    !bOnlyEnPassants && !bMultipleQueens && !bOnlyPromotions && !bOnlyUnderpromotions && !bOnlyNoPromotions &&
    !bMine && !bNotMine && !bHaveMatchBoard && !bHaveMatchForce && (num_white_pieces == -1) &&
    (num_black_pieces == -1) && !bOppositeColoredBishops && !bSameColoredBishops && !bTwoBishops &&
    !bOppositeSideCastling && !bSameSideCastling && !bLessThan2Castles)
    printf("%s\n",filename);

  curr_game.curr_move--;
  orientation = curr_game.orientation;

  if (bToggle)
    curr_game.orientation ^= 1;

  if (bSearchAllMoves) {
    set_initial_board(&curr_game);
    curr_game.curr_move = 0;

    bPrintedFilename = false;
    bPrintedBoard = false;

    for (curr_game.curr_move = 0; curr_game.curr_move < curr_game.num_moves; curr_game.curr_move++) {
      update_board(&curr_game,NULL,NULL);

      if (curr_game.curr_move == dbg_move)
        afl_dbg = 1;

      if (bMine) {
        if ((curr_game.curr_move % 2) != orientation)
          continue;
      }
      else if (bNotMine) {
        if ((curr_game.curr_move % 2) == orientation)
          continue;
      }

      if (num_white_pieces != -1) {
        num_pieces = count_num_pieces(WHITE,&curr_game);

        if (num_pieces != num_white_pieces)
          continue;
      }

      if (num_black_pieces != -1) {
        num_pieces = count_num_pieces(BLACK,&curr_game);

        if (num_pieces != num_black_pieces)
          continue;
      }

      if (bHaveMatchBoard) {
        match = match_board(curr_game.board,match_board1,bExactMatch);

        if (!match)
          continue;
      }

      if (bHaveMatchForce) {
        get_piece_counts(curr_game.board,curr_piece_counts);

        if (!piece_counts_match(curr_piece_counts,match_piece_counts,bExactMatch))
          continue;
      }

      if (bOnlyChecks) {
        if (!(curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK))
          continue;
      }

      if (bOnlyNoChecks) {
        if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK)
          continue;
      }

      if (bOnlyMates) {
        if (!(curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_MATE))
          continue;
      }

      if (bOnlyNoMates) {
        if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_MATE)
          continue;
      }

      if (bOnlyCastles) {
        if (!(curr_game.moves[curr_game.curr_move].special_move_info & (SPECIAL_MOVE_KINGSIDE_CASTLE | SPECIAL_MOVE_QUEENSIDE_CASTLE)))
          continue;
      }

      if (bOnlyPromotions) {
        if (!(curr_game.moves[curr_game.curr_move].special_move_info & (SPECIAL_MOVE_PROMOTION_QUEEN | SPECIAL_MOVE_PROMOTION_ROOK | SPECIAL_MOVE_PROMOTION_KNIGHT | SPECIAL_MOVE_PROMOTION_BISHOP)))
          continue;
      }

      if (bOnlyUnderpromotions) {
        if (!(curr_game.moves[curr_game.curr_move].special_move_info & (SPECIAL_MOVE_PROMOTION_ROOK | SPECIAL_MOVE_PROMOTION_KNIGHT | SPECIAL_MOVE_PROMOTION_BISHOP)))
          continue;
      }

      if (bOnlyNoPromotions) {
        if (curr_game.moves[curr_game.curr_move].special_move_info & (SPECIAL_MOVE_PROMOTION_QUEEN | SPECIAL_MOVE_PROMOTION_ROOK | SPECIAL_MOVE_PROMOTION_KNIGHT | SPECIAL_MOVE_PROMOTION_BISHOP))
          continue;
      }

      if (bOnlyCaptures) {
        if (!(curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CAPTURE))
          continue;
      }

      if (bOnlyEnPassants) {
        if (!(curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE))
          continue;
      }

      if (bMultipleQueens) {
        if (!multiple_queens((unsigned char *)&curr_game.board))
          continue;
      }

      if (bOppositeColoredBishops) {
        if (!opposite_colored_bishops((unsigned char *)&curr_game.board))
          continue;
      }

      if (bSameColoredBishops) {
        if (!same_colored_bishops((unsigned char *)&curr_game.board))
          continue;
      }

      if (bTwoBishops) {
        if (!two_bishops((unsigned char *)&curr_game.board))
          continue;
      }

      if (bOppositeSideCastling) {
        if (!opposite_side_castling(&curr_game))
          continue;
      }

      if (bSameSideCastling) {
        if (!same_side_castling(&curr_game))
          continue;
      }

      if (bLessThan2Castles) {
        if (!less_than_2_castles(&curr_game))
          continue;
      }

      if (bOnlyChecks || bOnlyNoChecks || bOnlyMates || bOnlyNoMates || bOnlyCastles || bOnlyPromotions ||
        bOnlyUnderpromotions || bOnlyNoPromotions || bOnlyCaptures || bOnlyEnPassants || bMultipleQueens ||
        bHaveMatchBoard || bHaveMatchForce || bMine || bNotMine || bOppositeColoredBishops || bSameColoredBishops ||
        bTwoBishops || bOppositeSideCastling || bSameSideCastling || bLessThan2Castles) {

        if (!bPrintedFilename) {
          printf("%s\n",filename);

          if (bTerse)
            break;

          bPrintedFilename = true;
        }
      }

      if (bPrintedBoard)
        putchar(0x0a);

      printf("curr_move = %d\n",curr_game.curr_move);

      if (!bMoveNumberOnly) {
        putchar(0x0a);
        print_bd(&curr_game);
        print_special_moves(&curr_game);
        bPrintedBoard = true;

        if (bPrintPieceCounts) {
          get_piece_counts(curr_game.board,curr_piece_counts);
          print_piece_counts(curr_piece_counts);
        }
      }
    }
  }
  else {
    bPrintedBoard = false;

    if (quiz_number != -1) {
      initial_move = (quiz_number - 1) * 2;

      if (bBlack)
        initial_move++;

      if (initial_move >= curr_game.num_moves) {
        printf("initial_move must be < %d\n",curr_game.num_moves);
        continue;
      }

      set_initial_board(&curr_game);
      curr_game.curr_move = 0;

      for (n = 0; n <= initial_move; n++) {
        update_board(&curr_game,NULL,NULL);

        if (n < initial_move)
          curr_game.curr_move++;
      }
    }

    bSkip = false;

    if (bMine) {
      if ((curr_game.curr_move % 2) != orientation)
        bSkip = true;
    }
    else if (bNotMine) {
      if ((curr_game.curr_move % 2) == orientation)
        bSkip = true;
    }

    if (!bSkip) {
      if (num_white_pieces != -1) {
        num_pieces = count_num_pieces(WHITE,&curr_game);

        if (num_pieces != num_white_pieces)
          bSkip = true;
      }
    }

    if (!bSkip) {
      if (num_black_pieces != -1) {
        num_pieces = count_num_pieces(BLACK,&curr_game);

        if (num_pieces != num_black_pieces)
          bSkip = true;
      }
    }

    if (!bSkip && bHaveMatchBoard) {
      match = match_board(curr_game.board,match_board1,bExactMatch);

      if (!match)
        bSkip = true;
    }

    if (!bSkip && bHaveMatchForce) {
      get_piece_counts(curr_game.board,curr_piece_counts);

      if (!piece_counts_match(curr_piece_counts,match_piece_counts,bExactMatch))
        bSkip = true;
    }

    if (!bSkip && bOnlyChecks) {
      if (!(curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK))
        bSkip = true;
    }

    if (!bSkip && bOnlyNoChecks) {
      if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CHECK)
        bSkip = true;
    }

    if (!bSkip && bOnlyMates) {
      if (!(curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_MATE))
        bSkip = true;
    }

    if (!bSkip && bOnlyNoMates) {
      if (curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_MATE)
        bSkip = true;
    }

    if (!bSkip && bOnlyCastles) {
      if (!(curr_game.moves[curr_game.curr_move].special_move_info & (SPECIAL_MOVE_KINGSIDE_CASTLE | SPECIAL_MOVE_QUEENSIDE_CASTLE)))
        bSkip = true;
    }

    if (!bSkip && bOnlyPromotions) {
      if (!(curr_game.moves[curr_game.curr_move].special_move_info & (SPECIAL_MOVE_PROMOTION_QUEEN | SPECIAL_MOVE_PROMOTION_ROOK | SPECIAL_MOVE_PROMOTION_KNIGHT | SPECIAL_MOVE_PROMOTION_BISHOP)))
        bSkip = true;
    }

    if (!bSkip && bOnlyUnderpromotions) {
      if (!(curr_game.moves[curr_game.curr_move].special_move_info & (SPECIAL_MOVE_PROMOTION_ROOK | SPECIAL_MOVE_PROMOTION_KNIGHT | SPECIAL_MOVE_PROMOTION_BISHOP)))
        bSkip = true;
    }

    if (!bSkip && bOnlyNoPromotions) {
      if (curr_game.moves[curr_game.curr_move].special_move_info & (SPECIAL_MOVE_PROMOTION_QUEEN | SPECIAL_MOVE_PROMOTION_ROOK | SPECIAL_MOVE_PROMOTION_KNIGHT | SPECIAL_MOVE_PROMOTION_BISHOP))
        bSkip = true;
    }

    if (!bSkip && bOnlyCaptures) {
      if (!(curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_CAPTURE))
        bSkip = true;
    }

    if (!bSkip && bOnlyEnPassants) {
      if (!(curr_game.moves[curr_game.curr_move].special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE))
        bSkip = true;
    }

    if (!bSkip && bMultipleQueens) {
      if (!multiple_queens((unsigned char *)&curr_game.board))
        bSkip = true;
    }

    if (!bSkip && bOppositeColoredBishops) {
      if (!opposite_colored_bishops((unsigned char *)&curr_game.board))
        bSkip = true;
    }

    if (!bSkip && bSameColoredBishops) {
      if (!same_colored_bishops((unsigned char *)&curr_game.board))
        bSkip = true;
    }

    if (!bSkip && bTwoBishops) {
      if (!two_bishops((unsigned char *)&curr_game.board))
        bSkip = true;
    }

    if (!bSkip && bOppositeSideCastling) {
      if (!opposite_side_castling(&curr_game))
        bSkip = true;
    }

    if (!bSkip && bSameSideCastling) {
      if (!same_side_castling(&curr_game))
        bSkip = true;
    }

    if (!bSkip && bLessThan2Castles) {
      if (!less_than_2_castles(&curr_game))
        bSkip = true;
    }

    if (!bSkip) {
      if (bOnlyChecks || bOnlyNoChecks || bOnlyMates || bOnlyNoMates || bOnlyCastles ||
        bOnlyPromotions || bOnlyUnderpromotions || bOnlyNoPromotions ||
        bOnlyCaptures || bMultipleQueens || bHaveMatchBoard || bHaveMatchForce ||
        bMine || bNotMine || (num_white_pieces != -1) || (num_black_pieces != -1) ||
        bOppositeColoredBishops || bSameColoredBishops || bTwoBishops || bOppositeSideCastling ||
        bSameSideCastling || bLessThan2Castles) {
        printf("%s\n",filename);
      }

      if (!bTerse) {
        printf("curr_move = %d\n",curr_game.curr_move);
        putchar(0x0a);
        print_bd(&curr_game);
        print_special_moves(&curr_game);
        bPrintedBoard = true;

        if (bPrintPieceCounts) {
          get_piece_counts(curr_game.board,curr_piece_counts);
          print_piece_counts(curr_piece_counts);
        }
      }
    }
  }

  if (bBoardBin)
    write_board_to_binfile(curr_game.board,&argv[board_bin_arg][10]);

  if (bPrintedBoard)
    putchar(0x0a);

  }

  fclose(fptr);

  return 0;
}
