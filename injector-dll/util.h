#pragma once

#include "globals.h"

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t *modName);

//int addText(std::vector<std::string> &buf, const std::string &message);
//
//void editText(std::vector<std::string> &buf, int index, const std::string &newMessage);

//int addText(const std::string &message);

//void editText(int index, const std::string &newMessage);

//void editText(int index, const std::string &newMessage, bool isDashboard = false, short dashboardYOffset = 0);