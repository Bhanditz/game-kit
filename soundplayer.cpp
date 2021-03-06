/*
*soundplayer.cpp: Many helpful sound functions.
By 2MB Solutions: https//2mb.solutions
Released under the MIT license. See license.txt for details.
Billy: https://stormdragon.tk/
Michael: https://michaeltaboada.me
*/
#include <allegro5/allegro.h>
#include "dynamic_menu.h"
#include <keyboard.h>
#include <sound.h>
#include <string>
#include <misc.h>
#include <menu_helper.h>
#include "soundplayer.h"



int power_bar(string soundstring, int p, int min, int max, float wait)
{
    sound s;
    if(!s.load(soundstring))
        {
            log("Could not load power bar sound.\n");
            return -100;
        }
    s.set_pitch(min);
    s.set_loop(true);
    if(!s.play())
        {
            log("Could not play power bar sound.\n");
            return -100;
        }
    if(p > min-1)
        {
            for(int x = min; x <= p; x++)
                {
                    al_rest(wait);
                    s.set_pitch(s.get_pitch()+1);
                }
        }
    else
        {
            keyboard kb;
            int dir = 1;
            while(!kb.key_pressed(ALLEGRO_KEY_SPACE))
                {
                    if(kb.key_pressed(ALLEGRO_KEY_ESCAPE) || ((kb.key_down(ALLEGRO_KEY_ALT) || kb.key_down(ALLEGRO_KEY_ALTGR)) && kb.key_pressed(ALLEGRO_KEY_F4)))
                        {
                            return -100;
                        }
                    if (kb.key_pressed(ALLEGRO_KEY_BACKSPACE))
                        {
                            return -101;
                        }
                    al_rest(wait);
                    s.set_pitch(s.get_pitch()+dir);
                    if(s.get_pitch() >= max)
                        {
                            dir = -1;
                        }
                    else if(s.get_pitch() <= min)
                        {
                            dir = 1;
                        }
                }
        }
    s.stop();
    return s.get_pitch();
}

int direction_bar(string soundstring, int d, int min, int max, float wait)
{
    sound s;
    if(!s.load(soundstring))
        {
            log("Could not load direction bar sound.\n");
            return -100;
        }
    s.set_pan(min);
    s.set_loop(true);
    if(!s.play())
        {
            log("Could not play direction bar sound.\n");
            return -100;
        }
    if(d >= min && d <= max)
        {
            for (int x = min; x <= d; x++)
                {
                    al_rest(wait);
                    s.set_pan(x);
                }
        }
    else
        {
            keyboard kb;
            int dir = 1;
            while(!kb.key_pressed(ALLEGRO_KEY_SPACE))
                {
                    if(kb.key_pressed(ALLEGRO_KEY_ESCAPE) || ((kb.key_down(ALLEGRO_KEY_ALT) || kb.key_down(ALLEGRO_KEY_ALTGR)) && kb.key_pressed(ALLEGRO_KEY_F4)))
                        {
                            return -100;
                        }
                    if(kb.key_pressed(ALLEGRO_KEY_BACKSPACE))
                        {
                            return -101;
                        }
                    al_rest(wait);
                    s.set_pan(s.get_pan()+dir);
                    if(s.get_pan() >= max)
                        {
                            dir = -1;
                        }
                    else if(s.get_pan() <= min)
                        {
                            dir = 1;
                        }
                }
        }
    s.stop();
    return s.get_pan();
}

void fade(sound* s, int start, int end, float time)
{
    if(!s)
        {
            return;
        }
    s->set_gain(start);
    for(int x = 0; x < time/0.005; x++)
        {
            al_rest(0.005);
            s->set_gain(s->get_gain()+((end-start)/(time/0.005)+start));
        }
    s->stop();
}

bool play_sound_wait(sound* s)
{
    if(!s->play())
        {
            log("Could not play sound file.\n");
            return false;
        }
    while(s->is_playing())
        {
            al_rest(0.005);
        }
    return true;
}

bool play_sound_wait(string s)
{
    sound so;
    if(!so.load(s))
        {
            log(((string)("Could not load sound ")+s+".\n").c_str());
            return false;
        }
    else
        {
            return play_sound_wait(&so);
        }
}

void learn_sounds()
{
    vector<string>* vec = get_dir_children("sounds", 1);
    vector<string> real_items;
    if (vec)
        {
            for (unsigned int x = 0; x < vec->size(); x++)
                {
                    if((*vec)[x].find("-music") == string::npos)
                        {
                            string temp = (*vec)[x];
                            string::size_type pos = temp.find("-");
                            while(pos != string::npos)
                                {
                                    temp.replace(pos, 1, " ");
                                    pos = temp.find("-", pos+1);
                                }
                            temp.replace(temp.find("."), string::npos, "");
                            real_items.push_back(temp);
                        }
                    else
                        {
                            vec->erase(vec->begin()+x);
                            x--;
                        }
                }
            real_items.push_back("back to main menu");
            dynamic_menu* menu = create_menu(real_items, vector<string>());
            int ran = menu->run_extended("", "Use your arrow keys to navigate the menu, and enter to play the sound. Pressing space while the sound is playing will stop it.", 1, true);
            keyboard kb;
            while(ran != -1 && ran != 0 && ran != (int)(real_items.size()))
                {
                    sound s;
                    s.load((string)("sounds/")+(*vec)[ran-1]);
                    s.play();
                    while(s.is_playing())
                        {
                            if(kb.key_pressed(ALLEGRO_KEY_SPACE))
                                {
                                    break;
                                }
                        }
                    s.stop();
                    ran = menu->run_extended("", "", ran, false);
                }
            delete vec;
            delete menu;
        }
    else
        {
            log("Learn game sounds: the vector returned was NULL!\n");
        }
}

