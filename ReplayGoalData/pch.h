#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include "bakkesmod/plugin/bakkesmodplugin.h"

#pragma comment(lib, "CPPRP.lib")

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include <fstream>

#include "CPPBitReader.h"
#include "ReplayFile.h"
#include "exceptions/ReplayException.h"
#include "exceptions/ParseException.h"
#include "data/NetworkData.h"
#include "NetworkDataParsers.h"
#include "data/ReplayFileData.h"
#include "PropertyParser.h"

#include "json.hpp"

#include "imgui/imgui.h"

#include "fmt/core.h"
#include "fmt/ranges.h"


using namespace nlohmann;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector, X, Y, Z)

extern std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

template<typename S, typename... Args>
void LOG(const S& format_str, Args&&... args)
{
	_globalCvarManager->log(fmt::format(format_str, args...));
}