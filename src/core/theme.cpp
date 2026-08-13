#include "theme.hpp"
#include "terminal.hpp"
#include <iostream>
#include <algorithm>

namespace theme {
    namespace col {
        const std::string reset = "\033[0m";
        const std::string bold  = "\033[1m";
        const std::string dim   = "\033[2m";
        const std::string green = "\033[38;2;140;220;160m";

        std::string fg(int r, int g, int b) {
            return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
        }
        
        std::string bg(int r, int g, int b) {
            return "\033[48;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
        }
    }

    namespace msg {
        void err(const std::string& text) {
            std::cerr << "    " << col::fg(230, 100, 100) << "- " << col::reset << text << "\n";
        }
        void info(const std::string& text) {
            std::cout << "    " << col::fg(120, 200, 220) << "* " << col::reset << text << "\n";
        }
        void success(const std::string& text) {
            std::cout << "    " << col::fg(140, 220, 160) << "+ " << col::reset << text << "\n";
        }
    }

    namespace ui {
        // Utilizing a standard braille spinner to replace the corrupted original source string
        const std::vector<std::string> spinner = {
            "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" 
        };
        
        std::string repeat_glyph(const std::string& glyph, int n) {
            std::string out;
            out.reserve(glyph.size() * static_cast<size_t>(std::max(0, n)));
            for (int i = 0; i < n; ++i) out += glyph;
            return out;
        }

        std::string prompt_box(const std::string& hint) {
            int width = std::min(term::width() - 2, 78);
            if (width < 24) width = 24;
            
            int inner = width - 2;
            const std::string horiz = "\xE2\x94\x80";
            const std::string tl    = "\xE2\x95\xAD";
            const std::string tr    = "\xE2\x95\xAE";
            const std::string bl    = "\xE2\x95\xB0";
            const std::string br    = "\xE2\x95\xAF";
            const std::string vert  = "\xE2\x94\x82";
            
            std::string border = repeat_glyph(horiz, inner);
            std::string cyan = col::fg(120, 200, 220);
            
            int prompt_width = 3; 
            int max_input_width = inner - prompt_width;
            
            std::cout << "  " << cyan << tl << border << tr << col::reset << "\n";
            std::cout << "  " << cyan << vert << col::reset << " "
                      << col::fg(236, 72, 153) << "> " << col::reset
                      << std::string(max_input_width, ' ')
                      << cyan << vert << col::reset << "\n";
            std::cout << "  " << cyan << bl << border << br << col::reset << "\n";
            
            int lines_to_move_up = 2;
            if (!hint.empty()) {
                std::cout << "  " << col::dim << hint << col::reset << "\n";
                lines_to_move_up = 3;
            }
            
            std::cout << "\x1b[" << lines_to_move_up << "A\r\x1b[6C" << std::flush;
            
            std::string line;
            if (!std::getline(std::cin, line)) {
                std::cout << "\n\n";
                std::exit(0);
            }
            
            std::string visible = line;
            if ((int)visible.length() > max_input_width) {
                visible = (max_input_width > 3)
                     ? visible.substr(0, max_input_width - 3) + "..."
                     : visible.substr(0, max_input_width);
            }
            
            int pad = std::max(0, max_input_width - (int)visible.length());
            std::cout << "\x1b[1A\r"
                      << "  " << cyan << vert << col::reset << " "
                      << col::fg(236, 72, 153) << "> " << col::reset
                      << visible << std::string(pad, ' ')
                      << cyan << vert << col::reset << "\n";
                      
            if (lines_to_move_up - 1 > 0) {
                std::cout << "\x1b[" << (lines_to_move_up - 1) << "B";
            }
            
            return line;
        }
    }
}