#include "core/terminal.hpp"
#include "core/theme.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

int main() {
    std::signal(SIGINT, term::handle_exit);
    std::signal(SIGTERM, term::handle_exit);

    term::setup();

    std::cout << "\033[?25l";     // Hide cursor
    std::cout << "\033[2J\033[H"; // Clear screen and move to top-left
    
    int frame = 0;
    while (true) {
        std::cout << "\r    "
                  << theme::col::green << theme::ui::spinner[frame] << theme::col::reset
                  << " Working on your quest..." << std::flush;
        frame = (frame + 1) % theme::ui::spinner.size();
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }
    
    std::cout << "\033[?25h"; 
    return 0;
}