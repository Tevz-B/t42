#include <fcntl.h>
#include <ncurses.h>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/*********************************************************/
/*             Globals                                   */
/*********************************************************/

static int screen_width;
static int screen_height;
static int input_line_index;

static const char blank_char = '-';
static const std::string input_line_prefix = "$ :";

using Location = std::pair<int, int>;
using Words = std::vector<std::pair<Location, std::string>>;
Words current_words;

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
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> words;

std::string wordFromFile() { return words[rand() % words.size()]; }

bool readWordsFromFile(const char* filename) {
    std::ifstream in(filename, std::ios_base::in);
    if (!in.is_open()) {
        return false;
    }

    std::string file_contents;

    std::string line;
    while(getline(in, line))
        file_contents.append(line);

    words.clear();
    std::istringstream iss(file_contents);
    std::string word;

    // Extract words separated by whitespace
    while (iss >> word) {
        words.push_back(word);
    }

    return true;
}

void setScreenDimensions(int width, int height) {
    screen_width = width;
    screen_height = height;
    input_line_index = screen_height;
}

std::string newWordOld() {
    const std::string words[]{"hello", "something", "hi", "goat", "cpp"};
    const int word_count = 5;
    const int word_idx = rand() % word_count;
    return words[word_idx];
}

std::string newGeneratedWord() {
    const int word_count = 2;
    const std::string words[word_count]{"j", "k", /* "up", "down" */};
    const int word_idx = rand() % word_count;
    auto rnd_wrd_suff = words[word_idx];
    auto rnd_wrd_pre = std::to_string(rand() % 1000);
    return rnd_wrd_pre + rnd_wrd_suff;
}

std::pair<int, int> newWordLocation(int word_length) {
    int x = rand() % (screen_width - word_length);
    int y = rand() % (screen_height - 2); // exclude last line (input), and first line (manual)
    ++y; // exclude first line
    return {y, x};
}

auto& addNewWord() { // auto w = new_generated_word();
    auto w = wordFromFile();
    auto loc = newWordLocation((int)w.length());
    current_words.push_back({loc, w});
    return current_words.back();
}

void printEmpty() {
    printw("Press Control-D to exit :^D\n");           // First line
    for (uint16_t i = 0; i < screen_height - 1; i++) { // The rest
        printw("%s\n", std::string(screen_width, blank_char).c_str());
    }
}

void printWords(std::string& input_line, int& y, int& x) {
    std::vector<std::string> rm_words;
    for (const auto& [loc, w] : current_words) {
        std::regex re(input_line);
        std::smatch m;
        if (std::regex_search(w, m, re)) {
            // full match
            if (m.prefix().length() == 0 && m.suffix().length() == 0) {
                y = input_line_index;
                x = input_line_prefix.length();
                input_line.clear();
                rm_words.push_back(w);
                continue;
            }

            // partial match
            int x, y;
            getyx(stdscr, y, x);
            move(loc.first, loc.second);
            printw("%s", m.prefix().str().c_str());
            attron(COLOR_PAIR(CP_RED));
            printw("%s", m[0].str().c_str());
            attroff(COLOR_PAIR(CP_RED));
            printw("%s", m.suffix().str().c_str());
            move(x, y);
            continue;
        }
        mvprintw(loc.first, loc.second, "%s", w.c_str());
    }

    for (const auto& w : rm_words) {
        // remove typed word
        current_words.erase(
            std::remove_if(current_words.begin(), current_words.end(),
                           [&w](const auto& i) { return i.second == w; }),
            current_words.end());
        // current_words.pop_back();

        auto& [loc, word] = addNewWord();
        // Print new word
        mvprintw(loc.first, loc.second, "%s", word.c_str());
    }
}

/*********************************************************/
/*             Main                                      */
/*********************************************************/

int init() {
    // Init debug log

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
            "    -s         size of window in characters (not pixels): {lines}x{columns}\n"
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
    int height = 40;

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
    if (width < 10 || height < 5) {
        printf("Window dimensions must be at least 10 x 5");
        return 1;
    }
    std::srand(rnd_seed);
    setScreenDimensions(width, height);

    if (init()) {
        printf("Failed to init\n");
        return 1;
    }

    while (true) {
        clear(); // Clear the screen
        move(0, 0); // Move cursor
        attron(COLOR_PAIR(CP_GREEN));
        printw("%s", "some_word");
        attroff(COLOR_PAIR(CP_GREEN));

        // Handle Input
        int ch = getch(); // Get user input
        switch (ch) {
                // backspace
            case KEY_BACKSPACE:
            case 127:
                break;
            // whitespace - ignore
            case '\n':
            case '\t':
            case '\r':
            case ' ':
                break;
            // quit (Control-D)
            case 4:
                goto exitLoop;
            case 23:
                break;
            // other chars
            default:
                // insert into input line
                break;
        }
    }
exitLoop:

    // Clean up
    endwin();

    return 0;
}
