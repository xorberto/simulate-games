#include "terminal.hpp"
#include <iostream>
#include <limits>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>
#endif

namespace term {
    void setup() {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            }
        }
#endif
    }

    int width() {
        int w = 80;
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        }
#else
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
            w = ws.ws_col;
        }
#endif
        return w;
    }

    void clear_screen() {
#ifdef _WIN32
        system("cls");
#else
        std::cout << "\033[2J\033[H";
#endif
    }

    void wait_for_exit() {
        std::cout << "\nPress Enter to exit...";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }

    void handle_exit(int) {
        std::cout << "\r\033[K\033[?25h" << std::flush; // Clear line and show cursor
        std::exit(0);
    }
}