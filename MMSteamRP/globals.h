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