#include <iostream>
#include <string>
#include <cpprest/streams.h>

using namespace std;
using namespace utility;

enum icon_t {
	none,
	play,
	stop,
	pause,
	ff,
	rew,
	mute,
	idle=10,
};

void sanitize(string &incoming);
void set_title(string_t newtitle);
string get_title();
string get_fixedtitle();
void set_year(int newyear);
string get_year();
void set_icon(icon_t icon_val);
icon_t get_icon_val();
string get_icon(int index);
void set_mode(string_t new_mode);
string get_mode();
void set_playerid(int id);
int get_playerid();
void set_time(int rem_secs, int percentage, string_t time, string_t totaltime);
string get_time();
string get_shorttime();
string get_endtime();
void set_speed(int speed);
void set_episode_info(int season, int episode, string_t showtitle);
string get_episode_info();
int get_extra_info();
void set_tv_info(string_t channel, int channel_number, string_t show);
string get_tv_info();
void set_song_info(string_t artist, string_t album, int track);
string get_song_info();
string music_info();
string get_track();
void set_volume(bool muted, int vol);
int get_volume();

void show_stop();
void stop_time_timer();
void start_time_timer(int start_interval_secs);
void start_idle_timer();
void stop_timers();
void stop_idle_timer();