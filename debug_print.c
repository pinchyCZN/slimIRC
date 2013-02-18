#include <windows.h>
#include <stdio.h>
#include <conio.h>
typedef struct{
	char *name;
	int val;
}NAMES;
NAMES wm_names[]={
{"WM_NULL",0x0000},
{"WM_CREATE",0x0001},
{"WM_DESTROY",0x0002},
{"WM_MOVE",0x0003},
{"WM_SIZE",0x0005},
{"WM_ACTIVATE",0x0006},
{"WM_SETFOCUS",0x0007},
{"WM_KILLFOCUS",0x0008},
{"WM_ENABLE",0x000A},
{"WM_SETREDRAW",0x000B},
{"WM_SETTEXT",0x000C},
{"WM_GETTEXT",0x000D},
{"WM_GETTEXTLENGTH",0x000E},
{"WM_PAINT",0x000F},
{"WM_CLOSE",0x0010},
{"WM_QUERYENDSESSION",0x0011},
{"WM_QUIT",0x0012},
{"WM_QUERYOPEN",0x0013},
{"WM_ERASEBKGND",0x0014},
{"WM_SYSCOLORCHANGE",0x0015},
{"WM_ENDSESSION",0x0016},
{"WM_SHOWWINDOW",0x0018},
{"WM_WININICHANGE",0x001A},
{"WM_DEVMODECHANGE",0x001B},
{"WM_ACTIVATEAPP",0x001C},
{"WM_FONTCHANGE",0x001D},
{"WM_TIMECHANGE",0x001E},
{"WM_CANCELMODE",0x001F},
{"WM_SETCURSOR",0x0020},
{"WM_MOUSEACTIVATE",0x0021},
{"WM_CHILDACTIVATE",0x0022},
{"WM_QUEUESYNC",0x0023},
{"WM_GETMINMAXINFO",0x0024},
{"WM_PAINTICON",0x0026},
{"WM_ICONERASEBKGND",0x0027},
{"WM_NEXTDLGCTL",0x0028},
{"WM_SPOOLERSTATUS",0x002A},
{"WM_DRAWITEM",0x002B},
{"WM_MEASUREITEM",0x002C},
{"WM_DELETEITEM",0x002D},
{"WM_VKEYTOITEM",0x002E},
{"WM_CHARTOITEM",0x002F},
{"WM_SETFONT",0x0030},
{"WM_GETFONT",0x0031},
{"WM_SETHOTKEY",0x0032},
{"WM_GETHOTKEY",0x0033},
{"WM_QUERYDRAGICON",0x0037},
{"WM_COMPAREITEM",0x0039},
{"WM_GETOBJECT",0x003D},
{"WM_COMPACTING",0x0041},
{"WM_COMMNOTIFY",0x0044},
{"WM_WINDOWPOSCHANGING",0x0046},
{"WM_WINDOWPOSCHANGED",0x0047},
{"WM_POWER",0x0048},
{"WM_COPYDATA",0x004A},
{"WM_CANCELJOURNAL",0x004B},
{"WM_NOTIFY",0x004E},
{"WM_INPUTLANGCHANGEREQUEST",0x0050},
{"WM_INPUTLANGCHANGE",0x0051},
{"WM_TCARD",0x0052},
{"WM_HELP",0x0053},
{"WM_USERCHANGED",0x0054},
{"WM_NOTIFYFORMAT",0x0055},
{"WM_CONTEXTMENU",0x007B},
{"WM_STYLECHANGING",0x007C},
{"WM_STYLECHANGED",0x007D},
{"WM_DISPLAYCHANGE",0x007E},
{"WM_GETICON",0x007F},
{"WM_SETICON",0x0080},
{"WM_NCCREATE",0x0081},
{"WM_NCDESTROY",0x0082},
{"WM_NCCALCSIZE",0x0083},
{"WM_NCHITTEST",0x0084},
{"WM_NCPAINT",0x0085},
{"WM_NCACTIVATE",0x0086},
{"WM_GETDLGCODE",0x0087},
{"WM_SYNCPAINT",0x0088},
{"WM_NCMOUSEMOVE",0x00A0},
{"WM_NCLBUTTONDOWN",0x00A1},
{"WM_NCLBUTTONUP",0x00A2},
{"WM_NCLBUTTONDBLCLK",0x00A3},
{"WM_NCRBUTTONDOWN",0x00A4},
{"WM_NCRBUTTONUP",0x00A5},
{"WM_NCRBUTTONDBLCLK",0x00A6},
{"WM_NCMBUTTONDOWN",0x00A7},
{"WM_NCMBUTTONUP",0x00A8},
{"WM_NCMBUTTONDBLCLK",0x00A9},
{"WM_KEYFIRST",0x0100},
{"WM_KEYDOWN",0x0100},
{"WM_KEYUP",0x0101},
{"WM_CHAR",0x0102},
{"WM_DEADCHAR",0x0103},
{"WM_SYSKEYDOWN",0x0104},
{"WM_SYSKEYUP",0x0105},
{"WM_SYSCHAR",0x0106},
{"WM_SYSDEADCHAR",0x0107},
{"WM_KEYLAST",0x0108},
{"WM_IME_STARTCOMPOSITION",0x010D},
{"WM_IME_ENDCOMPOSITION",0x010E},
{"WM_IME_COMPOSITION",0x010F},
{"WM_IME_KEYLAST",0x010F},
{"WM_INITDIALOG",0x0110},
{"WM_COMMAND",0x0111},
{"WM_SYSCOMMAND",0x0112},
{"WM_TIMER",0x0113},
{"WM_HSCROLL",0x0114},
{"WM_VSCROLL",0x0115},
{"WM_INITMENU",0x0116},
{"WM_INITMENUPOPUP",0x0117},
{"wm_systimer",0x0118},
{"WM_MENUSELECT",0x011F},
{"WM_MENUCHAR",0x0120},
{"WM_ENTERIDLE",0x0121},
{"WM_MENURBUTTONUP",0x0122},
{"WM_MENUDRAG",0x0123},
{"WM_MENUGETOBJECT",0x0124},
{"WM_UNINITMENUPOPUP",0x0125},
{"WM_MENUCOMMAND",0x0126},
{"WM_CTLCOLORMSGBOX",0x0132},
{"WM_CTLCOLOREDIT",0x0133},
{"WM_CTLCOLORLISTBOX",0x0134},
{"WM_CTLCOLORBTN",0x0135},
{"WM_CTLCOLORDLG",0x0136},
{"WM_CTLCOLORSCROLLBAR",0x0137},
{"WM_CTLCOLORSTATIC",0x0138},
{"WM_MOUSEFIRST",0x0200},
{"WM_MOUSEMOVE",0x0200},
{"WM_LBUTTONDOWN",0x0201},
{"WM_LBUTTONUP",0x0202},
{"WM_LBUTTONDBLCLK",0x0203},
{"WM_RBUTTONDOWN",0x0204},
{"WM_RBUTTONUP",0x0205},
{"WM_RBUTTONDBLCLK",0x0206},
{"WM_MBUTTONDOWN",0x0207},
{"WM_MBUTTONUP",0x0208},
{"WM_MBUTTONDBLCLK",0x0209},
{"WM_MOUSEWHEEL",0x020A},
{"WM_MOUSELAST",0x020A},
{"WM_MOUSELAST",0x0209},
{"WM_PARENTNOTIFY",0x0210},
{"WM_ENTERMENULOOP",0x0211},
{"WM_EXITMENULOOP",0x0212},
{"WM_NEXTMENU",0x0213},
{"WM_SIZING",0x0214},
{"WM_CAPTURECHANGED",0x0215},
{"WM_MOVING",0x0216},
{"WM_POWERBROADCAST",0x0218},
{"WM_DEVICECHANGE",0x0219},
{"WM_MDICREATE",0x0220},
{"WM_MDIDESTROY",0x0221},
{"WM_MDIACTIVATE",0x0222},
{"WM_MDIRESTORE",0x0223},
{"WM_MDINEXT",0x0224},
{"WM_MDIMAXIMIZE",0x0225},
{"WM_MDITILE",0x0226},
{"WM_MDICASCADE",0x0227},
{"WM_MDIICONARRANGE",0x0228},
{"WM_MDIGETACTIVE",0x0229},
{"WM_MDISETMENU",0x0230},
{"WM_ENTERSIZEMOVE",0x0231},
{"WM_EXITSIZEMOVE",0x0232},
{"WM_DROPFILES",0x0233},
{"WM_MDIREFRESHMENU",0x0234},
{"WM_IME_SETCONTEXT",0x0281},
{"WM_IME_NOTIFY",0x0282},
{"WM_IME_CONTROL",0x0283},
{"WM_IME_COMPOSITIONFULL",0x0284},
{"WM_IME_SELECT",0x0285},
{"WM_IME_CHAR",0x0286},
{"WM_IME_REQUEST",0x0288},
{"WM_IME_KEYDOWN",0x0290},
{"WM_IME_KEYUP",0x0291},
{"WM_MOUSEHOVER",0x02A1},
{"WM_MOUSELEAVE",0x02A3},
{"WM_CUT",0x0300},
{"WM_COPY",0x0301},
{"WM_PASTE",0x0302},
{"WM_CLEAR",0x0303},
{"WM_UNDO",0x0304},
{"WM_RENDERFORMAT",0x0305},
{"WM_RENDERALLFORMATS",0x0306},
{"WM_DESTROYCLIPBOARD",0x0307},
{"WM_DRAWCLIPBOARD",0x0308},
{"WM_PAINTCLIPBOARD",0x0309},
{"WM_VSCROLLCLIPBOARD",0x030A},
{"WM_SIZECLIPBOARD",0x030B},
{"WM_ASKCBFORMATNAME",0x030C},
{"WM_CHANGECBCHAIN",0x030D},
{"WM_HSCROLLCLIPBOARD",0x030E},
{"WM_QUERYNEWPALETTE",0x030F},
{"WM_PALETTEISCHANGING",0x0310},
{"WM_PALETTECHANGED",0x0311},
{"WM_HOTKEY",0x0312},
{"WM_PRINT",0x0317},
{"WM_PRINTCLIENT",0x0318},
{"WM_HANDHELDFIRST",0x0358},
{"WM_HANDHELDLAST",0x035F},
{"WM_AFXFIRST",0x0360},
{"WM_AFXLAST",0x037F},
{"WM_PENWINFIRST",0x0380},
{"WM_PENWINLAST",0x038F},
{"WM_APP",0x8000},
{"WM_USER",0x0400},
0
};

char *virtual_keys[256]={
"key0", //0
"LBUTTON", //1
"RBUTTON", //2
"CANCEL", //3
"MBUTTON", //4
"key5", //5
"key6", //6
"key7", //7
"BACK", //8
"TAB", //9
"key10", //10
"key11", //11
"CLEAR", //12
"RETURN", //13
"key14", //14
"key15", //15
"SHIFT", //16
"CONTROL", //17
"MENU", //18
"PAUSE", //19
"CAPITAL", //20
"HANGEUL", //21
"key22", //22
"JUNJA", //23
"FINAL", //24
"HANJA", //25
"key26", //26
"ESCAPE", //27
"CONVERT", //28
"NONCONVERT", //29
"ACCEPT", //30
"MODECHANGE", //31
"SPACE", //32
"PRIOR", //33
"NEXT", //34
"END", //35
"HOME", //36
"LEFT", //37
"UP", //38
"RIGHT", //39
"DOWN", //40
"SELECT", //41
"PRINT", //42
"EXECUTE", //43
"SNAPSHOT", //44
"INSERT", //45
"DELETE", //46
"HELP", //47
"0", //48
"1", //49
"2", //50
"3", //51
"4", //52
"5", //53
"6", //54
"7", //55
"8", //56
"9", //57
"key58", //58
"key59", //59
"key60", //60
"key61", //61
"key62", //62
"key63", //63
"key64", //64
"A", //65
"B", //66
"C", //67
"D", //68
"E", //69
"F", //70
"G", //71
"H", //72
"I", //73
"J", //74
"K", //75
"L", //76
"M", //77
"N", //78
"O", //79
"P", //80
"Q", //81
"R", //82
"S", //83
"T", //84
"U", //85
"V", //86
"W", //87
"X", //88
"Y", //89
"Z", //90
"LWIN", //91
"RWIN", //92
"APPS", //93
"key94", //94
"key95", //95
"NUMPAD0", //96
"NUMPAD1", //97
"NUMPAD2", //98
"NUMPAD3", //99
"NUMPAD4", //100
"NUMPAD5", //101
"NUMPAD6", //102
"NUMPAD7", //103
"NUMPAD8", //104
"NUMPAD9", //105
"MULTIPLY", //106
"ADD", //107
"SEPARATOR", //108
"SUBTRACT", //109
"DECIMAL", //110
"DIVIDE", //111
"F1", //112
"F2", //113
"F3", //114
"F4", //115
"F5", //116
"F6", //117
"F7", //118
"F8", //119
"F9", //120
"F10", //121
"F11", //122
"F12", //123
"F13", //124
"F14", //125
"F15", //126
"F16", //127
"F17", //128
"F18", //129
"F19", //130
"F20", //131
"F21", //132
"F22", //133
"F23", //134
"F24", //135
"key136", //136
"key137", //137
"key138", //138
"key139", //139
"key140", //140
"key141", //141
"key142", //142
"key143", //143
"NUMLOCK", //144
"SCROLL", //145
"key146", //146
"key147", //147
"key148", //148
"key149", //149
"key150", //150
"key151", //151
"key152", //152
"key153", //153
"key154", //154
"key155", //155
"key156", //156
"key157", //157
"key158", //158
"key159", //159
"LSHIFT", //160
"RSHIFT", //161
"LCONTROL", //162
"RCONTROL", //163
"LMENU", //164
"RMENU", //165
"key166", //166
"key167", //167
"key168", //168
"key169", //169
"key170", //170
"key171", //171
"key172", //172
"key173", //173
"key174", //174
"key175", //175
"key176", //176
"key177", //177
"key178", //178
"key179", //179
"key180", //180
"key181", //181
"key182", //182
"key183", //183
"key184", //184
"key185", //185
"key186", //186
"key187", //187
"key188", //188
"key189", //189
"key190", //190
"key191", //191
"key192", //192
"key193", //193
"key194", //194
"key195", //195
"key196", //196
"key197", //197
"key198", //198
"key199", //199
"key200", //200
"key201", //201
"key202", //202
"key203", //203
"key204", //204
"key205", //205
"key206", //206
"key207", //207
"key208", //208
"key209", //209
"key210", //210
"key211", //211
"key212", //212
"key213", //213
"key214", //214
"key215", //215
"key216", //216
"key217", //217
"key218", //218
"key219", //219
"key220", //220
"key221", //221
"key222", //222
"key223", //223
"key224", //224
"key225", //225
"key226", //226
"key227", //227
"key228", //228
"PROCESSKEY", //229
"key230", //230
"key231", //231
"key232", //232
"key233", //233
"key234", //234
"key235", //235
"key236", //236
"key237", //237
"key238", //238
"key239", //239
"key240", //240
"key241", //241
"key242", //242
"key243", //243
"key244", //244
"key245", //245
"ATTN", //246
"CRSEL", //247
"EXSEL", //248
"EREOF", //249
"PLAY", //250
"ZOOM", //251
"NONAME", //252
"PA1", //253
"OEM_CLEAR", //254
"key255", //255
};
void print_key(int key)
{
	if(key>(sizeof(virtual_keys)/sizeof(char *)))
		printf("key=%i\n",key);
	else
		printf("key=%s (%i) (%02X)\n",virtual_keys[key],key,key);
}

int get_key_str(int key,char *str)
{
	if(key>(sizeof(virtual_keys)/sizeof(char *)))
		sprintf(str,"key=%i",key);
	else
		sprintf(str,"%s",virtual_keys[key]);
	return TRUE;
}
int print_capture(BYTE *keys,char *str,int len)
{
	int i,count=0,max=4;
	str[0]=0;
	for(i=0;i<256;i++){
		if(keys[i]!=0){
			if(count!=0)
				strcat(str,"+");
			_snprintf(str+strlen(str),len-strlen(str),"%s",virtual_keys[i]);
			count++;
			if(count>=max)
				break;
		}
	}
	return TRUE;
}
void print_msg(int msg,int lparam,int wparam,int hwnd)
{
	int i;
	DWORD time;
	time=GetTickCount();
	for(i=0;i<1000;i++){
		if(wm_names[i].name==0)
			break;
		if(wm_names[i].val==msg){
			printf("%-20s lparam=%08X wparam=%08X %08X x=%3i y=%3i\n",wm_names[i].name,lparam,wparam,hwnd,LOWORD(lparam),HIWORD(lparam));
			return;
		}
	}
	printf("msg=%08X lparam=%08X wparam=%08X %08X x=%3i y=%3i\n",msg,lparam,wparam,hwnd,LOWORD(lparam),HIWORD(lparam));
}
int save_input(BYTE *keys,int inp)
{
	if(inp>=256)
		return 0;
	keys[inp]=1;
	return TRUE;
}
int sanatize_string(char *str,int len)
{
	int i;
	for(i=0;i<len;i++){
		if((str[i]>='0' && str[i]<='9') || (str[i]>='A' && str[i]<='Z') || (str[i]>='a' && str[i]<='z'))
			;
		else
			str[i]=' ';
	}
	return TRUE;
}
/*
//----------------------------------------------------------------------------
// ObjectWindows
// (C) Copyright 1992, 1994 by Borland International, All Rights Reserved
//
//   Implementation of class TEventHandler
//----------------------------------------------------------------------------
#include <owl/owlpch.h>
#include <owl/point.h>
#include <owl/eventhan.h>
#include <stdlib.h>

DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlMsg, 1, 0);
                             // diagnostic group for message tracing

#if defined(__TRACE) || defined(__WARN)
  struct WMSTR {
    uint    msg;
    char*   str;
  };

  static  WMSTR  StrTab[] = {
    { WM_CREATE,                "WM_CREATE"               },  // 0x0001
    { WM_DESTROY,               "WM_DESTROY"              },  // 0x0002
    { WM_MOVE,                  "WM_MOVE"                 },  // 0x0003
    { WM_SIZE,                  "WM_SIZE"                 },  // 0x0005
    { WM_ACTIVATE,              "WM_ACTIVATE"             },  // 0x0006
    { WM_SETFOCUS,              "WM_SETFOCUS"             },  // 0x0007
    { WM_KILLFOCUS,             "WM_KILLFOCUS"            },  // 0x0008
    { 0x0009,                   "wm_setvisible"           },  // 0x0009
    { WM_ENABLE,                "WM_ENABLE"               },  // 0x000A
    { WM_SETREDRAW,             "WM_SETREDRAW"            },  // 0x000B
    { WM_SETTEXT,               "WM_SETTEXT"              },  // 0x000C
    { WM_GETTEXT,               "WM_GETTEXT"              },  // 0x000D
    { WM_GETTEXTLENGTH,         "WM_GETTEXTLENGTH"        },  // 0x000E
    { WM_PAINT,                 "WM_PAINT"                },  // 0x000F
    { WM_CLOSE,                 "WM_CLOSE"                },  // 0x0010
    { WM_QUERYENDSESSION,       "WM_QUERYENDSESSION"      },  // 0x0011
    { WM_QUIT,                  "WM_QUIT"                 },  // 0x0012
    { WM_QUERYOPEN,             "WM_QUERYOPEN"            },  // 0x0013
    { WM_ERASEBKGND,            "WM_ERASEBKGND"           },  // 0x0014
    { WM_SYSCOLORCHANGE,        "WM_SYSCOLORCHANGE"       },  // 0x0015
    { WM_ENDSESSION,            "WM_ENDSESSION"           },  // 0x0016
    { 0x0017,                   "WM_SYSTEMERROR"          },  // 0x0017
    { WM_SHOWWINDOW,            "WM_SHOWWINDOW"           },  // 0x0018
    { WM_CTLCOLOR,              "WM_CTLCOLOR"             },  // 0x0019
    { WM_WININICHANGE,          "WM_WININICHANGE"         },  // 0x001A
    { WM_DEVMODECHANGE,         "WM_DEVMODECHANGE"        },  // 0x001B
    { WM_ACTIVATEAPP,           "WM_ACTIVATEAPP"          },  // 0x001C
    { WM_FONTCHANGE,            "WM_FONTCHANGE"           },  // 0x001D
    { WM_TIMECHANGE,            "WM_TIMECHANGE"           },  // 0x001E
    { WM_CANCELMODE,            "WM_CANCELMODE"           },  // 0x001F
    { WM_SETCURSOR,             "WM_SETCURSOR"            },  // 0x0020
    { WM_MOUSEACTIVATE,         "WM_MOUSEACTIVATE"        },  // 0x0021
    { WM_CHILDACTIVATE,         "WM_CHILDACTIVATE"        },  // 0x0022
    { WM_QUEUESYNC,             "WM_QUEUESYNC"            },  // 0x0023
    { WM_GETMINMAXINFO,         "WM_GETMINMAXINFO"        },  // 0x0024
    { 0x0026,                   "wm_painticon"            },  // 0x0026
    { WM_ICONERASEBKGND,        "WM_ICONERASEBKGND"       },  // 0x0027
    { WM_NEXTDLGCTL,            "WM_NEXTDLGCTL"           },  // 0x0028
    { 0x0029,                   "wm_alttabactive"         },  // 0x0029
    { WM_SPOOLERSTATUS,         "WM_SPOOLERSTATUS"        },  // 0x002A
    { WM_DRAWITEM,              "WM_DRAWITEM"             },  // 0x002B
    { WM_MEASUREITEM,           "WM_MEASUREITEM"          },  // 0x002C
    { WM_DELETEITEM,            "WM_DELETEITEM"           },  // 0x002D
    { WM_VKEYTOITEM,            "WM_VKEYTOITEM"           },  // 0x002E
    { WM_CHARTOITEM,            "WM_CHARTOITEM"           },  // 0x002F
    { WM_SETFONT,               "WM_SETFONT"              },  // 0x0030
    { WM_GETFONT,               "WM_GETFONT"              },  // 0x0031
    { 0x0032,                   "wm_sethotkey"            },  // 0x0032
    { 0x0033,                   "wm_gethotkey"            },  // 0x0033
    { 0x0034,                   "wm_filesyschange"        },  // 0x0034
    { 0x0035,                   "wm_isactiveicon"         },  // 0x0035
    { 0x0036,                   "wm_queryparkicon"        },  // 0x0036
    { WM_QUERYDRAGICON,         "WM_QUERYDRAGICON"        },  // 0x0037
    { WM_COMPAREITEM,           "WM_COMPAREITEM"          },  // 0x0039
    { WM_COMPACTING,            "WM_COMPACTING"           },  // 0x0041
    { 0x0042,                   "wm_otherwindowcreated"   },  // 0x0042
    { 0x0043,                   "wm_otherwindowdestroyed" },  // 0x0043
    { WM_COMMNOTIFY,            "WM_COMMNOTIFY"           },  // 0x0044
    { WM_WINDOWPOSCHANGING,     "WM_WINDOWPOSCHANGING"    },  // 0x0046
    { WM_WINDOWPOSCHANGED,      "WM_WINDOWPOSCHANGED"     },  // 0x0047
    { WM_POWER,                 "WM_POWER"                },  // 0x0048
    { WM_NCCREATE,              "WM_NCCREATE"             },  // 0x0081
    { WM_NCDESTROY,             "WM_NCDESTROY"            },  // 0x0082
    { WM_NCCALCSIZE,            "WM_NCCALCSIZE"           },  // 0x0083
    { WM_NCHITTEST,             "WM_NCHITTEST"            },  // 0x0084
    { WM_NCPAINT,               "WM_NCPAINT"              },  // 0x0085
    { WM_NCACTIVATE,            "WM_NCACTIVATE"           },  // 0x0086
    { WM_GETDLGCODE,            "WM_GETDLGCODE"           },  // 0x0087
    { 0x0088,                   "wm_syncpaint"            },  // 0x0088
    { 0x0089,                   "wm_synctask"             },  // 0x0089
    { WM_NCMOUSEMOVE,           "WM_NCMOUSEMOVE"          },  // 0x00A0
    { WM_NCLBUTTONDOWN,         "WM_NCLBUTTONDOWN"        },  // 0x00A1
    { WM_NCLBUTTONUP,           "WM_NCLBUTTONUP"          },  // 0x00A2
    { WM_NCLBUTTONDBLCLK,       "WM_NCLBUTTONDBLCLK"      },  // 0x00A3
    { WM_NCRBUTTONDOWN,         "WM_NCRBUTTONDOWN"        },  // 0x00A4
    { WM_NCRBUTTONUP,           "WM_NCRBUTTONUP"          },  // 0x00A5
    { WM_NCRBUTTONDBLCLK,       "WM_NCRBUTTONDBLCLK"      },  // 0x00A6
    { WM_NCMBUTTONDOWN,         "WM_NCMBUTTONDOWN"        },  // 0x00A7
    { WM_NCMBUTTONUP,           "WM_NCMBUTTONUP"          },  // 0x00A8
    { WM_NCMBUTTONDBLCLK,       "WM_NCMBUTTONDBLCLK"      },  // 0x00A9
    { WM_KEYDOWN,               "WM_KEYDOWN"              },  // 0x0100
    { WM_KEYUP,                 "WM_KEYUP"                },  // 0x0101
    { WM_CHAR,                  "WM_CHAR"                 },  // 0x0102
    { WM_SYSKEYDOWN,            "WM_SYSKEYDOWN"           },  // 0x0104
    { WM_SYSKEYUP,              "WM_SYSKEYUP"             },  // 0x0105
    { WM_SYSCHAR,               "WM_SYSCHAR"              },  // 0x0106
    { WM_SYSDEADCHAR,           "WM_SYSDEADCHAR"          },  // 0x0107
    { WM_INITDIALOG,            "WM_INITDIALOG"           },  // 0x0110
    { WM_COMMAND,               "WM_COMMAND"              },  // 0x0111
    { WM_SYSCOMMAND,            "WM_SYSCOMMAND"           },  // 0x0112
    { WM_TIMER,                 "WM_TIMER"                },  // 0x0113
    { WM_HSCROLL,               "WM_HSCROLL"              },  // 0x0114
    { WM_VSCROLL,               "WM_VSCROLL"              },  // 0x0115
    { WM_INITMENU,              "WM_INITMENU"             },  // 0x0116
    { WM_INITMENUPOPUP,         "WM_INITMENUPOPUP"        },  // 0x0117
    { 0x0118,                   "wm_systimer"             },  // 0x0118
    { WM_MENUSELECT,            "WM_MENUSELECT"           },  // 0x011F
    { WM_MENUCHAR,              "WM_MENUCHAR"             },  // 0x0120
    { WM_ENTERIDLE,             "WM_ENTERIDLE"            },  // 0x0121
    { 0x0131,                   "wm_lbtrackpoint"         },  // 0x0131
    { WM_MOUSEMOVE,             "WM_MOUSEMOVE"            },  // 0x0200
    { WM_LBUTTONDOWN,           "WM_LBUTTONDOWN"          },  // 0x0201
    { WM_LBUTTONUP,             "WM_LBUTTONUP"            },  // 0x0202
    { WM_LBUTTONDBLCLK,         "WM_LBUTTONDBLCLK"        },  // 0x0203
    { WM_RBUTTONDOWN,           "WM_RBUTTONDOWN"          },  // 0x0204
    { WM_RBUTTONUP,             "WM_RBUTTONUP"            },  // 0x0205
    { WM_RBUTTONDBLCLK,         "WM_RBUTTONDBLCLK"        },  // 0x0206
    { WM_MBUTTONDOWN,           "WM_MBUTTONDOWN"          },  // 0x0207
    { WM_MBUTTONUP,             "WM_MBUTTONUP"            },  // 0x0208
    { WM_MBUTTONDBLCLK,         "WM_MBUTTONDBLCLK"        },  // 0x0209
    { WM_PARENTNOTIFY,          "WM_PARENTNOTIFY"         },  // 0x0210
    { 0x0211,                   "wm_entermenuloop"        },  // 0x0211
    { 0x0212,                   "wm_exitmenuloop"         },  // 0x0212
    { WM_MDICREATE,             "WM_MDICREATE"            },  // 0x0220
    { WM_MDIDESTROY,            "WM_MDIDESTROY"           },  // 0x0221
    { WM_MDIACTIVATE,           "WM_MDIACTIVATE"          },  // 0x0222
    { WM_MDIRESTORE,            "WM_MDIRESTORE"           },  // 0x0223
    { WM_MDINEXT,               "WM_MDINEXT"              },  // 0x0224
    { WM_MDIMAXIMIZE,           "WM_MDIMAXIMIZE"          },  // 0x0225
    { WM_MDITILE,               "WM_MDITILE"              },  // 0x0226
    { WM_MDICASCADE,            "WM_MDICASCADE"           },  // 0x0227
    { WM_MDIICONARRANGE,        "WM_MDIICONARRANGE"       },  // 0x0228
    { WM_MDIGETACTIVE,          "WM_MDIGETACTIVE"         },  // 0x0229
    { 0x022A,                   "wm_dropobject"           },  // 0x022A
    { 0x022B,                   "wm_querydropobject"      },  // 0x022B
    { 0x022C,                   "wm_begindrag"            },  // 0x022C
    { 0x022D,                   "wm_dragloop"             },  // 0x022D
    { 0x022E,                   "wm_dragselect"           },  // 0x022E
    { 0x022F,                   "wm_dragmove"             },  // 0x022F
    { WM_MDISETMENU,            "WM_MDISETMENU"           },  // 0x0230
    { 0x0231,                   "wm_entersizemove"        },  // 0x0231
    { 0x0232,                   "wm_exitsizemove"         },  // 0x0232
    { WM_DROPFILES,             "WM_DROPFILES"            },  // 0x0233
    { WM_CUT,                   "WM_CUT"                  },  // 0x0300
    { WM_COPY,                  "WM_COPY"                 },  // 0x0301
    { WM_PASTE,                 "WM_PASTE"                },  // 0x0302
    { WM_CLEAR,                 "WM_CLEAR"                },  // 0x0303
    { WM_UNDO,                  "WM_UNDO"                 },  // 0x0304
    { WM_RENDERFORMAT,          "WM_RENDERFORMAT"         },  // 0x0305
    { WM_RENDERALLFORMATS,      "WM_RENDERALLFORMATS"     },  // 0x0306
    { WM_DESTROYCLIPBOARD,      "WM_DESTROYCLIPBOARD"     },  // 0x0307
    { WM_DRAWCLIPBOARD,         "WM_DRAWCLIPBOARD"        },  // 0x0308
    { WM_PAINTCLIPBOARD,        "WM_PAINTCLIPBOARD"       },  // 0x0309
    { WM_VSCROLLCLIPBOARD,      "WM_VSCROLLCLIPBOARD"     },  // 0x030A
    { WM_SIZECLIPBOARD,         "WM_SIZECLIPBOARD"        },  // 0x030B
    { WM_ASKCBFORMATNAME,       "WM_ASKCBFORMATNAME"      },  // 0x030C
    { WM_CHANGECBCHAIN,         "WM_CHANGECBCHAIN"        },  // 0x030D
    { WM_HSCROLLCLIPBOARD,      "WM_HSCROLLCLIPBOARD"     },  // 0x030E
    { WM_QUERYNEWPALETTE,       "WM_QUERYNEWPALETTE"      },  // 0x030F
    { WM_PALETTEISCHANGING,     "WM_PALETTEISCHANGING"    },  // 0x0310
    { WM_PALETTECHANGED,        "WM_PALETTECHANGED"       },  // 0x0311
  };
*/