/**
 * @file net.hpp
 * @author melektron
 * @brief WiFi and networking functionality
 * @version 0.1
 * @date 2023-07-06
 * 
 * @copyright Copyright FrenchBakery (c) 2023
 * 
 */

#pragma once

namespace net
{
    /**
     * @brief starts the networking task(s), trying to
     * establish and maintain a WiFi connection to the
     * configured WiFi access point and waiting to send
     * data to a server.
     * 
     */
    void init();
};