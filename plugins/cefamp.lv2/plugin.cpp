/*
    cefamp.cpp - This file is part of cefamp.lv2
    Copyright (C) 2013  Michael Fisher <mfisher31@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "../../lvtk/src/plugin.cpp"

#include <cmath>
#include "lvtk/plugin.hpp"
#include "ports.h"

using namespace lvtk;
using std::vector;

#define CEFAMP_URI  "http://lvtoolkit.org/plugins/cefamp"

class Ports {
public:
    enum ID {
        Input = 0, Output, Volume, Count
    };
};

/** Convert a value in decibels to gain */
template <typename T>
static T cefamp_db_to_gain (const T db, const T inf = (T) -70)
{
    return db > inf ? std::pow ((T) 10.0, db * (T) 0.05) : T();
}

class CefAmp : public Plugin<CefAmp, URID<true> >
{
public:

    CefAmp (double rate)
        : Plugin<CefAmp, URID<true> > (Ports::Count)
    {
        samplerate = rate;
        last_volume = 0.0f;
        last_gain = cefamp_db_to_gain (last_volume);
    }

    void run (uint32_t nframes)
    {
        const float* in = this->p (Ports::Input);
        float *out = this->p (Ports::Output);
        const float volume = *this->p (Ports::Volume);

        if (volume != last_volume) {
            last_volume = volume;
            last_gain = cefamp_db_to_gain (last_volume);
            std::clog << "[cefamp] volume: " << last_volume << std::endl;
        }

        for (uint32_t i = 0; i < nframes; ++i)
            out[i] = in[i] * last_gain;
    }

private:
    double samplerate;
    float last_volume;
    float last_gain;
};

static int cefamp_desc_id = CefAmp::register_class (CEFAMP_URI);

