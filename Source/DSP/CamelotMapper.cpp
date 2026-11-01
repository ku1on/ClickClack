#include "CamelotMapper.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <unordered_map>

namespace clickclack {

static std::string toUpper(std::string s) {
    for (auto& c : s) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return s;
}

static std::string toLower(std::string s) {
    for (auto& c : s) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return s;
}

std::string CamelotMapper::normalizeRootNote(const std::string& root) {
    std::string s = root;
    // Trim whitespace
    s.erase(0, s.find_first_not_of(" \t\n\r"));
    s.erase(s.find_last_not_of(" \t\n\r") + 1);

    if (s.empty()) return "";

    char first = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
    std::string result(1, first);

    if (s.length() > 1) {
        char accidental = s[1];
        if (accidental == '#' || accidental == 'b' || accidental == 'B') {
            result += (accidental == '#' ? '#' : 'b');
        }
    }

    // Standardize to common spellings
    if (result == "DB") return "Db";
    if (result == "EB") return "Eb";
    if (result == "GB") return "Gb";
    if (result == "AB") return "Ab";
    if (result == "BB") return "Bb";

    return result;
}

std::string CamelotMapper::normalizeScale(const std::string& scale) {
    std::string low = toLower(scale);
    if (low.find("min") != std::string::npos || low == "m") {
        return "minor";
    }
    return "major";
}

std::string CamelotMapper::toCamelot(const std::string& rootNote, const std::string& scale) {
    std::string root = normalizeRootNote(rootNote);
    std::string sc = normalizeScale(scale);

    if (root.empty()) return "--";

    bool isMinor = (sc == "minor");

    // Standard Camelot mappings:
    // Minor keys (A column):
    // 1A: Ab/G#m, 2A: Eb/D#m, 3A: Bb/A#m, 4A: Fm, 5A: Cm, 6A: Gm,
    // 7A: Dm, 8A: Am, 9A: Em, 10A: Bm, 11A: F#/Gbm, 12A: C#/Dbm
    static const std::unordered_map<std::string, std::string> minorMap = {
        {"Ab", "1A"}, {"G#", "1A"},
        {"Eb", "2A"}, {"D#", "2A"},
        {"Bb", "3A"}, {"A#", "3A"},
        {"F",  "4A"},
        {"C",  "5A"},
        {"G",  "6A"},
        {"D",  "7A"},
        {"A",  "8A"},
        {"E",  "9A"},
        {"B",  "10A"},
        {"F#", "11A"}, {"Gb", "11A"},
        {"C#", "12A"}, {"Db", "12A"}
    };

    // Major keys (B column):
    // 1B: B, 2B: F#/Gb, 3B: Db/C#, 4B: Ab/G#, 5B: Eb/D#, 6B: Bb/A#,
    // 7B: F, 8B: C, 9B: G, 10B: D, 11B: A, 12B: E
    static const std::unordered_map<std::string, std::string> majorMap = {
        {"B",  "1B"},
        {"F#", "2B"}, {"Gb", "2B"},
        {"Db", "3B"}, {"C#", "3B"},
        {"Ab", "4B"}, {"G#", "4B"},
        {"Eb", "5B"}, {"D#", "5B"},
        {"Bb", "6B"}, {"A#", "6B"},
        {"F",  "7B"},
        {"C",  "8B"},
        {"G",  "9B"},
        {"D",  "10B"},
        {"A",  "11B"},
        {"E",  "12B"}
    };

    if (isMinor) {
        auto it = minorMap.find(root);
        if (it != minorMap.end()) return it->second;
    } else {
        auto it = majorMap.find(root);
        if (it != majorMap.end()) return it->second;
    }

    return "--";
}

std::string CamelotMapper::formatKeyDisplay(const std::string& rootNote, const std::string& scale) {
    std::string root = normalizeRootNote(rootNote);
    if (root.empty()) return "---";

    std::string sc = normalizeScale(scale);
    std::string scCap = (sc == "minor" ? "Minor" : "Major");
    return root + " " + scCap;
}

std::string CamelotMapper::formatAll(float bpm, const std::string& rootNote, const std::string& scale) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << bpm << " BPM";
    
    std::string keyDisplay = formatKeyDisplay(rootNote, scale);
    std::string camelot = toCamelot(rootNote, scale);

    if (keyDisplay != "---" && !camelot.empty() && camelot != "--") {
        oss << " | " << keyDisplay << " | " << camelot;
    }
    return oss.str();
}

} // namespace clickclack
