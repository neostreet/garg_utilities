#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#include "garg.h"
#include "garg.fun"
#include "garg.glb"
#include "garg.mac"
#include "bitfuns.h"

static unsigned char initial_board[] = {
  (unsigned char)0x23, (unsigned char)0x45, (unsigned char)0x64, (unsigned char)0x32,
  (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff,
  (unsigned char)0xed, (unsigned char)0xcb, (unsigned char)0xac, (unsigned char)0xde
};

static struct piece_info initial_white_pieces[] = {
  ROOK_ID,0,0,
  KNIGHT_ID,1,0,
  BISHOP_ID,2,0,
  GARGANTUA_ID,3,0,
  KING_ID,4,0,
  BISHOP_ID,5,0,
  KNIGHT_ID,6,0,
  ROOK_ID,7,0,
  PAWN_ID,8,0,
  PAWN_ID,9,0,
  PAWN_ID,10,0,
  PAWN_ID,11,0,
  PAWN_ID,12,0,
  PAWN_ID,13,0,
  PAWN_ID,14,0,
  PAWN_ID,15,0
};

static struct piece_info initial_black_pieces[] = {
  PAWN_ID * -1,48,0,
  PAWN_ID * -1,49,0,
  PAWN_ID * -1,50,0,
  PAWN_ID * -1,51,0,
  PAWN_ID * -1,52,0,
  PAWN_ID * -1,53,0,
  PAWN_ID * -1,54,0,
  PAWN_ID * -1,55,0,
  ROOK_ID * -1,56,0,
  KNIGHT_ID * -1,57,0,
  BISHOP_ID * -1,58,0,
  GARGANTUA_ID * -1,59,0,
  KING_ID * -1,60,0,
  BISHOP_ID * -1,61,0,
  KNIGHT_ID * -1,62,0,
  ROOK_ID * -1,63,0
};

static char corrupted_msg[] = "game corrupted\n";
static char bad_castle[] = "bad castle";
static char bad_pawn_move[] = "bad pawn move";

static char *bad_piece_move[] = {
  "bad rook move",
  "bad knight move",
  "bad bishop move",
  "bad gargantua move",
  "bad king move"
};

extern int bHaveGame;
extern int afl_dbg;

int line_number(char *word,int wordlen)
{
  int n;

  for (n = 0; n < wordlen; n++) {
    if ((word[n] < '0') || (word[n] > '9')) {
      if ((word[n] == '.') && (n == wordlen - 1))
        break;

      return false;
    }
  }

  return true;
}

int get_piece_type_ix(int chara)
{
  int n;

  for (n = 0; n < NUM_PIECE_TYPES; n++)
    if (chara == piece_ids[n])
      return n;

  return 0; /* should never happen */
}

void set_initial_board(struct game *gamept)
{
  int n;

  for (n = 0; n < CHARS_IN_BOARD; n++)
    gamept->board[n] = initial_board[n];

  initialize_piece_info(gamept);
}

void initialize_piece_info(struct game *gamept)
{
  int n;

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    gamept->white_pieces[n].piece_id = initial_white_pieces[n].piece_id;
    gamept->white_pieces[n].current_board_position = initial_white_pieces[n].current_board_position;
    gamept->white_pieces[n].move_count = initial_white_pieces[n].move_count;
    gamept->black_pieces[n].piece_id = initial_black_pieces[n].piece_id;
    gamept->black_pieces[n].current_board_position = initial_black_pieces[n].current_board_position;
    gamept->black_pieces[n].move_count = initial_black_pieces[n].move_count;
  }
}

int read_game(char *filename,struct game *gamept,char *err_msg)
{
  FILE *fptr;
  int chara;
  int end_of_file;
  int m;
  int n;
  char word[WORDLEN];
  int wordlen;
  bool bHaveFirstWord;
  int direction;
  int word_no;
  int dbg;
  int retval;
  int got_error;
  bool bBlack;

  bzero(gamept,sizeof (struct game));

  gamept->curr_move = -1;

  if ((fptr = fopen(filename,"r")) == NULL)
    return 1;

  fscanf(fptr,"%d",&gamept->orientation);  /* get board orientation */
                                   /* 0 = standard, 1 = black on bottom */

  end_of_file = get_word(fptr,word,WORDLEN,&wordlen);
  bHaveFirstWord = true;

  set_initial_board(gamept);

  end_of_file = 0;
  gamept->curr_move = 0;

  gamept->title[0] = 0;
  word_no = 0;
  got_error = 0;

  for ( ; ; ) {
    if (word_no || !bHaveFirstWord)
      end_of_file = get_word(fptr,word,WORDLEN,&wordlen);

    if (end_of_file)
      break;

    word_no++;

    if (gamept->curr_move == dbg_move)
      dbg = 1;

    /* ignore line numbers: */
    if (line_number(word,wordlen))
      continue;

    if (!strncmp(word,"title",5)) {
      m = 5;
      n = 0;

      for ( ; ; ) {
        chara = word[m++];

        if (!chara)
          break;

        if (n < MAX_TITLE_LEN - 1) {
          if (chara == '\\')
            gamept->title[n++] = ' ';
          else
            gamept->title[n++] = chara;
        }
      }

      gamept->title[n] = 0;

      for (m = n-1; m >= 0; m--) {
        if (gamept->title[m] == ' ')
          break;
      }

      if (m >= 0) {
        m++;

        if (!strcmp(&gamept->title[m],"1-0"))
          gamept->result = WHITE_WIN;
        else if (!strcmp(&gamept->title[m],"0-1"))
          gamept->result = BLACK_WIN;
        else if (!strcmp(&gamept->title[m],"1/2-1/2"))
          gamept->result = DRAW;
      }

      continue;
    }

    if (gamept->curr_move & 0x1)
      direction = -1;           /* black's move */
    else
      direction = 1;            /* white's move */

    gamept->moves[gamept->curr_move].special_move_info = 0;

    switch(word[0]) {
      case 'O':
      case '0':
        retval = do_castle(gamept,direction,word,wordlen,&gamept->moves[gamept->curr_move]);

        if (retval) {
          /*printf(corrupted_msg);*/
          strcpy(err_msg,bad_castle);

          got_error = 2;
        }

        break;

      case 'R':
      case 'N':
      case 'B':
      case 'G':
      case 'K':
        retval = do_piece_move(gamept,direction,word,wordlen,&gamept->moves[gamept->curr_move]);

        if (retval) {
          /*printf(corrupted_msg);*/
          strcpy(err_msg,bad_piece_move[get_piece_type_ix(word[0])]);

          got_error = 3;
        }

        break;

      default:
        retval = do_pawn_move(gamept,direction,word,wordlen,&gamept->moves[gamept->curr_move]);

        if (retval) {
          /*printf(corrupted_msg);*/
          strcpy(err_msg,bad_pawn_move);

          got_error = 4;
        }

        break;
    }

    if (got_error)
      break;

    update_board(gamept,NULL,NULL,false);

    gamept->curr_move++;
    gamept->moves[gamept->curr_move].special_move_info = 0;
    gamept->num_moves = gamept->curr_move;

    if (gamept->curr_move == dbg_move)
      dbg = 1;

    bBlack = gamept->curr_move & 0x1;

    if (debug_fptr && (debug_level == 3)) {
      fprintf(debug_fptr,"read_game: curr_move = %d\n",gamept->curr_move);
      fprint_bd3(gamept->board,gamept->orientation,debug_fptr);
    }

    if (player_is_in_check(bBlack,gamept->board,gamept->curr_move)) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_CHECK;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_CHECK\n",gamept->curr_move);
    }

    if (garg_is_attacked(bBlack,gamept->board,gamept->curr_move)) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_GARG_IS_ATTACKED;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_GARG_IS_ATTACKED\n",gamept->curr_move);
    }

    if (mate_in_one_exists(gamept)) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_MATE_IN_ONE;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_MATE_IN_ONE\n",gamept->curr_move);
    }
  }

  fclose(fptr);

  legal_moves_count = 0;
  get_legal_moves(gamept,&legal_moves[0],&legal_moves_count);

  if (!legal_moves_count) {
    // determine if this is a checkmate or a stalemate
    if (gamept->moves[gamept->curr_move-1].special_move_info & SPECIAL_MOVE_CHECK) {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_MATE;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_MATE\n",gamept->curr_move);
    }
    else {
      gamept->moves[gamept->curr_move-1].special_move_info |= SPECIAL_MOVE_STALEMATE;

      if (debug_fptr && (debug_level == 3))
        fprintf(debug_fptr,"read_game: curr_move = %d, set SPECIAL_MOVE_STALEMATE\n",gamept->curr_move);
    }
  }

  if (got_error)
    return got_error;

  calculate_seirawan_counts(gamept);

  return 0;
}

int read_binary_game(char *filename,struct game *gamept)
{
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

  if (bHaveGame)
    afl_dbg = 1;

  if (debug_level == 2) {
    if (debug_fptr != NULL)
      fprintf(debug_fptr,"read_binary_game: %s\n",filename);
  }

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1) {
    if (debug_level == 2) {
      if (debug_fptr != NULL)
        fprintf(debug_fptr,"read_binary_game: open failed\n");
    }

    return 1;
  }

  bytes_to_read = sizeof (struct game) - (sizeof gamept->moves + sizeof gamept->board +
    sizeof gamept->white_pieces + sizeof gamept->black_pieces);

  bytes_read = read(fhndl,(char *)gamept,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    if (debug_level == 2) {
      if (debug_fptr != NULL)
        fprintf(debug_fptr,"read_binary_game: bytes_to_read = %d, bytes_read = %d\n",bytes_to_read,bytes_read);
    }

    close(fhndl);
    return 2;
  }

  bytes_to_read = gamept->num_moves * sizeof (struct move);

  if (bytes_to_read) {
    bytes_read = read(fhndl,(char *)gamept->moves,bytes_to_read);

    if (bytes_read != bytes_to_read) {
      if (debug_level == 2) {
        if (debug_fptr != NULL)
          fprintf(debug_fptr,"read_binary_game: bytes_to_read = %d, bytes_read = %d\n",bytes_to_read,bytes_read);
      }

      close(fhndl);
      return 3;
    }
  }

  close(fhndl);

  position_game(gamept,gamept->curr_move);

  return 0;
}

int write_binary_game(char *filename,struct game *gamept)
{
  int fhndl;
  unsigned int bytes_to_write;
  unsigned int bytes_written;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1)
    return 1;

  bytes_to_write = sizeof (struct game) - (sizeof gamept->moves + sizeof gamept->board +
    sizeof gamept->white_pieces + sizeof gamept->black_pieces);

  bytes_written = write(fhndl,(char *)gamept,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 2;
  }

  bytes_to_write = gamept->num_moves * sizeof (struct move);

  bytes_written = write(fhndl,(char *)gamept->moves,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 3;
  }

  close(fhndl);

  return 0;
}

int ignore_character(int chara)
{
  if ((chara == 0x0d) ||
    (chara == '(') ||
    (chara == ')') ||
    (chara == 'x') ||
    (chara == '=') ||
    (chara == '+') ||
    (chara == '#'))
    return true;

  return false;
}

int get_word(FILE *fptr,char *word,int maxlen,int *wordlenpt)
{
  int chara;
  int started;
  int comment;
  int end_of_file;
  int wordlen;

  wordlen = 0;
  started = 0;
  comment = 0;
  end_of_file = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    /* end of file ? */
    if (feof(fptr)) {
      end_of_file = 1;
      fclose(fptr);
      break;
    }

    // ignore carriage returns and other characters
    if (ignore_character(chara))
      continue;

    /* end of line ? */
    if (chara == 0x0a) {
      if (started)
        break;
      else {
        comment = 0;
        continue;
      }
    }

    /* in comment ? */
    if (comment)
      continue;

    /* comment marker ? */
    if (!wordlen && (chara == '/')) {
      comment = 1;
      continue;
    }

    /* white space ? */
    if ((chara == 0x09) || (chara == ' '))
      if (started)
        break;
      else
        continue;

    if (!(started))
      started = 1;

    if (wordlen < maxlen - 1)
      word[wordlen++] = chara;
  }

  word[wordlen] = 0;
  *wordlenpt = wordlen;

  return end_of_file;
}

static int update_board_calls;
static int dbg_update_board_call;
static int dbg;

void update_board(struct game *gamept,int *invalid_squares,int *num_invalid_squares,bool bScratch)
{
  int n;
  bool bBlack;
  int from_piece;
  int to_piece;
  bool bKingsideCastle = false;
  bool bGargsideCastle = false;
  bool bEnPassantCapture = false;
  int square_to_clear;

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  update_board_calls++;

  if (dbg_update_board_call == update_board_calls)
    dbg = 1;

  bBlack = (gamept->curr_move % 2);

  from_piece = get_piece1(gamept->board,gamept->moves[gamept->curr_move].from);
  to_piece = get_piece1(gamept->board,gamept->moves[gamept->curr_move].to);

  if (!bScratch && (from_piece * to_piece < 0))
    gamept->moves[gamept->curr_move].special_move_info |= SPECIAL_MOVE_CAPTURE;

  if (debug_fptr && (debug_level == 15)) {
    fprintf(debug_fptr,"update_board (%d): curr_move = %d, special_move_info = %x\n",
      update_board_calls,gamept->curr_move,gamept->moves[gamept->curr_move].special_move_info);
  }

  if (!bInGetLegalMoves) {
    if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE)
      bKingsideCastle = true;
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_GARGSIDE_CASTLE)
      bGargsideCastle = true;
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE)
      bEnPassantCapture = true;
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_PROMOTION_GARGANTUA)
      from_piece = (bBlack ? GARGANTUA_ID * -1 : GARGANTUA_ID);
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_PROMOTION_ROOK)
      from_piece = (bBlack ? ROOK_ID * -1 : ROOK_ID);
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_PROMOTION_BISHOP)
      from_piece = (bBlack ? BISHOP_ID * -1 : BISHOP_ID);
    else if (gamept->moves[gamept->curr_move].special_move_info & SPECIAL_MOVE_PROMOTION_KNIGHT)
      from_piece = (bBlack ? KNIGHT_ID * -1 : KNIGHT_ID);
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
      set_piece1(gamept->board,5,ROOK_ID);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 5;

      set_piece1(gamept->board,7,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 7;
    }
    else {
      // it's Blacks's move
      set_piece1(gamept->board,61,ROOK_ID * -1);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 61;

      set_piece1(gamept->board,63,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 63;
    }
  }
  else if (bGargsideCastle) {
    if (!(gamept->curr_move % 2)) {
      // it's White's move
      set_piece1(gamept->board,3,ROOK_ID);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 3;

      set_piece1(gamept->board,0,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 0;
    }
    else {
      // it's Blacks's move
      set_piece1(gamept->board,59,ROOK_ID * -1);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 59;

      set_piece1(gamept->board,56,0);

      if (invalid_squares)
        invalid_squares[(*num_invalid_squares)++] = 56;
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

  if (debug_fptr && (debug_level == 9)) {
    if (invalid_squares) {
      for (n = 0; n < *num_invalid_squares; n++) {
        fprintf(debug_fptr,"update_board (%d): invalid_squares[%d] = %d\n",
          update_board_calls,n,invalid_squares[n]);
      }
    }

    if (!bScratch)
      fprint_bd3(gamept->board,gamept->orientation,debug_fptr);
  }

  update_piece_info(gamept);
}

void update_piece_info(struct game *gamept)
{
  int n;
  char from;
  char to;
  int special_move_info;
  int debug;
  unsigned char board[CHARS_IN_BOARD];

  if (gamept->curr_move == dbg_move)
    dbg = 1;

  from = gamept->moves[gamept->curr_move].from;
  to = gamept->moves[gamept->curr_move].to;
  special_move_info = gamept->moves[gamept->curr_move].special_move_info;

  if (!(gamept->curr_move % 2)) {
    // it's White's move
    if (special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE) {
      gamept->white_pieces[4].current_board_position = 6;
      gamept->white_pieces[4].move_count++;
      gamept->white_pieces[7].current_board_position = 5;
      gamept->white_pieces[7].move_count++;
    }
    else if (special_move_info & SPECIAL_MOVE_GARGSIDE_CASTLE) {
      gamept->white_pieces[4].current_board_position = 2;
      gamept->white_pieces[4].move_count++;
      gamept->white_pieces[0].current_board_position = 3;
      gamept->white_pieces[0].move_count++;
    }
    else {
      for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
        if (gamept->white_pieces[n].current_board_position == from)
          break;
      }

      if (n == NUM_PIECES_PER_PLAYER)
        return; // should never happen

      gamept->white_pieces[n].current_board_position = to;
      gamept->white_pieces[n].move_count++;

      if (special_move_info & SPECIAL_MOVE_PROMOTION_GARGANTUA)
        gamept->white_pieces[n].piece_id = GARGANTUA_ID;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_ROOK)
        gamept->white_pieces[n].piece_id = ROOK_ID;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_KNIGHT)
        gamept->white_pieces[n].piece_id = KNIGHT_ID;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_BISHOP)
        gamept->white_pieces[n].piece_id = BISHOP_ID;
      else if (special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE) {
        for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
          if (gamept->black_pieces[n].current_board_position == to - NUM_FILES)
            break;
        }

        if (n == NUM_PIECES_PER_PLAYER)
          return; // should never happen

        gamept->black_pieces[n].current_board_position = -1;
      }

      if (!(special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE)) {
        for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
          if (gamept->black_pieces[n].current_board_position == to)
            break;
        }

        if (n < NUM_PIECES_PER_PLAYER)
          gamept->black_pieces[n].current_board_position = -1;
      }
    }
  }
  else {
    // it's Blacks's move
    if (special_move_info & SPECIAL_MOVE_KINGSIDE_CASTLE) {
      gamept->black_pieces[12].current_board_position = 62;
      gamept->black_pieces[12].move_count++;
      gamept->black_pieces[15].current_board_position = 61;
      gamept->black_pieces[15].move_count++;
    }
    else if (special_move_info & SPECIAL_MOVE_GARGSIDE_CASTLE) {
      gamept->black_pieces[12].current_board_position = 58;
      gamept->black_pieces[12].move_count++;
      gamept->black_pieces[8].current_board_position = 59;
      gamept->black_pieces[8].move_count++;
    }
    else {
      for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
        if (gamept->black_pieces[n].current_board_position == from)
          break;
      }

      if (n == NUM_PIECES_PER_PLAYER)
        return; // should never happen

      gamept->black_pieces[n].current_board_position = to;
      gamept->black_pieces[n].move_count++;

      if (special_move_info & SPECIAL_MOVE_PROMOTION_GARGANTUA)
        gamept->black_pieces[n].piece_id = GARGANTUA_ID * -1;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_ROOK)
        gamept->black_pieces[n].piece_id = ROOK_ID * -1;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_KNIGHT)
        gamept->black_pieces[n].piece_id = KNIGHT_ID * -1;
      else if (special_move_info & SPECIAL_MOVE_PROMOTION_BISHOP)
        gamept->black_pieces[n].piece_id = BISHOP_ID * -1;
      else if (special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE) {
        for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
          if (gamept->white_pieces[n].current_board_position == to + NUM_FILES)
            break;
        }

        if (n == NUM_PIECES_PER_PLAYER)
          return; // should never happen

        gamept->white_pieces[n].current_board_position = -1;
      }

      if (!(special_move_info & SPECIAL_MOVE_EN_PASSANT_CAPTURE)) {
        for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
          if (gamept->white_pieces[n].current_board_position == to)
            break;
        }

        if (n < NUM_PIECES_PER_PLAYER)
          gamept->white_pieces[n].current_board_position = -1;
      }
    }
  }

  if (debug_fptr && (debug_level == 4)) {
    fprintf(debug_fptr,"update_piece_info: curr_move = %d, num_moves = %d\n",gamept->curr_move,gamept->num_moves);
    fprint_piece_info(gamept,debug_fptr);
    populate_board_from_piece_info(gamept,board);
    fprint_bd3(board,gamept->orientation,debug_fptr);
  }
}

void fprint_piece_info(struct game *gamept,FILE *fptr)
{
  int n;

  fprintf(fptr,"fprint_piece_info: curr_move = %d\n",gamept->curr_move);
  fprintf(fptr,"White:\n");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->white_pieces[n].current_board_position == -1) {
      fprintf(fptr,"  %s %d %d\n",
        piece_names[gamept->white_pieces[n].piece_id - 1],
        gamept->white_pieces[n].current_board_position,
        gamept->white_pieces[n].move_count);
    }
    else {
      fprintf(fptr,"  %s %c%c %d\n",
        piece_names[gamept->white_pieces[n].piece_id - 1],
        'a' + FILE_OF(gamept->white_pieces[n].current_board_position),
        '1' + RANK_OF(gamept->white_pieces[n].current_board_position),
        gamept->white_pieces[n].move_count);
    }
  }

  fprintf(fptr,"Black:\n");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->black_pieces[n].current_board_position == -1) {
      fprintf(fptr,"  %s %d %d\n",
        piece_names[(gamept->black_pieces[n].piece_id * -1) - 1],
        gamept->black_pieces[n].current_board_position,
        gamept->black_pieces[n].move_count);
    }
    else {
      fprintf(fptr,"  %s %c%c %d\n",
        piece_names[(gamept->black_pieces[n].piece_id * -1) - 1],
        'a' + FILE_OF(gamept->black_pieces[n].current_board_position),
        '1' + RANK_OF(gamept->black_pieces[n].current_board_position),
        gamept->black_pieces[n].move_count);
    }
  }
}

void print_piece_info(struct game *gamept)
{
  int n;

  printf("White:\n");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->white_pieces[n].current_board_position == -1) {
      printf("  %s %d %d\n",
        piece_names[gamept->white_pieces[n].piece_id - 1],
        gamept->white_pieces[n].current_board_position,
        gamept->white_pieces[n].move_count);
    }
    else {
      printf("  %s %c%c %d\n",
        piece_names[gamept->white_pieces[n].piece_id - 1],
        'a' + FILE_OF(gamept->white_pieces[n].current_board_position),
        '1' + RANK_OF(gamept->white_pieces[n].current_board_position),
        gamept->white_pieces[n].move_count);
    }
  }

  printf("Black:\n");

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->black_pieces[n].current_board_position == -1) {
      printf("  %s %d %d\n",
        piece_names[(gamept->black_pieces[n].piece_id * -1) - 1],
        gamept->black_pieces[n].current_board_position,
        gamept->black_pieces[n].move_count);
    }
    else {
      printf("  %s %c%c %d\n",
        piece_names[(gamept->black_pieces[n].piece_id * -1) - 1],
        'a' + FILE_OF(gamept->black_pieces[n].current_board_position),
        '1' + RANK_OF(gamept->black_pieces[n].current_board_position),
        gamept->black_pieces[n].move_count);
    }
  }
}

void print_piece_info2(struct piece_info *info_pt)
{
  int n;
  char piece_id;

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    piece_id = info_pt[n].piece_id;

    if (piece_id < 0)
      piece_id *= -1;

    if (info_pt[n].current_board_position == -1) {
      printf("  %s %d %d\n",
        piece_names[piece_id - 1],
        info_pt[n].current_board_position,
        info_pt[n].move_count);
    }
    else {
      printf("  %s %c%c %d\n",
        piece_names[piece_id - 1],
        'a' + FILE_OF(info_pt[n].current_board_position),
        '1' + RANK_OF(info_pt[n].current_board_position),
        info_pt[n].move_count);
    }
  }
}

void populate_board_from_piece_info(struct game *gamept,unsigned char *board)
{
  int n;
  unsigned int bit_offset;

  for (n = 0; n < CHARS_IN_BOARD; n++)
    board[n] = 0;

  for (n = 0; n < NUM_PIECES_PER_PLAYER; n++) {
    if (gamept->white_pieces[n].current_board_position != -1) {
      bit_offset = gamept->white_pieces[n].current_board_position * BITS_PER_BOARD_SQUARE;
      set_bits(BITS_PER_BOARD_SQUARE,board,bit_offset,gamept->white_pieces[n].piece_id);
    }

    if (gamept->black_pieces[n].current_board_position != -1) {
      bit_offset = gamept->black_pieces[n].current_board_position * BITS_PER_BOARD_SQUARE;
      set_bits(BITS_PER_BOARD_SQUARE,board,bit_offset,gamept->black_pieces[n].piece_id);
    }
  }
}

int compare_boards(unsigned char *board1,unsigned char *board2)
{
  int n;

  for (n = 0; n < CHARS_IN_BOARD; n++) {
    if (board1[n] != board2[n])
      return 0;
  }

  return 1;
}

void copy_game(struct game *gamept_to,struct game *gamept_from)
{
  memcpy(gamept_to,gamept_from,sizeof (struct game));
}

void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
{
  int chara;
  int local_line_len;

  local_line_len = 0;

  for ( ; ; ) {
    chara = fgetc(fptr);

    if (feof(fptr))
      break;

    if (chara == '\n')
      break;

    if (local_line_len < maxllen - 1)
      line[local_line_len++] = (char)chara;
  }

  line[local_line_len] = 0;
  *line_len = local_line_len;
}

#define MAX_LINE_LEN 256
static char line[MAX_LINE_LEN];

int populate_board_from_board_file(unsigned char *board,char *filename)
{
  int m;
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  int chara;
  int piece;

  if ((fptr = fopen(filename,"r")) == NULL)
    return 1;

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    if (line_len != 19)
      return 2;

    for (n = 0; n < NUM_FILES; n++) {
      chara = line[n*2];

      if (chara == '.') {
        piece = 0;
        set_piece2(board,7 - line_no,n,piece);
      }
      else {
        if (chara == 'p')
          set_piece2(board,7 - line_no,n,PAWN_ID);
        else if (chara == 'P')
          set_piece2(board,7 - line_no,n,PAWN_ID * -1);
        else if (chara == 'e')
          set_piece2(board,7 - line_no,n,EMPTY_ID);
        else {
          for (m = 0; m < NUM_PIECE_TYPES; m++) {
            if (chara == piece_ids[m]) {
              piece = (m + 2) * -1;
              break;
            }
            else if (chara == piece_ids[m] - 'A' + 'a') {
              piece = (m + 2);
              break;
            }
          }

          if (m < NUM_PIECE_TYPES)
            set_piece2(board,7 - line_no,n,piece);
        }
      }
    }

    line_no++;
  }

  fclose(fptr);

  return 0;
}

int write_board_to_binfile(unsigned char *board,char *filename)
{
  int fhndl;
  unsigned int bytes_to_write;
  unsigned int bytes_written;

  if ((fhndl = open(filename,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
      S_IREAD | S_IWRITE)) == -1)
    return 1;

  bytes_to_write = CHARS_IN_BOARD;

  bytes_written = write(fhndl,(char *)board,bytes_to_write);

  if (bytes_written != bytes_to_write) {
    close(fhndl);
    return 2;
  }

  close(fhndl);

  return 0;
}

void calculate_seirawan_counts(struct game *gamept)
{
  int m;
  int n;
  int o;
  int direction;
  int piece;
  int low;
  int high;

  for (m = 0; m < 2; m++) {
    seirawan_count[m] = 0;

    if (!m)
      direction = 1;
    else
      direction = -1;

    for (n = 0; n < NUM_BOARD_SQUARES; n++) {
      piece = get_piece1(gamept->board,n);

      if (piece * direction > 0) {
        if (piece < 0) {
          low = 0;
          high = 32;
        }
        else {
          low = 32;
          high = 64;
        }

        for (o = low; o < high; o++) {
          if (square_attacks_square(gamept->board,n,o))
            seirawan_count[m]++;
        }
      }
    }
  }

  if (debug_fptr && (debug_level == 16)) {
    fprintf(debug_fptr,"calculate_seirawan_counts: board:\n");
    fprint_bd2(gamept->board,debug_fptr);
    fprintf(debug_fptr,"calculate_seirawan_counts: White: %d Black: %d\n",
      seirawan_count[0],seirawan_count[1]);
  }
}
