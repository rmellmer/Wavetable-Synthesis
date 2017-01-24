/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include "Arduino.h"
#include "AudioStream.h"

class AudioSynthWavetable : public AudioStream
{
public:
	AudioSynthWavetable(void)
		: AudioStream(0, NULL)
		, waveform(NULL)
		, length(0)
		, length_bits(0)
		, sample_freq(440.0)
		, playing(0)
		, tone_phase(0)
		, max_phase(0)
		, tone_incr(0)
		, tone_amp(0)
	{}

	void setSample(const unsigned int* data);
	void play(void);
	void playFrequency(float freq);
	void playNote(byte note);
	void stop(void);
	bool isPlaying(void) { return playing; }
	void frequency(float freq);

	void setFreqAmp(float freq, float amp) {
		frequency(freq);
		amplitude(amp);
	}

	void amplitude(float v) {
		v = (v < 0.0) ? 0.0 : (v > 1.0) ? 1.0 : v;
		tone_amp = (uint16_t)(32767.0*v);
	}

	virtual void update(void);

private:
	uint32_t* waveform;
	int length, length_bits;
	float sample_freq;
	uint8_t playing;
	uint32_t tone_phase;
	uint32_t max_phase;
	uint32_t tone_incr;
	uint16_t tone_amp;
};
