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

static unsigned char initial_board[] = {
  (unsigned char)0x72, (unsigned char)0x34, (unsigned char)0x56, (unsigned char)0x43, (unsigned char)0x27,
  (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11, (unsigned char)0x11,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00, (unsigned char)0x00,
  (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff, (unsigned char)0xff,
  (unsigned char)0x9e, (unsigned char)0xdc, (unsigned char)0xba, (unsigned char)0xcd, (unsigned char)0xe9,
};

static char *bad_piece_move[] = {
  "bad rook move",
  "bad knight move",
  "bad bishop move",
  "bad queen move",
  "bad king move",
  "bad gargantua move"
};

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
}

int read_binary_game(char *filename,struct game *gamept)
{
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

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

  bytes_read = read(fhndl,(char *)gamept->moves,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    if (debug_level == 2) {
      if (debug_fptr != NULL)
        fprintf(debug_fptr,"read_binary_game: bytes_to_read = %d, bytes_read = %d\n",bytes_to_read,bytes_read);
    }

    close(fhndl);
    return 3;
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
    (chara == '+'))
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
    if (chara == '/') {
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

int populate_initial_board_from_board_file(char *filename)
{
  return populate_board_from_board_file(initial_board,filename);
}

int populate_board_from_bin_board_file(unsigned char *board,char *filename)
{
  struct stat stat_buf;
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

  if (stat(filename,&stat_buf) == -1)
    return 1;

  if (stat_buf.st_size != CHARS_IN_BOARD)
    return 2;

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1)
    return 3;

  bytes_to_read = CHARS_IN_BOARD;

  bytes_read = read(fhndl,(char *)board,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    close(fhndl);
    return 4;
  }

  close(fhndl);

  return 0;
}

int populate_piece_counts_from_piece_count_file(int *piece_counts,char *filename)
{
  struct stat stat_buf;
  int fhndl;
  unsigned int bytes_to_read;
  unsigned int bytes_read;

  if (stat(filename,&stat_buf) == -1)
    return 1;

  if (stat_buf.st_size != (NUM_PIECE_TYPES_0 * 2) * sizeof (int))
    return 2;

  if ((fhndl = open(filename,O_RDONLY | O_BINARY)) == -1)
    return 3;

  bytes_to_read = (NUM_PIECE_TYPES_0 * 2) * sizeof (int);

  bytes_read = read(fhndl,(char *)piece_counts,bytes_to_read);

  if (bytes_read != bytes_to_read) {
    close(fhndl);
    return 4;
  }

  close(fhndl);

  return 0;
}

int populate_initial_board_from_bin_board_file(char *filename)
{
  return populate_board_from_bin_board_file(initial_board,filename);
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

int count_num_pieces(int color,struct game *gamept)
{
  int n;
  short piece;
  int count;

  count = 0;

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(gamept->board,n);

    if (!piece)
      continue;

    if (color == WHITE) {
      if (piece > 0)
        count++;
    }
    else {
      if (piece < 0)
        count++;
    }
  }

  return count;
}

void get_piece_counts(unsigned char *board,int *piece_counts)
{
  int n;
  short piece;

  for (n = 0; n < NUM_PIECE_TYPES_0 * 2; n++) {
    piece_counts[n] = 0;
  }

  for (n = 0; n < NUM_BOARD_SQUARES; n++) {
    piece = get_piece1(board,n);

    if (!piece)
      continue;

    if (piece > 0) {
      piece_counts[piece - 1]++;
    }
    else {
      piece *= -1;
      piece_counts[NUM_PIECE_TYPES_0 + piece - 1]++;
    }
  }
}

int piece_counts_match(int *piece_counts,int *match_piece_counts,bool bExactMatch)
{
  int n;

  for (n = 0; n < NUM_PIECE_TYPES_0 * 2; n++) {
    if (match_piece_counts[n] == -1) {
      if (piece_counts[n])
        return 0;
      else
        continue;
    }

    if (bExactMatch) {
      if (piece_counts[n] != match_piece_counts[n])
        return 0;
    }
    else {
      if (match_piece_counts[n] && (piece_counts[n] != match_piece_counts[n]))
        return 0;
    }
  }

  return 1;
}

void print_piece_counts(int *piece_counts)
{
  int n;
  int square;

  putchar(0x0a);

  for (n = 0; n < NUM_PIECE_TYPES_0 * 2; n++) {
    if (!n)
      square = 1;
    else if (n == NUM_PIECE_TYPES_0)
      square = -1;

    if (n < (NUM_PIECE_TYPES_0 * 2) - 1) {
      printf("%c %d ",format_square(square),piece_counts[n]);

      if (square > 0)
        square++;
      else
        square--;
    }
    else
      printf("%c %d\n",format_square(square),piece_counts[n]);
  }
}
