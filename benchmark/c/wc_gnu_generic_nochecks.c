#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <wchar.h>
#include <wctype.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <string.h>



/* Size of atomic reads. */
#define BUFFER_SIZE (64 * 1024)
static char buf[BUFFER_SIZE + 1];

static bool
wc_lines (char const *file, int fd, uintmax_t *lines_out)
{
  size_t bytes_read;
  uintmax_t lines, bytes;

  lines = bytes = 0;

  while ((bytes_read = read (fd, buf, BUFFER_SIZE)) > 0) {
      if (bytes_read == -1) { return false; }

      bytes += bytes_read;

      char *p = buf;
      char *end = buf + bytes_read;

      /* rawmemchr is more efficient with longer lines.  */
      *end = '\n';
      while ((p = rawmemchr (p, '\n')) < end) {
//      while ((p = memchr (p, '\n', BUFFER_SIZE)) < end) {
          ++p;
          ++lines;
      }
  }

  *lines_out = lines;

  return true;
}



int
main (int argc, char **argv)
{
    bool ok;
    uintmax_t lines;

    posix_fadvise (STDIN_FILENO, 0, 0, POSIX_FADV_SEQUENTIAL);

    ok = wc_lines (NULL, STDIN_FILENO, &lines);
    if (ok) {
        printf("%ld\n", lines);
    } else {
        printf("error %d\n", errno);
    }


    return 0;
}

