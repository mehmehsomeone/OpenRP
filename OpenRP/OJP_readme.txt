==============================
Open Jedi Project (OJP) Readme
==============================

Basic Release Version:  0.0.4
Enhanced Release Version: 0.0.1
Vehicles Release Version:  0.0.2
Skins Release Version: 0.0.1

Stuff in this Release:  


JKA Version Required: 1.01

NOTE:  This readme might not be part of an official OJP release since this readme is required to be included in all OJP based projects.  If this isn't an official OJP release, this document will only partially apply.


========================
0000 - Table of Contents
========================

	0000..................Table of Contents
	0001..................Introduction
	0002..................Installation
	0003..................What's New?
	0004..................Features
	     0004.1...........Basic
	     0004.2...........Enhanced
	     0004.3...........Vehicles
	0005..................Using Our Work
	0006..................Submitting Stuff to OJP
	     0006.1...........Submission Guidelines
	0007..................Known Issues
	     0007.1...........Issues with Basic Features
	0008..................Credits
	0009..................Contact Information
	0010..................Legal Stuff

	
	
===================
0001 - Introduction
===================

The Open Jedi Project is a coding/modding collaboration with the intent of maximizing the features and fun factor for all Jedi Knight Academy (JKA) mods.  We work together by contributing fun, interesting, and useful game features so that everyone can benefit.

We operate on what's basically an open source system. Open source basically means that the source code is freely available and accessible by all. See the "Using Our Work" section for details about rights and permissions.

Our design philosophy is to make everything as separated and customizable as possible to allow developers and players to choose what features they wish to use. 

We currently have two distributions (gameplay/code changes), Basic and Enhanced. We also have two packs (non-gameplay related), Vehicles and Skins.

Basic has two main features. One is bug fixes and balance fixes, neither which will severely alter game play. They are designed to be the "unofficial patch" for bugs and game play problems. Basic can still be considered vanilla Jedi Academy. The other main feature is map enhancements. Things such as new entities, expanded AI, vehicles, scripting and effects system can allow mappers to create far more immersive and fun maps. Since Basic also aims towards recreating all the single player entities and code, it is possible for mappers to create full featured cooperative and single player games and levels using the multiplayer engine. This allows modders to make other enhancements such as new weapons, AI, etc., not possible using the single player engine. 

Enhanced is a superset of Basic, meaning it has anything and everything included in the Basic distribution. The difference is that it adds many significant gameplay alterations. It is a playable mod, but also a code base for other developers. To decrease the possibility of mods based off Enhanced loosing originality, we are keeping our game play feature list generic and flexible. New features will be generic, expandable, and flexible so other developers can easily adopt them to their mods. It won't venture out side of Jedi Academy's principle game play, so there won't be anything that makes this drastically unique in terms of pure game play. Players get a full mod that shares the same basic principles and ideas that stock Jedi Academy offers. Developers get a solid, flexible code base that includes the basic, fundamental features you would want to find in most mods, allowing them to spend their time on what makes their work truly unique. An example of a feature for Enhanced would be an extended version of the player class system seen in Siege, one that is available in more game types, and is far more flexible.

The Vehicles package is a collection of additional vehicles for JKA.  We're including them in the project to allow OJP compatible maps to use additional vehicles without having to include said vehicles in their release packages.

The Skins package is a collection of quality modder created skins that have been turned 
into customizable player classes.  We've done this to allow more player options, to 
allow SP useage of these skins, to allow servers and players to quickly get a pack of 
quality skins, and to prevent a lot of redundent data from clogging up your game 
directory.  

We have a web forum set up for OJP.  You can find it at...
http://www.lucasforums.com/forumdisplay.php?s=&forumid=542

We have a website pending.  Please be patient.



===================
0002 - Installation
===================

To Install:

Just unzip this file to your jedi Academy directory.  At this point, each new version of OJP components replaces the older versions (of that distribution/pack).  We've made them have sequencial file names to prevent issues with same file names so you shouldn't HAVE to delete the older version (it's just recommended).  

The exception is with OJP Basic versions of 0.0.3 or lower.  Those, must be deleted for things to work correctly.

To Run:

	Basic:
		Run from your Setup->Mods Menu.

	Enhanced:
		Run from your Setup->Mods Menu.

	Vehicles:
		Automatic

	Skins:
		Automatic

To Uninstall:

	Basic:
		Delete your /GameData/ojpbasic directory.
	
	Enhanced:
		Delete your /GameData/ojpenhanced directory.
		
	Vehicles:
		Delete the ojp_vehicles_002 from your /GameData/base directory.

	Skins:
		Delete the ojp_skins_001 from your /GameData/base directory.



==================
0003 - What's New?
==================

What's New? has been moved to OJP_changelog.txt


		
===============
0004 - Features
===============


--------------
0004.1 - Basic
--------------

	Extra Vehicle Implementation (VEHICLE_IMP)
	Multiplayer, Vehicles

	What It Does:

		Allows vehicle creators more flexability to set up their vehicle behaviours. These features were added specifically to support the Droideka vehicle, but these features can also be used by any VH_WALKER class vehicle. Some of the features are also useable by any vehicle of any class.
		See OJP_vehicles.txt for full details of the new options avaliable.


	Ambient Player Sounds (AMBIENTEV)
	Multiplayer, Sounds, Animevents

	What It Does:

		Players now have the ability to setup two fully customizable ambient sounds (Vader's resporator breathing, sensor ping, breathing, whatever you want) thru use of a new animevent type.  See ojp_readme.txt and ojp_animevents.txt for details.


	Animevents System Overhaul (ANIMEVENTS)
	Multiplayer, Sounds, Animations, Animevents

	What It Does: 

		The animation events system controls how most of the player sound effects are controlled in JKA.  OJP has heavily modified the system to add back in SP animevent features that were removed for MP and add additional functionally.  Changes include:

	- Ambient Sound Support (See Ambient Player Sounds)   

	- Dynamic/Custom Sounds:
		A part of the SP animevents.cfgs, this support was missing from MP until now.  These are sounds that are player model dependant.  IE, Vader's pain sounds are different than Jan's.

	- Per-Model animevents.cfg:
		You can use per-model animevents.cfgs.  This means that you can personalize each person to have individualized sound effects and even special event!  A good example what you can do with this would be to make a certain character scream before certain attacks or have their spine crackle from hard impacts.

See OJP_readme.txt and OJP_animevents.txt for details on how to customize/use animevents.cfgs.


	Bug Fix 001 (BUGFIX1) - Animation.cfg fix for BOTH_WALKBACK2
	Single Player, Animations

	What It Does:
		Fixes an animation bug seen when walking backwards with an ignited single saber in single player by correcting the number of frames
	
	Special Notes:
		This fix will be obsolete once a game patch comes out.  Remember to remove this file before you patch the game.
		Might cause problems when playing multiplayer.  If you have any problems, just move the file out of the /base directory. 

	Bug Fix 002 (BUGFIX2)
	Multiplayer, Animations

	What It Does:
		Fixes a major issue with the animation timer calculation in MP.  The animation timers are now calculated correctly and this should help with animations that play in Force Rage, Force Speed, or at odd saber animation speeds.


	Duel Taunts in all Gametypes (ALLTAUNTS)
	Multiplayer, Emotes

	What It Does:
	Duel taunts now work in all gametypes.  These can be setup using theControls menu or by using the following commands (same as the original duel commands):

	/bow - "Show respect to your opponent."
	/meditate - "Sit and wait patiently for your opponent to make the first move."
	/flourish - "Show off a little."
	/gloat - "Celebrate your victory over your opponent."


	Hidden JKA Saber Hilts (HSABERS)
	Multiplayer

	What It Does:
		Adds to multiplayer the Retribution (Desann's), the Skywalker (Luke Skywalker's Saber), and the Stinger (default Reborn's) saber hilts.  These hilts come with the game but are hidden by baseJKA.


	Hidden Lannik Racto Skin
	Multiplayer
	
	What It Does:
		Makes the Lannik Racto skin selectable in the MP skins menu.  Also created icon for said skin.


	Match Warm Ups (FIXWARMUP)
	Multiplayer, Server

	What It Does:
		You can now set a warm up period before the beginning of all non-duel gametypes.  You can find options for this in the create server menu under the advanced tab.  Also see OJP_cvars.txt for details on the cvars that control the warm ups.


	More Force Disable Options
	Multiplayer, Server

	What It Does:
		You can now set "Jump Only" and "Neutrals Only" for the Force Disable (now called "Force Powers") option.

	All On 	  = All Force Powers Enabled
	All Off   = All Force Powers Disabled
	Jump Only = Only Force Jump Enabled 
	Neutrals Only = Only Saber Defense, Saber Offense, Jump, Seeing, Push, and Pull are enabled.

	NOTE:  Force Jump defaults to level 1 when disabled.  Saber Defense/Offense default to level 3 when disabled.


	More Weapon Disable Options (MOREWEAPOPTIONS)
	Multiplayer, Weapons

	What It Does:
		You can now disable any weapon in the game in any combination except for having all weapons (including melee) disabled. This is controled by the weapondisable cvars (g_weaponDisable and g_duelWeaponDisable).  Ammo now correctly disappears when the weapons for that ammo type are disabled.  You now have menu options for Melee Only, Sabers & Melee Only, and No Explosives.


	More Vehicles (MOREVEHICLES)
	Multiplayer, Vehicles

	What It Does:
		The game code has been changed to up the vehicle limit from 16 to 64 different vehicle files.  This means you can have 64 different types of vehicle models (You can spawn up to 128 vehicles total.)  Note:  Different vehicle skins count against the vehicle model total.


	Improved Botrouting
	Multiplayer, Bots
	
	What It Does:
		Botrouting controls how MP bots navigate a given maps.  We've gone thru and improved the botrouting for the following maps:
		mp/duel3
		mp/duel6
		mp/duel8
		mp/ffa5


	Old JK2 Gametypes (OLDGAMETYPES)
	Multiplayer, Basic, gametypes
	
	What it Does:
		Reintroduces the old JK2 gametypes Holocron FFA, Jedi Master, and Capture the Ysalimari.	
		

	RGB Character Color Menu Options
	Multiplayer, Single Player, Menus

	What It Does:
		 Adds a submenu that allows you to pick any color for your character's color with simple slide bars or float values.  To enter a new float value, just click on the float value seen next to the slider bar and enter in a new number.


	Realistic Saber Menu Options
	Single Player, Sabers, Menus

	What It Does: 
		Adds a menu option to SP that allows you to turn on different levels of saber damage realism. You can find the option under Setup->Options->Saber Realism.

	Normal = Default Saber Damage / Dismemberment
	Boosted Damage = Boosted Saber Damage
	Realistic Dismemberment = Attack swings now do lethal damage and can cut off multiple limbs at once.
	Realistic Idle = Idle Sabers now do lethal damage.  Also includes the features of Realistic Dismemberment.

	Please note that this option defaults to Normal each time you start up the game.  We currently don't know of a way around that.  Sorry.


	SP Dual/Staff Menu
	Single Player, Sabers

	What It Does:
		The SP menu file has been altered to allow you to use the dual sabers/saber staff from the beginning of the game.  This also allows you to choice a red saber blade in SP.


	Various Bot Tweaks
	Multiplayer, Bots
	
	What It Does:

	 - The Luke bot now uses the skywalker saber (single_skywalker) that is included in the game.

	 - Chewie now speaks in Wookie.

	 - The Lannik Racto bot now uses the correct model.


	Various Menu Tweaks
	Multiplayer, Single Player, Menus

	What It Does:

	- Shadow options are now in both SP and MP.  Projected shadows are now an option.

	- Light Flare option in the MP ingame More video Options menu.

	- Dynamic Glow option in the MP ingame More video Options menu.


	Various Server Tweaks
	Multiplayer

	What It Does:

	- g_saberdamagescale is now a server info cvar.  This means you can now use third party server browsers, like the All Seeing Eye, to create filters useing this cvar.


	Various UI Tweaks
	Multiplayer

	What It Does:

	- The map loading screen is now less vague about the server's disabled Force Powers setting.  It also now has support for the More Force Disable Options feature.

	- Server filters for OJP Basic and OJP Enhanced. (SERVERFILTERS)

		

-----------------
0004.2 - Enhanced
-----------------

	Melee System Overhaul
	Multiplayer, Melee

	What It Does:

	- All Players in all gametypes have melee combat (WP_MELEE) by default (you don't need to cheat anymore).  
		Melee Mode:
		-----------
		Primary Fire - Punches
		Secondary Fire - Kicks
		Primary + Secondary - Grapple Moves
			Default:  	Head Lock
			+Forward:  	Punching Grapple
			+Backward:	Kneeing Grapple

	- You can now use the select saber button to quickly toggle between an ignited saber and melee mode.  You can go back to the traditional system (where that same command toggles the saber on/off by setting cg_sabermelee to 0).  Please note that you can also use /togglesaber to toggle the saber on/off reguardless of your cg_sabermelee setting. 
		
	- Added in headlock (BOTH_KYLE_PA_3).  Just don't press forward or backward while attempting a grapple to try a headlock.
	- Defender now comes out of the kneeing grapple facing in the correct direction.
	- Defender now plays get up animation (BOTH_GETUP3) instead of snapping at the end of a kneeing grapple.
	- Fixed the grapple body positioning.


-----------------
0004.3 - Vehicles
-----------------

	Droideka.
	Multiplayer, Vehicle
	Model Names:
		droideka
	NOTE: THis vehicle requires OJP Basic v0.0.5 or OJP Enhanced 0.0.2 to operate correctly.

	Description:
		"Unlike the spindly battle droids, whose humanoid builds allow them a degree of versatility, droidekas are designed with one sole function in mind: the complete annihilation of their targets. The bronzen-armored combat automata has a frightening build, insectoid in its mix of curves and sharp angles. Slung at the end of heavy arms are immense twin blasters, which unleash destructive energy at a pounding pace. The destroyer droid can completely envelope itself in a globe of protective energy via its compact deflector shield generators. 
Although slow and awkward on its three-legged gait, the droideka can transform into a much speedier shape. By curling into itself, the droideka can turn into a disk-shaped form, which can roll on smooth surfaces at impressive speeds." (starwars.com/databank)


	Eopie
	Multiplayer, Vehicle
	Model Names:
		eopie - eopie with saddle
		eopiewild - eopie without saddle

	Description:
		A slow, ugly, and gassy beast of burden used on Tatooine.  Seen in Episode 1.


	Sith Speeder
	Multiplayer, Vehicle
	Model Names:
		sithspeeder

	Description:
		"A pared-down crescent-shaped conveyance, Darth Maul's Sith speeder is a very utilitarian craft stripped of all non-essential features to deliver the swiftest speeds possible. Deployed from the Sith Infiltrator, Maul's speeder bike traverses distances across land when use of his starship is deemed unnecessary.  When searching for the escaped Queen Amidala of the Naboo, Maul used the Sith speeder to soar towards the Queen's starship on Tatooine.  He lept from the moving vehicle and immediately engaged in a duel with Jedi Master Qui-Gon Jinn."  (starwars.com/databank)

	Weapons:
		(Dropped) Hovering Mines - Can be shoot.  Explodes on after a period of time.  High Damage.

	Stap
	Multiplayer, Vehicle
	Model Name:
		stap

	Description:
		"The STAP (single trooper aerial platform) is a slim, agile craft sporting a pair of blaster cannons.  STAPs are often deployed as support vehicles in conjunction with larger craft.  The design draws inspiration from similar civilian vehicles called airhooks.  Trade Federation engineers refitted the design with greater performance and reliability.  High-voltage energy cells fuel the tiny craft's drive turbines, which afford the STAP impressive maneuverability." (starwars.com/databank) 

	Weapons:
		(2, Forward) Blaster Cannon 
	

--------------
0004.4 - Skins
--------------

	Hoth Custom Player Clothing
	Multiplayer, Single Player, Player Classes

 	What It Does:
		Adds the ability to select the Hoth clothes for all the built-in player classes.  You can now also select the color of Hoth outfit.

	Technical Notes:
		For most of the player classes, I've shared the icons by placing dummy .png icon files (so the menu tries to load them) in the player model directories and then using shader referencing to use the general icons.  The generic icons are located in the jedi_hoth directory.  
		Please note that the jedi_kdm, jedi_rm, and jedi_tf have custom torso icons and jedi_tf has a custom lower icon.  Those custom icons are found in the individual model directories.


=====================
0005 - Using Our Work
=====================

We have few rules for using our work as part of your own projects:

 - You must include this readme in any public releases of your mod.  This doesn't apply if you're only using OJP features that you wrote yourself.
 - You must treat your fellow coders and the project with respect.
 - You may NOT use our work for ANY commercial purposes without the author's direct permission.

Please don't violate these rules; they are here for everyone's benefit.

We have a public CVS repository set up to let you directly access the OJP source materials to keep up-to-date with latest additions to the project.  However, the process to access the repository is a bit complicated, so please email one of the moderators for assistance.

In addition, we are still waiting for the MP SDK so there will probably not be anything to see for a while anyway.

We suggest that you:

 - Submit any cool features from your work that you think other developers may benefit from. 
 - Keep in contact with us about your project.  The more information we have, the better we can coordinate OJP to help you and the community.  We also like to know that people are using and enjoying our works.



==============================
0006 - Submitting Stuff to OJP
==============================


We are looking mostly for new code features, but are not limited to that. We are usually looking for generic, flexible, and adaptable features that anyone can work into their own code. Features that are drastically unique or special probably do not belong here, because we believe in keeping individuality and uniqueness among mods.

We also accept patches. If you see something in our code that has a problem, you can submit a patch for it. A patch would usually be replacement code or files.

Before you consider submitting, take note that we won't let you desubmit or remove your works from the project. Allowing people to do so would cause too many problems for the project. While your work will remain your work, submitting stuff to OJP means that you give us the rights to use your work and modify it freely as part of the project forever.

In addition, your work won't necessarily be turned on or even in every compiled version of OJP.  Some features will be disabled by default to allow people to just fire up and play OJP without confusion.

That being said, if you have something to submit just contact one of the OJP moderators.

DO NOT E-MAIL MOD MATERIALS TO STAFF MEMBERS WITHOUT ASKING FIRST! Just contact one of us, tell us a summary of your patch or feature, and if we think it fits the project, we'll accept it.

If you think you would like to actively participate in developing OJP, we can give you write access to the CVS repository, so that you may work on it yourself. This doesn't have to be a commitment, but if you would like to just submit features or patches separately, go ahead.


------------------------------
0006.1 - Submission Guidelines
------------------------------

 - Document your work as much as possible.  Be sure to add mentions of your work in the readme and other project documents.

 - Make your work as clean and tight as possible.

 - Follow the coding guidelines.  Try to keep your code as separated from other code as is reasonable.  Label EVERY coding change (from basejka) with appropriate coding tags.  If you're creating a new feature, you'll get to determine what the tag name will be.  Try to pick something that is simple and easy to search for.

 - NEVER DELETE FILES/DIRECTORIES/ETC FROM THE CVS REPOSITORY.  If it is necessary, the moderators will handle it.  



===================
0007 - Known Issues
===================


-----------------------------------
0007.1 - Issues with Basic Features
-----------------------------------

	Hidden Player Class Clothes (Hoth):

	- In the Hoth mission of the single player campaign, your Hoth outfit will lose its color selection abilities.  
	This is because the Hoth cold suit that is selectable in the menu is a slightly modified version of the original Hoth skin. The Hoth maps use the originals, so you're basically stuck with this issue unless you activate cheat mode and manually switch back to the selectable Hoth.

	- Twi'lek Female doesn't have selectable colors for her Hoth suit. 
	This is because the color selection system can only handle one custom color at a time.  In this case, the color is already being used for your player's skin tone.  Sorry! 



==============
0008 - Credits
==============

Coding:

	Original Jedi Knight Academy source code:  Raven Software


OJP Administration:

	Documentation:  Emon, Razor Ace

	Original OJP Concept:  Razor Ace

	OJP Organizational Planning:  Emon, RenegadeofPhunk, Razor Ace


OJP Basic:

	Extra Vehicle Implementation: RenegadeOfPhunk

	Ambient Player Sounds:  Razor Ace

	Animevents System Overhaul:  Razor Ace

	Bug Fix 1:  Razor Ace

	Botrouting:  Razor Ace
	
	Duel Taunts in all Gametypes:  Razor Ace

	Hidden JKA Sabers:  TiM
		Menu Names:  Razor Ace

	Hidden Lannik Racto Skin:  Razor Ace

	Match Warm Ups:  Razor Ace

	More Force Disable Options:  Razor Ace

	More Vehicles:  Razor Ace

	More Weapon Disable Options:  Razor Ace

	Old JK2 Gametypes:  Razor Ace, -ONE-Mushroom

	Realistic Saber Menu Settings:  Razor Ace

	RGB Character Color Menu Options:  Razor Ace

	SP Dual/Staff Menu:  Aryyn

	Various Bot Tweaks:  Razor Ace

	Various Menu Tweaks:  Razor Ace

	Various Server Tweaks:  Razor Ace

	Various UI Tweaks:  Razor Ace


OJP Enhanced:

	Melee Overhaul:  Razor Ace


Skins:

	Hoth Custom Player Clothing:  Razor Ace


Vehicles:

	Droideka: 
		Model / Textures / Anims:	Duncan_10158
		Sound:				RenegadeOfPhunk

	Eopie:  Chairwalker

	Stab:  
		Model/Textures:	Monsoontide
		Vehicle Files:	Duncan_10158

	Sith Speeder: 
		Model/Textures:  Monsoontide
		Vehicle Files:  Duncan_10158
		Beta Testers:  Sunburn, Nym259, Tesla, Scouttrooper, Pnut_Master


Special Thanks:

Raven Software
George Lucas
Lucas Entertainment Company (LEC)



==========================
0009 - Contact Information
==========================

OJP Project Moderators:

	Razor Ace
	Email:  razorace@hotmail.com
	MSN:  Razor Ace (razorace@hotmail.com)
	ICQ:  618641
	AIM:  razorsaces
	Yahoo:  razorsaces

	RenegadeOfPhunk
	Email:  renegadeofphunk@3dactionplanet.com
	ICQ:  253305779 (Knobby)
	(To search for any of my code, search for 'ROP')


OJP General Discussion Board:

	http://www.lucasforums.com/forumdisplay.php?s=&forumid=542


==================
0010 - Legal Stuff
==================

We are making no claim on Raven Software's, ID Software's, or LEC's intellectual properties.  The above rules only apply to the additional works created by OJP contributors.  Ravensoft's, ID's, and LEC's code and additional materials is only included for ease of use.  All applicable licensing agreements still apply.  Please don't sue us.

End of Line.


