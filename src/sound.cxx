#include <SDL2/SDL.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "headers/engine.hxx"
#include <iostream>
#include <assert.h>

namespace CHL {
void play_s(uint32_t source) {
    alSourceStop(source);
    alSourcePlay(source);
}

void play_always_s(uint32_t source) {
    alSourcePause(source);
    alSourcei(source, AL_LOOPING, AL_TRUE);
    alSourcePlay(source);
}

void stop_s(uint32_t source) {
    alSourceStop(source);
}

void pause_s(uint32_t source) {
    alSourcePause(source);
}

void delete_source(uint32_t source) {
    alDeleteSources(1, &source);
}

uint32_t create_new_source(sound* s, instance* i) {
    ALuint source;
    alGenSources(1, &source);

    vec3 sourcePos(i->position.x, -i->position.y, -1);
    vec3 sourceVel(0, 0, 0);

    alSourcei(source, AL_BUFFER, s->al_buffer);
    alSourcef(source, AL_PITCH, 1.0f);
    alSourcef(source, AL_GAIN, 1.0f);
    alSource3f(source, AL_POSITION, sourcePos.x, sourcePos.y, sourcePos.z);
    alSource3f(source, AL_VELOCITY, sourceVel.x, sourceVel.y, sourcePos.z);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    return source;
}

sound::sound(const std::string& file)
    : device_id(0), buffer(nullptr), buffer_size(0) {
    if (!load(file))
        throw std::runtime_error("can't load sound");
}

bool sound::load(const std::string& file) {
    alGenBuffers(1, &al_buffer);
    alGenSources(1, &al_source);

    int error;

    SDL_AudioSpec wavspec;
    uint32_t wavlen;
    uint8_t* wavbuf;

    if (!SDL_LoadWAV(file.c_str(), &wavspec, &wavbuf, &wavlen))
        return false;

    // map wav header to openal format
    ALenum format;
    switch (wavspec.format) {
        case AUDIO_U8:
        case AUDIO_S8:
            format = wavspec.channels == 2 ? AL_FORMAT_STEREO8
                                           : AL_FORMAT_MONO8;
            break;
        case AUDIO_U16:
        case AUDIO_S16:
            format = wavspec.channels == 2 ? AL_FORMAT_STEREO16
                                           : AL_FORMAT_MONO16;
            break;
        default:
            SDL_FreeWAV(wavbuf);
            return CHL_FAILURE;
    }

    alBufferData(al_buffer, format, wavbuf, wavlen, wavspec.freq);
    if ((error = alGetError()) != AL_NO_ERROR) {
        std::cerr << "openal error: " << error << std::endl;
        return CHL_FAILURE;
    }

    SDL_FreeWAV(wavbuf);
    vec3 sourcePos(0, 0, -2);    // source position
    vec3 sourceVel(0, 0, 0);     // source velocity (used for doppler
    // effect)

    alSourcei(al_source, AL_BUFFER, al_buffer);
    alSourcef(al_source, AL_PITCH, 1.0f);
    alSourcef(al_source, AL_GAIN, 1.0f);
    alSource3f(al_source, AL_POSITION, sourcePos.x, sourcePos.y, sourcePos.z);
    alSource3f(al_source, AL_VELOCITY, sourceVel.x, sourceVel.y, sourcePos.z);
    alSourcei(al_source, AL_LOOPING, AL_FALSE);

    return true;
}
void sound::play() {
    play_s(al_source);
}

void sound::play_always() {
    play_always_s(al_source);
}

void sound::pause() {
    pause_s(al_source);
}

void sound::stop() {
    stop_s(al_source);
}

sound::~sound() {
    alDeleteBuffers(1, &al_buffer);
    alDeleteSources(1, &al_source);
}

}    // namespace CHL