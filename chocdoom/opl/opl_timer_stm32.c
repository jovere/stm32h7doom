//
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2025 STM32 Port
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// DESCRIPTION:
//     OPL timer for STM32 (no threading)
//     Processes callbacks synchronously when OPL samples are requested.
//

#include "opl_timer.h"
#include "opl_queue.h"

// Current time in microseconds
static uint64_t current_time = 0;

// If non-zero, callbacks are currently paused
static int opl_timer_paused = 0;

// Offset in microseconds to adjust time due to pausing
static uint64_t pause_offset = 0;

// Queue of callbacks waiting to be invoked
static opl_callback_queue_t *callback_queue = NULL;

// Timer running state
static int timer_running = 0;

/**
 * @brief Start the OPL timer (no thread on STM32)
 */
int OPL_Timer_StartThread(void)
{
    if (callback_queue == NULL)
    {
        callback_queue = OPL_Queue_Create();
    }

    current_time = 0;
    opl_timer_paused = 0;
    pause_offset = 0;
    timer_running = 1;

    return 1;
}

/**
 * @brief Stop the OPL timer
 */
void OPL_Timer_StopThread(void)
{
    if (callback_queue != NULL)
    {
        OPL_Queue_Destroy(callback_queue);
        callback_queue = NULL;
    }

    timer_running = 0;
}

/**
 * @brief Set a callback to be invoked after specified microseconds
 */
void OPL_Timer_SetCallback(uint64_t us, opl_callback_t callback, void *data)
{
    if (callback_queue != NULL)
    {
        OPL_Queue_Push(callback_queue, callback, data,
                       current_time + us - pause_offset);
    }
}

/**
 * @brief Clear all pending callbacks
 */
void OPL_Timer_ClearCallbacks(void)
{
    if (callback_queue != NULL)
    {
        OPL_Queue_Clear(callback_queue);
    }
}

/**
 * @brief Adjust callback times by a factor
 */
void OPL_Timer_AdjustCallbacks(float factor)
{
    if (callback_queue != NULL)
    {
        OPL_Queue_AdjustCallbacks(callback_queue, current_time, factor);
    }
}

/**
 * @brief Lock - No-op on STM32 (no threading)
 */
void OPL_Timer_Lock(void)
{
    /* No mutex needed - single-threaded audio callback */
}

/**
 * @brief Unlock - No-op on STM32 (no threading)
 */
void OPL_Timer_Unlock(void)
{
    /* No mutex needed - single-threaded audio callback */
}

/**
 * @brief Set paused state
 */
void OPL_Timer_SetPaused(int paused)
{
    opl_timer_paused = paused;
}

/**
 * @brief Advance timer and process callbacks
 *
 * Called before generating OPL samples to process any pending MIDI events.
 *
 * @param us Microseconds to advance
 */
void OPL_Timer_AdvanceTime(uint64_t us)
{
    if (!timer_running || callback_queue == NULL)
        return;

    // Advance current time
    current_time += us;

    // If paused, update pause offset
    if (opl_timer_paused)
    {
        pause_offset += us;
        return;
    }

    // Process all callbacks that are due
    while (!OPL_Queue_IsEmpty(callback_queue))
    {
        uint64_t next_time = OPL_Queue_Peek(callback_queue) + pause_offset;

        // If callback time hasn't arrived yet, stop processing
        if (next_time > current_time)
            break;

        // Pop and invoke callback
        opl_callback_t callback;
        void *callback_data;

        if (OPL_Queue_Pop(callback_queue, &callback, &callback_data))
        {
            callback(callback_data);
        }
    }
}
