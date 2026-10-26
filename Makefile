# Usage: make [build|configure|clean|rebuild] [PRESET=<preset>]

PRESET ?= desktop-release

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
