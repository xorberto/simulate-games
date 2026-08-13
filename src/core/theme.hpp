#pragma once
#include <string>
#include <vector>

namespace theme {
    namespace col {
        extern const std::string reset;
        extern const std::string bold;
        extern const std::string dim;
        extern const std::string green;
        
        std::string fg(int r, int g, int b);
        std::string bg(int r, int g, int b);
    }

    namespace msg {
        void err(const std::string& text);
        void info(const std::string& text);
        void success(const std::string& text);
    }

    namespace ui {
        extern const std::vector<std::string> spinner;
        std::string repeat_glyph(const std::string& glyph, int n);
        std::string prompt_box(const std::string& hint = "");
    }
}