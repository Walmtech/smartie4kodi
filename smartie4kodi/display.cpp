/*
smartie4kodi - A DLL to pass information to LCDSmartie

Copyright (c) 2022 Simon Walmsley

Forked from Kodi4Smartie - Copyright (C) 2016  Chris Vavruska

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "stdafx.h"

#include <string>
#include <ctime>
#include <iostream>
#include <regex>
#include <cpprest/json.h>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <mutex>
#include <map>
#include <regex>

#include "interface.hpp"
#include "logging.hpp"
#include "display.hpp"
#include "config.hpp"

using namespace std;
using namespace web;
using namespace utility;

icon_t icon = none;
string title = "";
string fixedtitle = "";
int secondsleft = 0;
int player_id = 0;
string_t mode;
string_t prev_mode;
string_t time_str;
string_t shorttime_str;
string_t episode_info;
string_t more_info;
string_t tv_info;
string_t artist_info;
static string prev_title;
string_t album_info;
int track_info;
int year;
HANDLE reset_timer = NULL;
HANDLE time_timer = NULL;
HANDLE idle_timer = NULL;
bool continue_timer;
std::mutex mtx;
int extra_info = 0;
int volume = 0;
bool muted;

typedef map<std::regex *, string> regex_map_t;
extern regex_map_t regex_map;

void CALLBACK reset_fired(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
void stop_reset_timer();

void sanitize(string &incoming)
{
	regex_map_t::iterator it;

	//execute each regex 
	for (it = regex_map.begin(); it != regex_map.end(); it++)
	{
		incoming = regex_replace(incoming, *it->first, it->second);
	}
}

void set_title(string_t newtitle)
{
	mtx.lock();

	title = string(newtitle.begin(), newtitle.end());

	if (title.length() > 0)
	{
		sanitize(title);

		//remove leading whitespace/cr/lf
		int pos = title.find_first_not_of("\n\r\t ");
		title = title.substr(pos);
		//if title = 
		fixedtitle = title;
	}
	//::log("title=%s", title.c_str());

	mtx.unlock();
}

string get_title()
{
	mtx.lock();
	string convert = title;
	mtx.unlock();

	return convert;
}

string get_fixedtitle()
{
	mtx.lock();
	string convert = fixedtitle;
	mtx.unlock();

	return convert;
}

void set_year(int newyear)
{
	mtx.lock();
	year = newyear;
	mtx.unlock();
}

string get_year()
{
	string formatedyear;
	mtx.lock();
	if (year != 0)
	{
		formatedyear = " (" + std::to_string(year) + ")";
	}
	else
	{
		formatedyear = "";
	}
	mtx.unlock();
	return formatedyear;
}

void set_icon(icon_t icon_val)
{
	mtx.lock();
	switch (icon_val)
	{
	case play:
	case stop:
	case pause:
	case ff:
	case rew:
	case idle:
		icon = icon_val;
		break;
	default:
		icon = none;
		break;
	}
	mtx.unlock();
}

icon_t get_icon_val()
{
	icon_t ret_val;

	mtx.lock();
	ret_val = icon;
	mtx.unlock();
	return ret_val;
}

string get_icon(int custom)
{
	static int cust[] = { 0, 176, 158, 131, 132, 133, 134, 135, 136 };
	string display = string("$CustomChar(") + std::to_string(custom);
	mtx.lock();

	icon_t icon_to_display = icon;
	if (muted == true)
	{
		icon_to_display = mute;
	}

	switch (icon_to_display)
	{
	case none:
	case idle:
		display = " ";
		break;
	case play:
		display += string(", 16, 24, 28, 30, 28, 24, 16, 0");
		break;
	case stop:
		display += string(", 14, 14, 14, 14, 14, 14, 14, 0");
		break;
	case pause:
		display += string(", 27, 27, 27, 27, 27, 27, 27, 0");
		break;
	case ff:
		display += string(", 0, 20, 26, 29, 29, 26, 20, 0");
		break;
	case rew:
		display += string(", 0, 5, 11, 23, 23, 11, 5, 0");
		break;
	case mute:
		display += string(", 8, 13, 10, 15, 11, 26, 12, 8");
		break;
	default:
		display += string(", 63, 0, 0, 0, 0, 0, 0, 63");
		break;
	}
	if (icon_to_display != none)
	{
		display += string(")$Chr(") + std::to_string(cust[custom]) + string(")");
	}
	mtx.unlock();
	return display;
}

void set_mode(string_t new_mode)
{
	mtx.lock();

	if (mode != new_mode)
	{
		mode = new_mode;
		log("Mode set to %ls", new_mode.c_str());
	}
	mtx.unlock();
}

string get_mode()
{
	mtx.lock();
	string convert = string(mode.begin(), mode.end());
	mtx.unlock();

	return convert;
}

void set_playerid(int id)
{
	mtx.lock();
	player_id = abs(id); //as we sometimes get -1 make it 1!
	mtx.unlock();
}

int get_playerid()
{
	mtx.lock();
	int ret_val = player_id;
	mtx.unlock();

	return ret_val;
}

string_t center(string_t in)
{
	if (in.length() < get_config(cLCD_WIDTH))
	{
		int mid = (get_config(cLCD_WIDTH) - in.length()) / 2;
		string_t spaces;
		for (int loop = 0; loop < mid; loop++)
		{
			spaces += string_t(U(" "));
		}
		return spaces + in;
	}
	return in;
}

void set_time(int remsecs, int percentage, string_t time, string_t totaltime)
{
	mtx.lock();
	if (get_config(cUSE_BARS))
	{
		int mode = get_config(cBAR_MODE);
		if (mode > 0)
		{
			int bars = (int)round((double)get_config(cLCD_WIDTH) * ((double)percentage / 100));
			time_str.clear();
			for (int x = 0; x < (int)get_config(cLCD_WIDTH); x++)
			{
				switch (mode)
				{
				default: // mode 1
					time_str.append((x < bars) ? string_t(U(">")) : string_t(U(" ")));
					break;
				case 2:
					time_str.append((x < bars) ? string_t(U(">")) : string_t(U("-")));
					break;
				case 3:
					time_str.append((x == bars) ? string_t(U(">")) : string_t(U("-")));
					break;
				}
			}
			bars = ((int)round((double)get_config(cLCD_WIDTH) - 2) * ((double)percentage / 100));
			shorttime_str.clear();
			for (int x = 0; x < ((int)get_config(cLCD_WIDTH) - 2); x++)
			{
				switch (mode)
				{
				default: // mode 1
					shorttime_str.append((x < bars) ? string_t(U(">")) : string_t(U(" ")));
					break;
				case 2:
					shorttime_str.append((x < bars) ? string_t(U(">")) : string_t(U("-")));
					break;
				case 3:
					shorttime_str.append((x == bars) ? string_t(U(">")) : string_t(U("-")));
					break;
				}
			}
		}
		else
		{
			time_str = string_t(U("$Bar(")) + std::to_wstring(percentage) + string_t(U(",100,")) +
				std::to_wstring(get_config(cLCD_WIDTH)) + string_t(U(")"));
			shorttime_str = string_t(U("$Bar(")) + std::to_wstring(percentage) + string_t(U(",100,")) +
				std::to_wstring((get_config(cLCD_WIDTH) - 2)) + string_t(U(")"));
		}
	}
	else
	{
		time_str = center(time + string_t(U("/")) + totaltime);
		shorttime_str = time + string_t(U("/")) + totaltime;

	}
	secondsleft = remsecs;
	mtx.unlock();
}

string get_time()
{
	mtx.lock();
	string convert = string(time_str.begin(), time_str.end());
	mtx.unlock();
	return convert;
}

string get_endtime()
{
	mtx.lock();
	string convert = (get_config_str(sENDAT));
	if (secondsleft < 1)
	{
		convert += " --:--";
		mtx.unlock();
		return convert;
	}
	int  qoutient, seconds, hours, minutes;
	struct tm time_now;
	time_t now = time(NULL);
	localtime_s(&time_now, &now);
	
	//Convert Total Seconds to Hours Minutes and Seconds
	qoutient = secondsleft / 60;
	seconds = (secondsleft % 60);
	minutes = (qoutient % 60);
	hours = (qoutient / 60);
	//Add on current time
	seconds += time_now.tm_sec;
	minutes += time_now.tm_min;
	hours += time_now.tm_hour;
	//Round seconds
	if (seconds > 30)
	{
		++minutes;
	}
	if (seconds > 90)
	{
		++minutes;
	}
	//Handle Minutes 
	if (minutes > 59)
	{
		++hours;
		minutes -= 60;
	}
	string mins_str = std::to_string(minutes);
	if (minutes < 10) //pad
	{
		mins_str = "0" + mins_str;
	}
	//Handle Hours
	if (hours > 23)
	{
		hours -= 24;
	}
	if (hours > 11)
	{
		hours -= 12;
		mins_str += " PM";
	}
	else
	{
		mins_str += " AM";
	}
	if (hours == 0)
	{
		hours = 12;
	}
	convert += " " + std::to_string(hours) + ":" + mins_str;
	mtx.unlock();
	return convert;
}

string get_shorttime()
{
	mtx.lock();
	if (prev_title.compare("") != 0)
	{
		shorttime_str = string_t(utility::conversions::to_string_t(title));
	}
	string convert = string(shorttime_str.begin(), shorttime_str.end());
	mtx.unlock();
	return convert;
}


void set_speed(int speedval)
{
	stop_time_timer();
	mtx.lock();
	string speedtext;

	if (speedval == 1) 
	{
		icon = play;
		if (prev_title.compare("") != 0)
		{
			title = prev_title;
			prev_title = "";
		}
		else
		{
			shorttime_str = string_t(utility::conversions::to_string_t(get_config_str(sRESUME)));
		}
	}
	else if (speedval == 0)
	{
		icon = pause;
		shorttime_str = string_t(utility::conversions::to_string_t(get_config_str(sPAUSE)));
	}
	else
	{
		icon = rew;
		speedtext = string(get_config_str(sREWIND));
		if (speedval > 1)
		{
			icon = ff;
			speedtext = string(get_config_str(sFF));
		}
		speedtext += string(" ") + std::to_string(abs(speedval)) + string("X");
		if (prev_title.compare("") == 0)
		{
			prev_title = title; //Store title if not done already
		}
		title = speedtext;
	}
	mtx.unlock();
	start_time_timer(1);
}

void set_volume(bool mute, int vol)
{
	mtx.lock();
	if (muted != mute)
	{ 
		muted = mute;
		volume = vol;
		mtx.unlock();
	}
	else
	{
		volume = vol;
		stop_reset_timer();
		if (mode.compare(U("volume")) != 0)
		{
			prev_mode = mode;
		}
		mtx.unlock();
		set_mode(U("volume"));
		CreateTimerQueueTimer(&reset_timer, NULL, reset_fired, NULL, get_config(cRESET_DELAY), 0, 0);
	}
}

int get_volume()
{
	mtx.lock();
	int vol = volume;
	mtx.unlock();

	return vol;
}

void set_episode_info(int season, int episode, string_t showtitle)
{
	mtx.lock();
	extra_info = 1;
	string_t season_str = string_t(U("0") + std::to_wstring(season)).substr(1, 2);
	string_t episode_str = string_t(U("0") + std::to_wstring(episode)).substr(1, 2);
	episode_info = U("S") + season_str + U("E") + episode_str + U(": ") + showtitle;
	mtx.unlock();
}

string get_episode_info()
{
	string convert;
	mtx.lock();
	convert = string(episode_info.begin(), episode_info.end());
	mtx.unlock();

	return convert;
}

int get_extra_info()
{
	mtx.lock();
	int ret_val = extra_info;
	mtx.unlock();
	return extra_info;
}

void set_tv_info(string_t channel, int channel_number, string_t show)
{
	set_title(show);
	mtx.lock();
	extra_info = 1;
	tv_info = string_t(utility::conversions::to_string_t(get_config_str(sCHANNEL))) + string_t(U(" ")) + std::to_wstring(channel_number) + string_t(U(" - ")) + channel;
	mtx.unlock();
}

string get_tv_info()
{
	string convert;
	mtx.lock();
	convert = string(tv_info.begin(), tv_info.end());
	mtx.unlock();
	return convert;
}

void set_song_info(string_t artist, string_t album, int track)
{
	mtx.lock();
	extra_info = 7;
	artist_info = artist;
	album_info = album;
	track_info = track;
	mtx.unlock();
}

string get_song_info()
{
	//return different information based on extra_info count
	string convert;
	mtx.lock();
	if (extra_info > 6)
	{
		convert = string(artist_info.begin(), artist_info.end());
	}
	else if (extra_info > 3)
	{
		convert = string(album_info.begin(), album_info.end());
	}
	else
	{
		if (track_info)
		{
			convert = string(get_config_str(sTRACK)) + std::to_string(track_info);
		}
		else
		{
			//if no track then display the album for an extra cycle
			convert = string(album_info.begin(), album_info.end());
			extra_info = 0;
		}
	}
	mtx.unlock();

	return string("$Center(") + convert + string(")");
}

string get_album()
{
	//return different information based on extra_info count
	string convert;
	mtx.lock();
	convert = string(album_info.begin(), album_info.end());
	mtx.unlock();
	return convert;
}

string get_artist()
{
	//return different information based on extra_info count
	string convert;
	mtx.lock();
	convert = string(artist_info.begin(), artist_info.end());
	mtx.unlock();
	return convert;
}

string get_track()
{
	//return different information based on extra_info count
	string convert;
	mtx.lock();
	if (track_info)
	{
		convert = std::to_string(track_info) + ": ";
	}
	mtx.unlock();
	return convert;
}

void CALLBACK reset_fired(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	mtx.lock();
	if (icon == stop)
	{
		reset_timer = NULL;
		icon = none;
		title = "";
		clear_shorttime();
		mode = U("");
	}
	if (mode.compare(U("volume")) == 0)
	{
		reset_timer = NULL;
		mode = prev_mode;
	}
	mtx.unlock();

	if (get_icon_val() == none)
	{
		start_idle_timer();
	}
}

void stop_reset_timer()
{
	if (reset_timer)
	{
		DeleteTimerQueueTimer(NULL, reset_timer, NULL);
		reset_timer = NULL;
	}
}

void show_stop()
{
	secondsleft = 0;
	stop_time_timer();
	set_icon(stop);
	shorttime_str = string_t(utility::conversions::to_string_t(get_config_str(sSTOP)));
	title = string(get_config_str(sSTOP));
	//set_title(string_t(utility::conversions::to_string_t(get_config_str(sSTOP))));
	
	CreateTimerQueueTimer(&reset_timer, NULL, reset_fired, NULL, get_config(cRESET_DELAY), 0, 0);
}

void clear_shorttime()
{
	shorttime_str = string_t(utility::conversions::to_string_t(""));
}

void CALLBACK update_time(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	try 
	{
		get_time_properties_from_player(get_playerid());
		mtx.lock();
		if (extra_info > 0)
		{
			extra_info--;
		}
		mtx.unlock();
	}
	catch (...)
	{

	}
}

void start_time_timer(int start_interval_secs)
{
	stop_time_timer();
	CreateTimerQueueTimer(&time_timer, NULL, update_time, NULL, start_interval_secs*1000, 1000, 0);
}

void stop_time_timer()
{
	//log("stop time timer");
	if (time_timer)
	{
		DeleteTimerQueueTimer(NULL, time_timer, INVALID_HANDLE_VALUE);
		time_timer = NULL;
	}
}

void stop_idle_timer()
{
	if (idle_timer)
	{
		DeleteTimerQueueTimer(NULL, idle_timer, INVALID_HANDLE_VALUE);
		idle_timer = NULL;
		::log("Idle timer:stop");
	}
}

void CALLBACK idle_handler(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	set_icon(idle);
	::log("Idle timer:fire");
}

void start_idle_timer()
{
	if (get_config(CIDLE_TIMER))
	{
		set_icon(none);
		stop_idle_timer();
		CreateTimerQueueTimer(&idle_timer, NULL, idle_handler, NULL, get_config(CIDLE_TIMER) * 1000, 0, 0);
		::log("Idle timer:start %d", get_config(CIDLE_TIMER));
	}
}

void stop_timers()
{
	stop_reset_timer();
	stop_time_timer();
}