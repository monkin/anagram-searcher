#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef union {
    char chars[32];
    __int128_t blocks[2];
} word_t;

int position = 0;
char output[4 * 1024] = { 0 };

const char *skip_word(const char *p, const char *end) {
    for (; p < end; p++) {
        if (*p == '\n' || *p == '\r') {
            return p;
        }
    }
    return end;
}

const char *skip_spaces(const char *p, const char *end) {
    for (; p < end; p++) {
        if (*p != '\n' && *p != '\r') {
            return p;
        }
    }
    return end;
}

long long get_time() {
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * ((long long) 1e6) + currentTime.tv_usec;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Invalid argumets count.\n");
        printf("Usage: executable path/to/dictionary.txt word\n");
        return -1;
    }

    long long startTime = get_time();

    const char *arg1 = argv[1];
    const char *arg2 = argv[2];

    int sum = 0;
    int length = 0;
    char max = 0;
    word_t word = { 0 };
    
    for (length = 0; length < 32 && arg2[length]; length++) {
        char c = arg2[length];
        sum += c;
        word.chars[length] = c;
        if (c > max) {
            max = c;
        }
    }

    const int file = open(arg1, O_RDONLY);
    if (file < 0) {
        printf("Failed to open dictionary: '%s'", arg1);
        return -1;
    }

    struct stat stats;
    fstat(file, &stats);

    const size_t size = stats.st_size;

    const char *p1 = mmap(0, size, PROT_READ, MAP_PRIVATE, file, 0);
    const char *p2 = p1 + size;

    for (const char *p = skip_spaces(p1, p2), *e = skip_word(p, p2); p < p2; p = skip_spaces(e, p2), e = skip_word(p, p2)) {
        // because words in file are ordered
        if (p[0] > max) {
            break;
        }

        if ((e - p) == length) {
            int wsum = 0;
            for (const char *i = p; i < e; i++) {
                wsum += *i;
            }

            if (wsum == sum) {
                int different = 0;
                word_t w = word;
                for (const char *i = p; i < e; i++) {
                    int replaced = 0;
                    for (int j = 0; j < length; j++) {
                        if (w.chars[j] == *i) {
                            if (j != i - p) {
                                different = 1;
                            }
                            replaced = 1;
                            w.chars[j] = 0;
                            break;
                        }
                    }
                    if (!replaced) {
                        break;
                    }
                }

                if (different && w.blocks[0] == 0 && w.blocks[1] == 0) {
                    output[position++] = ',';

                    for (const char *c = p; c < e; c++) {
                        output[position++] = *c;
                    }
                }
            }
        }
    }

    long long stopTime = get_time();

    printf("%lld%s\n", stopTime - startTime, output);

    return 0;
}