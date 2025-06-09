// --- Console Rendering Thread (in main .cpp or dedicated .cpp) ---
#include "pch.h" // Your precompiled headers
#include "console.h" // Your new console.h, which includes globals.h
#include <iostream>   // For std::cout, std::endl, std::flush
#include <chrono>     // For std::chrono::milliseconds
#include <thread>     // For std::this_thread::sleep_for
#include <algorithm>  // For std::max, std::min (if needed)
#include <mutex>      // For std::unique_lock (though it pulls in std::lock_guard too)


// Get current cursor position (needed for printing log)
COORD GetConsoleCursorPosition(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if(GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return csbi.dwCursorPosition;
    }
    return { 0, 0 };
}

// Set cursor position
void SetCursor(HANDLE hConsole, COORD pos) {
    SetConsoleCursorPosition(hConsole, pos);
}

// Get console buffer info (for window size, scroll position)
CONSOLE_SCREEN_BUFFER_INFO GetConsoleBufferInfo(HANDLE hConsole) {
    CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    return csbi;
}

DWORD WINAPI ConsoleRenderThread(LPVOID lpParam) {
    if(g_hConsoleOutput == NULL || g_hConsoleOutput == INVALID_HANDLE_VALUE) {
        g_bRenderThreadRunning = false;
        return 1;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi_prev = GetConsoleBufferInfo(g_hConsoleOutput); // Store previous info to detect scrolls
    COORD currentLogPrintPos = { 0, 0 }; // Tracks where the *next* log line should be printed

    std::unique_lock<std::mutex> lock(g_templatesMutex);

    while(g_bRenderThreadRunning) {
        // Wait for a notification or a timeout to prevent busy-waiting
        g_renderCondition.wait_for(lock, std::chrono::milliseconds(50),
            [&] {
                if(!g_bRenderThreadRunning) return true; // Exit condition
                // Check if any lines are dirty
                for(const auto &line : g_templates) {
                    if(line.dirty) return true;
                }
                return false;
            });

        if(!g_bRenderThreadRunning) break; // Exit if shutdown

        // Get current console info
        CONSOLE_SCREEN_BUFFER_INFO csbi_current = GetConsoleBufferInfo(g_hConsoleOutput);

        // --- Handle Scrolling and Full Log Redraw if Window Scrolled ---
        // If the top visible line changed (i.e., console scrolled), we need to redraw the visible log.
        bool consoleScrolled = (csbi_current.dwCursorPosition.Y < currentLogPrintPos.Y && currentLogPrintPos.Y > 0);
        // Or if the window top changed
        if(csbi_current.srWindow.Top != csbi_prev.srWindow.Top) {
            consoleScrolled = true;
            // When scrolled, we essentially lose knowledge of where old lines were.
            // Best to just re-print the visible portion of the log.
        }
        csbi_prev = csbi_current; // Update for next iteration

        // --- Process Dirty Lines ---
        COORD tempCursorPos = GetConsoleCursorPosition(g_hConsoleOutput); // Store current cursor before drawing
        short consoleHeight = csbi_current.srWindow.Bottom - csbi_current.srWindow.Top + 1;

        for(int i = 0; i < g_templates.size(); ++i) {
            ConsoleLine &line = g_templates[i]; // Get a mutable reference

            if(line.dirty) {
                if(line.isDashboardLine) {
                    // --- Handle Dashboard Line Update ---
                    // Calculate absolute Y position from bottom
                    short targetY = csbi_current.srWindow.Bottom - line.dashboardYOffset;
                    COORD dashboardPos = { 0, targetY };

                    SetCursor(g_hConsoleOutput, dashboardPos);
                    std::cout << line.content;
                    // Clear trailing characters if new message is shorter
                    size_t currentLineLength = line.content.length();
                    // Need to store the *actual* last printed length to clear correctly
                    // For dashboard lines, we can assume full row clear.
                    // Or track last printed length, which would require storing it in ConsoleLine.
                    // For simplicity, let's just assume we clear the whole line width for a dashboard.
                    // A better way would be to pass `oldMessageLength` from editText or store `printedLength`
                    // In this design, we don't have old length info here. Max width or fixed width.
                    // Let's assume a max line width, e.g., 80 chars.
                    short assumedMaxWidth = csbi_current.dwSize.X; // Width of console buffer
                    if(currentLineLength < assumedMaxWidth - 1) { // -1 for newline char in content
                        for(int j = 0; j < (assumedMaxWidth - currentLineLength - 1); ++j) {
                            std::cout << " "; // Print spaces
                        }
                    }
                    std::cout.flush(); // Ensure immediate display
                } else {
                    // --- Handle Scrolling Log Line (addText or non-dashboard editText) ---
                    // If console scrolled, we must redraw the visible log portion
                    // For `addText` (new lines), we append. For `editText` (non-dashboard), we print a new line.
                    SetCursor(g_hConsoleOutput, currentLogPrintPos); // Move to the bottom of the log area
                    std::cout << line.content; // Print the line
                    std::cout.flush();
                    currentLogPrintPos.Y++; // Move cursor down for next log line
                    currentLogPrintPos.X = 0; // Reset X
                    // Note: This does NOT do in-place update for normal log lines.
                    // It's still appending or re-appending after scroll.
                    // This avoids the complex COORD management for individual scrolling lines.
                }
                line.dirty = false; // Mark as clean after rendering
            }
        }
        // Restore cursor to its state before this render pass (likely end of the log)
        SetCursor(g_hConsoleOutput, currentLogPrintPos);
        std::cout.flush();

        // Lock released automatically here as unique_lock goes out of scope before next wait.
    }
    return 0;
}