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

#include "wavetable.h"
#include "utility/dspinst.h"

void AudioWavetable::soundOn(const unsigned int *data, double mult)
{
	uint32_t format;
	multiplier = mult;
	playing = 0;
	prior = 0;
	format = *data++;
	attack_next = data;
	attack_start = data;
	attack_length = format & 0xFFFFFF;
	playing = format >> 24;
}

void AudioWavetable::soundOn(void)
{
    uint32_t format;
    playing = 0;
    prior = 0;
    format = *data + 1;
    //format = *data++;
    attack_next = data;
    attack_start = data;
    attack_length = format & 0xFFFFFF;
    playing = format >> 24;
}

void AudioWavetable::soundOff(void)
{
	playing = 0;
}

void AudioWavetable::setData(const unsigned int *data)
{
    this->data = data;
    attack_start = data;
}

extern "C" {
extern const int16_t ulaw_decode_table[256];
};

void AudioWavetable::update(void)
{
	audio_block_t *block;
	const unsigned int *in;
	int16_t *out;
	uint32_t tmp32, consumed;
	int16_t s0, s1, s2, s3, s4;
	int i;

	if (!playing) return;
	block = allocate();
	if (block == NULL) return;

	//Serial.write('.');

	out = block->data;
	in = attack_next;
	s0 = prior;

	switch (playing) {
	  case 0x01: // u-law encoded, 44100 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 4) {
			tmp32 = *in++;
			*out++ = ulaw_decode_table[(tmp32 >> 0) & 255];
			*out++ = ulaw_decode_table[(tmp32 >> 8) & 255];
			*out++ = ulaw_decode_table[(tmp32 >> 16) & 255];
			*out++ = ulaw_decode_table[(tmp32 >> 24) & 255];
		}
		consumed = 128;
		break;

	  case 0x81: // 16 bit PCM, 44100 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 2) {
			tmp32 = *in++;
			*out++ = (int16_t)(tmp32 & 65535);
			*out++ = (int16_t)(tmp32 >> 16);
		}
		consumed = 128;
		break;

	  case 0x02: // u-law encoded, 22050 Hz 
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 8) {
			tmp32 = *in++;
			s1 = ulaw_decode_table[(tmp32 >> 0) & 255];
			s2 = ulaw_decode_table[(tmp32 >> 8) & 255];
			s3 = ulaw_decode_table[(tmp32 >> 16) & 255];
			s4 = ulaw_decode_table[(tmp32 >> 24) & 255];
			*out++ = (s0 + s1) >> 1;
			*out++ = s1;
			*out++ = (s1 + s2) >> 1;
			*out++ = s2;
			*out++ = (s2 + s3) >> 1;
			*out++ = s3;
			*out++ = (s3 + s4) >> 1;
			*out++ = s4;
			s0 = s4;
		}
		consumed = 64;
		break;

	  case 0x82: // 16 bits PCM, 22050 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 4) {
			tmp32 = *in++;
			s1 = (int16_t)(tmp32 & 65535);
			s2 = (int16_t)(tmp32 >> 16);
			*out++ = (s0 + s1) >> 1;
			*out++ = s1;
			*out++ = (s1 + s2) >> 1;
			*out++ = s2;
			s0 = s2;
		}
		consumed = 64;
		break;

	  case 0x03: // u-law encoded, 11025 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 16) {
			tmp32 = *in++;
			s1 = ulaw_decode_table[(tmp32 >> 0) & 255];
			s2 = ulaw_decode_table[(tmp32 >> 8) & 255];
			s3 = ulaw_decode_table[(tmp32 >> 16) & 255];
			s4 = ulaw_decode_table[(tmp32 >> 24) & 255];
			*out++ = (s0 * 3 + s1) >> 2;
			*out++ = (s0 + s1)     >> 1;
			*out++ = (s0 + s1 * 3) >> 2;
			*out++ = s1;
			*out++ = (s1 * 3 + s2) >> 2;
			*out++ = (s1 + s2)     >> 1;
			*out++ = (s1 + s2 * 3) >> 2;
			*out++ = s2;
			*out++ = (s2 * 3 + s3) >> 2;
			*out++ = (s2 + s3)     >> 1;
			*out++ = (s2 + s3 * 3) >> 2;
			*out++ = s3;
			*out++ = (s3 * 3 + s4) >> 2;
			*out++ = (s3 + s4)     >> 1;
			*out++ = (s3 + s4 * 3) >> 2;
			*out++ = s4;
			s0 = s4;
		}
		consumed = 32;
		break;

	  case 0x83: // 16 bit PCM, 11025 Hz
		for (i=0; i < AUDIO_BLOCK_SAMPLES; i += 8) {
			tmp32 = *in++;
			s1 = (int16_t)(tmp32 & 65535);
			s2 = (int16_t)(tmp32 >> 16);
			*out++ = (s0 * 3 + s1) >> 2;
			*out++ = (s0 + s1)     >> 1;
			*out++ = (s0 + s1 * 3) >> 2;
			*out++ = s1;
			*out++ = (s1 * 3 + s2) >> 2;
			*out++ = (s1 + s2)     >> 1;
			*out++ = (s1 + s2 * 3) >> 2;
			*out++ = s2;
			s0 = s2;
		}
		consumed = 32;
		break;

	  default:
		release(block);
		playing = 0;
		return;
	}
	prior = s0;
	attack_next = in;
	if (attack_length > consumed) {
		attack_length -= consumed;
	} else {
		playing = 0;
        //data = attack_start;
	}
	transmit(block);
	release(block);
}


#define B2M_88200 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT / 2.0)
#define B2M_44100 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT) // 97352592
#define B2M_22050 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT * 2.0)
#define B2M_11025 (uint32_t)((double)4294967296000.0 / AUDIO_SAMPLE_RATE_EXACT * 4.0)


uint32_t AudioWavetable::positionMillis(void)
{
	uint8_t p;
	const uint8_t *n, *b;
	uint32_t b2m;

	__disable_irq();
	p = playing;
	n = (const uint8_t *)attack_next;
	b = (const uint8_t *)attack_start;
	__enable_irq();
	switch (p) {
	  case 0x81: // 16 bit PCM, 44100 Hz
		b2m = B2M_88200;  break;
	  case 0x01: // u-law encoded, 44100 Hz
	  case 0x82: // 16 bits PCM, 22050 Hz
		b2m = B2M_44100;  break;
	  case 0x02: // u-law encoded, 22050 Hz
	  case 0x83: // 16 bit PCM, 11025 Hz
		b2m = B2M_22050;  break;
	  case 0x03: // u-law encoded, 11025 Hz
		b2m = B2M_11025;  break;
	  default:
		return 0;
	}
	if (p == 0) return 0;
	return ((uint64_t)(n - b) * b2m) >> 32;
}

uint32_t AudioWavetable::lengthMillis(void)
{
	uint8_t p;
	const uint32_t *b;
	uint32_t b2m;

	__disable_irq();
	p = playing;
	b = (const uint32_t *)attack_start;
	__enable_irq();
	switch (p) {
	  case 0x81: // 16 bit PCM, 44100 Hz
	  case 0x01: // u-law encoded, 44100 Hz
		b2m = B2M_44100;  break;
	  case 0x82: // 16 bits PCM, 22050 Hz
	  case 0x02: // u-law encoded, 22050 Hz
		b2m = B2M_22050;  break;
	  case 0x83: // 16 bit PCM, 11025 Hz
	  case 0x03: // u-law encoded, 11025 Hz
		b2m = B2M_11025;  break;
	  default:
		return 0;
	}
	return ((uint64_t)(*(b - 1) & 0xFFFFFF) * b2m) >> 32;
}


