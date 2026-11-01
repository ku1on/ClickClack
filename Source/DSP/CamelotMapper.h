#pragma once

#include <string>

namespace clickclack {

class CamelotMapper {
public:
    static std::string toCamelot(const std::string& rootNote, const std::string& scale);
    static std::string normalizeRootNote(const std::string& rootNote);
    static std::string normalizeScale(const std::string& scale);
    static std::string formatKeyDisplay(const std::string& rootNote, const std::string& scale);
    static std::string formatAll(float bpm, const std::string& rootNote, const std::string& scale);
};

} // namespace clickclack
