#include "core/terminal.hpp"
#include "core/theme.hpp"
#include "core/search.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/ioctl.h>

// Define the external symbols created by the Linux Assembly file
extern "C" const unsigned char payload_data[];
extern "C" const size_t payload_size;
#endif

using json = nlohmann::json;
namespace fs = std::filesystem;

void print_banner() {
    using namespace theme;
    std::cout << "  " << col::bold << col::fg(230, 170, 220) << "Simulate Games" << col::reset << "\n\n";
    std::cout << "  " << col::dim << "Tips for getting started:" << col::reset << "\n";
    std::cout << "  " << col::fg(150, 190, 245) << "1." << col::reset << " Type a game name (or part of one) to search, e.g. \"Roblox\".\n";
    std::cout << "  " << col::fg(150, 190, 245) << "2." << col::reset << " A list number (e.g. \"38\") still works directly.\n";
    std::cout << "  " << col::fg(150, 190, 245) << "3." << col::reset << " If several games match, you'll be asked to pick one.\n";
    std::cout << "  " << col::fg(150, 190, 245) << "4." << col::reset << " Press Ctrl+C at any time to exit.\n\n";
}

size_t select_game(const json& games) {
    using namespace theme;
    auto confirm_selection = [&](size_t idx) {
        std::cout << "\n    " << col::green << "Selected: " << col::reset << games[idx]["name"].get<std::string>() << "\n";
        return idx;
    };
    
    while (true) {
        std::string query = theme::ui::prompt_box();
        
        if (query.empty()) {
            std::cout << "\n    " << col::fg(230, 100, 100) << "Empty query. Try again." << col::reset << "\n\n";
            continue;
        }
        
        long direct = search::try_parse_index(query, games.size());
        if (direct >= 0) {
            return confirm_selection(static_cast<size_t>(direct));
        }
        
        std::vector<search::Match> matches = search::find_matches(query, games);
        if (matches.empty()) {
            std::cout << "\n    " << col::fg(230, 100, 100) << "No games matched \"" << query << "\". Try again." << col::reset << "\n\n";
            continue;
        }
        
        if (matches.size() == 1 || matches[0].score == 10000) {
            return confirm_selection(matches[0].index);
        }
        
        std::cout << "\n  " << col::fg(230, 190, 90) << "Multiple games matched \"" << query << "\":" << col::reset << "\n";
        size_t shown = std::min<size_t>(matches.size(), 9);
        for (size_t i = 0; i < shown; ++i) {
            std::cout << "    " << col::fg(150, 190, 245) << (i + 1) << ")" << col::reset << " " << games[matches[i].index]["name"].get<std::string>() << "\n";
        }
        
        std::cout << "\n  " << col::dim << "Pick a number above, or type a more specific search." << col::reset << "\n\n";

        std::string sub = theme::ui::prompt_box();
        long sub_idx = search::try_parse_index(sub, shown);
        if (sub_idx >= 0) {
            return confirm_selection(matches[static_cast<size_t>(sub_idx)].index);
        }
        
        std::vector<search::Match> retry = search::find_matches(sub, games);
        if (!retry.empty() && (retry.size() == 1 || retry[0].score == 10000)) {
            return confirm_selection(retry[0].index);
        }
        if (retry.empty()) {
            std::cout << "\n    " << col::fg(230, 100, 100) << "No games matched \"" << sub << "\". Try again." << col::reset << "\n\n";
            continue;
        }
        std::cout << "    " << col::fg(230, 190, 90) << "Still multiple matches, let's narrow it down again." << col::reset << "\n\n";
    }
}

bool launch_process(const fs::path& exe_path, const fs::path& working_dir) {
#ifdef _WIN32
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    std::wstring w_exe = exe_path.wstring();
    std::wstring w_dir = working_dir.wstring();
    
    if (CreateProcessW(w_exe.c_str(), NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, w_dir.c_str(), &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    } else {
        theme::msg::err("Failed to launch process. Windows Error: " + std::to_string(GetLastError()));
        return false;
    }
#else
    std::string cmd = "cd \"" + working_dir.string() + "\" && \"./" + exe_path.filename().string() + "\"";
    int result = system(cmd.c_str());
    if (result != 0) {
        theme::msg::err("Failed to execute shell command. Exit code: " + std::to_string(result));
        return false;
    }
    return true;
#endif
}

int main() {
    term::setup();
    term::clear_screen();
    print_banner();
    
    std::ifstream f("info.json");
    if (!f.is_open()) {
        theme::msg::err("info.json not found in the working directory.");
        term::wait_for_exit();
        return 1;
    }
    
    json data;
    try {
        f >> data;
    } catch (const json::parse_error& e) {
        theme::msg::err("Error parsing info.json: " + std::string(e.what()));
        term::wait_for_exit();
        return 1;
    }
    
    auto games = data["games"];
    std::cout << "  " << theme::col::bold << "Available games:" << theme::col::reset << "\n";
    for (size_t i = 0; i < games.size(); ++i) {
        std::cout << "    " << theme::col::fg(150, 190, 245) << (i + 1) << ")" << theme::col::reset << " " << games[i]["name"].get<std::string>() << "\n";
    }
    std::cout << "\n";
    
    size_t choice = select_game(games);
    auto selected = games[choice];
    std::string name = selected["name"];
    std::string relative_path = selected["path"];
    std::string executable_name = selected["executable"];
    
#ifndef _WIN32
    std::replace(relative_path.begin(), relative_path.end(), '\\', '/');
    std::replace(executable_name.begin(), executable_name.end(), '\\', '/');
#endif

    fs::path target_dir = fs::path("data") / relative_path;
    fs::path target_exe = target_dir / executable_name;
    
    if (!fs::exists(target_dir)) {
        theme::msg::info("Creating directory structure: " + relative_path);
        fs::create_directories(target_dir);
    }
    
    if (!fs::exists(target_exe)) {
        theme::msg::info("Deploying " + executable_name + "...");
        const char* data_ptr = nullptr;
        size_t data_size = 0;
        
#ifdef _WIN32
        HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(101), RT_RCDATA);
        if (!hRes) {
            theme::msg::err("Failed to locate embedded Windows payload.");
            term::wait_for_exit();
            return 1;
        }
        HGLOBAL hData = LoadResource(NULL, hRes);
        data_size = SizeofResource(NULL, hRes);
        data_ptr = static_cast<const char*>(LockResource(hData));
#else
        data_ptr = reinterpret_cast<const char*>(payload_data);
        data_size = payload_size;
#endif

        std::ofstream out(target_exe, std::ios::binary);
        if (!out) {
            theme::msg::err("Failed to write to " + target_exe.string());
            term::wait_for_exit();
            return 1;
        }
        
        out.write(data_ptr, data_size);
        out.close();
        
#ifndef _WIN32
        fs::permissions(target_exe, fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec, fs::perm_options::add);
#endif
    }
    
    theme::msg::success("Launching " + name + "...");
    if (!launch_process(target_exe, target_dir)) {
        term::wait_for_exit();
        return 1;
    }
    
    return 0;
}