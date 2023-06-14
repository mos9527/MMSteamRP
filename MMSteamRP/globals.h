#pragma once
#define _ITERATOR_DEBUG_LEVEL 0
// Disable checked iterators for std::string pointers (and etc) to work with the game
#define WIN32_LEAN_AND_MEAN
// Standard Libaries
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <xstring>
#include <vector>
#include <mutex>
#include <map>
#include <algorithm>
#include <io.h>
#include <fcntl.h>
// Dependencies
#include <Windows.h>
#include <detours.h>
// Utils
#include <constant.h>
#include <utils/stringutils.h>
#include <utils/logging.h>
#include <utils/helpers.h>
#include <utils/sigscan.h>
typedef uint8_t _BYTE;
typedef uint64_t _QWORD;
typedef uint32_t _DWORD;
struct PvSelector
{
	_DWORD diff;
	_DWORD diffEx;
	_QWORD pad1;
	_QWORD pad2;
	_QWORD gameModeIndex;
	_DWORD PVID;
	enum GameDifficulty {
		Easy,
		Normal,
		Hard,
		Extreme,
		ExExtreme
	};
	const char* getGameDifficultyString() {
		if (diffEx) return "ExExtreme";
		switch (diff)
		{
		case 0:return "Easy";
		case 1:return "Normal";
		case 2:return "Hard";
		case 3:return "Extreme";
		default:
			return "Easy";
		}
	}
	const char* getGameModeString() {
		switch (gameModeIndex) {
		case 0x0:
			return "RhythmGame";
		case 0x1:
			return "Practice";
		case 0x0101:
			return "PV Viewer";
		case 0x010101:
			return "PV Viewer / AltPv";
		default:
			return "Rhythm Game";
		}
	}
};
inline PvSelector* pvsel;
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
inline PVWaitScreenInfoStruct* PVWaitScreenInfo;