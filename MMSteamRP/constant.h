#pragma once
#include <globals.h>
// String literals
#define DEFAULT_RP_MESSAGE u8"初音ミク Project DIVA MEGA39's+"
static const char* LanguageTypeStrings[] = { u8"Japanese",u8"English",u8"SChinese",u8"TChinese",u8"Korean",u8"French",u8"Italian",u8"German",u8"Spanish" };
static const char* DisplayStatusStrings[] = { "On Screen","Not available" };
static const char* LyricDisplayStatusStrings[] = { "Ended","Playing","Paused","Ready" };
static const char* Difficulties[] = { "Easy", "Normal", "Hard", "Extreme", "Extra Extreme" };
static const char* GameModes[] = { "Ryhthm Game" ,  "Practice Mode" ,"PV Viewer" };
// Structs & Enums
// TODO : Find functions that uses the following addresses
// And sigscan them to extract them automatically.
static const uint32_t* PVEvent = (uint32_t*)0x1412EE324;
static const float* PVTimestamp = (float*)0x1412EE340;
static const uint32_t* PVPlaying = (uint32_t*)0x1412F0258;
static const uint32_t* Difficulty = (uint32_t*)0x1416E2B90;
static const uint32_t* DifficultyIsExtra = (uint32_t*)0x1416E2B94;
static const uint32_t* GameMode = (uint32_t*)0x1416E2BA8;
static const uint32_t* PVID = (uint32_t*)0x1416E2BB0;
static const uint8_t* Language = (uint8_t*)0x14CC105F4;
struct PVWaitScreenInfoStruct {
    std::string Name;
    std::string Music;      // .music
    std::string MusicVideo; // .illustrator
    std::string Lyrics;     // .lyrics
    std::string Arranger;   // .arranger
    std::string Manipulator;// .manipulator
    std::string PV;         // .pv_editor
    std::string GuitarPlayer;//.guitar_player
};
static PVWaitScreenInfoStruct* PVWaitScreenInfo = (PVWaitScreenInfoStruct*)0x14CC0B5F8;
inline const char* GameDifficultyString() {
    return Difficulties[*Difficulty + *DifficultyIsExtra];
}
inline const char* GameLanguageString() {
    return LanguageTypeStrings[*Language];
}
inline const char* GameModeString() {
    return GameModes[((*GameMode) & 1) + (((*GameMode) & 0x100) > 0)];
}