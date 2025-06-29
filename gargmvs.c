#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "garg.h"
#include "garg.glb"
#include "garg.mac"
#include "garg.fun"

static struct game scratch;

int do_castle(struct game *gamept,int direction,char *word,int wordlen,struct move *move_ptr)
{
  int n;
  int rank;
  int squares_to_check[2];
  int special_move_info;

  if (debug_fptr && (debug_level == 11)) {
    fprintf(debug_fptr,"do_castle: piece_info:\n");
    fprint_piece_info(gamept,debug_fptr);
  }

  // don't allow castling if the player is in check
  if (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_CHECK) {
    return 1;
  }

  if (direction == 1)  /* if white's move: */
    rank = 0;
  else
    rank = 7;

  /* make sure the king is on his original square: */
  if (get_piece2(gamept->board,rank,4) != 6 * direction) {
    do_castle_failures++;
    return 2;
  }

  /* make sure the king hasn't previously moved */
  if (direction == 1) {
    if (gamept->white_pieces[4].move_count)
      return 3;
  }
  else {
    if (gamept->black_pieces[12].move_count)
      return 4;
  }

  if (wordlen == 3) {  /* kingside castle */
    /* make sure there is a rook in the corner: */
    if (get_piece2(gamept->board,rank,7) != 2 * direction) {
      do_castle_failures++;
      return 5;
    }

    /* make sure the rook hasn't previously moved */
    if (direction == 1) {
      if (gamept->white_pieces[7].move_count)
        return 6;
    }
    else {
      if (gamept->black_pieces[15].move_count)
        return 7;
    }

    /* make sure there are empty squares between king and rook: */
    if (get_piece2(gamept->board,rank,5) || get_piece2(gamept->board,rank,6)) {
      do_castle_failures++;
      return 8;
    }

    /* make sure the empty squares are not attacked */
    squares_to_check[0] = POS_OF(rank,5);
    squares_to_check[1] = POS_OF(rank,6);

    for (n = 0; n < 2; n++) {
      if (any_opponent_piece_attacks_square(squares_to_check[n],(direction != 1),gamept->board,gamept->curr_move)) {
        do_castle_failures++;
        return 9;
      }
    }

    if (direction == 1) {  /* if white's move: */
      move_ptr->from = 4;
      move_ptr->to = 6;
    }
    else {
      move_ptr->from = 60;
      move_ptr->to = 62;
    }

    special_move_info = SPECIAL_MOVE_KINGSIDE_CASTLE;
  }
  else if (wordlen == 5) {  /* gargside castle */
    /* make sure there is a rook in the corner: */
    if (get_piece2(gamept->board,rank,0) != 2 * direction) {
      do_castle_failures++;
      return 10;
    }

    /* make sure the rook hasn't previously moved */
    if (direction == 1) {
      if (gamept->white_pieces[0].move_count)
        return 11;
    }
    else {
      if (gamept->black_pieces[8].move_count)
        return 12;
    }

    /* make sure there are empty squares between king and rook: */
    if (get_piece2(gamept->board,rank,1) || get_piece2(gamept->board,rank,2) || get_piece2(gamept->board,rank,3)) {
      do_castle_failures++;
      return 13;
    }

    /* make sure the empty squares are not attacked */
    squares_to_check[0] = POS_OF(rank,2);
    squares_to_check[1] = POS_OF(rank,3);

    for (n = 0; n < 2; n++) {
      if (any_opponent_piece_attacks_square(squares_to_check[n],(direction != 1),gamept->board,gamept->curr_move)) {
        do_castle_failures++;
        return 14;
      }
    }

    if (direction == 1) {  /* if white's move: */
      move_ptr->from = 4;
      move_ptr->to = 2;
    }
    else {
      move_ptr->from = 60;
      move_ptr->to = 58;
    }

    special_move_info = SPECIAL_MOVE_GARGSIDE_CASTLE;
  }
  else {
    do_castle_failures++;
    return 15;
  }

  move_ptr->special_move_info = special_move_info;
  do_castle_successes++;

  return 0;  /* success */
}

int do_pawn_move(struct game *gamept,int direction,char *word,int wordlen,struct move *move_ptr)
{
  int n;
  int file;
  int to_rank;
  int rank;
  int capture_file;
  int piece;
  int which_piece;
  int dbg;
  int move_distance;

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  if (debug_fptr && (debug_level == 11))
    fprintf(debug_fptr,"do_pawn_move: curr_move = %d, word = %s\n",gamept->curr_move,word);

  /*printf("%s\n",word);/*for now*/
  file = word[0] - 'a';

  if ((file < 0) || (file > 7))
    return 1;

  /* pawn advance or capture: */
  rank = word[1] - '1';

  if ((rank >= 0) && (rank <= 7)) {
    to_rank = rank;

    /* pawn advance */
    move_ptr->special_move_info = 0;
    move_ptr->to = POS_OF(rank,file);

    for (n = 0; n < 2; n++) {
      rank -= direction;

      if ((rank >= 1) && (rank <= 6)) {
        piece = get_piece2(gamept->board,rank,file);

        if (piece == direction)
          break;

        if (piece)
          return 2;
      }
    }

    if (n == 2)
      return 3;

    if (n == 1) {
      /* only allow two square move from initial position */
      if (direction == 1) {
        if (rank != 1)
          return 4;
      }
      else if (rank != 6)
        return 5;
    }

    move_ptr->from = POS_OF(rank,file);

    /* handle pawn promotion */
    if (direction == 1) {
      /* white's move */
      if (to_rank == 7) {
        if (wordlen != 3)
          return 6;

        switch (word[2]) {
          case 'G':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_GARGANTUA;

            break;
          case 'R':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_ROOK;

            break;
          case 'N':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_KNIGHT;

            break;
          case 'B':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_BISHOP;

            break;
        }
      }
    }
    else {
      /* black's move */
      if (to_rank == 0) {
        if (wordlen != 3)
          return 8;

        switch (word[2]) {
          case 'G':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_GARGANTUA;

            break;
          case 'R':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_ROOK;

            break;
          case 'N':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_KNIGHT;

            break;
          case 'B':
            move_ptr->special_move_info =
              SPECIAL_MOVE_PROMOTION_BISHOP;

            break;
        }
      }
    }

    goto check_for_illegal_move;
  }

  /* pawn capture */
  capture_file = word[1] - 'a';

  if ((capture_file < 0) || (capture_file > 7))
    return 10;

  if (capture_file > file) {
    if (capture_file - file != 1)
      return 11;
  }
  else if (file - capture_file != 1)
    return 12;

  if ((wordlen >= 2) && (wordlen <= 4)) {
    if (direction == 1)
      rank = 1;
    else
      rank = 6;

    for (n = 0; n < 6; n++, rank += direction) {
      /*printf("%d %d\n",get_piece2(gamept->board,rank,file),
        get_piece2(gamept->board,rank+direction,capture_file));/*for now*/
      if ((get_piece2(gamept->board,rank,file) == direction) &&
        (get_piece2(gamept->board,rank+direction,capture_file) * direction < 0)) {

        if ((wordlen == 3) || (wordlen == 4))
          if (word[2] - '1' != rank+direction)
            continue;

        move_ptr->from = POS_OF(rank,file);
        move_ptr->to = POS_OF(rank+direction,capture_file);

        if (wordlen == 4) {
          switch (word[3]) {
            case 'G':
              move_ptr->special_move_info =
                SPECIAL_MOVE_PROMOTION_GARGANTUA;

              break;
            case 'R':
              move_ptr->special_move_info =
                SPECIAL_MOVE_PROMOTION_ROOK;

              break;
            case 'N':
              move_ptr->special_move_info =
                SPECIAL_MOVE_PROMOTION_KNIGHT;

              break;
            case 'B':
              move_ptr->special_move_info =
                SPECIAL_MOVE_PROMOTION_BISHOP;

              break;
          }
        }

        goto check_for_illegal_move;
      }
    }

    // handle en passant captures
    if (wordlen == 3) {
      if (direction == -1) {
        // black
        if (word[2] == '3') {
          if ((get_piece2(gamept->board,3,file) == direction) &&
              (get_piece2(gamept->board,3,capture_file) == direction * -1)) {
            move_ptr->from = POS_OF(3,file);
            move_ptr->to = POS_OF(2,capture_file);
            move_ptr->special_move_info |=
              SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;
            goto check_for_illegal_move;
          }
        }
      }
      else {
        // white
        if (word[2] == '6') {
          if ((get_piece2(gamept->board,4,file) == direction) &&
              (get_piece2(gamept->board,4,capture_file) == direction * -1)) {
            move_ptr->from = POS_OF(4,file);
            move_ptr->to = POS_OF(5,capture_file);
            move_ptr->special_move_info |=
              SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;
            goto check_for_illegal_move;
          }
        }
      }
    }

    return 13;
  }

  return 14;

check_for_illegal_move:

  if (!move_is_legal(gamept,move_ptr->from,move_ptr->to)) {
    if (debug_fptr && (debug_level == 11)) {
      fprintf(debug_fptr,"do_pawn_move: about to return 15, curr_move = %d\n",
        gamept->curr_move);
      fprint_bd3(gamept->board,gamept->orientation,debug_fptr);
    }

    return 15;
  }

  move_distance = gamept->moves[gamept->curr_move].from - gamept->moves[gamept->curr_move].to;

  if (move_distance < 0)
    move_distance *= -1;

  if (move_distance == NUM_FILES * 2) {
    gamept->moves[gamept->curr_move].special_move_info |= SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE;
  }

  return 0;
}

int do_pawn_move2(struct game *gamept)
{
  bool bWhiteMove;
  int start_rank;
  int start_file;
  int end_rank;
  int end_file;
  int rank_diff;
  int file_diff;
  int retval;

  bWhiteMove = (move_start_square_piece > 0);

  if (bWhiteMove) {
    // white pawn move

    if (move_start_square > move_end_square)
      return 1; // failure
  }
  else {
    // black pawn move

    if (move_start_square < move_end_square)
      return 2; // failure
  }

  start_rank = RANK_OF(move_start_square);
  start_file = FILE_OF(move_start_square);
  end_rank = RANK_OF(move_end_square);
  end_file = FILE_OF(move_end_square);

  if (start_rank >= end_rank)
    rank_diff = start_rank - end_rank;
  else
    rank_diff = end_rank - start_rank;

  if (start_file >= end_file)
    file_diff = start_file - end_file;
  else
    file_diff = end_file - start_file;

  if (file_diff == 0) {
    if (move_end_square_piece)
      return 3; // failure
  }

  if (rank_diff == 0)
    return 4; // failure

  if (file_diff > 1)
    return 5; // failure

  if (rank_diff > 2)
    return 6; // failure

  if (rank_diff > 1) {
    if (file_diff)
      return 7; // failure

    if (bWhiteMove) {
      if (start_rank != 1)
        return 8; // failure
    }
    else {
      if (start_rank != 6)
        return 9; // failure
    }
  }

  if (file_diff == 1) {
    if (rank_diff != 1)
      return 10; // failure

    if (!move_end_square_piece) {
      // check for en passant capture
      if (bWhiteMove && (start_rank == 4) &&
        (get_piece2(gamept->board,4,end_file) == PAWN_ID * -1) &&
        (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE)) {

        gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;
      }
      else if (!bWhiteMove && (start_rank == 3) &&
        (get_piece2(gamept->board,3,end_file) == PAWN_ID) &&
        (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE)) {

        gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;
      }
      else
        return 11; // failure
    }
  }

  if (!move_is_legal(gamept,move_start_square,move_end_square))
    return 12;

  gamept->moves[gamept->curr_move].from = move_start_square;
  gamept->moves[gamept->curr_move].to = move_end_square;
  retval = 0;

  if (rank_diff > 1)
    gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE;

  return retval; // success
}

int get_piece_id_ix(char piece)
{
  int which_piece;

  /* calculate the id to search for: */
  for (which_piece = 0; which_piece < NUM_PIECE_TYPES; which_piece++) {
    if (piece_ids[which_piece] == piece)
      break;
  }

  return which_piece;
}

int (*piece_functions[])(struct game *,int,int,int,int) = {
  rook_move,
  knight_move,
  bishop_move,
  gargantua_move,
  king_move
};

int (*piece_functions2[])(struct game *) = {
  rook_move2,
  knight_move2,
  bishop_move2,
  gargantua_move2,
  king_move2
};

int do_piece_move(struct game *gamept,int direction,char *word,int wordlen,struct move *move_ptr)
{
  int which_piece;
  int search_piece;
  int curr_file;
  int file_start;
  int file_end;
  int curr_rank;
  int rank_start;
  int rank_end;
  int where;
  int to_file;
  int to_rank;
  int to_piece;
  int retval;
  int dbg;

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  if (debug_fptr && (debug_level == 11))
    fprintf(debug_fptr,"do_piece_move: curr_move = %d, word = %s\n",gamept->curr_move,word);

  if (wordlen == 4) {
    where = word[1];

    if ((where >= 'a') && (where <= 'h')) {
      file_start = where - 'a';
      file_end = file_start + 1;
      rank_start = 0;
      rank_end = 8;
    }
    else if ((where >= '1') && (where <= '8')) {
      file_start = 0;
      file_end = 8;
      rank_start = where - '1';
      rank_end = rank_start + 1;
    }
    else
      return 1;
  }
  else {
    file_start = 0;
    file_end = 8;
    rank_start = 0;
    rank_end = 8;
  }

  if (!get_to_position(word,wordlen,&to_file,&to_rank))
    return 2;

  which_piece = get_piece_id_ix(word[0]);

  /* error if not found (should never happen): */
  if (which_piece == NUM_PIECE_TYPES)
    return 3;

  search_piece = (which_piece + 2) * direction; /* calculate search id */
  to_piece = get_piece2(gamept->board,to_rank,to_file);

  /* don't allow a capture of same color piece*/
  if (to_piece * direction > 0)
    return 4;

  move_ptr->special_move_info = 0;

  /* search the board for the search piece: */
  for (curr_rank = rank_start; curr_rank < rank_end; curr_rank++) {
    for (curr_file = file_start; curr_file < file_end; curr_file++) {
      if (get_piece2(gamept->board,curr_rank,curr_file) == search_piece) {
        /* see if a possible piece move: */
        retval = (*piece_functions[which_piece])(
          gamept,curr_file,curr_rank,to_file,to_rank);

        if (!retval) {
          move_ptr->from = POS_OF(curr_rank,curr_file);
          move_ptr->to = POS_OF(to_rank,to_file);

          if (move_is_legal(gamept,move_ptr->from,move_ptr->to))
            return 0;  /* success */
        }
      }
    }
  }

  return 5;
}

int do_piece_move2(struct game *gamept)
{
  int which_piece;
  int retval;

  which_piece = move_start_square_piece;

  if (which_piece < 0)
    which_piece *= -1;

  which_piece -= ROOK_ID;

  retval = (*piece_functions2[which_piece])(gamept);

  if (retval)
    return 1;

  if (!move_is_legal(gamept,move_start_square,move_end_square)) {
    gamept->moves[gamept->curr_move].special_move_info = 0;
    return 1;
  }

  gamept->moves[gamept->curr_move].from = move_start_square;
  gamept->moves[gamept->curr_move].to = move_end_square;
  return 0;  /* success */
}

int get_to_position(char *word,int wordlen,int *to_filept,int *to_rankpt)
{
  *to_filept = word[wordlen - 2] - 'a';

  if ((*to_filept < 0) || (*to_filept > 7))
    return false;

  *to_rankpt = word[wordlen - 1] - '1';

  if ((*to_rankpt < 0) || (*to_rankpt > 7))
    return false;

  return true;
}

int rook_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  int n;

  if (file1 == file2) {
    if (rank1 > rank2) {
      for (n = rank2 + 1; n < rank1; n++)
        if (get_piece2(gamept->board,n,file1))
          return 1;  /* failure */
    }
    else
      for (n = rank1 + 1; n < rank2; n++)
        if (get_piece2(gamept->board,n,file1))
          return 2;  /* failure */

    return 0;  /* success */
  }

  if (rank1 == rank2) {
    if (file1 > file2) {
      for (n = file2 + 1; n < file1; n++)
        if (get_piece2(gamept->board,rank1,n))
          return 1;  /* failure */
    }
    else
      for (n = file1 + 1; n < file2; n++)
        if (get_piece2(gamept->board,rank1,n))
          return 2;  /* failure */

    return 0;  /* success */
  }

  return 3;  /* failure */
}

int rook_move2(
  struct game *gamept
)
{
  int retval;

  retval = rook_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
}

int knight_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  int dist1;
  int dist2;

  dist1 = (file1 - file2);

  if (dist1 < 0)
    dist1 *= -1;

  dist2 = (rank1 - rank2);

  if (dist2 < 0)
    dist2 *= -1;

  if ((dist1 == 1) && (dist2 == 2))
    return 0;  /* success */

  if ((dist1 == 2) && (dist2 == 1))
    return 0;  /* success */

  return 1;    /* failure */
}

int knight_move2(
  struct game *gamept
)
{
  int retval;

  retval = knight_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
}

int bishop_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  int dist1;
  int dist2;
  int file_dir;
  int rank_dir;
  int piece;

  dist1 = (file1 - file2);

  if (dist1 < 0) {
    dist1 *= -1;
    file_dir = 1;
  }
  else
    file_dir = -1;

  dist2 = (rank1 - rank2);

  if (dist2 < 0) {
    dist2 *= -1;
    rank_dir = 1;
  }
  else
    rank_dir = -1;

  if (dist1 != dist2)
    return 1;  /* failure */

  /* make sure there are no intervening pieces */
  for ( ; ; ) {
    file1 += file_dir;

    if (file1 == file2)
      break;

    rank1 += rank_dir;

    piece = get_piece2(gamept->board,rank1,file1);

    if (piece)
      return 2;  /* failure */
  }

  return 0;  /* success */
}

int bishop_move2(
  struct game *gamept
)
{
  int retval;

  retval = bishop_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
}

int gargantua_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  if (!rook_move(gamept,file1,rank1,file2,rank2))
    return 0;  /* success */

  if (!bishop_move(gamept,file1,rank1,file2,rank2))
    return 0;  /* success */

  if (!knight_move(gamept,file1,rank1,file2,rank2))
    return 0;  /* success */

  return 1;    /* failure */
}

int gargantua_move2(
  struct game *gamept
)
{
  int retval;

  retval = gargantua_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
}

int king_move(
  struct game *gamept,
  int file1,
  int rank1,
  int file2,
  int rank2
)
{
  int n;
  int dist1;
  int dist2;
  int should_be_empty1;
  int should_be_empty2;
  int should_be_empty3;
  int should_be_rook;
  int squares_to_check[2];
  int dbg;

  if (debug_fptr && (debug_level == 11)) {
    fprintf(debug_fptr,"king_move: piece_info:\n");
    fprint_piece_info(gamept,debug_fptr);
  }

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  // first, check if this is a castling move
  if (!(gamept->curr_move % 2)) {
    // it's White's move

    // check for kingside castle
    if ((file1 == 4) && (rank1 == 0) && (file2 == 6) && (rank2 == 0)) {
      // don't allow castling if the player is in check
      if (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_CHECK) {
        ;
      }
      else {
        // don't allow castling if the king moves over or onto a controlled square
        squares_to_check[0] = 5;
        squares_to_check[1] = 6;

        for (n = 0; n < 2; n++) {
          if (any_opponent_piece_attacks_square(squares_to_check[n],false,gamept->board,gamept->curr_move))
            break;
        }

        if (n < 2) {
          ;
        }
        else {
          should_be_empty1 = get_piece1(gamept->board,5);
          should_be_empty2 = get_piece1(gamept->board,6);
          should_be_rook = get_piece1(gamept->board,7);

          if (!should_be_empty1 && !should_be_empty2 && (should_be_rook == ROOK_ID)) {
            // make sure the neither the king nor the rook has previously moved */
            if (!gamept->white_pieces[4].move_count && !gamept->white_pieces[7].move_count) {
              gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_KINGSIDE_CASTLE;
              return 0;
            }
          }
        }
      }
    }

    // check for gargside castle
    if ((file1 == 4) && (rank1 == 0) && (file2 == 2) && (rank2 == 0)) {
      // don't allow castling if the player is in check
      if (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_CHECK) {
        ;
      }
      else {
        // don't allow castling if the king moves over or onto a controlled square
        squares_to_check[0] = 2;
        squares_to_check[1] = 3;

        for (n = 0; n < 2; n++) {
          if (any_opponent_piece_attacks_square(squares_to_check[n],false,gamept->board,gamept->curr_move))
            break;
        }

        if (n < 2) {
          ;
        }
        else {
          should_be_empty1 = get_piece1(gamept->board,1);
          should_be_empty2 = get_piece1(gamept->board,2);
          should_be_empty3 = get_piece1(gamept->board,3);
          should_be_rook = get_piece1(gamept->board,0);

          if (!should_be_empty1 && !should_be_empty2 && !should_be_empty3 && (should_be_rook == ROOK_ID)) {
            // make sure the neither the king nor the rook has previously moved */
            if (!gamept->white_pieces[4].move_count && !gamept->white_pieces[0].move_count) {
              gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_GARGSIDE_CASTLE;
              return 0;
            }
          }
        }
      }
    }
  }
  else {
    // it's Black's move

    // check for kingside castle
    if ((file1 == 4) && (rank1 == 7) && (file2 == 6) && (rank2 == 7)) {
      // don't allow castling if the player is in check
      if (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_CHECK) {
        ;
      }
      else {
        // don't allow castling if the king moves over or onto a controlled square
        squares_to_check[0] = 61;
        squares_to_check[1] = 62;

        for (n = 0; n < 2; n++) {
          if (any_opponent_piece_attacks_square(squares_to_check[n],true,gamept->board,gamept->curr_move))
            break;
        }

        if (n < 2) {
          ;
        }
        else {
          should_be_empty1 = get_piece1(gamept->board,61);
          should_be_empty2 = get_piece1(gamept->board,62);
          should_be_rook = get_piece1(gamept->board,63);

          if (!should_be_empty1 && !should_be_empty2 && (should_be_rook == ROOK_ID * -1)) {
            // make sure the neither the king nor the rook has previously moved */
            if (!gamept->black_pieces[12].move_count && !gamept->black_pieces[15].move_count) {
              gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_KINGSIDE_CASTLE;
              return 0;
            }
          }
        }
      }
    }

    // check for gargside castle
    if ((file1 == 4) && (rank1 == 7) && (file2 == 2) && (rank2 == 7)) {
      // don't allow castling if the player is in check
      if (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_CHECK) {
        ;
      }
      else {
        // don't allow castling if the king moves over or onto a controlled square
        squares_to_check[0] = 58;
        squares_to_check[1] = 59;

        for (n = 0; n < 2; n++) {
          if (any_opponent_piece_attacks_square(squares_to_check[n],true,gamept->board,gamept->curr_move))
            break;
        }

        if (n < 2) {
          ;
        }
        else {
          should_be_empty1 = get_piece1(gamept->board,57);
          should_be_empty2 = get_piece1(gamept->board,58);
          should_be_empty3 = get_piece1(gamept->board,59);
          should_be_rook = get_piece1(gamept->board,56);

          if (!should_be_empty1 && !should_be_empty2 && !should_be_empty3 && (should_be_rook == ROOK_ID * -1)) {
            // make sure the neither the king nor the rook has previously moved */
            if (!gamept->black_pieces[12].move_count && !gamept->black_pieces[8].move_count) {
              gamept->moves[gamept->curr_move].special_move_info = SPECIAL_MOVE_GARGSIDE_CASTLE;
              return 0;
            }
          }
        }
      }
    }
  }

  dist1 = (file1 - file2);

  if (dist1 < 0)
    dist1 *= -1;

  dist2 = (rank1 - rank2);

  if (dist2 < 0)
    dist2 *= -1;

  if ((dist1 < 2) && (dist2 < 2))
    return 0;  /* success */

  return 1;  /* failure */
}

int king_move2(
  struct game *gamept
)
{
  int retval;

  retval = king_move(
    gamept,
    FILE_OF(move_start_square),
    RANK_OF(move_start_square),
    FILE_OF(move_end_square),
    RANK_OF(move_end_square)
    );

  return retval;
}


bool move_is_legal(struct game *gamept,char from,char to)
{
  // don't allow moves which would put the mover in check; use a scratch game
  // to achieve this

  bool bBlack;
  int dbg;

  if (!bInGetLegalMoves) {
    if (gamept->curr_move == dbg_move)
      dbg = 1;
    else
      dbg = 2;
  }

  if (debug_fptr && (debug_level == 5)) {
    fprintf(debug_fptr,"move_is_legal: curr_move = %d, special_move_info = %x, before update_board\n",
      gamept->curr_move,gamept->moves[gamept->curr_move].special_move_info);
    fprint_bd3(gamept->board,gamept->orientation,debug_fptr);
  }

  bBlack = gamept->curr_move & 0x1;
  copy_game(&scratch,gamept);

  if ((scratch.moves[scratch.curr_move].from != from) || (scratch.moves[scratch.curr_move].to != to))
    dbg = 1;

  scratch.moves[scratch.curr_move].from = from;
  scratch.moves[scratch.curr_move].to = to;
  update_board(&scratch,NULL,NULL,true);

  if (debug_fptr && (debug_level == 5)) {
    fprintf(debug_fptr,"move_is_legal: curr_move = %d, special_move_info = %x, after update_board\n",
      gamept->curr_move,gamept->moves[gamept->curr_move].special_move_info);
    fprint_bd3(scratch.board,scratch.orientation,debug_fptr);
  }

  if (player_is_in_check(bBlack,scratch.board,scratch.curr_move)) {
    if (debug_fptr && (debug_level == 5)) {
      fprintf(debug_fptr,"move_is_legal: about to return false, curr_move = %d\n",
        scratch.curr_move);
      fprint_bd3(scratch.board,scratch.orientation,debug_fptr);
    }

    return false;
  }

  return true;
}

void get_legal_moves(struct game *gamept,struct move *legal_moves,int *legal_moves_count)
{
  int n;
  bool bWhiteToMove;
  struct piece_info *info_pt;
  unsigned char board1[CHARS_IN_BOARD];
  unsigned char board2[CHARS_IN_BOARD];
  char piece_id;
  int retval;

  bInGetLegalMoves = true;
  bWhiteToMove = !(gamept->curr_move % 2);

  if (bWhiteToMove)
    info_pt = gamept->white_pieces;
  else
    info_pt = gamept->black_pieces;

  copy_board(gamept->board,board1);
  populate_board_from_piece_info(gamept->white_pieces,gamept->black_pieces,board2);

  retval = compare_boards(board1,board2);

  if (debug_fptr && (debug_level == 10)) {
    if (!retval) {
      fprintf(debug_fptr,"get_legal_moves: board1 and board2 don't match, gamept->curr_move = %d\n",
        gamept->curr_move);
      fprintf(debug_fptr,"get_legal_moves: board1\n");
      fprint_bd3(board1,gamept->orientation,debug_fptr);
      fprintf(debug_fptr,"get_legal_moves: board2\n");
      fprint_bd3(board2,gamept->orientation,debug_fptr);
    }
    else {
      fprintf(debug_fptr,"get_legal_moves: board1 and board2 match, gamept->curr_move = %d\n",gamept->curr_move);
      fprintf(debug_fptr,"get_legal_moves: board1\n");
      fprint_bd3(board1,gamept->orientation,debug_fptr);
    }
  }

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (info_pt[n].current_board_position == -1)
      continue;

    piece_id = info_pt[n].piece_id;

    if (piece_id < 0)
      piece_id *= -1;

    switch (piece_id) {
      case PAWN_ID:
        legal_pawn_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case ROOK_ID:
        legal_rook_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case KNIGHT_ID:
        legal_knight_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case BISHOP_ID:
        legal_bishop_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case GARGANTUA_ID:
        legal_gargantua_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
      case KING_ID:
        legal_king_moves(gamept,info_pt[n].current_board_position,legal_moves,legal_moves_count);

        break;
    }
  }

  bInGetLegalMoves = false;
}

struct move_offset pawn_offsets[] = {
  1, 0
};
#define NUM_PAWN_OFFSETS (sizeof pawn_offsets / sizeof(struct move_offset))

static int promotions[] = {
  SPECIAL_MOVE_PROMOTION_GARGANTUA,
  SPECIAL_MOVE_PROMOTION_ROOK,
  SPECIAL_MOVE_PROMOTION_KNIGHT,
  SPECIAL_MOVE_PROMOTION_BISHOP
};
#define NUM_PROMOTIONS (sizeof promotions / sizeof(int))

void legal_pawn_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int m;
  int n;
  int square;
  int rank;
  int file;
  int work_rank;
  int work_file;
  int square2;
  bool bBlack;
  int num_legal_moves_before;
  int num_legal_moves;
  char to;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
  bBlack = gamept->curr_move & 0x1;
  num_legal_moves_before = *legal_moves_count;

  // first, see if there's a legal two square pawn advance

  if (!bBlack) {
    if (rank == 1) {
      if (!get_piece2(gamept->board,2,file) && !get_piece2(gamept->board,3,file)) {
        to = POS_OF(3,file);

        if (move_is_legal(gamept,current_board_position,to)) {
          if (*legal_moves_count < MAX_LEGAL_MOVES) {
            legal_moves[*legal_moves_count].from = current_board_position;
            legal_moves[*legal_moves_count].to = to;
            legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE;

            (*legal_moves_count)++;
          }
        }
      }
    }
  }
  else {
    if (rank == 6) {
      if (!get_piece2(gamept->board,5,file) && !get_piece2(gamept->board,4,file)) {
        to = POS_OF(4,file);

        if (move_is_legal(gamept,current_board_position,to)) {
          if (*legal_moves_count < MAX_LEGAL_MOVES) {
            legal_moves[*legal_moves_count].from = current_board_position;
            legal_moves[*legal_moves_count].to = to;
            legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE;

            (*legal_moves_count)++;
          }
        }
      }
    }
  }

  // now check for a one square pawn advance

  for (n = 0; n < NUM_PAWN_OFFSETS; n++) {
    if (!bBlack)
      work_rank = rank + pawn_offsets[n].rank_offset;
    else
      work_rank = rank - pawn_offsets[n].rank_offset;

    work_file = file + pawn_offsets[n].file_offset;

    if ((work_rank < 0) || (work_rank >= NUM_RANKS))
      continue;

    if ((work_file < 0) || (work_file >= NUM_FILES))
      continue;

    square2 = get_piece2(gamept->board,work_rank,work_file);

    // pawn can only advance to an empty square
    if (square2)
      continue;

    to = POS_OF(work_rank,work_file);

    if (!move_is_legal(gamept,current_board_position,to))
      continue;

    if (!bBlack) {
      if (work_rank == NUM_RANKS - 1) {
        for (m = 0; m < NUM_PROMOTIONS; m++) {
          if (*legal_moves_count < MAX_LEGAL_MOVES) {
            legal_moves[*legal_moves_count].from = current_board_position;
            legal_moves[*legal_moves_count].to = to;
            legal_moves[*legal_moves_count].special_move_info = promotions[m];

            (*legal_moves_count)++;
          }
        }

        continue;
      }
    }
    else {
      if (!work_rank) {
        for (m = 0; m < NUM_PROMOTIONS; m++) {
          if (*legal_moves_count < MAX_LEGAL_MOVES) {
            legal_moves[*legal_moves_count].from = current_board_position;
            legal_moves[*legal_moves_count].to = to;
            legal_moves[*legal_moves_count].special_move_info = promotions[m];

            (*legal_moves_count)++;
          }
        }

        continue;
      }
    }

    if (*legal_moves_count < MAX_LEGAL_MOVES) {
      legal_moves[*legal_moves_count].from = current_board_position;
      legal_moves[*legal_moves_count].to = to;
      legal_moves[*legal_moves_count].special_move_info = 0;

      (*legal_moves_count)++;
    }
  }

  // now check for captures

  if (!bBlack)
    work_rank = rank + 1;
  else
    work_rank = rank - 1;

  if ((work_rank >= 0) && (work_rank < NUM_RANKS)) {
    for (n = 0; n < 2; n++) {
      if (!n)
        work_file = file - 1;
      else
        work_file = file + 1;

      if ((work_file < 0) || (work_file >= NUM_FILES))
        continue;

      square2 = get_piece2(gamept->board,work_rank,work_file);

      // pawn can only capture an enemy piece
      if (square * square2 >= 0)
        continue;

      to = POS_OF(work_rank,work_file);

      if (!move_is_legal(gamept,current_board_position,to))
        continue;

      if (!bBlack) {
        if (work_rank == NUM_RANKS - 1) {
          for (m = 0; m < NUM_PROMOTIONS; m++) {
            if (*legal_moves_count < MAX_LEGAL_MOVES) {
              legal_moves[*legal_moves_count].from = current_board_position;
              legal_moves[*legal_moves_count].to = to;
              legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_CAPTURE | promotions[m];

              (*legal_moves_count)++;
            }
          }

          continue;
        }
      }
      else {
        if (!work_rank) {
          for (m = 0; m < NUM_PROMOTIONS; m++) {
            if (*legal_moves_count < MAX_LEGAL_MOVES) {
              legal_moves[*legal_moves_count].from = current_board_position;
              legal_moves[*legal_moves_count].to = to;
              legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_CAPTURE | promotions[m];

              (*legal_moves_count)++;
            }
          }

          continue;
        }
      }

      if (*legal_moves_count < MAX_LEGAL_MOVES) {
        legal_moves[*legal_moves_count].from = current_board_position;
        legal_moves[*legal_moves_count].to = to;
        legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_CAPTURE;

        (*legal_moves_count)++;
      }
    }
  }

  // now check for en passant captures

  if (!bBlack) {
    if (rank == 4) {
      if (file > 0) {
        square2 = get_piece2(gamept->board,rank,file-1);

        if (square2 == PAWN_ID * -1) {
          if ((gamept->moves[gamept->curr_move - 1].to == current_board_position - 1) &&
              (gamept->moves[gamept->curr_move - 1].special_move_info & SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE)) {

            if (*legal_moves_count < MAX_LEGAL_MOVES) {
              legal_moves[*legal_moves_count].from = current_board_position;
              legal_moves[*legal_moves_count].to = current_board_position + 7;
              legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;

              (*legal_moves_count)++;
            }
          }
        }
      }

      if (file < NUM_FILES - 1) {
        square2 = get_piece2(gamept->board,rank,file+1);

        if (square2 == PAWN_ID * -1) {
          if ((gamept->moves[gamept->curr_move - 1].to == current_board_position + 1) &&
              (gamept->moves[gamept->curr_move - 1].special_move_info & SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE)) {

            if (*legal_moves_count < MAX_LEGAL_MOVES) {
              legal_moves[*legal_moves_count].from = current_board_position;
              legal_moves[*legal_moves_count].to = current_board_position + 9;
              legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;

              (*legal_moves_count)++;
            }
          }
        }
      }
    }
  }
  else {
    if (rank == 3) {
      if (file > 0) {
        square2 = get_piece2(gamept->board,rank,file-1);

        if (square2 == PAWN_ID) {
          if ((gamept->moves[gamept->curr_move - 1].to == current_board_position - 1) &&
              (gamept->moves[gamept->curr_move - 1].special_move_info & SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE)) {

            if (*legal_moves_count < MAX_LEGAL_MOVES) {
              legal_moves[*legal_moves_count].from = current_board_position;
              legal_moves[*legal_moves_count].to = current_board_position - 7;
              legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;

              (*legal_moves_count)++;
            }
          }
        }
      }

      if (file < NUM_FILES - 1) {
        square2 = get_piece2(gamept->board,rank,file+1);

        if (square2 == PAWN_ID) {
          if ((gamept->moves[gamept->curr_move - 1].to == current_board_position + 1) &&
              (gamept->moves[gamept->curr_move - 1].special_move_info & SPECIAL_MOVE_TWO_SQUARE_PAWN_ADVANCE)) {

            if (*legal_moves_count < MAX_LEGAL_MOVES) {
              legal_moves[*legal_moves_count].from = current_board_position;
              legal_moves[*legal_moves_count].to = current_board_position - 9;
              legal_moves[*legal_moves_count].special_move_info = SPECIAL_MOVE_CAPTURE | SPECIAL_MOVE_EN_PASSANT_CAPTURE;

              (*legal_moves_count)++;
            }
          }
        }
      }
    }
  }

  num_legal_moves = *legal_moves_count - num_legal_moves_before;

  if (debug_fptr && (debug_level == 12)) {
    fprintf(debug_fptr,"legal_pawn_moves: curr_move = %d, current_board_position = %d, num_legal_moves = %d\n",
      gamept->curr_move,current_board_position,num_legal_moves);
  }
}

struct move_offset rook_offsets[] = {
  1,   0,
  0,   1,
  -1,  0,
  0,  -1
};
#define NUM_ROOK_OFFSETS (sizeof rook_offsets / sizeof(struct move_offset))

void legal_rook_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int m;
  int n;
  int square;
  int rank;
  int file;
  int work_rank;
  int work_file;
  int square2;
  int num_legal_moves_before;
  int num_legal_moves;
  char to;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
  num_legal_moves_before = *legal_moves_count;

  for (n = 0; n < NUM_ROOK_OFFSETS; n++) {
    for (m = 0; m < NUM_RANKS - 1; m++) {
      work_rank = rank + (rook_offsets[n].rank_offset * (m + 1));
      work_file = file + (rook_offsets[n].file_offset * (m + 1));

      if ((work_rank < 0) || (work_rank >= NUM_RANKS))
        break;

      if ((work_file < 0) || (work_file >= NUM_FILES))
        break;

      square2 = get_piece2(gamept->board,work_rank,work_file);

      // can't capture a piece of the same color
      if ((square * square2) > 0)
        break;

      to = POS_OF(work_rank,work_file);

      if (move_is_legal(gamept,current_board_position,to)) {
        if (*legal_moves_count < MAX_LEGAL_MOVES) {
          legal_moves[*legal_moves_count].from = current_board_position;
          legal_moves[*legal_moves_count].to = to;
          legal_moves[*legal_moves_count].special_move_info = 0;
          (*legal_moves_count)++;
        }
      }

      if (square2)
        break;
      else
        continue;
    }
  }

  num_legal_moves = *legal_moves_count - num_legal_moves_before;

  if (debug_fptr && (debug_level == 12)) {
    fprintf(debug_fptr,"legal_rook_moves: curr_move = %d, current_board_position = %d, num_legal_moves = %d\n",
      gamept->curr_move,current_board_position,num_legal_moves);
  }
}

struct move_offset knight_offsets[] = {
  1, 2,
  2, 1,
  1, -2,
  2, -1,
  -1, -2,
  -2, -1,
  -1, 2,
  -2, 1
};
#define NUM_KNIGHT_OFFSETS (sizeof knight_offsets / sizeof(struct move_offset))

void legal_knight_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int n;
  int square;
  int rank;
  int file;
  int work_rank;
  int work_file;
  int square2;
  int num_legal_moves_before;
  int num_legal_moves;
  char to;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
  num_legal_moves_before = *legal_moves_count;

  for (n = 0; n < NUM_KNIGHT_OFFSETS; n++) {
    work_rank = rank + knight_offsets[n].rank_offset;
    work_file = file + knight_offsets[n].file_offset;

    if ((work_rank < 0) || (work_rank >= NUM_RANKS))
      continue;

    if ((work_file < 0) || (work_file >= NUM_FILES))
      continue;

    square2 = get_piece2(gamept->board,work_rank,work_file);

    // can't capture a piece of the same color
    if ((square * square2) > 0)
      continue;

    to = POS_OF(work_rank,work_file);

    if (!move_is_legal(gamept,current_board_position,to))
      continue;

    if (*legal_moves_count < MAX_LEGAL_MOVES) {
      legal_moves[*legal_moves_count].from = current_board_position;
      legal_moves[*legal_moves_count].to = to;
      legal_moves[*legal_moves_count].special_move_info = 0;
      (*legal_moves_count)++;
    }
  }

  num_legal_moves = *legal_moves_count - num_legal_moves_before;

  if (debug_fptr && (debug_level == 12)) {
    fprintf(debug_fptr,"legal_knight_moves: curr_move = %d, current_board_position = %d, num_legal_moves = %d\n",
      gamept->curr_move,current_board_position,num_legal_moves);
  }
}

struct move_offset bishop_offsets[] = {
  1,   1,
  1,  -1,
  -1,  1,
  -1, -1
};
#define NUM_BISHOP_OFFSETS (sizeof bishop_offsets / sizeof(struct move_offset))

void legal_bishop_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int m;
  int n;
  int square;
  int rank;
  int file;
  int work_rank;
  int work_file;
  int square2;
  int num_legal_moves_before;
  int num_legal_moves;
  char to;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
  num_legal_moves_before = *legal_moves_count;

  for (n = 0; n < NUM_BISHOP_OFFSETS; n++) {
    for (m = 0; m < NUM_RANKS - 1; m++) {
      work_rank = rank + (bishop_offsets[n].rank_offset * (m + 1));
      work_file = file + (bishop_offsets[n].file_offset * (m + 1));

      if ((work_rank < 0) || (work_rank >= NUM_RANKS))
        break;

      if ((work_file < 0) || (work_file >= NUM_FILES))
        break;

      square2 = get_piece2(gamept->board,work_rank,work_file);

      // can't capture a piece of the same color
      if ((square * square2) > 0)
        break;

      to = POS_OF(work_rank,work_file);

      if (move_is_legal(gamept,current_board_position,to)) {
        if (*legal_moves_count < MAX_LEGAL_MOVES) {
          legal_moves[*legal_moves_count].from = current_board_position;
          legal_moves[*legal_moves_count].to = to;
          legal_moves[*legal_moves_count].special_move_info = 0;
          (*legal_moves_count)++;
        }
      }

      if (square2)
        break;
      else
        continue;
    }
  }

  num_legal_moves = *legal_moves_count - num_legal_moves_before;

  if (debug_fptr && (debug_level == 12)) {
    fprintf(debug_fptr,"legal_bishop_moves: curr_move = %d, current_board_position = %d, num_legal_moves = %d\n",
      gamept->curr_move,current_board_position,num_legal_moves);
  }
}

void legal_gargantua_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int num_legal_moves_before;
  int num_legal_moves;

  num_legal_moves_before = *legal_moves_count;

  legal_rook_moves(gamept,current_board_position,legal_moves,legal_moves_count);
  legal_bishop_moves(gamept,current_board_position,legal_moves,legal_moves_count);
  legal_knight_moves(gamept,current_board_position,legal_moves,legal_moves_count);

  num_legal_moves = *legal_moves_count - num_legal_moves_before;

  if (debug_fptr && (debug_level == 12)) {
    fprintf(debug_fptr,"legal_gargantua_moves: curr_move = %d, current_board_position = %d, num_legal_moves = %d\n",
      gamept->curr_move,current_board_position,num_legal_moves);
  }
}

struct move_offset king_offsets[] = {
  1,   0,
  0,   1,
  -1,  0,
  0,  -1,
  1,   1,
  1,  -1,
  -1,  1,
  -1, -1
};
#define NUM_KING_OFFSETS (sizeof king_offsets / sizeof(struct move_offset))

void legal_king_moves(struct game *gamept,char current_board_position,struct move *legal_moves,int *legal_moves_count)
{
  int n;
  int square;
  int rank;
  int file;
  int work_rank;
  int work_file;
  int square2;
  int num_legal_moves_before;
  int num_legal_moves;
  char to;

  square = get_piece1(gamept->board,current_board_position);
  rank = RANK_OF(current_board_position);
  file = FILE_OF(current_board_position);
  num_legal_moves_before = *legal_moves_count;

  for (n = 0; n < NUM_KING_OFFSETS; n++) {
    work_rank = rank + king_offsets[n].rank_offset;
    work_file = file + king_offsets[n].file_offset;

    if ((work_rank < 0) || (work_rank >= NUM_RANKS))
      continue;

    if ((work_file < 0) || (work_file >= NUM_FILES))
      continue;

    square2 = get_piece2(gamept->board,work_rank,work_file);

    // can't capture a piece of the same color
    if ((square * square2) > 0)
      continue;

    to = POS_OF(work_rank,work_file);

    if (!move_is_legal(gamept,current_board_position,to))
      continue;

    if (*legal_moves_count < MAX_LEGAL_MOVES) {
      legal_moves[*legal_moves_count].from = current_board_position;
      legal_moves[*legal_moves_count].to = to;
      legal_moves[*legal_moves_count].special_move_info = 0;
      (*legal_moves_count)++;
    }
  }

  num_legal_moves = *legal_moves_count - num_legal_moves_before;

  if (debug_fptr && (debug_level == 12)) {
    fprintf(debug_fptr,"legal_king_moves: curr_move = %d, current_board_position = %d, num_legal_moves = %d\n",
      gamept->curr_move,current_board_position,num_legal_moves);
  }
}

static struct move work_legal_moves[MAX_LEGAL_MOVES];

static struct move debug_mate_in_one_move;
static int debug_n;

bool mate_in_one_exists(struct game *gamept)
{
  int n;
  struct game work_game;
  int work_legal_moves_count;
  bool bBlack;
  int dbg;
  int dbg1;
  int dbg2;

  legal_moves_count = 0;
  get_legal_moves(gamept,legal_moves,&legal_moves_count);

  if (gamept->curr_move == dbg_move)
    dbg1 = 1;
  else
    dbg1 = 0;

  if (debug_fptr && (debug_level == 14)) {
    fprintf(debug_fptr,"mate_in_one_exists: curr_move = %d, legal_moves_count = %d\n",
      gamept->curr_move,legal_moves_count);
  }

  for (n = 0; n < legal_moves_count; n++) {
    if ((dbg1 == 1) && (legal_moves[n].from == debug_mate_in_one_move.from) && (legal_moves[n].to == debug_mate_in_one_move.to)) {
      dbg2 = 1;
    }
    else
      dbg2 = 0;

    copy_game(&work_game,gamept);
    work_game.moves[work_game.curr_move].from = legal_moves[n].from;
    work_game.moves[work_game.curr_move].to = legal_moves[n].to;
    work_game.moves[work_game.curr_move].special_move_info = legal_moves[n].special_move_info;
    update_board(&work_game,NULL,NULL,true);

    if ((work_game.curr_move == dbg_move) && (n == debug_n))
      dbg = 1;

    work_game.curr_move++;

    bBlack = work_game.curr_move & 0x1;

    if (player_is_in_check(bBlack,work_game.board,work_game.curr_move)) {
      if (debug_fptr && (debug_level == 11)) {
        fprintf(debug_fptr,"mate_in_one_exists: piece_info:\n");
        fprint_piece_info(&work_game,debug_fptr);
      }

      work_legal_moves_count = 0;
      get_legal_moves(&work_game,work_legal_moves,&work_legal_moves_count);

      if (debug_fptr && (debug_level == 14)) {
        fprintf(debug_fptr,"mate_in_one_exists: work_game.curr_move = %d, work_legal_moves_count = %d\n",
          work_game.curr_move,work_legal_moves_count);
        fprint_bd3(work_game.board,work_game.orientation,debug_fptr);
      }

      if (!work_legal_moves_count) {
        if (debug_fptr && (debug_level == 14)) {
          fprintf(debug_fptr,"mate_in_one_exists: curr_move = %d, returning true\n",
            gamept->curr_move,legal_moves_count);
        }

        return true;
      }
    }
  }

  return false;
}

bool white_to_move(struct game *gamept)
{
  if (!gamept->black_moves_first)
    return !(gamept->curr_move % 2);
  else
    return (gamept->curr_move % 2);
}
