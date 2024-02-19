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

#include <x86intrin.h>


/* This must be below 16 KB (16384) or else the accumulators can
   theoretically overflow, producing wrong result. This is 2*32 bytes below,
   so there is no single bytes in the optimal case. */
#define BUFSIZE (16320)


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


bool
wc_lines (char const *file, int fd, uintmax_t *lines_out)
{
  __m256i accumulator;
  __m256i accumulator2;
  __m256i zeroes;
  __m256i endlines;
  __m256i avx_buf[BUFSIZE / sizeof (__m256i)];
  __m256i *datap;
  uintmax_t lines = 0;
  size_t bytes_read = 0;


  if (!lines_out)
    return false;

  /* Using two parallel accumulators gave a good performance increase.
     Adding a third gave no additional benefit, at least on an
     Intel Xeon E3-1231v3.  Maybe on a newer CPU with additional vector
     execution engines it would be a win. */
  accumulator = _mm256_setzero_si256 ();
  accumulator2 = _mm256_setzero_si256 ();
  zeroes = _mm256_setzero_si256 ();
  endlines = _mm256_set1_epi8 ('\n');

  while ((bytes_read = safe_read (fd, avx_buf, sizeof (avx_buf))) > 0)
    {
      __m256i to_match;
      __m256i to_match2;
      __m256i matches;
      __m256i matches2;

      if (bytes_read == SAFE_READ_ERROR)
        {
          return false;
        }

      datap = avx_buf;
      char *end = ((char *)avx_buf) + bytes_read;

      #define AVX_PARALLEL 2
      while (bytes_read >= 32 * AVX_PARALLEL)
        {
          to_match = _mm256_load_si256 (datap);
          to_match2 = _mm256_load_si256 (datap + 1);

          matches = _mm256_cmpeq_epi8 (to_match, endlines);
          matches2 = _mm256_cmpeq_epi8 (to_match2, endlines);
          /* Compare will set each 8 bit integer in the register to 0xFF
             on match.  When we subtract it the 8 bit accumulators
             will underflow, so this is equal to adding 1. */
          accumulator = _mm256_sub_epi8 (accumulator, matches);
          accumulator2 = _mm256_sub_epi8 (accumulator2, matches2);

          datap += 2;
          bytes_read -= 32 * AVX_PARALLEL;
        }

      /* Horizontally add all 8 bit integers in the register,
         and then reset it */
      accumulator = _mm256_sad_epu8 (accumulator, zeroes);
      lines +=   _mm256_extract_epi16 (accumulator, 0)
               + _mm256_extract_epi16 (accumulator, 4)
               + _mm256_extract_epi16 (accumulator, 8)
               + _mm256_extract_epi16 (accumulator, 12);
      accumulator = _mm256_setzero_si256 ();

      accumulator2 = _mm256_sad_epu8 (accumulator2, zeroes);
      lines +=   _mm256_extract_epi16 (accumulator2, 0)
               + _mm256_extract_epi16 (accumulator2, 4)
               + _mm256_extract_epi16 (accumulator2, 8)
               + _mm256_extract_epi16 (accumulator2, 12);
      accumulator2 = _mm256_setzero_si256 ();

      /* Finish up any left over bytes */
      char *p = (char *)datap;
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

    posix_fadvise (STDIN_FILENO, 0, 0, POSIX_FADV_SEQUENTIAL);

    ok = wc_lines (NULL, STDIN_FILENO, &lines);
    if (ok) {
        printf("%ld\n", lines);
    } else {
        printf("error %d\n", errno);
    }


    return 0;
}

