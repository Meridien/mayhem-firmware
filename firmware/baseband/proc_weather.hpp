/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2016 Furrtek
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
/*
    Creator: @htotoo
*/

#ifndef __PROC_WEATHER_H__
#define __PROC_WEATHER_H__

#include "baseband_processor.hpp"
#include "baseband_thread.hpp"
#include "rssi_thread.hpp"
#include "message.hpp"
#include "dsp_decimate.hpp"

#include "fprotos/weatherprotos.hpp"

class WeatherProcessor : public BasebandProcessor {
   public:
    void execute(const buffer_c8_t& buffer) override;
    void on_message(const Message* const message) override;

   private:
    static constexpr size_t baseband_fs = 4'000'000;  // it works, I think we need to write that master clock in the baseband_threat , even later we decimate it.
    static constexpr uint32_t usperTick = 500 * 8;    // In current sw , we do not scale it due to clock. We scaled it due to less array buffer sampes due to /8 decimation.
                                                      // TODO , Pending to investigate , why ticks are not proportional to the SR clock, 500 nseg (2Mhz) , 250 nseg (4Mhz) ??? ;previous comment :  "we nees ms to has to divide by 1000"

    /* Array Buffer aux. used in decim0 and decim1 IQ c16 signed  data ; (decim0 defines the max length of the array) */
    std::array<complex16_t, 512> dst{};  // decim0 /4 ,  2048/4 = 512 complex I,Q
    const buffer_c16_t dst_buffer{
        dst.data(),
        dst.size()};

    /* Decimates */
    dsp::decimate::FIRC8xR16x24FS4Decim4 decim_0{};
    dsp::decimate::FIRC16xR16x16Decim2 decim_1{};

    uint32_t currentDuration = 0;
    uint32_t threshold = 0x0630;  // will overwrite after the first iteration
    bool currentHiLow = false;
    bool configured{false};

    // for debug
    uint32_t cnt = 0;
    uint32_t tm = 0;

    WeatherProtos protoList{};  // holds all the protocols we can parse
    void configure(const WeatherRxConfigureMessage& message);

    /* NB: Threads should be the last members in the class definition. */
    BasebandThread baseband_thread{baseband_fs, this, baseband::Direction::Receive};
    RSSIThread rssi_thread{};
};

#endif /*__PROC_WEATHER_H__*/