#include <stdio.h>

#define MAX_LINE_LEN 1024
static char line[MAX_LINE_LEN];

static char usage[] = "usage: result_changed_histogram filename\n";
static char couldnt_open[] = "couldn't open %s\n";

struct hist_element {
  int orig_result;
  int new_result;
  int count;
};

static struct hist_element elements[] = {
  1, 2, 0,
  2, 1, 0,
  1, 3, 0,
  3, 1, 0,
  2, 3, 0,
  3, 2, 0
};
#define NUM_ELEMENTS (sizeof elements / sizeof (struct hist_element))

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen);

int main(int argc,char **argv)
{
  int n;
  FILE *fptr;
  int line_len;
  int line_no;
  int orig_result;
  int new_result;

  if (argc != 2) {
    printf(usage);
    return 1;
  }

  if ((fptr = fopen(argv[1],"r")) == NULL) {
    printf(couldnt_open,argv[1]);
    return 2;
  }

  line_no = 0;

  for ( ; ; ) {
    GetLine(fptr,line,&line_len,MAX_LINE_LEN);

    if (feof(fptr))
      break;

    line_no++;

    sscanf(line,"%d %d",&orig_result,&new_result);

    for (n = 0; n < NUM_ELEMENTS; n++) {
      if ((elements[n].orig_result == orig_result) && (elements[n].new_result == new_result))
        elements[n].count++;
    }
  }

  for (n = 0; n < NUM_ELEMENTS; n++) {
    printf("%d to %d: %d\n",elements[n].orig_result,elements[n].new_result,elements[n].count);
  }

  fclose(fptr);

  return 0;
}

static void GetLine(FILE *fptr,char *line,int *line_len,int maxllen)
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
