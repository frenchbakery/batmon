/**
 * @file battery.hpp
 * @author melektron
 * @brief battery cell functions
 * @version 0.1
 * @date 2023-07-05
 * 
 * @copyright Copyright FrenchBakery (c) 2023
 * 
 */

#pragma once

namespace battery
{
    /**
     * @return int voltage of cell 1 (lower cell) in mV
     */
    int read_cell1();

    /**
     * @return int voltage of cell 2 (lower cell) in mV
     */
    int read_cell2();
}