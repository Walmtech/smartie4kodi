# smartie4kodi
Kodi support for LCD Smartie

Fork of orginal Kodi4Smartie to setup some new features for my own needs.
Moved source from Nuget to vcpkg and retargeted to Visual Studio 2022.
Removed the Test section I wasn't using. 
Renamed to follow Kodi Trademark rules.

Currently V1.03 added extra dll calls for features I wanted fixed Function 12 If Kodi Closed and Reopened.

![](/docs/smartie4kodi1.png)

SMARTIE4KODI is a DLL for LCD Smartie ([http://lcdsmartie.sourceforge.net/](http://lcdsmartie.sourceforge.net/)) that allows information about what is playing in Kodi ([http://kodi.tv](http://kodi.tv)) to be displayed on an LCD Character display. See the LCD Smartie web site for supported devices.


## My Setup
Windows 11 x64<br>
LCD Smartie 5.5 https://github.com/stokie-ant/lcdsmartie-laz<br>
Kodi v19.3 (Matrix)<br>
2x16 Large OLED Display SOC1602B (WS0010 compatible) controller<br>
by Ardunio Nano Backpack running matrix orbital emulation code<br>

Smartie4kodi uses the depreciated C++ REST SDK ([https://github.com/Microsoft/cpprestsdk/](https://github.com/Microsoft/cpprestsdk)).

You will need the MS C++ Resistrubutable installed https://aka.ms/vs/17/release/vc_redist.x86.exe

