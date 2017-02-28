#include "AudioSynthWavetable.h"
#include <dspinst.h>
#include <SerialFlash.h>

#define TIME_TEST_ON
//#define ENVELOPE_DEBUG

#ifdef TIME_TEST_ON
#define TIME_TEST(INTERVAL, CODE_BLOCK_TO_TEST) \
static float MICROS_AVG = 0.0; \
static int TEST_CUR_CNT = 0; \
static int TEST_LST_CNT = 0; \
static int NEXT_DISPLAY = 0; \
static int TEST_TIME_ACC = 0; \
int micros_start = micros(); \
CODE_BLOCK_TO_TEST \
int micros_end = micros(); \
TEST_TIME_ACC += micros_end - micros_start; \
++TEST_CUR_CNT; \
if (NEXT_DISPLAY < micros_end) { \
	MICROS_AVG += (TEST_TIME_ACC - TEST_CUR_CNT * MICROS_AVG) / (TEST_LST_CNT + TEST_CUR_CNT); \
	NEXT_DISPLAY = micros_end + INTERVAL*1000; \
	TEST_LST_CNT += TEST_CUR_CNT; \
	TEST_TIME_ACC = TEST_CUR_CNT = 0; \
	Serial.printf("avg: %f, n: %i\n", MICROS_AVG, TEST_LST_CNT); \
}
#else
#define TIME_TEST(INTERVAL, CODE_BLOCK_TO_TEST) do { } while(0); \
CODE_BLOCK_TO_TEST
#endif

#ifdef ENVELOPE_DEBUG
#define PRINT_ENV(NAME) Serial.printf("%14s-- mult:%06.4f%% of UNITY_GAIN inc:%06.4f%% of UNITY_GAIN count:%i\n", #NAME, float(mult)/float(UNITY_GAIN), float(inc)/float(UNITY_GAIN), count);
#else
#define PRINT_ENV(NAME) do { } while(0);
#endif


void AudioSynthWavetable::stop(void) {
	cli();
	envelopeState = STATE_RELEASE;
	count = current_sample->RELEASE_COUNT;
	inc = -(mult + 4) / (count * 8);
	PRINT_ENV(STATE_RELEASE)
	sei();
}

void AudioSynthWavetable::playFrequency(float freq, int amp) {
	setState(freqToNote(freq), amp, freq);
}

void AudioSynthWavetable::playNote(int note, int amp) {
	setState(note, amp, noteToFreq(note));
}

void AudioSynthWavetable::setState(int note, int amp, float freq) {
	cli();
	int i;
	envelopeState = STATE_IDLE;
	for (i = 0; note > instrument->sample_note_ranges[i]; i++);
	current_sample = &instrument->samples[i];
	if (current_sample == NULL) return;
	setFrequency(freq);
	tone_phase = inc = mult = 0;
	count = current_sample->DELAY_COUNT;
	//amplitude(midi_volume_transform(amp));
	tone_amp = 2855;
	envelopeState = STATE_DELAY;
	PRINT_ENV(STATE_DELAY)
		state_change = true;
	sei();
}

void AudioSynthWavetable::setFrequency(float freq) {
	//float per_hz_increment_rate = ((0x80000000 >> (index_bits - 1)) * cents_offset * rate_coef) / sample_freq + 0.5;
	tone_incr = freq * current_sample->PER_HERTZ_PHASE_INCREMENT;
	//(0x80000000 >> (index_bits - 1) by itself results in a tone_incr that
	//steps through the wavetable sample one element at a time; from there we
	//only need to scale based a ratio of freq/sample_freq for the desired increment
	//tone_incr = cents_offset * ((rate_coef * freq) / sample_freq) * (0x80000000 >> (index_bits - 1)) + 0.5;
}

void AudioSynthWavetable::update(void) {
	cli();
	if (envelopeState == STATE_IDLE) {
		sei()
			return;
	}
	this->state_change = false;
	const sample_data* s = (const sample_data*)current_sample;
	uint32_t tone_phase = this->tone_phase;
	uint32_t tone_incr = this->tone_incr;
	uint16_t tone_amp = this->tone_amp;
	envelopeStateEnum  envelopeState = this->envelopeState;
	int32_t count = this->count;
	int32_t mult = this->mult;
	int32_t inc = this->inc;
	sei();

	audio_block_t* block;
	int16_t* out;
	uint32_t index, scale;
	int32_t s1, s2, v1, v2, v3;
	uint32_t* w12, * w34;
	uint32_t* p;
	uint32_t* end;
	uint32_t tmp1, tmp2;

	block = allocate();
	if (block == NULL) return;

	out = block->data;

	//assuming 16 bit PCM, 44100 Hz
	TIME_TEST(10000,
	for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i+=2) {
		//tone_phase = tone_phase >= s->LOOP_PHASE_END ? tone_phase - s->LOOP_PHASE_LENGTH : tone_phase;
		//index = tone_phase >> (32 - s->INDEX_BITS);
		//scale = (tone_phase << s->INDEX_BITS) >> 16;
		//s1 = waveform[index];
		//s2 = waveform[index + 1];
		//v1 = s1 * int32_t(0xFFFF - scale);
		//v2 = s2 * int32_t(scale);
		//v3 = (v1 + v2) >> 16;
		//*out++ = v3; //(int16_t)((v3 * tone_amp) >> 16);
		//			 //*out++ = v3;
		//tone_phase += tone_incr;

		tone_phase = tone_phase >= s->LOOP_PHASE_END ? tone_phase - s->LOOP_PHASE_LENGTH : tone_phase;
		index = tone_phase >> (32 - s->INDEX_BITS);
		scale = (tone_phase << s->INDEX_BITS) >> 16;
		w12 = (uint32_t*)(s->sample + index);
		s1 = signed_multiply_32x16t(scale, *w12);
		s1 = signed_multiply_accumulate_32x16b(s1, (0xFFFF - scale), *w12);
		tone_phase += tone_incr;
		*out++ = s1;

		tone_phase = tone_phase >= s->LOOP_PHASE_END ? tone_phase - s->LOOP_PHASE_LENGTH : tone_phase;
		index = tone_phase >> (32 - s->INDEX_BITS);
		scale = (tone_phase << s->INDEX_BITS) >> 16;
		w34 = (uint32_t*)(s->sample + index);
		s2 = signed_multiply_32x16t(scale, *w12);
		s2 = signed_multiply_accumulate_32x16b(s2, (0xFFFF - scale), *w34);
		tone_phase += tone_incr;
		*out++ = s2;

		//*((uint32_t*)(out+i)) = pack_16b_16b(s2, s1);
	}
	); //end TIME_TEST


	//*********************************************************************
	//Envelope code
	//*********************************************************************


	p = (uint32_t *)block->data;
	// p increments by 1 for every 2 samples processed.
	end = p + AUDIO_BLOCK_SAMPLES / 2;


	while (p < end) {
		// we only care about the state when completing a region
		if (count <= 0) switch (envelopeState) {
		case STATE_DELAY:
			envelopeState = STATE_ATTACK;
			count = s->ATTACK_COUNT;
			inc = UNITY_GAIN / (count * 8);
			PRINT_ENV(STATE_ATTACK);
			continue;
		case STATE_ATTACK:
			mult = UNITY_GAIN;
			envelopeState = STATE_HOLD;
			count = s->HOLD_COUNT;
			inc = 0;
			PRINT_ENV(STATE_HOLD);
			continue;
		case STATE_HOLD:
			envelopeState = STATE_DECAY;
			count = s->DECAY_COUNT;
			inc = (-s->SUSTAIN_MULT) / (count * 8);
			PRINT_ENV(STATE_DECAY);
			continue;
		case STATE_DECAY:
			mult = UNITY_GAIN - s->SUSTAIN_MULT;
			envelopeState = mult < UNITY_GAIN / 10000 ? STATE_RELEASE : STATE_SUSTAIN;
			inc = 0;
			continue;
		case STATE_SUSTAIN:
			count = INT32_MAX;
			PRINT_ENV(STATE_SUSTAIN);
			continue;
		case STATE_RELEASE:
			envelopeState = STATE_IDLE;
			for (; p < end; ++p) *p = 0;
			PRINT_ENV(STATE_IDLE);
			continue;
		default:
			p = end;
			PRINT_ENV(DEFAULT);
			continue;
		}
		// process 8 samples, using only mult and inc
		mult += inc;
		tmp1 = signed_multiply_32x16b(mult >> 15, p[0]);
		mult += inc;
		tmp2 = signed_multiply_32x16t(mult >> 15, p[0]);
		p[0] = pack_16b_16b(tmp2, tmp1);
		mult += inc;
		tmp1 = signed_multiply_32x16b(mult >> 15, p[1]);
		mult += inc;
		tmp2 = signed_multiply_32x16t(mult >> 15, p[1]);
		p[1] = pack_16b_16b(tmp2, tmp1);
		mult += inc;
		tmp1 = signed_multiply_32x16b(mult >> 15, p[2]);
		mult += inc;
		tmp2 = signed_multiply_32x16t(mult >> 15, p[2]);
		p[2] = pack_16b_16b(tmp2, tmp1);
		mult += inc;
		tmp1 = signed_multiply_32x16b(mult >> 15, p[3]);
		mult += inc;
		tmp2 = signed_multiply_32x16t(mult >> 15, p[3]);
		p[3] = pack_16b_16b(tmp2, tmp1);

		p += 4;
		count--;
	}

	cli();
	if (this->state_change == false) {
		this->tone_phase = tone_phase;
		this->envelopeState = envelopeState;
		this->count = count;
		this->mult = mult;
		this->inc = inc;
	}
	sei();

	transmit(block);
	release(block);
}
