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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


#define BUFFER_SIZE (16 * 1024)



#define ALPHABET_LEN 256
#define max(a, b) ((a < b) ? b : a)

// BAD-CHARACTER RULE.
// delta1 table: delta1[c] contains the distance between the last
// character of pat and the rightmost occurrence of c in pat.
//
// If c does not occur in pat, then delta1[c] = patlen.
// If c is at string[i] and c != pat[patlen-1], we can safely shift i
//   over by delta1[c], which is the minimum distance needed to shift
//   pat forward to get string[i] lined up with some character in pat.
// c == pat[patlen-1] returning zero is only a concern for BMH, which
//   does not have delta2. BMH makes the value patlen in such a case.
//   We follow this choice instead of the original 0 because it skips
//   more. (correctness?)
//
// This algorithm runs in alphabet_len+patlen time.
void make_delta1(ptrdiff_t *delta1, uint8_t *pat, size_t patlen) {
    for (int i=0; i < ALPHABET_LEN; i++) {
        delta1[i] = patlen;
    }
    for (int i=0; i < patlen; i++) {
        delta1[pat[i]] = patlen-1 - i;
    }
}

// true if the suffix of word starting from word[pos] is a prefix
// of word
bool is_prefix(uint8_t *word, size_t wordlen, ptrdiff_t pos) {
    int suffixlen = wordlen - pos;
    // could also use the strncmp() library function here
    // return ! strncmp(word, &word[pos], suffixlen);
    for (int i = 0; i < suffixlen; i++) {
        if (word[i] != word[pos+i]) {
            return false;
        }
    }
    return true;
}

// length of the longest suffix of word ending on word[pos].
// suffix_length("dddbcabc", 8, 4) = 2
size_t suffix_length(uint8_t *word, size_t wordlen, ptrdiff_t pos) {
    size_t i;
    // increment suffix length i to the first mismatch or beginning
    // of the word
    for (i = 0; (word[pos-i] == word[wordlen-1-i]) && (i <= pos); i++);
    return i;
}

// GOOD-SUFFIX RULE.
// delta2 table: given a mismatch at pat[pos], we want to align
// with the next possible full match could be based on what we
// know about pat[pos+1] to pat[patlen-1].
//
// In case 1:
// pat[pos+1] to pat[patlen-1] does not occur elsewhere in pat,
// the next plausible match starts at or after the mismatch.
// If, within the substring pat[pos+1 .. patlen-1], lies a prefix
// of pat, the next plausible match is here (if there are multiple
// prefixes in the substring, pick the longest). Otherwise, the
// next plausible match starts past the character aligned with
// pat[patlen-1].
//
// In case 2:
// pat[pos+1] to pat[patlen-1] does occur elsewhere in pat. The
// mismatch tells us that we are not looking at the end of a match.
// We may, however, be looking at the middle of a match.
//
// The first loop, which takes care of case 1, is analogous to
// the KMP table, adapted for a 'backwards' scan order with the
// additional restriction that the substrings it considers as
// potential prefixes are all suffixes. In the worst case scenario
// pat consists of the same letter repeated, so every suffix is
// a prefix. This loop alone is not sufficient, however:
// Suppose that pat is "ABYXCDBYX", and text is ".....ABYXCDEYX".
// We will match X, Y, and find B != E. There is no prefix of pat
// in the suffix "YX", so the first loop tells us to skip forward
// by 9 characters.
// Although superficially similar to the KMP table, the KMP table
// relies on information about the beginning of the partial match
// that the BM algorithm does not have.
//
// The second loop addresses case 2. Since suffix_length may not be
// unique, we want to take the minimum value, which will tell us
// how far away the closest potential match is.
void make_delta2(ptrdiff_t *delta2, uint8_t *pat, size_t patlen) {
    ssize_t p;
    size_t last_prefix_index = 1;

    // first loop
    for (p=patlen-1; p>=0; p--) {
        if (is_prefix(pat, patlen, p+1)) {
            last_prefix_index = p+1;
        }
        delta2[p] = last_prefix_index + (patlen-1 - p);
    }

    // second loop
    for (p=0; p < patlen-1; p++) {
        size_t slen = suffix_length(pat, patlen, p);
        if (pat[p - slen] != pat[patlen-1 - slen]) {
            delta2[patlen-1 - slen] = patlen-1 - p + slen;
        }
    }
}

// Returns pointer to first match.
// See also glibc memmem() (non-BM) and std::boyer_moore_searcher (first-match).
uint8_t* boyer_moore (uint8_t *string, size_t stringlen, uint8_t *pat, size_t patlen) {
    ptrdiff_t delta1[ALPHABET_LEN];
    ptrdiff_t delta2[patlen]; // C99 VLA
    make_delta1(delta1, pat, patlen);
    make_delta2(delta2, pat, patlen);

    // The empty pattern must be considered specially
    if (patlen == 0) {
        return string;
    }

    size_t i = patlen - 1;        // str-idx
    while (i < stringlen) {
        ptrdiff_t j = patlen - 1; // pat-idx
        while (j >= 0 && (string[i] == pat[j])) {
            --i;
            --j;
        }
        if (j < 0) {
            return &string[i+1];
        }

        ptrdiff_t shift = max(delta1[string[i]], delta2[j]);
        i += shift;
    }
    return NULL;
}

static bool
do_search (uint8_t *buf_start, size_t buf_len, uint8_t *pattern, size_t p_len)
{
  uint8_t *p=buf_start;
  uint8_t *q;
  size_t len_remain = buf_len;

  //printf ("DEBUG2: searching buf_start=%p (len: %ld) for pattern '%s' (len: %ld)\n", buf_start, buf_len, pattern, p_len);
  
  while (p < buf_start+buf_len) {
    q = p;
    //printf ("debug: calling: boyer_moore(%p, %ld, %p, %ld);\n", p, len_remain, pattern, p_len);
 
    p = boyer_moore(p, len_remain, pattern, p_len);
    if (!p) break;

    printf ("result: %p - i.e. offset=%ld: '%.*s', len_remain=%ld\n", p, p-buf_start, /* 4* */p_len, p, len_remain);    // FIXME: hardcoded print 4x the length of pattern
    
    p++;
    len_remain -= (p-q);
  }
    
  

      
  return true;
}



int
main (int argc, char **argv)
{
    bool ok;

    int          fd;
    off_t        offset, pa_offset;
    size_t       file_len, p_len;
    uint8_t      *addr, *p_buf;
    struct stat  sb;

    if (argc != 3) {
        fprintf(stderr, "%s pattern file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[2], O_RDONLY);
    if (fd == -1)
        handle_error("open");

    if (fstat(fd, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");

    p_buf = argv[1];
    p_len = strlen (p_buf);
    //printf ("DEBUG: searching for pattern: '%s' of len %ld\n", p_buf, p_len);

    posix_fadvise (fd, 0, 0, POSIX_FADV_SEQUENTIAL);

    offset = 0;
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
        /* offset for mmap() must be page aligned */

    if (offset >= sb.st_size) {
        fprintf(stderr, "offset is past end of file\n");
        exit(EXIT_FAILURE);
    }

    file_len = sb.st_size - offset;

    addr = mmap(NULL, file_len + offset - pa_offset, PROT_READ,
                MAP_PRIVATE, fd, pa_offset);
    if (addr == MAP_FAILED) handle_error("mmap");

	//printf ("DEBUG: mmaped %s from offset %ld file_len %ld\n", argv[2], offset, file_len);

    ok = do_search (addr, file_len, p_buf, p_len);
    if (!ok) {
        printf("error\n");
    }

    munmap(addr, file_len + offset - pa_offset);
    close(fd);

    exit(EXIT_SUCCESS);
}

