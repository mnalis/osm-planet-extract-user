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
#define BUFFER_SIZE (16 * 1024)

enum { SYS_BUFSIZE_MAX = INT_MAX >> 20 << 20 };
 #define SAFE_READ_ERROR ((size_t) -1)

#ifdef EINTR
# define IS_EINTR(x) ((x) == EINTR)
#else
# define IS_EINTR(x) 0
#endif

/* Read(write) up to COUNT bytes at BUF from(to) descriptor FD, retrying if
   interrupted.  Return the actual number of bytes read(written), zero for EOF,
   or SAFE_READ_ERROR(SAFE_WRITE_ERROR) upon error.  */
size_t
safe_read (int fd, void *buf, size_t count)
{
  for (;;)
    {
      ssize_t result = read (fd, buf, count);

      if (0 <= result)
        return result;
      else if (IS_EINTR (errno))
        continue;
      else if (errno == EINVAL && SYS_BUFSIZE_MAX < count)
        count = SYS_BUFSIZE_MAX;
      else
        return result;
    }
}


static bool
wc_lines (char const *file, int fd, uintmax_t *lines_out)
{
  size_t bytes_read;
  uintmax_t lines = 0;
  char buf[BUFFER_SIZE + 1];

  while ((bytes_read = safe_read (fd, buf, BUFFER_SIZE)) > 0)
    {

      if (bytes_read == SAFE_READ_ERROR) {
          return false;
        }

      char *p = buf;
      char *end = buf + bytes_read;

      while (p != end)
            lines += *p++ == '\n';
    }

  *lines_out = lines;

  return true;
}



int
main (int argc, char **argv)
{
    bool ok;
    uintmax_t lines;

    //posix_fadvise (STDIN_FILENO, 0, 0, POSIX_FADV_SEQUENTIAL);

    ok = wc_lines (NULL, STDIN_FILENO, &lines);
    if (ok) {
        printf("%ld\n", lines);
    } else {
        printf("error %d\n", errno);
    }


    return 0;
}

