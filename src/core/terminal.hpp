#pragma once

namespace term {
    void setup();
    int width();
    void clear_screen();
    void wait_for_exit();
    void handle_exit(int);
}