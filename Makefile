.POSIX:
.PHONY: all clean build-dirs

CFLAGS := -Wall -Werror -Iinclude -fPIC $(DEBUG)
LDFLAGS :=
SHARED_LDFLAGS := $(LDFLAGS)

all: build/bin/semver build/lib/libsemver.so build/lib/libsemver.a
clean:
	rm -rf build

build-dirs:
	@mkdir -p build/bin build/lib

build/bin/semver: build/bin/semver.o build/lib/libsemver.a
	$(CC) -o $@ $^ $(LDFLAGS)

build/lib/libsemver.so: build/lib/semver.o
	$(CC) -shared -o $@ $^ $(SHARED_LDFLAGS)

build/lib/libsemver.a: build/lib/semver.o
	$(AR) rcs $@ $^

build/%.o: src/%.c include/semver.h | build-dirs
	$(CC) $(CFLAGS) -c -o $@ $<
