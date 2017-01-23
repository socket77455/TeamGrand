/**--------------------------------------------------------------------------**\
					=======================================
					 y_scriptinit - Two useful callbacks.
					=======================================
Description:
	This file provides "OnScriptInit" and "OnScriptExit" which are called at the
	start and end of the current script, regardless of what the type of the
	script is (note: doesn't support NPC modes).  It also provides
	"YSI_FILTERSCRIPT" as a (partial) replacement for "FILTERSCRIPT" which
	detects what the mode is at runtime for a more reliable system (but it is a
	run-time variable, not a compile-time constant).
	
	Now also provides OnCodeInit, which comes before OnScriptInit and should
	mostly be used for @emit code generation and other assembly related stuff
	(since it is called by the JIT if that exists).
Legal:
	Version: MPL 1.1
	
	The contents of this file are subject to the Mozilla Public License Version 
	1.1 (the "License"); you may not use this file except in compliance with 
	the License. You may obtain a copy of the License at 
	http://www.mozilla.org/MPL/
	
	Software distributed under the License is distributed on an "AS IS" basis,
	WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
	for the specific language governing rights and limitations under the
	License.
	
	The Original Code is the YSI scriptinit include.
	
	The Initial Developer of the Original Code is Alex "Y_Less" Cole.
	Portions created by the Initial Developer are Copyright (C) 2011
	the Initial Developer. All Rights Reserved.
	
	Contributors:
		ZeeX, koolk, JoeBullet/Google63, g_aSlice/Slice
	
	Thanks:
		JoeBullet/Google63 - Handy arbitrary ASM jump code using SCTRL.
		ZeeX - Very productive conversations.
		koolk - IsPlayerinAreaEx code.
		TheAlpha - Danish translation.
		breadfish - German translation.
		Fireburn - Dutch translation.
		yom - French translation.
		50p - Polish translation.
		Zamaroht - Spanish translation.
		Dracoblue, sintax, mabako, Xtreme, other coders - Producing other modes
			for me to strive to better.
		Pixels^ - Running XScripters where the idea was born.
		Matite - Pestering me to release it and using it.
	
	Very special thanks to:
		Thiadmer - PAWN, whose limits continue to amaze me!
		Kye/Kalcor - SA:MP.
		SA:MP Team past, present and future - SA:MP.
	
Version:
	1.0
Changelog:
	23/04/14:
		Reduced code complexity.
		Supports y_hooks better, without relying on it.
	30/04/11:
		First version
\**--------------------------------------------------------------------------**/

/*

     ad88888ba                                              
    d8"     "8b              ,d                             
    Y8,                      88                             
    `Y8aaaaa,    ,adPPYba, MM88MMM 88       88 8b,dPPYba,   
      `"""""8b, a8P_____88   88    88       88 88P'    "8a  
            `8b 8PP"""""""   88    88       88 88       d8  
    Y8a     a8P "8b,   ,aa   88,   "8a,   ,a88 88b,   ,a8"  
     "Y88888P"   `"Ybbd8"'   "Y888  `"YbbdP'Y8 88`YbbdP"'   
                                               88           
                                               88           

*/

#if defined _INC_y_scriptinit
	#endinput
#endif
#define _INC_y_scriptinit

#include <a_samp>

#include "..\YSI_Internal\y_natives"
#include "..\YSI_Internal\y_prehook"

forward _ScriptInit_FixState();

CHAIN_HOOK(ScriptInit)
#undef CHAIN_ORDER
#define CHAIN_ORDER CHAIN_NEXT(ScriptInit)

stock
	bool:YSI_FILTERSCRIPT = false;

static
	bool:YSI_g_sCodeJITed = false,
	bool:YSI_g_sCodeJITing = false;

stock bool:JIT_Exists()
{
	return YSI_g_sCodeJITing;
}

stock bool:JIT_Complete()
{
	return YSI_g_sCodeJITed;
}

forward OnJITCompile();

public OnJITCompile()
{
	state _ALS : _ALS_go;
	state _script_init_fix_state : false;
	// Got it being called twice for some reason.  I think it was a file glitch,
	// but just make sure.
	if (YSI_g_sCodeJITing)
		return 0;
	YSI_g_sCodeJITing = true;
	ScriptInit_OnCodeInit();
	new ret = ScriptInit_OnJITCompile();
	if (ret)
		YSI_g_sCodeJITed = true;
	return ret;
}

/*

    88             88          
    88             ""   ,d     
    88                  88     
    88 8b,dPPYba,  88 MM88MMM  
    88 88P'   `"8a 88   88     
    88 88       88 88   88     
    88 88       88 88   88,    
    88 88       88 88   "Y888  

*/

public OnFilterScriptInit()
{
	YSI_FILTERSCRIPT = true;
	state _script_init_fix_state : true;
	state _ALS : _ALS_go;
	if (!YSI_g_sCodeJITing)
		ScriptInit_OnCodeInit();
	ScriptInit_OnScriptInit();
	#if defined YSI_LOCK_MODE
		#if defined FILTERSCRIPT
			if (strlen(YSI_gLockData[5]) !=
				floatround(floatlog(_LOCK_LEN_0 + 1), floatround_ceil) + 1 +
				floatround(floatlog(_LOCK_LEN_1 + 1), floatround_ceil) + 1 +
				floatround(floatlog(_LOCK_LEN_2 + 1), floatround_ceil) + 1 +
				floatround(floatlog(_LOCK_LEN_3 + 1), floatround_ceil))
			{
				y_lock 6;
			}
		#else
			new
				str[16];
			format(str, sizeof (str), "%c%c%c%c%c%c%c%c%c%c%c", '%', 'd', '.', '%', 'd', '.', '%', 'd', '.', '%', 'd');
			format(str, sizeof (str), str, _LOCK_IP_0, _LOCK_IP_1, _LOCK_IP_2, _LOCK_IP_3);
			if (strlen(YSI_gLockData[5]) != strlen(str))
			{
				y_lock 7;
			}
		#endif
	#endif
	ScriptInit_OnFilterScriptInit();
	return 1;
}

public OnGameModeInit()
{
	state _ALS : _ALS_go;
	if (!YSI_FILTERSCRIPT)
	{
		state _script_init_fix_state : false;
		if (!YSI_g_sCodeJITing)
			ScriptInit_OnCodeInit();
		ScriptInit_OnScriptInit();
		#if defined YSI_LOCK_MODE
			if (YSI_gLockData[
				floatround(floatlog(_LOCK_LEN_0 + 1), floatround_ceil) + 1 +
				floatround(floatlog(_LOCK_LEN_1 + 1), floatround_ceil) + 1 +
				floatround(floatlog(_LOCK_LEN_2 + 1), floatround_ceil) +
				floatround(floatlog(_LOCK_LEN_3 + 1), floatround_ceil)] == '\0'
			 || YSI_gLockData[
				floatround(floatlog(_LOCK_LEN_0 + 1), floatround_ceil) + 1 +
				floatround(floatlog(_LOCK_LEN_1 + 1), floatround_ceil) + 1 +
				floatround(floatlog(_LOCK_LEN_2 + 1), floatround_ceil) + 1 +
				floatround(floatlog(_LOCK_LEN_3 + 1), floatround_ceil) + 1] == '\0')
			{
				y_lock 8;
			}
		#endif
	}
	ScriptInit_OnGameModeInit();
	return 1;
}

/*

    88888888888             88          
    88                      ""   ,d     
    88                           88     
    88aaaaa     8b,     ,d8 88 MM88MMM  
    88"""""      `Y8, ,8P'  88   88     
    88             )888(    88   88     
    88           ,d8" "8b,  88   88,    
    88888888888 8P'     `Y8 88   "Y888  

*/

public OnFilterScriptExit()
{
	ScriptInit_OnFilterScriptExit();
	if (YSI_FILTERSCRIPT)
		CallLocalFunction("ScriptInit_OnScriptExit", "");
	return 1;
}

public OnGameModeExit()
{
	ScriptInit_OnGameModeExit();
	if (!YSI_FILTERSCRIPT)
		CallLocalFunction("ScriptInit_OnScriptExit", "");
	return 1;
}

/*

    88888888888                                                                      88            
    88                                                                               88            
    88                                                                               88            
    88aaaaa  ,adPPYba,  8b,dPPYba, 8b      db      d8 ,adPPYYba, 8b,dPPYba,  ,adPPYb,88 ,adPPYba,  
    88""""" a8"     "8a 88P'   "Y8 `8b    d88b    d8' ""     `Y8 88P'   "Y8 a8"    `Y88 I8[    ""  
    88      8b       d8 88          `8b  d8'`8b  d8'  ,adPPPPP88 88         8b       88  `"Y8ba,   
    88      "8a,   ,a8" 88           `8bd8'  `8bd8'   88,    ,88 88         "8a,   ,d88 aa    ]8I  
    88       `"YbbdP"'  88             YP      YP     `"8bbdP"Y8 88          `"8bbdP"Y8 `"YbbdP"'  

*/

CHAIN_FORWARD:ScriptInit_OnCodeInit() = 1;
#if defined _ALS_OnCodeInit || defined OnCodeInit
	#error OnCodeInit defined
#endif
#define _ALS_OnCodeInit
#define OnCodeInit(%0) CHAIN_PUBLIC:ScriptInit_OnCodeInit(%0)

CHAIN_FORWARD:ScriptInit_OnScriptInit() = 1;
#if defined _ALS_OnScriptInit || defined OnScriptInit
	#error defined OnScriptInit
#endif
#define _ALS_OnScriptInit
#define OnScriptInit(%0) CHAIN_PUBLIC:ScriptInit_OnScriptInit(%0)

CHAIN_FORWARD:ScriptInit_OnJITCompile() = 1;
#if defined _ALS_OnJITCompile
	#undef OnJITCompile
#else
	#define _ALS_OnJITCompile
#endif
#define OnJITCompile(%0) CHAIN_PUBLIC:ScriptInit_OnJITCompile(%0)

CHAIN_FORWARD:ScriptInit_OnFilterScriptInit() = 1;
#if defined _ALS_OnFilterScriptInit
	#undef OnFilterScriptInit
#else
	#define _ALS_OnFilterScriptInit
#endif
#define OnFilterScriptInit(%0) CHAIN_PUBLIC:ScriptInit_OnFilterScriptInit(%0)

CHAIN_FORWARD:ScriptInit_OnGameModeInit() = 1;
#if defined _ALS_OnGameModeInit
	#undef OnGameModeInit
#else
	#define _ALS_OnGameModeInit
#endif
#define OnGameModeInit(%0) CHAIN_PUBLIC:ScriptInit_OnGameModeInit(%0)

CHAIN_FORWARD:ScriptInit_OnFilterScriptExit() = 1;
#if defined _ALS_OnFilterScriptExit
	#undef OnFilterScriptExit
#else
	#define _ALS_OnFilterScriptExit
#endif
#define OnFilterScriptExit(%0) CHAIN_PUBLIC:ScriptInit_OnFilterScriptExit(%0)

CHAIN_FORWARD:ScriptInit_OnGameModeExit() = 1;
#if defined _ALS_OnGameModeExit
	#undef OnGameModeExit
#else
	#define _ALS_OnGameModeExit
#endif
#define OnGameModeExit(%0) CHAIN_PUBLIC:ScriptInit_OnGameModeExit(%0)

CHAIN_FORWARD:ScriptInit_OnScriptExit() = 1;
#if defined _ALS_OnScriptExit
	#undef OnScriptExit
#else
	#define _ALS_OnScriptExit
#endif
#define OnScriptExit(%0) CHAIN_PUBLIC:ScriptInit_OnScriptExit(%0)

/*

     ad88888ba  88                                      88                                             
    d8"     "8b 88                        ,d            88                                             
    Y8,         88                        88            88                                             
    `Y8aaaaa,   88,dPPYba,  88       88 MM88MMM ,adPPYb,88  ,adPPYba,  8b      db      d8 8b,dPPYba,   
      `"""""8b, 88P'    "8a 88       88   88   a8"    `Y88 a8"     "8a `8b    d88b    d8' 88P'   `"8a  
            `8b 88       88 88       88   88   8b       88 8b       d8  `8b  d8'`8b  d8'  88       88  
    Y8a     a8P 88       88 "8a,   ,a88   88,  "8a,   ,d88 "8a,   ,a8"   `8bd8'  `8bd8'   88       88  
     "Y88888P"  88       88  `"YbbdP'Y8   "Y888 `"8bbdP"Y8  `"YbbdP"'      YP      YP     88       88 

*/

// I'm not quite sure what causes it, but there seems to be a bug in the
// compiler somewhere which messes up the "ysi_debug" automaton and the
// "YSI_FILTERSCRIPT" variable :(.
public _ScriptInit_FixState() <_script_init_fix_state : true>
{
}

public _ScriptInit_FixState() <_script_init_fix_state : false>
{
}
