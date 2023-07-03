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
    static void playLongSong(const std::vector<int> &notes, const std::vector<uint8_t> &durations);
}

static const std::vector<int> treble_melody{
    HZ_NOTE_E5,
    HZ_NOTE_E3,
    HZ_NOTE_B4,
    HZ_NOTE_C5,
    HZ_NOTE_D5,
    HZ_NOTE_E5,
    HZ_NOTE_D5,
    HZ_NOTE_C5,
    HZ_NOTE_B4,
    HZ_NOTE_A4,
    HZ_NOTE_A3,
    HZ_NOTE_A4,
    HZ_NOTE_C5,
    HZ_NOTE_E5,
    HZ_NOTE_A3,
    HZ_NOTE_D5,
    HZ_NOTE_C5,
    HZ_NOTE_B4,
    HZ_NOTE_E4,
    HZ_NOTE_G4,
    HZ_NOTE_C5,
    HZ_NOTE_D5,
    HZ_NOTE_E3,
    HZ_NOTE_E5,
    HZ_NOTE_E3,
    HZ_NOTE_C5,
    HZ_NOTE_A3,
    HZ_NOTE_A4,
    HZ_NOTE_A3,
    HZ_NOTE_A4,
    HZ_NOTE_A3,
    HZ_NOTE_B2,
    HZ_NOTE_C3,
    HZ_NOTE_D3,
    HZ_NOTE_D5,
    HZ_NOTE_F5,
    HZ_NOTE_A5,
    HZ_NOTE_C5,
    HZ_NOTE_C5,
    HZ_NOTE_G5,
    HZ_NOTE_F5,
    HZ_NOTE_E5,
    HZ_NOTE_C3,
    0,
    HZ_NOTE_C5,
    HZ_NOTE_E5,
    HZ_NOTE_A4,
    HZ_NOTE_G4,
    HZ_NOTE_D5,
    HZ_NOTE_C5,
    HZ_NOTE_B4,
    HZ_NOTE_E4,
    HZ_NOTE_B4,
    HZ_NOTE_C5,
    HZ_NOTE_D5,
    HZ_NOTE_G4,
    HZ_NOTE_E5,
    HZ_NOTE_G4,
    HZ_NOTE_C5,
    HZ_NOTE_E4,
    HZ_NOTE_A4,
    HZ_NOTE_E3,
    HZ_NOTE_A4,
    0,
};

static const std::vector<int> bass_melody{
    HZ_NOTE_E4,
    HZ_NOTE_E3,
    HZ_NOTE_A2,
    HZ_NOTE_E3,
    HZ_NOTE_C4,
    HZ_NOTE_E3,
    HZ_NOTE_A2,
    HZ_NOTE_E3,
    HZ_NOTE_D4,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
    HZ_NOTE_B3,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
    HZ_NOTE_C4,
    HZ_NOTE_E3,
    HZ_NOTE_A2,
    HZ_NOTE_E3,
    HZ_NOTE_A3,
    HZ_NOTE_E3,
    HZ_NOTE_A2,
    HZ_NOTE_E3,
    HZ_NOTE_GS3,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
    HZ_NOTE_B3,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
    HZ_NOTE_E4,
    HZ_NOTE_E3,
    HZ_NOTE_A2,
    HZ_NOTE_E3,
    HZ_NOTE_C4,
    HZ_NOTE_E3,
    HZ_NOTE_A2,
    HZ_NOTE_E3,
    HZ_NOTE_D4,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
    HZ_NOTE_B3,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
    HZ_NOTE_C4,
    HZ_NOTE_E3,
    HZ_NOTE_E4,
    HZ_NOTE_E3,
    HZ_NOTE_A4,
    HZ_NOTE_E3,
    HZ_NOTE_A2,
    HZ_NOTE_E3,
    HZ_NOTE_GS4,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
    HZ_NOTE_GS2,
    HZ_NOTE_E3,
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


static void Player::playLongSong(const std::vector<int> &notes, const std::vector<uint8_t> &durations)
{
    float speed = 1.2;

    if (notes.size() != durations.size())
        return;
    if (notes.size() == 0)
        return;

    auto it_note = notes.begin();
    auto it_duration = durations.begin();

    do
    {
        noTone(BUZZER);
        //printf("playing: %d, dur: %f\n", *it_note, (float)((64 * speed) / *it_duration));
        tone(BUZZER, *it_note);
        delay((1000 * speed) / *it_duration);
        if (player_should_exit) break;
        it_note++;
        it_duration++;
    } while (it_note != notes.end());
}

static void Player::task_fn(void*)
{
    for (;;)
    {
        printf("start to play song");
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