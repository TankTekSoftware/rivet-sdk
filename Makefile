# Usage: make [build|configure|clean|rebuild] [PRESET=<preset>]

# Without a preset, build the release preset for the computer make runs on.
ifeq ($(OS),Windows_NT)
    PRESET ?= windows-release
else ifeq ($(shell uname -s),Darwin)
    PRESET ?= macos-release
else
    PRESET ?= linux-release
endif

.DEFAULT_GOAL := build
.PHONY: build configure clean rebuild

build: configure
	cmake --build --preset=$(PRESET)

configure:
	cmake --preset=$(PRESET)

clean:
	cmake --build --preset=$(PRESET) --target clean

rebuild:
	$(MAKE) clean
	$(MAKE) build

.DEFAULT:
	@echo "Unknown target: $@"
	@echo "Usage: make [build|configure|clean|rebuild] [PRESET=<preset>]"
	@exit 1
