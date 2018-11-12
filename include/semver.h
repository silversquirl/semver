#pragma once

#include <stdbool.h>

struct semver_version {
    unsigned int major, minor, patch;
    const char *prerelease;
    const char *build_meta; // TODO: currently stored as part of `prerelease`
};

enum semver_pattern_operator {
    SVOP_LE = -2, // <=
    SVOP_LT = -1, // <
    SVOP_EQ = 0,  // ==
    SVOP_GT = 1,  // >
    SVOP_GE = 2,  // >=
};

struct semver_pattern {
    enum semver_pattern_operator op;
    struct semver_version version;
};

// These functions return true on success, false on error and may modify the input string
bool semver_parse(struct semver_version *v, char *version);
bool semver_parse_pattern(struct semver_pattern *p, char *pattern);

int semver_compare(struct semver_version a, struct semver_version b); // Returns {-1,0,1}
bool semver_match(struct semver_version version, struct semver_pattern pattern);

// Wrappers for the above functions that parse strings first
int semver_compare_s(const char *a, const char *b); // Returns 2 on error, {-1,0,1} on success
int semver_match_s(const char *version, const char *pattern); // Returns -1 on error, {0,1} on success

// Primarily for testing purposes. The resulting string must be freed by the caller with free()
char *semver_format(struct semver_version version);
