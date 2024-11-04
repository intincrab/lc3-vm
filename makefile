CC = gcc
OUT = out
ASSETS = assets
LC3_VM = $(OUT)/lc3

all: build

build:
	mkdir -p $(OUT)
	$(CC) src/main.c -o $(LC3_VM)

run-rogue: build
	$(LC3_VM) $(ASSETS)/rogue.obj

run-2048: build
	$(LC3_VM) $(ASSETS)/2048.obj

.PHONY: clean
clean:
	rm -rf $(OUT)

.PHONY: help
help:
	@echo "Available targets:"
	@echo "  run-rogue - Run the rogue.obj asset"
	@echo "  run-2048 - Run the 2048.obj asset"
	@echo "  clean - Remove the compiled output"

.DEFAULT_GOAL := help