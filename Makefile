.DEFAULT_GOAL := help

BUILD_DIR ?= build
BUILD_TYPE ?= Debug
JOBS ?= 2
INSTALL_PREFIX ?= /usr/local

.PHONY: help configure build test sanitizers examples run-examples format coverage install clean distclean

help: ## Show the available commands
	@awk 'BEGIN {FS = ":.*## "; print "Usage: make <command>\n"} /^[a-zA-Z_-]+:.*## / {printf "  %-14s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

configure: ## Configure a development build
	cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DYAMLPARSER_BUILD_TESTS=ON

build: configure ## Build the library
	cmake --build $(BUILD_DIR) --target yamlparser --parallel $(JOBS)

test: configure ## Build and run all tests
	cmake --build $(BUILD_DIR) --target run_tests --parallel $(JOBS)

sanitizers: ## Run tests with address and undefined-behavior sanitizers
	cmake -S . -B build-sanitizers \
		-DCMAKE_BUILD_TYPE=Debug \
		-DYAMLPARSER_BUILD_TESTS=ON \
		-DYAMLPARSER_ENABLE_SANITIZERS=ON
	cmake --build build-sanitizers --target run_tests --parallel $(JOBS)

examples: ## Build all usage examples
	cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DYAMLPARSER_BUILD_TESTS=OFF \
		-DYAMLPARSER_BUILD_EXAMPLES=ON
	cmake --build $(BUILD_DIR) --target examples --parallel $(JOBS)

run-examples: examples ## Run all usage examples
	cmake --build $(BUILD_DIR) --target run_examples --parallel 1

format: configure ## Format source, tests, and examples
	cmake --build $(BUILD_DIR) --target format

coverage: ## Run tests and generate HTML and console coverage reports
	cmake -S . -B build-coverage \
		-DCMAKE_BUILD_TYPE=Debug \
		-DYAMLPARSER_BUILD_TESTS=ON \
		-DYAMLPARSER_ENABLE_COVERAGE=ON
	cmake --build build-coverage --target gcovr_console --parallel $(JOBS)
	cmake --build build-coverage --target gcovr_html --parallel $(JOBS)

install: ## Install the library and CMake package
	cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DYAMLPARSER_BUILD_TESTS=OFF
	cmake --build $(BUILD_DIR) --target yamlparser --parallel $(JOBS)
	cmake --install $(BUILD_DIR) --prefix $(INSTALL_PREFIX)

clean: configure ## Remove compiled files but keep CMake configuration
	cmake --build $(BUILD_DIR) --target clean

distclean: ## Remove all generated build directories
	cmake -E remove_directory $(BUILD_DIR)
	cmake -E remove_directory build-coverage
	cmake -E remove_directory build-sanitizers
