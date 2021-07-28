
KeePerso Plugin: Plugin for KeePass2 - authentication with German Personalausweis. This plugin generates a hash-key from personalausweis-data to use with Masterpassword for a two-factor-authentication
Version: 0.7
Author: buergerservice.org e.V. <KeePerso@buergerservice.org>
Language: English


-------------
requirements:
-------------
KeePass2,
Plugin KeePerso (this plugin is not for KeePass1)
a 64bit Computer cause this Plugin is 64bit,
Visual C++ Redistributable for Visual Studio 2015/2017/2019,
AusweisApp2
cardreader (maybe you can use a new Android Handy as cardreader - connect in AusweisApp2)
for online identification ready Personalausweis - you can test it in AusweisApp2 with "Meine Daten einsehen"
internetaccess


-------------
installation:
-------------
please read "Plugin Installation and Uninstallation" in KeePass Help Center:
https://keepass.info/help/v2/plugins.html


-----------
how to use:
-----------
install Plugin - see part installation above
check in KeePass2 -> Menu -> Tools -> Plugins if Plugin KeePerso is shown
connect your cardreader and put Personalausweis on it
start AusweisApp2 if not already always running in background
start KeePass2

New Database:
KeePass2 -> Menu -> File -> New, ok, enter filename for database
in the next window "Create Master Key" enter Masterpassword, select "Show expert option" and "Key file / provider"
as keyprovider select in pulldownmenu KeePerso KeyProvider
you always should use a Masterpassword and if you want you can add the windows user account.
if you press ok youre asked to check if AusweisApp2, cardreader and Personalausweis are ready then press ok.
if the key is successful generated you get a window that says the key is ready. press ok.
in the window "Database Settings" you can give a name and other things and press ok.
next window is "Emergency Sheet", the database is ready.
you can store your entries in the database

When database exists:
when you enter again KeePass2, enter Masterpassword 
and you first have to produce again key
if you press ok youre asked to check if AusweisApp2, cardreader and Personalausweis are ready then press ok.
if the key is successful generated you get a window that says the key is ready. press ok.



----------------------------
known problems and questions
----------------------------
if i start KeePass i get errormessage from KeePass
	- check if your computer is 64 bit, cause the plugin is 64bit ->  start systeminformation
	- Install the Visual C++ Redistributable for Visual Studio 2015/2017/2019 on all systems where you want to use the plugin

when i start KeePerso in KeePass2, i get message "please check AusweisApp2, cardreader and Personalausweis. exiting plugin"
	- is AusweisApp2 running ?
	- is AusweisApp2 working for example with "Meine Daten einsehen"? Then cancel that
	- is cardreader connected ?
	- is Personalausweis on cardreader?
	- maybe disable firewall/viruswall/webfilter for test

is my PIN safe?
	- the PIN is only sent to AusweisApp2 and not stored. you can use a cardreader with keypad, then the plugin cant see the PIN.

i have problems with KeePass2
	- look KeePass helpcenter 
	https://keepass.info/help/base/index.html
	https://keepass.info/help/base/faq_tech.html

what data of my Personalausweis is used for the key?
	like you can see in the source used are this data
	FamilyNames
	GivenNames
	DateOfBirth
	PlaceOfBirth

is an attackscenario possible where someone takes the source and builds a new plugin where the data are not read
from Personalausweis and the attacker writes them direct in the code and generates the key?
	yes theoretical, but first he also must know the masterpassword and second if the user keeps his pc safe noone can install
	a attackerplugin or copy the database


---------------
versionhistory:
---------------
0.7 better workflow, new errormessage
0.6.5 german umlauts in personaldata corrected
0.6.4 new info for certificate (in menu) and personaldata, better workflow
0.6.3 changed workflow for PIN
0.6.2 changed workflowtest for card and retrycounter 
0.6.1 changed workflow for more than one cardreaders
0.6 new icon errorwindows, check retrycounter perso, new infowindow for running workflow
0.5.1 improvements errorcheck
0.5 check AusweisApp2-version
0.4 changed usability KeyProvider, test if key existing
0.3 usability, test if cardreader owns a keypad
0.2 test if cardreader and personalausweis are ready
0.1 start pilotversion


-----
build
-----
source was build with Visual Studio 2019
for building put KeePass-Portable in the KeePass-directory
for workflow.cpp /clr is switched off
libraries boost, openssl, jsoncpp


-------
license
-------
/*
  KeePerso - A Plugin for german Personalausweis
  Copyright (C) 2021 buergerservice.org e.V. <KeePerso@buergerservice.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
