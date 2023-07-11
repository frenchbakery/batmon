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

#include <el/retcode.hpp>

namespace net
{
    /**
     * @brief structure containing all the information reported to the server
     * curing the a periodic battery report.
     */
    struct report_t
    {
        int c1_voltage;
        int c2_voltage;
        int c1_warn_threshold;
        int c2_warn_threshold;
        int c1_alarm_threshold;
        int c2_alarm_threshold;
        int diff_alarm_threshold;
    };
    extern report_t report;

    /**
     * @brief starts the networking task(s), trying to
     * establish and maintain a WiFi connection to the
     * configured WiFi access point and waiting to send
     * data to a server.
     *
     */
    void init();

    /**
     * @brief tells the network task to send a report
     * if possible. If the network connection is down or drops while sending, 
     * the report will not be sent and not be resent after connection is 
     * up again. This isn't necessary as new reports are generated frequently enough
     * anyway.
     * 
     */
    void update();

};