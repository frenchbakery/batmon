/**
 * @file player.hpp
 * @author melektron
 * @brief music player for the create
 * @version 0.1
 * @date 2023-03-10
 *
 * @copyright Copyright FrenchBakery (c) 2023
 *
 */

#include <vector>
#include <cstdint>
#include <thread>
#include <cmath>
#include <Arduino.h>

#include "player.hpp"
#include "pitches.h"
#include "pins.h"



TaskHandle_t task_handle;

bool player_should_exit, player_has_exited;

namespace Player // private
{
    static void task_fn(void*);
    static void playLongSong(const std::vector<uint8_t> &notes, const std::vector<uint8_t> &durations);
}

static const std::vector<uint8_t> treble_melody{
    NR_NOTE_E5,
    NR_NOTE_E3,
    NR_NOTE_B4,
    NR_NOTE_C5,
    NR_NOTE_D5,
    NR_NOTE_E5,
    NR_NOTE_D5,
    NR_NOTE_C5,
    NR_NOTE_B4,
    NR_NOTE_A4,
    NR_NOTE_A3,
    NR_NOTE_A4,
    NR_NOTE_C5,
    NR_NOTE_E5,
    NR_NOTE_A3,
    NR_NOTE_D5,
    NR_NOTE_C5,
    NR_NOTE_B4,
    NR_NOTE_E4,
    NR_NOTE_G4,
    NR_NOTE_C5,
    NR_NOTE_D5,
    NR_NOTE_E3,
    NR_NOTE_E5,
    NR_NOTE_E3,
    NR_NOTE_C5,
    NR_NOTE_A3,
    NR_NOTE_A4,
    NR_NOTE_A3,
    NR_NOTE_A4,
    NR_NOTE_A3,
    NR_NOTE_B2,
    NR_NOTE_C3,
    NR_NOTE_D3,
    NR_NOTE_D5,
    NR_NOTE_F5,
    NR_NOTE_A5,
    NR_NOTE_C5,
    NR_NOTE_C5,
    NR_NOTE_G5,
    NR_NOTE_F5,
    NR_NOTE_E5,
    NR_NOTE_C3,
    0,
    NR_NOTE_C5,
    NR_NOTE_E5,
    NR_NOTE_A4,
    NR_NOTE_G4,
    NR_NOTE_D5,
    NR_NOTE_C5,
    NR_NOTE_B4,
    NR_NOTE_E4,
    NR_NOTE_B4,
    NR_NOTE_C5,
    NR_NOTE_D5,
    NR_NOTE_G4,
    NR_NOTE_E5,
    NR_NOTE_G4,
    NR_NOTE_C5,
    NR_NOTE_E4,
    NR_NOTE_A4,
    NR_NOTE_E3,
    NR_NOTE_A4,
    0,
};

static const std::vector<uint8_t> bass_melody{
    NR_NOTE_E4,
    NR_NOTE_E3,
    NR_NOTE_A2,
    NR_NOTE_E3,
    NR_NOTE_C4,
    NR_NOTE_E3,
    NR_NOTE_A2,
    NR_NOTE_E3,
    NR_NOTE_D4,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
    NR_NOTE_B3,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
    NR_NOTE_C4,
    NR_NOTE_E3,
    NR_NOTE_A2,
    NR_NOTE_E3,
    NR_NOTE_A3,
    NR_NOTE_E3,
    NR_NOTE_A2,
    NR_NOTE_E3,
    NR_NOTE_GS3,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
    NR_NOTE_B3,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
    NR_NOTE_E4,
    NR_NOTE_E3,
    NR_NOTE_A2,
    NR_NOTE_E3,
    NR_NOTE_C4,
    NR_NOTE_E3,
    NR_NOTE_A2,
    NR_NOTE_E3,
    NR_NOTE_D4,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
    NR_NOTE_B3,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
    NR_NOTE_C4,
    NR_NOTE_E3,
    NR_NOTE_E4,
    NR_NOTE_E3,
    NR_NOTE_A4,
    NR_NOTE_E3,
    NR_NOTE_A2,
    NR_NOTE_E3,
    NR_NOTE_GS4,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
    NR_NOTE_GS2,
    NR_NOTE_E3,
};

// note durations: 4 = quarter note, 8 = eighth note, etc
static const std::vector<uint8_t> treble_durations{
    8,
    8,
    8,
    8,
    8,
    16,
    16,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    4,
    8,
    8,
    16,
    16,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    16,
    16,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    4,
    4,
};

static const std::vector<uint8_t> bass_durations{
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
    8,
};


static void Player::playLongSong(const std::vector<uint8_t> &notes, const std::vector<uint8_t> &durations)
{
    float speed = 1.5;

    if (notes.size() != durations.size())
        return;
    if (notes.size() == 0)
        return;
    int sections = std::ceil((float)(notes.size()) / 16.0f);

    uint8_t scsong[32];
    int scnotes = notes.size();
    auto it_note = notes.begin();
    auto it_duration = durations.begin();

    do
    {
        noTone(BUZZER);
        tone(BUZZER, *it_note);
        delay((64 * speed) / *it_duration);
        if (player_should_exit) break;
        it_note++;
        it_duration++;
    } while (it_note != notes.end());
}

static void Player::task_fn(void*)
{
    for (;;)
    {
        playLongSong(treble_melody, treble_durations);
        if (player_should_exit) break;
        playLongSong(bass_melody, bass_durations);
        if (player_should_exit) break;
    }

    player_has_exited = true;
    vTaskDelete(NULL);
}

void Player::start()
{
    player_should_exit = false;
    xTaskCreate(
        task_fn,  // Function to implement the task
        "player_task", // Name of the task
        3500,          // Stack size in words
        NULL,          // Task input parameter
        1,             // Priority of the task
        &task_handle    // Task handle.
    );
}

void Player::stop()
{
    player_should_exit = true;
    while (!player_has_exited) delay(10);
}