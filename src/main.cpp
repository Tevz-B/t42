#include <fcntl.h>
#include <ncurses.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "log.h"


/**
 * 1. How to determine when to end?
 * 2. How to move word to new line?
 * 3. How to time?
 */

/*********************************************************/
/*             Globals                                   */
/*********************************************************/

static int g_screenWidth;
static int g_screeHeight;
static int g_inputLineIndex;

using Location = std::pair<int, int>;
using Words = std::vector<std::pair<Location, std::string>>;
Words g_currentWords;

// Color pairs
#define CP_RED 1
#define CP_BLUE 2
#define CP_GREEN 3

/*********************************************************/
/*             Structs                                   */
/*********************************************************/

/*********************************************************/
/*             Utils                                     */
/*********************************************************/

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> g_words;

std::string wordFromFile() { return g_words[rand() % g_words.size()]; }

bool readWordsFromFile(const char* filename) {
    std::ifstream in(filename, std::ios_base::in);
    if (!in.is_open()) {
        return false;
    }

    g_words.clear();

    std::string word;
    // Extract words separated by whitespace
    while (in >> word) {
        g_words.push_back(word);
    }

    return true;
}

void setScreenDimensions(int width, int height) {
    g_screenWidth = width;
    g_screeHeight = height;
    g_inputLineIndex = g_screeHeight;
}

// std::string newGeneratedWord() {
//     const int word_count = 2;
//     const std::string words[word_count]{"j", "k", /* "up", "down" */};
//     const int word_idx = rand() % word_count;
//     auto rnd_wrd_suff = words[word_idx];
//     auto rnd_wrd_pre = std::to_string(rand() % 1000);
//     return rnd_wrd_pre + rnd_wrd_suff;
// }

/*********************************************************/
/*             Main                                      */
/*********************************************************/

int init() {
    // Init debug log
    initLog();

    if (!readWordsFromFile("words.txt")) {
        printf("Failed to read words file\n");
        return 1;
    }

    // init terminal
    initscr();
    if (!has_colors()) {
        endwin();
        printf("Your terminal does not support color\n");
        return 1;
    }

    start_color();                    // Start color functionality
    use_default_colors();             // Use default terminal colors
    init_pair(CP_RED, COLOR_RED, -1); // -1 = transparent background
    init_pair(CP_BLUE, COLOR_BLUE, -1);
    init_pair(CP_GREEN, COLOR_GREEN, -1);
    // Initialize screen
    cbreak();             // Line buffering disabled
    keypad(stdscr, TRUE); // We get F1, F2, etc...
    noecho();             // Don't echo() while we do getch

    return 0;
}

void printUsage(FILE* stream, const char* program) {
    // clang-format off
    fprintf(stream,
            "t4 - Tevz's Terminal Typing Test\n"
            "\n"
            "usage: %s [-h | [-s size] [-f words_file] [-r random_seed]]\n"
            "\n"
            "options:\n"
            "    -h         show this message and exit\n"
            "    -s         size of window in characters (not pixels): {columns}x{lines}\n"
            "    -f         path to words file (default:words.txt)\n"
            "    -r         random seed (int)\n",
            program);
    // clang-format on
}

int main(int argc, char** argv) {
    std::string size_str;
    std::string filename = "words.txt";
    int rnd_seed = 42;
    int width = 60;
    int height = 5;

    for (int opt; (opt = getopt(argc, argv, "hs:f:r:")) != -1;) {
        switch (opt) {
            case 'h':
                return printUsage(stdout, argv[0]), 0;
            case 's': {
                size_str = optarg;
                auto split_pos = size_str.find('x');
                width = atoi(size_str.substr(0, split_pos).c_str());
                height = atoi(size_str.substr(split_pos + 1).c_str());
            }
                continue;
            case 'f':
                filename = optarg;
                continue;
            case 'r':
                rnd_seed = atoi(optarg);
                continue;
        }
    }
    if (filename.empty()) {
        printf("Words filename is empty");
        return 1;
    }
    if (width < 1 || height < 1) {
        printf("Window dimensions must be at least 1 x 1");
        return 1;
    }
    std::srand(rnd_seed);
    setScreenDimensions(width, height);

    if (init()) {
        printf("Failed to init\n");
        return 1;
    }

    // Behavior:
    //   - letter starts word type
    //   - space ends word type, any more spaces do nothing, wait for next letter
    //   - every wrong letter is colored red / yellow , every letter after end of word as well

    clear(); // Clear the screen
    move(0, 0); // Move cursor
    // Print words
    int x = 0;
    int y = 0;
    // for (auto& w : g_words) {
    for (int i = 0; i < 1000 && i < g_words.size(); ++i) {
        // auto& w = g_words[i];
        auto& w = g_words[rand() % g_words.size()];
        if ((w.size() + 1) > g_screenWidth) {
            printf("Word is longer thatn screen width");
            return 1;
        }
        x += w.size() + 1;
        if (x > g_screenWidth) {
            x = 0;
            printw("\n");
            if (++y >= g_screeHeight) {
                break;
            }
        }
        printw("%s ", w.data());
    }

    // attron(COLOR_PAIR(CP_GREEN));
    // printw("%s", "some_word");
    // attroff(COLOR_PAIR(CP_GREEN));

    move(0, 0); // Move cursor back to beginning

    time_t limit = 10000;
    time_t start = clock();

    while(true) {
        time_t diff = clock() - start;

        int ch = getch(); // Get user input
        LOG("keycode = %d, time: %zu", ch, diff);
        if( diff > limit ) {
            // TODO: dont exit but print finished and stats
            goto exitLoop;
        }
        switch (ch) {
            case 4: // Control-D : quit
                goto exitLoop;

            // Whitespace (except space) : ignore
            case '\n':
            case '\t':
            case '\r':
                break;

            // Backspace : delete char
            case KEY_BACKSPACE:
            case 127:
                getyx(stdscr, y, x);
                chgat(x, A_NORMAL, 0, nullptr);
                move(y, --x);
                break;


            // C-W, C-BS : delete word
            case 27:   // Ctrl-BS
            case 23: { // Ctrl-W : delete word
                getyx(stdscr, y, x);

                if (x==0) break;

                move(y, --x);
                chgat(x, A_NORMAL, 0, nullptr);

                for (--x; x >= 0; --x) {
                    move(y, x);
                    chgat(x, A_NORMAL, 0, nullptr);
                    if ((inch() & A_CHARTEXT) == ' ') {
                        move(y, ++x);
                        break;
                    }
                }
                break;
            }

            // Space : move to next word
            case ' ':
                if (ch == (inch() & A_CHARTEXT)) {
                    addch(ch | COLOR_PAIR(CP_GREEN));
                    if ((inch() & A_CHARTEXT) == ' ') {
                        getyx(stdscr, y, x);
                        move(++y, 0);
                    }
                }
                else { // Move until past ' '
                    getyx(stdscr, y, x);
                    while((inch() & A_CHARTEXT) != ' ') {
                        move(y, ++x);
                    }
                    move(y, ++x);
                }
                break;
            // Letters (and other) : check correctness
            default:
                // LOG( "ch=%d, ch2=%d, inch=%d, inch2=%d", ch, ch, inch(), inch() & A_CHARTEXT );
                int inchar = (inch() & A_CHARTEXT);
                if (ch == inchar) {
                    addch(ch | COLOR_PAIR(CP_GREEN));
                }
                // If positioned on wrong char - space: insert red chars
                else if(inchar == ' ') {
                    insch(ch | COLOR_PAIR(CP_RED));
                    addch(ch | COLOR_PAIR(CP_RED));
                }
                // If positioned on wrong char and not space
                else {
                    addch(inchar | COLOR_PAIR(CP_RED));
                }
                break;
        }
    }
exitLoop:

    // Clean up
    endwin();

    return 0;
}
