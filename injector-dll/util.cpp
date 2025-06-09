#include "pch.h"
#include "globals.h"


uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t *modName) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if(hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if(Module32First(hSnap, &modEntry)) {
            do {
                if(!_wcsicmp(modEntry.szModule, modName)) {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while(Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

//int addText(std::vector<std::string> &buf, const std::string &message) {
//    std::string text = std::format("[HackThread]: {}\n", message);
//    buf.push_back(text);
//
//    std::cout << text;
//    std::cout.flush();
//
//    return buf.size() - 1;
//}

//void editText(std::vector<std::string> &buf, int index, const std::string &newMessage) {
//    if(index < 0 || index >= buf.size()) {
//        throw std::out_of_range("Index out of bounds for textBuffer in editText.");
//    }
//    std::string text = std::format("[HackThread]: {}\n", newMessage);
//    buf[index] = text;
//}

//int addText(const std::string &message) { // Removed 'buf' parameter as it's global
//    std::string text = std::format("[HackThread]: {}\n", message);
//
//    // Acquire lock to modify the shared vector
//    std::lock_guard<std::mutex> lock(g_templatesMutex);
//    g_templates.push_back(text);
//    return g_templates.size() - 1;
//}
//
//void editText(int index, const std::string &newMessage) { // Removed 'buf' parameter
//    // Acquire lock to modify the shared vector
//    std::lock_guard<std::mutex> lock(g_templatesMutex);
//    if(index < 0 || index >= g_templates.size()) {
//        // You might want to handle this more gracefully, e.g., log an error
//        // to a dedicated error log entry or return a status code.
//        // For now, it will throw, so caller must catch.
//        throw std::out_of_range("Index out of bounds for g_templates in editText.");
//    }
//    g_templates[index] = std::format("[HackThread]: {}\n", newMessage);
//}

//int addText(const std::string &messageToAdd) {
//    std::string formattedMessage = std::format("[HackThread]: {}\n", messageToAdd);
//    {
//        std::lock_guard<std::mutex> lock(g_templatesMutex);
//        // New lines are not dashboard lines by default
//        g_templates.push_back({ formattedMessage, true, false, 0 });
//    }
//    g_renderCondition.notify_one();
//    return g_templates.size() - 1;
//}
//
//void editText(int index, const std::string &newMessage, bool isDashboard = false, short dashboardYOffset = 0) {
//    {
//        std::lock_guard<std::mutex> lock(g_templatesMutex);
//        if(index < 0 || index >= g_templates.size()) {
//            throw std::out_of_range("Index out of bounds for g_templates in editText.");
//        }
//        g_templates[index].content = std::format("[HackThread]: {}\n", newMessage);
//        g_templates[index].dirty = true;
//        g_templates[index].isDashboardLine = isDashboard; // Set dashboard flag
//        g_templates[index].dashboardYOffset = dashboardYOffset; // Store offset if dashboard
//    }
//    g_renderCondition.notify_one();
//}