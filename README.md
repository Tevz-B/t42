# t42 - Tevz's Terminal Typing Test 2

## Dependencies
- ncurses

## Build

`cd` into repository root dir and run `./build.sh`.


## Run

The binary needs a list of words. By default it looks for `words.txt` file, which you can provide with `-f` flag.
run `./build/t42`


## Options

  - `-h` : help
  - `-s` : size - set window size (default: `-s60x5`)
  - `-f` : words file where words will be split by whitespace (default: `words.txt`)
  - `-t` : time limit in seconds (default: `15`)
  - `-r` : random seed


