/*
 * debug_console.h
 *
 * Simple debug console for displaying network status and messages
 * on the LCD screen before Doom engine initialization.
 */

#ifndef DEBUG_CONSOLE_H_
#define DEBUG_CONSOLE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// Maximum number of lines to display
#define DEBUG_CONSOLE_MAX_LINES  20
#define DEBUG_CONSOLE_LINE_LENGTH 80

// Initialize the debug console
void DebugConsole_Init(void);

// Clear the console
void DebugConsole_Clear(void);

// Print a line of text (will scroll if buffer full)
void DebugConsole_Print(const char* text);

// Printf-style formatted output
void DebugConsole_Printf(const char* format, ...);

// Render the console to the screen
void DebugConsole_Draw(void);

// Easy switching between printf and console output
// Uncomment ONE of the following options:

// Option 1: Use standard printf only (serial output)
// #define DPRINT printf

// Option 2: Use debug console only (LCD output)
// #define DPRINT DebugConsole_Printf

// Option 3: Use both (serial AND LCD) - DEFAULT
#define DPRINT(...) do { printf(__VA_ARGS__); DebugConsole_Printf(__VA_ARGS__); } while(0)

#endif /* DEBUG_CONSOLE_H_ */
