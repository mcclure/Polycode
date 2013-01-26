/*
 Copyright (C) 2011 by Ivan Safrin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "PolySound.h"
#include <vorbis/vorbisfile.h>
#include "PolyString.h"
#include "PolyLogger.h"

#include "OSBasics.h"
#include <string>
#include <vector>

using namespace std;
using namespace Polycode;

size_t custom_readfunc(void *ptr, size_t size, size_t nmemb, void *datasource) {
	OSFILE *file = (OSFILE*) datasource;
	return OSBasics::read(ptr, size, nmemb, file);
}

int custom_seekfunc(void *datasource, ogg_int64_t offset, int whence){
	OSFILE *file = (OSFILE*) datasource;
	return OSBasics::seek(file, offset, whence);
}

int custom_closefunc(void *datasource) {
	OSFILE *file = (OSFILE*) datasource;
	return OSBasics::close(file);
}

long custom_tellfunc(void *datasource) {
	OSFILE *file = (OSFILE*) datasource;
	return OSBasics::tell(file);
}

Sound::Sound(const String& fileName) : sampleLength(-1) {
	soundLoaded = false;		
	loadFile(fileName);
	setIsPositional(false);
	
	setVolume(1.0);
	setPitch(1.0);
}

Sound::Sound(const char *data, int size, int channels, int freq, int bps) : sampleLength(-1) {
	ALuint buffer = loadBytes(data, size, freq, channels, bps);
	
	soundSource = GenSource(buffer);
	setIsPositional(false);
	soundLoaded = true;
		
	setVolume(1.0);
	setPitch(1.0);
}

void Sound::loadFile(String fileName) {

	if(soundLoaded) {
		alDeleteSources(1,&soundSource);	
	}

	String actualFilename = fileName;
	OSFILE *test = OSBasics::open(fileName, "rb");
	if(!test) {
		actualFilename = "default/default.wav";
	} else {
		OSBasics::close(test);	
	}
	
	String extension;
	size_t found;
	found=actualFilename.rfind(".");
	if (found!=string::npos) {
		extension = actualFilename.substr(found+1);
	} else {
		extension = "";
	}
	
	ALuint buffer;
	if(extension == "wav" || extension == "WAV") {
		buffer = loadWAV(actualFilename);			
	} else if(extension == "ogg" || extension == "OGG") {
		buffer = loadOGG(actualFilename);			
	}
	
	this->fileName = actualFilename;
	
	soundSource = GenSource(buffer);	
	
	setVolume(volume);
	setPitch(pitch);
	
	setReferenceDistance(referenceDistance);
	setMaxDistance(maxDistance);
	
	soundLoaded = true;
}

String Sound::getFileName() {
	return fileName;
}

Number Sound::getVolume() {
	return volume;
}

Number Sound::getPitch() {
	return pitch;
}

Sound::~Sound() {
	Logger::log("destroying sound...\n");
	alDeleteSources(1,&soundSource);
}

void Sound::soundCheck(bool result, const String& err) {
	if(!result)
		soundError(err);
}

void Sound::soundError(const String& err) {
	Logger::log("SOUND ERROR: %s\n", err.c_str());
}

unsigned long Sound::readByte32(const unsigned char buffer[4]) {
#if TAU_BIG_ENDIAN
    return (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
#else
    return (buffer[3] << 24) + (buffer[2] << 16) + (buffer[1] << 8) + buffer[0];
#endif
}

unsigned short Sound::readByte16(const unsigned char buffer[2]) {
#if TAU_BIG_ENDIAN
    return (buffer[0] << 8) + buffer[1];
#else
    return (buffer[1] << 8) + buffer[0];
#endif	
}

void Sound::Play(bool loop) {
	if(!loop) {
		alSourcei(soundSource, AL_LOOPING, AL_FALSE);
	} else {
		alSourcei(soundSource, AL_LOOPING, AL_TRUE);		
	}
	alSourcePlay(soundSource);
}

bool Sound::isPlaying() {
	ALenum state;
	alGetSourcei(soundSource, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
}


void Sound::setVolume(Number newVolume) {
	this->volume = newVolume;
	alSourcef(soundSource, AL_GAIN, newVolume);
}

void Sound::setPitch(Number newPitch) {
	this->pitch = newPitch;
	alSourcef(soundSource, AL_PITCH, newPitch);
}

void Sound::setSoundPosition(Vector3 position) {
	if(isPositional)
		alSource3f(soundSource,AL_POSITION, position.x, position.y, position.z);
}

void Sound::setSoundVelocity(Vector3 velocity) {
	if(isPositional)
		alSource3f(soundSource,AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void Sound::setSoundDirection(Vector3 direction) {
	if(isPositional)
		alSource3f(soundSource,AL_DIRECTION, direction.x, direction.y, direction.z);
}

void Sound::setOffset(int off) {
	alSourcei(soundSource, AL_SAMPLE_OFFSET, off);
}


Number Sound::getPlaybackTime() {
	float result = 0.0;
	alGetSourcef(soundSource, AL_SEC_OFFSET, &result);
	return result;
}

Number Sound::getPlaybackDuration() {
	ALint sizeInBytes;
	ALint channels;
	ALint bits;
	ALint bufferID;
	alGetSourcei(soundSource, AL_BUFFER, &bufferID);
	
	alGetBufferi(bufferID, AL_SIZE, &sizeInBytes);
	alGetBufferi(bufferID, AL_CHANNELS, &channels);
	alGetBufferi(bufferID, AL_BITS, &bits);

	int lengthInSamples = sizeInBytes * 8 / (channels * bits);

	ALint frequency;
	alGetBufferi(bufferID, AL_FREQUENCY, &frequency);
	Number durationInSeconds = (float)lengthInSamples / (float)frequency;
	
	return durationInSeconds;
}
		
int Sound::getOffset() {
	ALint off = -1;
	alGetSourcei(soundSource, AL_SAMPLE_OFFSET, &off);
	return off;
}

void Sound::seekTo(Number time) {
	if(time > getPlaybackDuration())
		return;
	alSourcef(soundSource, AL_SEC_OFFSET, time);
}

int Sound::getSampleLength() {
	return sampleLength;
}

void Sound::setPositionalProperties(Number referenceDistance, Number maxDistance) { 
	setReferenceDistance(referenceDistance);
	setMaxDistance(maxDistance);
}

void Sound::setReferenceDistance(Number referenceDistance) {
	this->referenceDistance = referenceDistance;
	alSourcef(soundSource,AL_REFERENCE_DISTANCE, referenceDistance);
}

void Sound::setMaxDistance(Number maxDistance) {
	this->maxDistance = maxDistance;
	alSourcef(soundSource,AL_MAX_DISTANCE, maxDistance);	
}
		
Number Sound::getReferenceDistance() {
	return referenceDistance;
}

Number Sound::getMaxDistance() {
	return maxDistance;
}


void Sound::setIsPositional(bool isPositional) {
	this->isPositional = isPositional;
	if(isPositional) {
		alSourcei(soundSource, AL_SOURCE_RELATIVE, AL_FALSE);
	} else {
		alSourcei(soundSource, AL_SOURCE_RELATIVE, AL_TRUE);	
		alSource3f(soundSource,AL_POSITION, 0,0,0);
		alSource3f(soundSource,AL_VELOCITY, 0,0,0);
		alSource3f(soundSource,AL_DIRECTION, 0,0,0);				
	}
}

void Sound::checkALError(const String& operation) {
	ALenum error = alGetError();
	if(error != AL_NO_ERROR) {
		switch(error) {
			case AL_NO_ERROR:
				soundError(operation + ": " +ALNoErrorStr);
				break;
			case AL_INVALID_NAME:
				soundError(operation +": " + ALInvalidNameStr);
				break;
			case AL_INVALID_ENUM:
				soundError(operation + ": " +ALInvalidEnumStr);
				break;
			case AL_INVALID_VALUE:
				soundError(operation + ": " +ALInvalidValueStr);
				break;
			case AL_INVALID_OPERATION:
				soundError(operation + ": " +ALInvalidOpStr);
				break;
			case AL_OUT_OF_MEMORY:
				soundError(operation + ": " +ALOutOfMemoryStr);
				break;
			default:
				soundError(operation + ": " +ALOtherErrorStr);
				break;
		}		
	}
}

void Sound::Stop() {
	alSourceStop(soundSource);
}

ALuint Sound::GenSource() {
	ALuint source;
	bool looping = false;
	ALfloat sourcePos[] = {0.0, 0.0, 0.0};
	ALfloat sourceVel[] = {0.0, 0.0, 0.0};
	
	alGetError();
	
	alGenSources(1, &source);
	checkALError("Generating sources");
	
	alSourcef(source, AL_PITCH, 1.0);
	alSourcef(source, AL_GAIN, 1.0);
	alSourcefv(source, AL_POSITION, sourcePos);
	alSourcefv(source, AL_VELOCITY, sourceVel);
	alSourcei(source, AL_LOOPING, looping);

	checkALError("Setting source properties");
	
	return source;
}

ALuint Sound::GenSource(ALuint buffer) {
	alGetError();
	ALuint source = GenSource();
	alSourcei(source, AL_BUFFER, buffer);	
	checkALError("Setting source buffer");
	return source;
}

ALuint Sound::loadBytes(const char *data, int size, int freq, int channels, int bps) {
	ALuint buffer = AL_NONE;
	ALenum format;
	if (channels == 1)
		format = (bps == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	else
		format = (bps == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	
	sampleLength = bps > 8 ? size / (bps/8) : -1;
	
	alGenBuffers(1, &buffer);
	soundCheck(alGetError() == AL_NO_ERROR, "LoadBytes: Could not generate buffer");
	soundCheck(AL_NONE != buffer, "LoadBytes: Could not generate buffer");
	
	alBufferData(buffer, format, data, size, freq);
	soundCheck(alGetError() == AL_NO_ERROR, "LoadBytes: Could not load buffer data");
	return buffer;
}

ALuint Sound::loadOGG(const String& fileName) {
	vector<char> buffer;
	
	ALuint bufferID = AL_NONE; 
	alGenBuffers(1, &bufferID);
	int endian = 0;             // 0 for Little-Endian, 1 for Big-Endian
	int bitStream;
	long bytes;
	char array[BUFFER_SIZE];    // Local fixed size array
	OSFILE *f;
	ALenum format;
	ALsizei freq;
	
	// Open for binary reading
	f = OSBasics::open(fileName.c_str(), "rb");		
	if(!f) {
		soundError("Error loading OGG file!\n");
		return bufferID;
	}
	vorbis_info *pInfo;
	OggVorbis_File oggFile;	
	
	ov_callbacks callbacks;
	callbacks.read_func = custom_readfunc;
	callbacks.seek_func = custom_seekfunc;
	callbacks.close_func = custom_closefunc;
	callbacks.tell_func = custom_tellfunc;
	
	ov_open_callbacks( (void*)f, &oggFile, NULL, 0, callbacks);
//	ov_open(f, &oggFile, NULL, 0);
	// Get some information about the OGG file
	pInfo = ov_info(&oggFile, -1);
	
	// Check the number of channels... always use 16-bit samples
	if (pInfo->channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;
	// end if
	
	// The frequency of the sampling rate
	freq = pInfo->rate;	
	do {
		// Read up to a buffer's worth of decoded sound data
		bytes = ov_read(&oggFile, array, BUFFER_SIZE, endian, 2, 1, &bitStream);
		// Append to end of buffer
		buffer.insert(buffer.end(), array, array + bytes);
	} while (bytes > 0);
	ov_clear(&oggFile);
	
	sampleLength = buffer.size() / sizeof(unsigned short);
	
	alBufferData(bufferID, format, &buffer[0], static_cast<ALsizei>(buffer.size()), freq);
	
	return bufferID;
}

ALuint Sound::loadWAV(const String& fileName) {
	long bytes;
	vector <char> data;
	ALsizei freq;
	
	// Local resources
	OSFILE *f = NULL;
	char *array = NULL;
	
	alGetError();
	
		// Open for binary reading
		f = OSBasics::open(fileName.c_str(), "rb");
		if (!f)
			soundError("LoadWav: Could not load wav from " + fileName);
		
		// buffers
		char magic[5];
		magic[4] = '\0';
		unsigned char buffer32[4];
		unsigned char buffer16[2];
		
		// check magic
		soundCheck(OSBasics::read(magic,4,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		soundCheck(String(magic) == "RIFF", "LoadWav: Wrong wav file format. This file is not a .wav file (no RIFF magic): "+ fileName );
		
		// skip 4 bytes (file size)
		OSBasics::seek(f,4,SEEK_CUR);
		
		// check file format
		soundCheck(OSBasics::read(magic,4,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		soundCheck(String(magic) == "WAVE", "LoadWav: Wrong wav file format. This file is not a .wav file (no WAVE format): "+ fileName );
		
		// check 'fmt ' sub chunk (1)
		soundCheck(OSBasics::read(magic,4,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		soundCheck(String(magic) == "fmt ", "LoadWav: Wrong wav file format. This file is not a .wav file (no 'fmt ' subchunk): "+ fileName );
		
		// read (1)'s size
		soundCheck(OSBasics::read(buffer32,4,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		unsigned long subChunk1Size = readByte32(buffer32);
		soundCheck(subChunk1Size >= 16, "Wrong wav file format. This file is not a .wav file ('fmt ' chunk too small, truncated file?): "+ fileName );
		
		// check PCM audio format
		soundCheck(OSBasics::read(buffer16,2,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		unsigned short audioFormat = readByte16(buffer16);
		soundCheck(audioFormat == 1, "LoadWav: Wrong wav file format. This file is not a .wav file (audio format is not PCM): "+ fileName );
		
		// read number of channels
		soundCheck(OSBasics::read(buffer16,2,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		unsigned short channels = readByte16(buffer16);
		
		// read frequency (sample rate)
		soundCheck(OSBasics::read(buffer32,4,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		unsigned long frequency = readByte32(buffer32);
		
		// skip 6 bytes (Byte rate (4), Block align (2))
		OSBasics::seek(f,6,SEEK_CUR);
		
		// read bits per sample
		soundCheck(OSBasics::read(buffer16,2,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		unsigned short bps = readByte16(buffer16);
		
		// check 'data' sub chunk (2)
		soundCheck(OSBasics::read(magic,4,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		soundCheck(String(magic) == "data", "LoadWav: Wrong wav file format. This file is not a .wav file (no data subchunk): "+ fileName );
		
		soundCheck(OSBasics::read(buffer32,4,1,f) == 1, "LoadWav: Cannot read wav file "+ fileName );
		unsigned long subChunk2Size = readByte32(buffer32);
		
		// The frequency of the sampling rate
		freq = frequency;
		soundCheck(sizeof(freq) == sizeof(frequency), "LoadWav: freq and frequency different sizes");
		
		array = new char[BUFFER_SIZE];
		
		while (data.size() != subChunk2Size) {
			// Read up to a buffer's worth of decoded sound data
			bytes = OSBasics::read(array, 1, BUFFER_SIZE, f);
			
			if (bytes <= 0)
				break;
			
			if (data.size() + bytes > subChunk2Size)
				bytes = subChunk2Size - data.size();
			
			// Append to end of buffer
			data.insert(data.end(), array, array + bytes);
		};
		
		delete []array;
		array = NULL;
		
		OSBasics::close(f);
		f = NULL;
				
		return loadBytes(&data[0], data.size(), freq, channels, bps);
//		if (buffer)
//			if (alIsBuffer(buffer) == AL_TRUE)
//				alDeleteBuffers(1, &buffer);
//		
//		if (array)
//			delete []array;
//		
//		if (f)
//			OSBasics::close(f);
//		
//		throw (e);

}
