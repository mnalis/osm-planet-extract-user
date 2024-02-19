// for rawmemchr(2)
#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


#define BUFFER_SIZE (16 * 1024)


static bool
wc_lines (char *start, size_t len, uintmax_t *lines_out)
{
  uintmax_t lines = 0;

      char *p = start;
      char *end = start + len;

/*
    ssize_t      s;

    s = write(STDOUT_FILENO, start, 30);
    if (s != 30) {
        if (s == -1)
            handle_error("write");

        fprintf(stderr, "partial write");
        exit(EXIT_FAILURE);
    }
//    return true;
*/

      /* rawmemchr is more efficient with longer lines.  */
      //*end = '\n';	// FIXME: we can't write to PROT_READ mmap(), and it would change the file anyway. So the file must end in newline or this will fail
      while ((p = rawmemchr (p, '\n')) < end) {
//      while ((p = memchr (p, '\n', BUFFER_SIZE)) < end) {
          ++p;
          ++lines;
      }

  *lines_out = lines;

  return true;
}



int
main (int argc, char **argv)
{
    bool ok;
    uintmax_t lines = 0;

    int          fd;
    off_t        offset, pa_offset;
    size_t       length;
    char         *addr;
    struct stat  sb;

    if (argc != 2) {
        fprintf(stderr, "%s file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        handle_error("open");

    if (fstat(fd, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");

    //posix_fadvise (fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    offset = 0;
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
        /* offset for mmap() must be page aligned */

    if (offset >= sb.st_size) {
        fprintf(stderr, "offset is past end of file\n");
        exit(EXIT_FAILURE);
    }

    length = sb.st_size - offset;

    addr = mmap(NULL, length + offset - pa_offset, PROT_READ,
                MAP_PRIVATE, fd, pa_offset);
    if (addr == MAP_FAILED) handle_error("mmap");

	//printf ("DEBUG: mmaped %s from offset %ld length %ld\n", argv[1], offset, length);

    ok = wc_lines (addr, length, &lines);
    if (ok) {
        printf("%ld\n", lines);
    } else {
        printf("error\n");
    }

    munmap(addr, length + offset - pa_offset);
    close(fd);

    exit(EXIT_SUCCESS);
}

