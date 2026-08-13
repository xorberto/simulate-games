#include "search.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>

namespace search {
    namespace {
        std::string to_lower(const std::string& s) {
            std::string out = s;
            std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return std::tolower(c); });
            return out;
        }

        std::vector<std::string> split_tokens(const std::string& s) {
            std::vector<std::string> tokens;
            std::istringstream iss(s);
            std::string tok;
            while (iss >> tok) tokens.push_back(tok);
            return tokens;
        }
    }

    int match_score(const std::string& query, const std::string& name) {
        std::string q = to_lower(query);
        std::string n = to_lower(name);

        if (q.empty()) return -1;
        if (q == n) return 10000; // Exact match
        
        std::vector<std::string> tokens = split_tokens(q);
        if (!tokens.empty()) {
            bool all_found = true;
            int score = 0;

            for (auto& t : tokens) {
                auto pos = n.find(t);
                if (pos == std::string::npos) { all_found = false; break; }
                
                // Earlier / word-start matches score a little higher
                bool at_word_start = (pos == 0 || n[pos - 1] == ' ');
                score += (at_word_start ? 15 : 8) + static_cast<int>(t.size());
            }

            if (all_found) return 5000 + score;
        }
        
        if (n.find(q) != std::string::npos) return 2000;
        
        /* Partial match
         *
         * size_t ni = 0;
         * int matched = 0, wanted = 0;
         * for (char c : q) {
         *    if (c == ' ') continue;
         *    wanted++;
         *    while (ni < n.size() && n[ni] != c) ni++;
         *    if (ni < n.size()) { matched++; ni++; }
         * }
         * if (wanted > 0 && matched == wanted) return 500;
         */
        
        return -1;
    }

    std::vector<Match> find_matches(const std::string& query, const nlohmann::json& games) {
        std::vector<Match> matches;

        for (size_t i = 0; i < games.size(); ++i) {
            std::string name = games[i]["name"].get<std::string>();
            int score = match_score(query, name);
            if (score >= 0) matches.push_back({i, score});
        }

        std::sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) { return a.score > b.score; });

        return matches;
    }

    long try_parse_index(const std::string& s, size_t game_count) {
        if (s.empty()) return -1;

        for (char c : s) {
            if (!std::isdigit(static_cast<unsigned char>(c))) return -1;
        }

        try {
            long v = std::stol(s);
            if (v >= 1 && static_cast<size_t>(v) <= game_count) return v - 1;
        } catch (...) {}
        
        return -1;
    }
}