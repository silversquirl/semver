#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "semver.h"

static bool _stoi(unsigned int *ret, const char *s, const char *end) {
    // Parse an int until end or NUL is reached
    *ret = 0;
    while (end ? s < end : *s) { // Loop until `end` or, if `end` is NULL, until a NUL byte
        if ('0' <= *s && *s <= '9') {
            *ret *= 10;
            *ret += *s - '0';
        } else {
            return false;
        }
        s++;
    }
    return true;
}

static inline size_t _itos(char *ret, size_t len, unsigned int n) {
    return snprintf(ret, len, "%u", n);
}

bool semver_parse(struct semver_version *v, char *ver) {
    char *sep;

    // Not part of semver spec, but node-semver does it
    while (*ver == ' ' || *ver == '\t' || *ver == '\n') ver++;
    if (*ver == '=') ver++;
    if (*ver == 'v') ver++;

    if (!(sep = strchr(ver, '.'))) return false;
    if (!_stoi(&v->major, ver, sep)) return false;
    ver = sep+1;

    if (!(sep = strchr(ver, '.'))) return false;
    if (!_stoi(&v->minor, ver, sep)) return false;
    ver = sep+1;

    if (!(sep = strchr(ver, '-'))) sep = NULL;
    if (!_stoi(&v->patch, ver, sep)) return false;

    v->prerelease = NULL;
    v->build_meta = NULL;
    if (sep) {
        ver = sep+1;
        v->prerelease = ver;
        // TODO: validate the pre-release identifiers according to semver rules

        if ((sep = strchr(ver, '+'))) {
            *sep = '\0';
            ver = sep+1;
            v->build_meta = ver;
        }
    }

    // FIXME: really, we should check that there are no non-whitespace chars after this and error if there are
    if ((sep = strchr(ver, ' '))) {
        *sep = '\0';
        ver = sep+1;
    }

    return true;
}

bool semver_parse_pattern(struct semver_pattern *p, char *pat) {
    while (*pat == ' ' || *pat == '\t' || *pat == '\n') pat++;

    switch (*pat) {
    case '<':
        p->op = SVOP_LT;
        pat++;
        if (*pat == '=') {
            p->op = SVOP_LE;
            pat++;
        }
        break;
    case '=':
        p->op = SVOP_EQ;
        pat++;
        break;
    case '>':
        p->op = SVOP_GT;
        pat++;
        if (*pat == '=') {
            p->op = SVOP_GE;
            pat++;
        }
        break;
    default:
        p->op = SVOP_EQ;
        break;
    }

    return semver_parse(&p->version, pat);
}

// A quick 'n' dirty reimplementation of strncmp for my specific use case
// If performance becomes an issue this should be rewritten
static int _compare_strings(const char *a, const char *aend, const char *b, const char *bend) {
    while (*a == *b && a < aend && b < bend) a++, b++;
    if (a < aend) return -1;
    if (b < bend) return 1;
    if (*a < *b) return -1;
    if (*a > *b) return 1;
    return 0;
}

static int _compare_dotted(const char *a, const char *b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;

    const char *asep, *bsep;
    do {
        asep = strchr(a, '.');

        bsep = strchr(b, '.');

        unsigned int x, y;
        if (_stoi(&x, a, asep) && _stoi(&y, b, bsep)) {
            // Numeric comparison
            if (x > y) return 1;
            if (x < y) return -1;
        } else {
            // Lexical comparison
            x = _compare_strings(a, asep, b, bsep);
            if (x != 0) return x;
        }
        a = asep+1;
        b = bsep+1;
    } while (asep && bsep);

    if (asep) return 1;
    if (bsep) return -1;
    return 0;
}

int semver_compare(struct semver_version a, struct semver_version b) {
    if (a.major > b.major) return 1;
    if (a.major < b.major) return -1;

    if (a.minor > b.minor) return 1;
    if (a.minor < b.minor) return -1;

    if (a.patch > b.patch) return 1;
    if (a.patch < b.patch) return -1;

    return _compare_dotted(a.prerelease, b.prerelease);
    // TODO: build metadata
}

bool semver_match(struct semver_version v, struct semver_pattern pat) {
    int c = semver_compare(v, pat.version);
    switch (pat.op) {
    case SVOP_LE: return c <= 0;
    case SVOP_LT: return c < 0;
    case SVOP_EQ: return c == 0;
    case SVOP_GT: return c > 0;
    case SVOP_GE: return c >= 0;
    default: return false; // Invalid operator
    }
}

// Wrappers for strings
int semver_compare_s(const char *a, const char *b) {
    char ac[strlen(a)+1], bc[strlen(b)+1]; // Non-const copies of a and b
    strcpy(ac, a);
    strcpy(bc, b);

    struct semver_version av, bv; // Parsed versions of ac and bc
    if (!semver_parse(&av, ac)) return 2;
    if (!semver_parse(&bv, bc)) return 2;
    return semver_compare(av, bv);
}

int semver_match_s(const char *version, const char *pattern) {
    char vc[strlen(version)+1], pc[strlen(pattern)+1]; // Non-const copies of version and pattern
    strcpy(vc, version);
    strcpy(pc, pattern);

    struct semver_version v;
    struct semver_pattern p;
    if (!semver_parse(&v, vc)) return -1;
    if (!semver_parse_pattern(&p, pc)) return -1;
    return semver_match(v, p);
}

char *semver_format(struct semver_version v) {
    size_t space = 255; // Will we *really* need any more than this? Probably not
    char *ver = malloc(space), *p = ver;

    size_t i;
    i = _itos(p, space, v.major);
    p += i;
    space -= i;
    if (!space) goto toolong; // Still gotta bounds check, juuuuuuust in case
    p++[0] = '.';
    space--;

    i = _itos(p, space, v.minor);
    p += i;
    space -= i;
    if (!space) goto toolong;
    p++[0] = '.';
    space--;

    i = _itos(p, space, v.patch);
    p += i;
    space -= i;
    if (!space) goto toolong;

    if (v.prerelease) {
        i = strlen(v.prerelease) + 1;
        if (i > space) goto toolong;
        p++[0] = '-';
        strcpy(p, v.prerelease);
        p += i;
        space += i;
    }

    // TODO: build metadata
    
    return ver;

toolong:
    free(ver);
    return NULL;
}
