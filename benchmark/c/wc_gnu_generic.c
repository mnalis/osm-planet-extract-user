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


/* Maximum number of bytes to read or write in a single system call.
   This can be useful for system calls like sendfile on GNU/Linux,
   which do not handle more than MAX_RW_COUNT bytes correctly.
   The Linux kernel MAX_RW_COUNT is at least INT_MAX >> 20 << 20,
   where the 20 comes from the Hexagon port with 1 MiB pages; use that
   as an approximation, as the exact value may not be available to us.

   Using this also works around a serious Linux bug before 2.6.16; see
   <https://bugzilla.redhat.com/show_bug.cgi?id=612839>.

   Using this also works around a Tru64 5.1 bug, where attempting
   to read INT_MAX bytes fails with errno == EINVAL.  See
   <https://lists.gnu.org/r/bug-gnu-utils/2002-04/msg00010.html>.

   Using this is likely to work around similar bugs in other operating
   systems.  */

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
wc_lines (char const *file, int fd, uintmax_t *lines_out, uintmax_t *bytes_out)
{
  size_t bytes_read;
  uintmax_t lines, bytes;
  char buf[BUFFER_SIZE + 1];
  bool long_lines = false;

  if (!lines_out || !bytes_out)
    {
      return false;
    }

  lines = bytes = 0;

  while ((bytes_read = safe_read (fd, buf, BUFFER_SIZE)) > 0)
    {

      if (bytes_read == SAFE_READ_ERROR) {
          return false;
        }

      bytes += bytes_read;

      char *p = buf;
      char *end = buf + bytes_read;
      uintmax_t plines = lines;

      if (! long_lines)
        {
          /* Avoid function call overhead for shorter lines.  */
          while (p != end)
            lines += *p++ == '\n';
        }
      else
        {
          /* rawmemchr is more efficient with longer lines.  */
          *end = '\n';
          while ((p = rawmemchr (p, '\n')) < end)
            {
              ++p;
              ++lines;
            }
        }

      /* If the average line length in the block is >= 15, then use
          memchr for the next block, where system specific optimizations
          may outweigh function call overhead.
          FIXME: This line length was determined in 2015, on both
          x86_64 and ppc64, but it's worth re-evaluating in future with
          newer compilers, CPUs, or memchr() implementations etc.  */
      if (lines - plines <= bytes_read / 15)
        long_lines = true;
      else
        long_lines = false;
    }

  *bytes_out = bytes;
  *lines_out = lines;

  return true;
}



int
main (int argc, char **argv)
{
    bool ok;
    uintmax_t lines, bytes;

    posix_fadvise (STDIN_FILENO, 0, 0, POSIX_FADV_SEQUENTIAL);

    ok = wc_lines (NULL, STDIN_FILENO, &lines, &bytes);
    if (ok) {
        printf("%ld\n", lines);
    } else {
        printf("error %d\n", errno);
    }


    return 0;
}

