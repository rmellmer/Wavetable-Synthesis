# Python script for decoding a .SF2 file for use with Wavetable library.
from sf2utils.sf2parse import Sf2File
import struct
import sys, getopt

BCOUNT = 0
WCOUNT = 1
BUF32 = 0

#def main(path, selection):
def main():
	global BCOUNT
	#Test code: Opens a specified .sf2 and prints out some info using sf2utils
	with open(sys.argv[1], 'rb') as sf2_file:
		sf2 = Sf2File(sf2_file)
		selection = int(sys.argv[2])
		print("SELECTION: " + str(selection))	
		valid = is_sample_valid(sf2.samples[selection])
		
		if valid[0] == False:
			error(valid[1])
			#return
			
		#Ignore extra 8 bits in the 24 bit specification
		sf2.samples[selection].sm24_offset = None
		
		#Get sample data from SF2
		sample = sf2.samples[selection]
		
		with open("SF2_Decoded_Samples.cpp", "w") as output_file:
			with open("SF2_Decoded_Samples.h", "w") as header_file:
				export_sample(output_file, header_file, sample, True)
			

#Write a sample out to C++ style data files. PCM is a bool which when True encodes in PCM. Otherwise, encode in ulaw.
def export_sample(file, header_file, sample, PCM):

	nameSplit = sample.name.split()
	name = nameSplit[0]; #trimming white space from sample name (uses only up to the first whitespace character)

	file.write("#include \"SF2_Decoded_Samples.h\"\n")
	raw_wav_data = sample.raw_sample_data
	start_loop = sample.start_loop
	end_loop = sample.end_loop
	duration = sample.duration

	B_COUNT = 0;
	#calculating lengths of each section#
	a_length = start_loop/2
	padlength = padding(a_length, 128)
	attacklen = ((a_length + padlength) * 2 + 3)/4 + 1
	
	l_length = (end_loop - start_loop)/2
	padlength = padding(l_length, 128)
	looplen = ((l_length + padlength) * 2 + 3) / 4 + 1
	
	d_length = (sample.end - (sample.start + sample.end_loop))/2
	padlength = padding(d_length, 128)
	decaylen = ((d_length + padlength) * 2 + 3)/4 + 1

	#generating all three sections as one array
	file.write("const unsigned int " + name + "_sample[" + str(attacklen+looplen+decaylen) + "] = {\n")
	
	#print out attack
	if PCM == True:
		format = 0x81
	else:
		format = 0x01

	i = 0
	#file.write("const unsigned int " + name + "_attack[" + str(attacklen) + "] = {\n")
	file.write("\n//********************attack********************//:\n");
	file.write("0x%0.8X," % (a_length | (format << 24)))
	while i < start_loop:
		audio = cc_to_int16(raw_wav_data[i], raw_wav_data[i+1])
		if PCM == True:
			# Use PCM Encoding
			print_bytes(file, audio)
			print_bytes(file, audio >> 8)
			#consuming 2 chars at a time, so add another increment
			i = i + 2
		else:
			# Using ulaw encoding
			print_bytes(file, ulaw_encode(audio))
	
	while padlength > 0:
		print_bytes(file, 0)
		padlength = padlength - 1
	
	#file.write("};\n")

	
	#print out loop
	i = start_loop
	#file.write("const unsigned int " + name + "_Loop[" + str(looplen) + "] = {\n")
	file.write("\n//*********************loop*********************//:\n");
	file.write("0x%0.8X," % (l_length | (format << 24)))
	while i < end_loop:
		audio = cc_to_int16(raw_wav_data[i], raw_wav_data[i+1])
		if PCM == True:
			# Use PCM Encoding
			print_bytes(file, audio)
			print_bytes(file, audio >> 8)
			#consuming 2 chars at a time, so add another increment
			i = i + 2
		else:
			# Using ulaw encoding
			print_bytes(file, ulaw_encode(audio))
	
	while padlength > 0:
		print_bytes(file, 0)
		padlength = padlength - 1
	
	#file.write("};\n")

	#print out decay
	end_index = sample.end - sample.start
	i = sample.end_loop
	#file.write("const unsigned int " + name + "_decay[" + str(decaylen) + "] = {\n")
	file.write("\n//********************decay********************//:\n");
	file.write("0x%0.8X," % (d_length | (format << 24)))
	while i < end_index:
		audio = cc_to_int16(raw_wav_data[i], raw_wav_data[i+1])
		if PCM == True:
			# Use PCM Encoding
			print_bytes(file, audio)
			print_bytes(file, audio >> 8)
			#consuming 2 chars at a time, so add another increment
			i = i + 2
		else:
			# Using ulaw encoding
			print_bytes(file, ulaw_encode(audio))
	
	while padlength > 0:
		print_bytes(file, 0)
		padlength = padlength - 1
	
	file.write("};\n")


	#Write sample to header file
	header_file.write("#include <string>\n\n\n")
	
	#header_file.write("extern const unsigned int " + name + "_Loop[" + str(looplen) + "];\n")
	#header_file.write("extern const unsigned int " + name + "_attack[" + str(attacklen) + "];\n")
	#header_file.write("extern const unsigned int " + name + "_decay[" + str(decaylen) + "];\n")

	header_file.write("extern const unsigned int " + name + "_sample[" + str(attacklen+looplen+decaylen) + "];\n")
	#for debugging:	
	print(sample);
	print(sample.name);
	print(sample.original_pitch);
	print(sample.sample_rate);
	print(sample.sample_type);
	print(sample.is_mono);

	header_file.write("struct " + name + "_info {\n")
	header_file.write("\tconst std::string SAMPLE_INFO = \"" + str(sample) + "\";\n")
	header_file.write("\tconst std::string SAMPLE_NAME = \"" + str(sample.name) + "\";\n")
	header_file.write("\tconst int ORIGINAL_PITCH = " + str(sample.original_pitch) + ";\n")
	header_file.write("\tconst int SAMPLE_RATE = " + str(sample.sample_rate) + ";\n")
	header_file.write("\tconst int SAMPLE_NAME = " + str(sample.sample_type) + ";\n")
	header_file.write("\tconst bool IS_MONO= " + str(sample.is_mono) + ";\n")
	header_file.write("\tconst int ATTACK_LENGTH " + str(attacklen) + ";\n")
	header_file.write("\tconst int LOOP_LENGTH " + str(looplen) + ";\n")
	header_file.write("\tconst int DECAY_LENGTH " + str(decaylen) + ";\n")
	header_file.write("};\n")
	
	
	

		
#Checks if the selected sample is valid. Input is a sample object, and output is 
#a tuple with (boolean, error_message - if any)
def is_sample_valid(sample):
	if sample.loop_duration >= sample.duration: return (False, 'Loop length >= sample length')
	if sample.end_loop > sample.duration: return (False, 'End loop index is larger than sample end index')
	return (True, None)

def error(message):
	print("ERROR: " + message)

#Copying functionality from wav2sketch.c
def print_bytes(file, b):
	global BCOUNT, WCOUNT, BUF32
	
	BUF32 = BUF32 | (b << (8 * BCOUNT))
	BCOUNT = BCOUNT + 1
	if BCOUNT >= 4:
		file.write("0x%0.8X" % BUF32)
		file.write(",")
		BUF32 = 0
		BCOUNT = 0
		WCOUNT = WCOUNT + 1
		if WCOUNT >= 8:
			file.write("\n")
			WCOUNT = 0

def padding(length, block):
	extra = length % block
	if extra == 0: return 0
	return block - extra
	
#Copying functionality from wav2sketch.c
def cc_to_int16(c1, c2):
	i1 = int(ord(c1))
	i2 = int(ord(c2))
	i1 = i1 & 255
	i2 = i2 & 255
	return (i2 << 8) | i1
	
#Copying functionality from wav2sketch.c
def ulaw_encode(audio):
	mag = 0
	neg = 0

	if audio >= 0:
		mag = audio
		neg = 0
	else:
		mag = audio * -1
		neg = 0x80
		
	mag = mag + 128
	if mag > 0x7FFF: mag = 0x7FFF
	if mag >= 0x4000: return neg | 0x70 | ((mag >> 10) & 0x0F)  # 01wx yz00 0000 0000
	if mag >= 0x2000: return neg | 0x60 | ((mag >> 9) & 0x0F)   # 001w xyz0 0000 0000
	if mag >= 0x1000: return neg | 0x50 | ((mag >> 8) & 0x0F)   # 0001 wxyz 0000 0000
	if mag >= 0x0800: return neg | 0x40 | ((mag >> 7) & 0x0F)   # 0000 1wxy z000 0000
	if mag >= 0x0400: return neg | 0x30 | ((mag >> 6) & 0x0F)   # 0000 01wx yz00 0000
	if mag >= 0x0200: return neg | 0x20 | ((mag >> 5) & 0x0F)   # 0000 001w xyz0 0000
	if mag >= 0x0100: return neg | 0x10 | ((mag >> 4) & 0x0F)   # 0000 0001 wxyz 0000
	return neg | 0x00 | ((mag >> 3) & 0x0F)   # 0000 0000 1wxy z000

if __name__ == "__main__":
    main()
