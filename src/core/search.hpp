#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace search {
    struct Match {
        size_t index;
        int score;
    };

    int match_score(const std::string& query, const std::string& name);
    std::vector<Match> find_matches(const std::string& query, const nlohmann::json& games);
    long try_parse_index(const std::string& s, size_t game_count);
}