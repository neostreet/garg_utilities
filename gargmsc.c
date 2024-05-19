#include <stdio.h>
#include "garg.h"
#include "garg.glb"
#include "garg.fun"
#include "garg.mac"
#include "bitfuns.h"

int format_square(int square)
{
  int bBlack;
  int return_char;

  if (!square)
    return (int)'.';

  if (square < 0) {
    bBlack = true;
    square *= -1;
  }
  else
    bBlack = false;

  if (square == 1)
    return_char = 'P';
  else
    return_char = piece_ids[square - 2];

  if (!bBlack)
    return_char += ('a' - 'A');

  return return_char;
}

void print_bd(struct game *gamept)
{
  int m;
  int n;
  int square;

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      square = get_piece2(gamept->board,(NUM_RANKS - 1) - m,n);
      printf("%c ",format_square(square));
    }

    putchar(0x0a);
  }
}

void fprint_game(struct game *gamept,char *filename)
{
  FILE *fptr;
  char buf[20];

  if ((fptr = fopen(filename,"w")) == NULL)
    return;

  fprintf(fptr,fmt_str,gamept->title);

  set_initial_board(gamept);

  for (gamept->curr_move = 0;
       gamept->curr_move <= gamept->num_moves;
       gamept->curr_move++) {

    sprintf_move(gamept,buf,20,true);
    fprintf(fptr,fmt_str,buf);

    update_board(gamept,NULL,NULL);
  }

  fclose(fptr);
}

void fprint_game2(struct game *gamept,FILE *fptr)
{
  char buf[20];

  fprintf(fptr,fmt_str,gamept->title);

  set_initial_board(gamept);

  for (gamept->curr_move = 0;
       gamept->curr_move <= gamept->num_moves;
       gamept->curr_move++) {

    sprintf_move(gamept,buf,20,true);
    fprintf(fptr,fmt_str,buf);

    update_board(gamept,NULL,NULL);
  }
}

void fprint_bd(struct game *gamept,char *filename)
{
  int m;
  int n;
  FILE *fptr;
  int square;

  if ((fptr = fopen(filename,"w")) == NULL)
    return;

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      square = get_piece2(gamept->board,(NUM_RANKS - 1) - m,n);
      fprintf(fptr,"%c ",format_square(square));
    }

    fputc(0x0a,fptr);
  }

  fclose(fptr);
}

void fprint_bd2(struct game *gamept,FILE *fptr)
{
  int m;
  int n;
  int square;

  for (m = 0; m < NUM_RANKS; m++) {
    for (n = 0; n < NUM_FILES; n++) {
      square = get_piece2(gamept->board,(NUM_RANKS - 1) - m,n);
      fprintf(fptr,"%c ",format_square(square));
    }

    fputc(0x0a,fptr);
  }
}

void fprint_moves(struct game *gamept,char *filename)
{
  int n;
  FILE *fptr;

  if ((fptr = fopen(filename,"w")) == NULL)
    return;

  for (n = 0; n < gamept->num_moves; n++) {
    fprintf(fptr,"%d %d %s\n",gamept->moves[n].from,gamept->moves[n].to,
      (gamept->moves[n].special_move_info ? special_moves[gamept->moves[n].special_move_info] : ""));
  }

  fclose(fptr);
}

void fprint_moves2(struct game *gamept,FILE *fptr)
{
  int n;

  for (n = 0; n < gamept->num_moves; n++) {
    fprintf(fptr,"%d %d %s\n",gamept->moves[n].from,gamept->moves[n].to,
      (gamept->moves[n].special_move_info ? special_moves[gamept->moves[n].special_move_info] : ""));
  }
}

void print_special_moves(struct game *gamept)
{
  int n;
  int and_val;
  int hit;

  and_val = 0x1;
  hit = 0;

  for (n = 0; n < num_special_moves; n++) {
    if (gamept->moves[gamept->curr_move].special_move_info & and_val) {
      if (hit)
        putchar(' ' );

      printf("%s",special_moves[n]);

      hit++;
    }

    and_val <<= 1;
  }

  if (hit)
    putchar(0x0a);
  else
    printf("SPECIAL_MOVE_NONE\n");
}

int match_board(unsigned char *board1,unsigned char *board2,bool bExactMatch)
{
  int m;
  int n;
  int square1;
  int square2;

  for (m = 0; m < 8; m++) {
    for (n = 0; n < 8; n++) {
      square1 = get_piece2(board1,7 - m,n);
      square2 = get_piece2(board2,7 - m,n);

      if (!bExactMatch) {
        if (square2) {
          if (square2 == EMPTY_ID) {
            if (square1)
              break;
          }
          else if (square1 != square2)
            break;
        }
      }
      else {
        if (square2 == EMPTY_ID) {
          if (square1)
            break;
        }
        else if (square1 != square2)
          break;
      }
    }

    if (n < 8)
      break;
  }

  if (m < 8)
    return 0;

  return 1;
}

bool multiple_queens(unsigned char *board)
{
  int n;
  int piece;
  int num_white_queens = 0;
  int num_black_queens = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (piece == QUEEN_ID)
      num_white_queens++;
    else if (piece == QUEEN_ID * -1)
      num_black_queens++;
  }

  if ((num_white_queens > 1) || (num_black_queens > 1))
    return true;

  return false;
}

bool opposite_colored_bishops(unsigned char *board)
{
  int n;
  int piece;
  int rank;
  int file;
  int num_white_bishops = 0;
  bool bWhiteBishopOnWhiteSquare;
  int num_black_bishops = 0;
  bool bBlackBishopOnWhiteSquare;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);
    rank = RANK_OF(n);
    file = FILE_OF(n);

    if (piece == BISHOP_ID) {
      num_white_bishops++;
      bWhiteBishopOnWhiteSquare = (rank % 2) ? (file % 2) : !(file % 2);
    }
    else if (piece == BISHOP_ID * -1) {
      num_black_bishops++;
      bBlackBishopOnWhiteSquare = (rank % 2) ? (file % 2) : !(file % 2);
    }
  }

  if ((num_white_bishops == 1) && (num_black_bishops == 1) && (bWhiteBishopOnWhiteSquare != bBlackBishopOnWhiteSquare))
    return true;

  return false;
}

bool same_colored_bishops(unsigned char *board)
{
  int n;
  int piece;
  int rank;
  int file;
  int num_white_bishops = 0;
  bool bWhiteBishopOnWhiteSquare;
  int num_black_bishops = 0;
  bool bBlackBishopOnWhiteSquare;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);
    rank = RANK_OF(n);
    file = FILE_OF(n);

    if (piece == BISHOP_ID) {
      num_white_bishops++;
      bWhiteBishopOnWhiteSquare = (rank % 2) ? (file % 2) : !(file % 2);
    }
    else if (piece == BISHOP_ID * -1) {
      num_black_bishops++;
      bBlackBishopOnWhiteSquare = (rank % 2) ? (file % 2) : !(file % 2);
    }
  }

  if ((num_white_bishops == 1) && (num_black_bishops == 1) && (bWhiteBishopOnWhiteSquare == bBlackBishopOnWhiteSquare))
    return true;

  return false;
}

bool two_bishops(unsigned char *board)
{
  int n;
  int piece;
  int num_white_bishops = 0;
  int num_black_bishops = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (piece == BISHOP_ID)
      num_white_bishops++;
    else if (piece == BISHOP_ID * -1)
      num_black_bishops++;
  }

  if (((num_white_bishops < 2) && (num_black_bishops == 2)) || ((num_white_bishops == 2) && (num_black_bishops < 2)))
    return true;

  return false;
}

bool opposite_side_castling(struct game *gamept)
{
  int n;
  bool bHaveKingsideCastle;
  bool bHaveQueensideCastle;

  bHaveKingsideCastle = false;
  bHaveQueensideCastle = false;

  for (n = 0; n < gamept->num_moves; n++) {
    if (gamept->moves[n].special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE) {
      bHaveKingsideCastle = true;

      if (bHaveQueensideCastle)
        return true;
    }
    else if (gamept->moves[n].special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE) {
      bHaveQueensideCastle = true;

      if (bHaveKingsideCastle)
        return true;
    }
  }

  return false;;
}

bool same_side_castling(struct game *gamept)
{
  int n;
  int kingside_castles;
  int queenside_castles;

  kingside_castles = 0;
  queenside_castles = 0;

  for (n = 0; n < gamept->num_moves; n++) {
    if (gamept->moves[n].special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE)
      kingside_castles++;
    else if (gamept->moves[n].special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE)
      queenside_castles++;
  }

  return ((kingside_castles == 2) || (queenside_castles == 2));
}

bool less_than_2_castles(struct game *gamept)
{
  int n;
  int castles;

  castles = 0;

  for (n = 0; n < gamept->num_moves; n++) {
    if (gamept->moves[n].special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE)
      castles++;
    else if (gamept->moves[n].special_move_info & SPECIAL_MOVE_QUEENSIDE_CASTLE)
      castles++;
  }

  return (castles < 2);
}

void position_game(struct game *gamept,int move)
{
  set_initial_board(gamept);

  for (gamept->curr_move = 0; gamept->curr_move < move; gamept->curr_move++) {
    update_board(gamept,NULL,NULL);
  }
}

static int update_board_calls;
static int dbg_update_board_call;
static int dbg;

void update_board(struct game *gamept,int *invalid_squares,int *num_invalid_squares)
{
  bool bBlack;
  int from_piece;
  int to_piece;
  bool bKingsideCastle = false;
  bool bQueensideCastle = false;
  bool bEnPassantCapture = false;
  int square_to_clear;

  update_board_calls++;

  if (dbg_update_board_call == update_board_calls)
    dbg = 0;

  bBlack = (gamept->curr_move % 2);

  from_piece = get_piece1(gamept->board,gamept->moves[gamept->curr_move].from);
  to_piece = get_piece1(gamept->board,gamept->moves[gamept->curr_move].to);

  if (from_piece * to_piece < 0)
    gamept->moves[gamept->curr_move].special_move_info |= SPECIAL_MOVE_CAPTURE;

  switch (gamept->moves[gamept->curr_move].special_move_info) {
    case SPECIAL_MOVE_KINGSIDE_CASTLE:
      bKingsideCastle = true;

      break;
    case SPECIAL_MOVE_QUEENSIDE_CASTLE:
      bQueensideCastle = true;

      break;
    case SPECIAL_MOVE_EN_PASSANT_CAPTURE:
      bEnPassantCapture = true;

      break;
    case SPECIAL_MOVE_PROMOTION_QUEEN:
      from_piece = (bBlack ? QUEEN_ID * -1 : QUEEN_ID);

      break;
    case SPECIAL_MOVE_PROMOTION_ROOK:
      from_piece = (bBlack ? ROOK_ID * -1 : ROOK_ID);

      break;
    case SPECIAL_MOVE_PROMOTION_BISHOP:
      from_piece = (bBlack ? BISHOP_ID * -1 : BISHOP_ID);

      break;
    case SPECIAL_MOVE_PROMOTION_KNIGHT:
      from_piece = (bBlack ? KNIGHT_ID * -1 : KNIGHT_ID);

      break;
    case SPECIAL_MOVE_PROMOTION_GARGANTUA:
      from_piece = (bBlack ? GARGANTUA_ID * -1 : GARGANTUA_ID);

      break;
  }

  if (invalid_squares) {
    *num_invalid_squares = 0;
    invalid_squares[(*num_invalid_squares)++] = gamept->moves[gamept->curr_move].from;
    invalid_squares[(*num_invalid_squares)++] = gamept->moves[gamept->curr_move].to;
  }

  set_piece1(gamept->board,gamept->moves[gamept->curr_move].to,from_piece);

  set_piece1(gamept->board,gamept->moves[gamept->curr_move].from,0);  /* vacate previous square */

  if (bKingsideCastle) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      set_piece1(gamept->board,6,ROOK_ID);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 6;

      set_piece1(gamept->board,8,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 8;
    }
    else {
      // it's Blacks's move
      set_piece1(gamept->board,76,ROOK_ID * -1);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 76;

      set_piece1(gamept->board,78,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 78;
    }
  }
  else if (bQueensideCastle) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      set_piece1(gamept->board,4,ROOK_ID);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 4;

      set_piece1(gamept->board,1,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 1;
    }
    else {
      // it's Blacks's move
      set_piece1(gamept->board,74,ROOK_ID * -1);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 74;

      set_piece1(gamept->board,71,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 71;
    }
  }
  else if (bEnPassantCapture) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      square_to_clear = gamept->moves[gamept->curr_move].to - NUM_FILES;
    }
    else {
      // it's Blacks's move
      square_to_clear = gamept->moves[gamept->curr_move].to + NUM_FILES;
    }

    set_piece1(gamept->board,square_to_clear,0);

    if (invalid_squares)
      invalid_squares[(*num_invalid_squares)++] = square_to_clear;
  }
}

int get_piece1(unsigned char *board,int board_offset)
{
  unsigned int bit_offset;
  unsigned short piece;
  int piece_int;

  bit_offset = board_offset * BITS_PER_BOARD_SQUARE;

  piece = get_bits(BITS_PER_BOARD_SQUARE,board,bit_offset);
  piece_int = piece;

  if (piece & 0x8)
    piece_int |= 0xfffffff0;

  return piece_int;
}

int get_piece2(unsigned char *board,int rank,int file)
{
  int board_offset;

  board_offset = rank * NUM_FILES + file;
  return get_piece1(board,board_offset);
}

static int set_piece_calls;
static int dbg_set_piece_call;
static int dbg_board_offset;
static int dbg_piece;

void set_piece1(unsigned char *board,int board_offset,int piece)
{
  unsigned int bit_offset;

  set_piece_calls++;

  if (dbg_set_piece_call == set_piece_calls)
    dbg = 0;

  if (debug_level == 2) {
    if (debug_fptr != NULL)
      fprintf(debug_fptr,"set_piece: board_offset = %d, piece %d\n",board_offset,piece);
  }

  bit_offset = board_offset * BITS_PER_BOARD_SQUARE;
  set_bits(BITS_PER_BOARD_SQUARE,board,bit_offset,piece);
}

void set_piece2(unsigned char *board,int rank,int file,int piece)
{
  int board_offset;

  board_offset = rank * NUM_FILES + file;
  set_piece1(board,board_offset,piece);
}
