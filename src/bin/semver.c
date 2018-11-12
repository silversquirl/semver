/* A ludicrously simple CLI for libsemver
 * Usage: semver <VERSION> <PATTERN>
 * Returns 0 if VERSION matches PATTERN, 1 if it does not and 2 if a parse error occurred
 */

#include <stdlib.h>
#include <stdio.h>
#include <semver.h>

int usage(const char *progname) {
    fprintf(stderr, "Usage: %s <VERSION> <PATTERN>\n", progname);
    fprintf(stderr, "Returns 0 if VERSION matches PATTERN or 1 otherwise\n");
    fprintf(stderr, "In the event of a parse error, 2 is returned\n");
    return 3;
}

int main(int argc, char *argv[]) {
    if (argc < 3) return usage(argv[0]);

    int ret = semver_match_s(argv[1], argv[2]);
    if (ret == 0) return 1;
    if (ret == 1) return 0;

    fprintf(stderr, "%s: Error parsing version or pattern.\n", argv[0]);
    return 2;
}
