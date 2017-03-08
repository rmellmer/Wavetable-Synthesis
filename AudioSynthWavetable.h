/* Audio Library for Teensy 3.X
 * Copyright (c) 2017, TeensyAudio PSU Team
 *
 * Development of this audio library was sponsored by PJRC.COM, LLC.
 * Please support PJRC's efforts to develop open source 
 * software by purchasing Teensy or other PJRC products.
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
#include <math.h>
#include <sample_data.h>
#include <stdint.h>

#define UNITY_GAIN INT32_MAX // Max amplitude
#define DEFAULT_AMPLITUDE 127
#define SAMPLES_PER_MSEC (AUDIO_SAMPLE_RATE_EXACT/1000.0)
#define TRIANGLE_INITIAL_PHASE (-0x40000000)

// int n in range 1..log2(AUDIO_BLOCK_SAMPLES/2)-1 (1..7 for AUDIO_BLOCK_SAMPLES == 128)
// where AUDIO_BLOCK_SAMPLES%n == 0, higher == more smooth and more CPU usage
#define LFO_SMOOTHNESS 3
#define LFO_PERIOD (AUDIO_BLOCK_SAMPLES/(1 << (LFO_SMOOTHNESS-1)))

#define ENVELOPE_PERIOD 8

enum envelopeStateEnum { STATE_IDLE, STATE_DELAY, STATE_ATTACK, STATE_HOLD, STATE_DECAY, STATE_SUSTAIN, STATE_RELEASE };

class AudioSynthWavetable : public AudioStream
{
public:
	/**
	 * Class constructor.
	 */
	AudioSynthWavetable(void) : AudioStream(0, NULL) {}

	/**
	 * @brief Set the instrument_data struct to be used as the playback instrument.
	 *
	 * A wavetable uses a set of samples to generate sound.
	 * This function is used to set the instrument samples.
	 * @param instrument a struct of type instrument_data, commonly prodced from a 
	 * decoded SoundFont file using the SoundFont Decoder Script which accompanies this library.
	 */
	void setInstrument(const instrument_data& instrument) {
		cli();
		this->instrument = &instrument;
		current_sample = NULL;
		env_state = STATE_IDLE;
		state_change = true;
		sei();
	}

	/**
	 * @brief Changes the amplitude to 'v'
	 *
	 * A value of 0 will set the synth output to minimum amplitude
	 * (i.e., no output). A value of 1 will set the output to the
	 * maximum amplitude. Amplitude is set linearly with intermediate
	 * values.
	 * @param v a value between 0.0 and 1.0
	 */
	void amplitude(float v) {
		v = (v < 0.0) ? 0.0 : (v > 1.0) ? 1.0 : v;
		tone_amp = (uint16_t)(UINT16_MAX*v);
	}

	/**
	 * @brief Scale midi_amp to a value between 0.0 and 1.0
	 * using a logarithmic tranformation.
	 *
	 * @param midi_amp a value between 0 and 127
	 * @return a value between 0.0 to 1.0
	 */
	static float midi_volume_transform(int midi_amp) {
		// scale midi_amp which is 0 t0 127 to be between
		// 0 and 1 using a logarithmic transformation
		return powf(midi_amp / 127.0, 4);
	}

	/**
	 * @brief Convert a MIDI note value to
	 * its corresponding frequency.
	 *
	 * @param note a value between 0 and 127
	 * @return a frequency
	 */
	static float noteToFreq(int note) {
		//440.0 * pow(2.0, (note - 69) / 12.0);
		float exp = note * (1.0 / 12.0) + 3.0313597;
		return powf(2.0, exp);
	}

	/**
	 * @brief Convert a frequency to the corressponding
	 * MIDI note value.
	 *
	 * @param freq the frequency value as a float to convert
	 * @return a MIDI note (between 0 - 127)
	 */
	static int freqToNote(float freq) {
		return (12.0 / 440.0) * log2f(freq) + 69.5;
	}

	// Defined in AudioSynthWavetable.cpp
	void stop(void);
	void playFrequency(float freq, int amp = DEFAULT_AMPLITUDE);
	void playNote(int note, int amp = DEFAULT_AMPLITUDE);
	bool isPlaying(void) { return env_state != STATE_IDLE; }
	virtual void update(void);
	
	envelopeStateEnum getEnvState(void) { return env_state; }

private:
	void setState(int note, int amp, float freq);
	void setFrequency(float freq);

	volatile bool state_change = false;

	volatile const instrument_data* instrument = NULL;
	volatile const sample_data* current_sample = NULL;

	//sample output state
	volatile uint32_t tone_phase = 0;
	volatile uint32_t tone_incr = 0;
	volatile uint16_t tone_amp = 0;

	//volume environment state
	volatile envelopeStateEnum  env_state = STATE_IDLE;
	volatile int32_t env_count = 0;
	volatile int32_t env_mult = 0;
	volatile int32_t env_incr = 0;

	//vibrato LFO state
	volatile uint32_t vib_count = 0;
	volatile uint32_t vib_phase = 0;
	volatile int32_t vib_pitch_offset_init = 0;
	volatile int32_t vib_pitch_offset_scnd = 0;

	//modulation LFO state
	volatile uint32_t mod_count = 0;
	volatile uint32_t mod_phase = TRIANGLE_INITIAL_PHASE;
	volatile int32_t mod_pitch_offset_init = 0;
	volatile int32_t mod_pitch_offset_scnd = 0;
};

