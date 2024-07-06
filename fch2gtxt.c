#include <stdio.h>
#include <string.h>

#define MAX_FILENAME_LEN 1024

static char filename[MAX_FILENAME_LEN];
static char gtxt_filename[MAX_FILENAME_LEN];

#define MAX_LINE_LEN 8192
static char line[MAX_LINE_LEN];

static char usage[] =
"usage: fch2gtxt (-debug) filename\n";

char couldnt_get_status[] = "couldn't get status of %s\n";
char couldnt_open[] = "couldn't open %s\n";

void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

static int build_gtxt_filename(
  char *ch_filename,
  int ch_filename_len,
  char *gtxt_filename,
  int max_filename_len);

void change_queens_to_gargantuas(int line_no,char *line,int line_len,int bDebug);

int main(int argc,char **argv)
{
  int curr_arg;
  bool bDebug;
  FILE *fptr0;
  FILE *fptr_in;
  FILE *fptr_out;
  int file_len;
  int ch_filename_len;
  int line_no;
  int line_len;
  int retval;

  if ((argc < 2) || (argc > 3)) {
    printf(usage);
    return 1;
  }

  bDebug = false;

  for (curr_arg = 1; curr_arg < argc; curr_arg++) {
    if (!strcmp(argv[curr_arg],"-debug"))
      bDebug = true;
    else
      break;
  }

  if (argc - curr_arg != 1) {
    printf(usage);
    return 2;
  }

  if ((fptr0 = fopen(argv[curr_arg],"r")) == NULL) {
    printf(couldnt_open,argv[curr_arg]);
    return 3;
  }

  for ( ; ; ) {
    GetLine(fptr0,filename,&file_len,MAX_FILENAME_LEN);

    if (feof(fptr0))
      break;

    ch_filename_len = strlen(filename);

    retval = build_gtxt_filename(filename,ch_filename_len,gtxt_filename,MAX_FILENAME_LEN);

    if (retval) {
      printf("build_gtxt_filename failed on %s: %d\n",filename,retval);
      continue;
    }

    if ((fptr_in = fopen(filename,"r")) == NULL) {
      printf(couldnt_open,filename);
      continue;
    }

    if ((fptr_out = fopen(gtxt_filename,"w")) == NULL) {
      printf(couldnt_open,gtxt_filename);
      fclose(fptr_in);
      continue;
    }

    line_no = 0;

    for ( ; ; ) {
      GetLine(fptr_in,line,&line_len,MAX_LINE_LEN);

      if (feof(fptr_in))
        break;

       line_no++;

      change_queens_to_gargantuas(line_no,line,line_len,bDebug);

      fprintf(fptr_out,"%s\n",line);
    }

    fclose(fptr_in);
    fclose(fptr_out);
  }

  return 0;
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

static int build_gtxt_filename(
  char *ch_filename,
  int ch_filename_len,
  char *gtxt_filename,
  int max_filename_len)
{
  int n;

  for (n = 0; n < ch_filename_len; n++) {
    if (ch_filename[n] == '.')
      break;
  }

  if (n == ch_filename_len)
    return 1;

  if (n + 5 > max_filename_len - 1)
    return 2;

  strncpy(gtxt_filename,ch_filename,n);
  strcpy(&gtxt_filename[n],".gtxt");

  return 0;
}

void change_queens_to_gargantuas(int line_no,char *line,int line_len,int bDebug)
{
  int n;

  for (n = 0; n < line_len; n++) {
    if ((line[n] >= '0') && (line[n] <= '9'))
      ;
    else
      break;
  }

  if (bDebug) {
    if ((n >= 0) && (n < line_len))
      printf("line_no = %d, line_len = %d, n = %d, line[n] = %c\n",line_no,line_len,n,line[n]);
    else
      printf("line_no = %d, line_len = %d, n = %d\n",line_no,line_len,n);
  }

  if (!n || (n == line_len) || (line[n] != '.'))
    return;

  for (n++; n < line_len; n++) {
    if (line[n] == 'Q')
      line[n] = 'G';
  }
}
