#include "../Source/DSP/CamelotMapper.h"
#include <cassert>
#include <iostream>
#include <string>

using namespace clickclack;

void testMajorKeys() {
    assert(CamelotMapper::toCamelot("B", "major") == "1B");
    assert(CamelotMapper::toCamelot("F#", "major") == "2B");
    assert(CamelotMapper::toCamelot("Gb", "major") == "2B");
    assert(CamelotMapper::toCamelot("Db", "major") == "3B");
    assert(CamelotMapper::toCamelot("C#", "major") == "3B");
    assert(CamelotMapper::toCamelot("Ab", "major") == "4B");
    assert(CamelotMapper::toCamelot("G#", "major") == "4B");
    assert(CamelotMapper::toCamelot("Eb", "major") == "5B");
    assert(CamelotMapper::toCamelot("D#", "major") == "5B");
    assert(CamelotMapper::toCamelot("Bb", "major") == "6B");
    assert(CamelotMapper::toCamelot("A#", "major") == "6B");
    assert(CamelotMapper::toCamelot("F", "major") == "7B");
    assert(CamelotMapper::toCamelot("C", "major") == "8B");
    assert(CamelotMapper::toCamelot("G", "major") == "9B");
    assert(CamelotMapper::toCamelot("D", "major") == "10B");
    assert(CamelotMapper::toCamelot("A", "major") == "11B");
    assert(CamelotMapper::toCamelot("E", "major") == "12B");
    std::cout << "[PASS] All 12 Major keys & enharmonics mapped correctly.\n";
}

void testMinorKeys() {
    assert(CamelotMapper::toCamelot("Ab", "minor") == "1A");
    assert(CamelotMapper::toCamelot("G#", "minor") == "1A");
    assert(CamelotMapper::toCamelot("Eb", "minor") == "2A");
    assert(CamelotMapper::toCamelot("D#", "minor") == "2A");
    assert(CamelotMapper::toCamelot("Bb", "minor") == "3A");
    assert(CamelotMapper::toCamelot("A#", "minor") == "3A");
    assert(CamelotMapper::toCamelot("F", "minor") == "4A");
    assert(CamelotMapper::toCamelot("C", "minor") == "5A");
    assert(CamelotMapper::toCamelot("G", "minor") == "6A");
    assert(CamelotMapper::toCamelot("D", "minor") == "7A");
    assert(CamelotMapper::toCamelot("A", "minor") == "8A");
    assert(CamelotMapper::toCamelot("E", "minor") == "9A");
    assert(CamelotMapper::toCamelot("B", "minor") == "10A");
    assert(CamelotMapper::toCamelot("F#", "minor") == "11A");
    assert(CamelotMapper::toCamelot("Gb", "minor") == "11A");
    assert(CamelotMapper::toCamelot("C#", "minor") == "12A");
    assert(CamelotMapper::toCamelot("Db", "minor") == "12A");
    std::cout << "[PASS] All 12 Minor keys & enharmonics mapped correctly.\n";
}

void testFormatting() {
    std::string formatted = CamelotMapper::formatAll(124.5f, "C", "minor");
    assert(formatted == "124.5 BPM | C Minor | 5A");
    std::cout << "[PASS] formatAll output: " << formatted << "\n";
}

int main() {
    std::cout << "Running CamelotMapper Unit Tests...\n";
    testMajorKeys();
    testMinorKeys();
    testFormatting();
    std::cout << "All CamelotMapper tests passed successfully!\n";
    return 0;
}
