/*
sound.cpp: sound handling class.
 * By 2MB Solutions: https//2mb.solutions
 * Released under the MIT license. See license.txt for details.
 * Billy: https://stormdragon.tk/
 * Michael: https://michaeltaboada.me
 * */
#include "sound.h"
int sound::sounds=0;
ALLEGRO_VOICE* sound::voice = NULL;
map<string, ALLEGRO_MIXER*> sound::mixers;
map<string, ALLEGRO_SAMPLE*> sound::samples;

sound::sound(void)
{
    if(!al_is_audio_installed())
        {
            al_install_audio();
            al_init_acodec_addon();
        }
    sounds++;
    si = NULL;
    looping = false;
    panval=0;
    gainval=0;
    pitchval=100;
    paused=false;
    active=false;
    if(mixers.find("default")==mixers.end())
        {
            ALLEGRO_MIXER* m = NULL;
            m = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
            if(!m)
                {
                    log("Default mixer could not be created!\n");
                    return;
                }
            mixers.insert(pair<string, ALLEGRO_MIXER*>("default", m));
            if(voice)
                {
                    if(!al_attach_mixer_to_voice(mixers.find("default")->second, voice))
                        {
                            log("Could not attach default mixer to default voice!\n");
                        }
                }
        }
    if(!voice)
        {
            ALLEGRO_VOICE* v = NULL;
            v = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
            if(!v)
                {
                    log("Default voice could not be created!\n");
                    return;
                }
            voice=v;
            if(!al_attach_mixer_to_voice(mixers.find("default")->second, voice))
                {
                    log("Could not attach default mixer to default voice!\n");
                    return;
                }
        }
}


sound::~sound(void)
{
    sounds--;
    stop();
    if(si)
        {
            al_destroy_sample_instance(si);
        }
    if(sounds==0)
        {
            map<string, ALLEGRO_SAMPLE*>::iterator it= samples.begin();
            while(it!=samples.end())
                {
                    al_destroy_sample(it->second);
                    samples.erase(it++);
                }
            map<string, ALLEGRO_MIXER*>::iterator it2= mixers.begin();
            while(it2!=mixers.end())
                {
                    al_destroy_mixer(it2->second);
                    mixers.erase(it2++);
                }
            if(voice)
                {
                    al_destroy_voice(voice);
                    voice=NULL;
                }
        }
}

void sound::log(string s)
{
//	printf(s.c_str());
}

/**
*Stop the sound.
*@return Whether the sound was successfully stopped.
**/
bool sound::stop()
{
    if(si)
        {
            return al_stop_sample_instance(si);
        }
    else
        {
            return false;
        }
    return false;
}

/**
*Load a file into the sound.
*@return Whether the sound was successfully loaded.
**/
bool sound::load(string file/**< [in] The file to load.**/, string mixer/**< [in] The mixer name to put  the sound on, can be excluded, which adds it to the default mixer.**/)
{
    if(mixers.find(mixer)==mixers.end())
        {
            ALLEGRO_MIXER* m = NULL;
            m = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
            if(!m)
                {
                    log("Could not create mixer "+mixer+"!\n");
                    return false;
                }
            mixers.insert(pair<string, ALLEGRO_MIXER*>(mixer, m));
        }
    if(!voice)
        {
            ALLEGRO_VOICE* v = NULL;
            v=al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
            if(!v)
                {
                    log("Could not create default voice!\n");
                    return false;
                }
            voice=v;
            if(!al_attach_mixer_to_voice(mixers.find(mixer)->second, voice))
                {
                    log("Could not attach mixer "+mixer+" to default voice!\n");
                    return false;
                }
        }
    if(samples.find(file)==samples.end())
        {
            ALLEGRO_SAMPLE* s=NULL;
            s = al_load_sample(((string)(al_path_cstr(al_get_standard_path(ALLEGRO_RESOURCES_PATH), ALLEGRO_NATIVE_PATH_SEP))+file).c_str());
            if(!s)
                {
                    log("Could not load file "+file+"!\n");
                    return false;
                }
            samples.insert(pair<string, ALLEGRO_SAMPLE*>(file, s));
        }
    if(si)
        {
            al_destroy_sample_instance(si);
        }
    si = al_create_sample_instance(samples.find(file)->second);
    if(!si)
        {
            log("could not create sample instance!\n");
            return false;
        }
    if(!al_attach_sample_instance_to_mixer(si, mixers.find(mixer)->second))
        {
            log("Could not attach the sample instance to the "+mixer+" mixer!\n");
            return false;
        }
    active=true;
    return true;
}

double sound::dB2lin( double dB )
{
    static const double DB_2_LOG = 0.11512925464970228420089957273422; // ln( 10 ) / 20
    return exp( dB * DB_2_LOG );
}

double sound::pan(double doubleval)
{
    float new_pan=doubleval;
    if(new_pan<-100)
        new_pan=-100;
    if(new_pan>100)
        new_pan=100;
    new_pan=0-(fabs(new_pan));
    new_pan=dB2lin(new_pan);
    new_pan=1.0-new_pan;
    if(doubleval<0)
        new_pan=0-new_pan;
    else
        new_pan=fabs(new_pan);
    if(new_pan<-1)
        new_pan=-1;
    if(new_pan>1)
        new_pan=1;
    return new_pan;
}

/**
*Gets the pan.
*@return The pan in decibel.
**/
double sound::get_pan()
{
    return panval;
}

/**
*Sets the pan in decibel.
*@return Whether the pan was successfully set.
**/
bool sound::set_pan(double pan_value/**< [in] The pan in decibel.**/)
{
    if(pan_value < -100)
        {
            pan_value = -100;
        }
    else if(pan_value >100)
        {
            pan_value=100;
        }
    double linpan = pan(pan_value);
    if(si)
        {
            if(!al_set_sample_instance_pan(si, linpan))
                {
                    return false;
                }
        }
    else
        {
            return false;
        }
    panval=pan_value;
    return true;
}

/**
*Check the gain of the sound.
*@return The gain.
**/
double sound::get_gain()
{
    return gainval;
}

/**
*Set the gain of the sound in decibel.
*@return Whether the gain was successfully set.
**/
bool sound::set_gain(double gain/**< [in] The gain in decibel.**/)
{
    if(gain < -100)
        {
            gain = -100;
        }
    else if(gain > 0)
        {
            gain = 0;
        }
    if(si)
        {
            double new_gain = dB2lin(gain);
#ifdef __gnu_linux__
            if(al_get_channel_count(al_get_sample_instance_channels(si)) == 1)
                {
                    new_gain = new_gain*sqrt(2);
                }
#endif
            if(!al_set_sample_instance_gain(si, new_gain))
                {
                    return false;
                }
        }
    else
        {
            return false;
        }
    gainval=gain;
    return true;
}

/**
*Get the pitch in decibel.
*@return The pitch.
**/
double sound::get_pitch()
{
    return pitchval;
}

/**
*Set the pitch.
*@return Whether the pitch was successfully set.
**/
bool sound::set_pitch(double pitch/**< [in] The pitch in decibel.**/)
{
    if(pitch < 1)
        {
            pitch=1;
        }
    if(si)
        {
            if(!al_set_sample_instance_speed(si, pitch/100.0))
                {
                    return false;
                }
        }
    else
        {
            return false;
        }
    pitchval=pitch;
    return true;
}

/**
*Check whether the sound loops.
*@return Whether hte file loops.
**/
bool sound::get_loop()
{
    return looping;
}

/**
*Set the file to loop or not.
*@return Whether the looping value was successfully set.
**/
bool sound::set_loop(bool loop/**< [in] Should the file loop?**/)
{
    if(si)
        {
            if(loop)
                {
                    if(!al_set_sample_instance_playmode(si, ALLEGRO_PLAYMODE_LOOP))
                        {
                            return false;
                        }
                }
            else
                {
                    if(!al_set_sample_instance_playmode(si, ALLEGRO_PLAYMODE_ONCE))
                        {
                            return false;
                        }
                }
        }
    else
        {
            return false;
        }
    looping=loop;
    return true;
}

/**
*Play the file.
*@return Whether the file was successfully started playing.
**/
bool sound::play()
{
    if(si)
        {
            if(paused)
                {
                    paused = !al_set_sample_instance_playing(si, true);
                    return !paused;
                }
            bool p =  al_play_sample_instance(si);
            this->set_gain(gainval);
            return p;
        }
    else
        {
            return false;
        }
    return false;
}

/**
*Check whether the sound is playing.
*@return Whether the sound is playing.
**/
bool sound::is_playing()
{
    if(si)
        {
            return al_get_sample_instance_playing(si);
        }
    else
        {
            return false;
        }
    return false;
}

/**
*Pause the sound.
*@return Whether the sound is paused.
**/
bool sound::pause()
{
    if(si)
        {
            if(paused)
                {
                    return true;
                }
            else
                {
                    paused = !al_set_sample_instance_playing(si, false);
                    return !paused;
                }
        }
    else
        {
            return false;
        }
    return false;
}

/**
*Check whether the sound is paused.
*@return Whether the sound is paused.
**/
bool sound::get_paused()
{
    return paused;
}

/**
*Check whether the sound has a file loaded into it.
*@return Whether the sound is active.
**/
bool sound::get_active()
{
    return active;
}
