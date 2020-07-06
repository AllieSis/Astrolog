/*
** Astrolog (Version 7.00) File: xdata.cpp
**
** IMPORTANT NOTICE: Astrolog and all chart display routines and anything
** not enumerated below used in this program are Copyright (C) 1991-2020 by
** Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
** Permission is granted to freely use, modify, and distribute these
** routines provided these credits and notices remain unmodified with any
** altered or distributed versions of the program.
**
** The main ephemeris databases and calculation routines are from the
** library SWISS EPHEMERIS and are programmed and copyright 1997-2008 by
** Astrodienst AG. The use of that source code is subject to the license for
** Swiss Ephemeris Free Edition, available at http://www.astro.com/swisseph.
** This copyright notice must not be changed or removed by any user of this
** program.
**
** Additional ephemeris databases and formulas are from the calculation
** routines in the program PLACALC and are programmed and Copyright (C)
** 1989,1991,1993 by Astrodienst AG and Alois Treindl (alois@astro.ch). The
** use of that source code is subject to regulations made by Astrodienst
** Zurich, and the code is not in the public domain. This copyright notice
** must not be changed or removed by any user of this program.
**
** The original planetary calculation routines used in this program have
** been copyrighted and the initial core of this program was mostly a
** conversion to C of the routines created by James Neely as listed in
** 'Manual of Computer Programming for Astrologers', by Michael Erlewine,
** available from Matrix Software.
**
** Atlas composed using data from https://www.geonames.org/ licensed under a
** Creative Commons Attribution 4.0 License. Time zone changes composed using
** public domain TZ database: https://data.iana.org/time-zones/tz-link.html
**
** The PostScript code within the core graphics routines are programmed
** and Copyright (C) 1992-1993 by Brian D. Willoughby (brianw@sounds.wa.com).
**
** More formally: This program is free software; you can redistribute it
** and/or modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of the
** License, or (at your option) any later version. This program is
** distributed in the hope that it will be useful and inspiring, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details, a copy of which is in the
** LICENSE.HTM file included with Astrolog, and at http://www.gnu.org
**
** Initial programming 8/28-30/1991.
** X Window graphics initially programmed 10/23-29/1991.
** PostScript graphics initially programmed 11/29-30/1992.
** Last code change made 6/4/2020.
*/

#include "astrolog.h"


#ifdef GRAPH
/*
******************************************************************************
** Graphics Global Variables.
******************************************************************************
*/

GS gs = {
#ifdef ISG
  ftNone,
#else
  ftBmp,
#endif
  fTrue, fTrue, fFalse, fFalse, fTrue, fTrue, fFalse, fTrue, fTrue, fFalse,
  fFalse, fFalse, fFalse, fFalse, fFalse, fFalse, fFalse, fFalse, fFalse,
  fFalse, fTrue, fFalse, DEFAULTX, DEFAULTY,
#ifdef WIN
  -10,
#else
  0,
#endif
  200, 100, 0, 0, 0, 3, 0, 0, 0.0, 0.0, BITMAPMODE, 0, 8.5, 11.0, NULL,
  0, 25, 11, oCore, 0.0, 1000, 0, 600, 1111, fFalse, fFalse, 7, "", ""};

GI gi = {
  0, fFalse, -1,
  NULL, 0, NULL, NULL, 0.0, fFalse, fFalse,
  2, 1, 1, 20, 10, kWhite, kBlack, kLtGray, kDkGray, 0, 0, 0, 0, -1, -1,
  NULL, 0, 0, NULL,
#ifdef SWISS
  NULL, 0,
#endif
#ifdef X11
  NULL, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
#ifdef PS
  fFalse, 0, fFalse, 0, 0, 1.0,
#endif
#ifdef META
  NULL, NULL, MAXMETA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
#endif
#ifdef WIRE
  NULL, 0, 0, -1, 0,
#endif
  };

#ifdef WIN
WI wi = {
  (HINSTANCE)NULL, (HWND)NULL, (HWND)NULL, (HMENU)NULL, (HACCEL)NULL, hdcNil,
  hdcNil, (HWND)NULL, (HPEN)NULL, (HBRUSH)NULL, (HFONT)NULL, (HANDLE)NULL,
  0, 0, 0, 0, 0, 0, 0, -1, -1,
  0, 0, 0, -1, fFalse, fTrue, fFalse, fTrue, fFalse, fFalse,
  1, fFalse, {0, 0, 0, 0}, fFalse, fFalse,

  /* Window user settings. */
  fFalse, fTrue, fTrue, fFalse, fTrue, fFalse, fFalse, fFalse, fFalse, fFalse,
  fFalse,
  0, kBlack, 1, 1000};

OPENFILENAME ofn = {
  sizeof(OPENFILENAME), (HWND)NULL, (HINSTANCE)NULL, NULL, NULL, 0, 1, NULL,
  cchSzMaxFile, NULL, cchSzMaxFile, NULL, NULL, OFN_OVERWRITEPROMPT, 0, 0,
  NULL, 0, NULL, NULL};

PRINTDLG prd = {
  sizeof(PRINTDLG), (HWND)NULL, (HGLOBAL)NULL, (HGLOBAL)NULL, hdcNil,
  PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC | PD_USEDEVMODECOPIES,
  0, 0, 0, 0, 1, (HINSTANCE)NULL, 0, NULL, NULL, (LPCSTR)NULL, (LPCSTR)NULL,
  (HGLOBAL)NULL, (HGLOBAL)NULL};

char szFileName[cchSzMaxFile];
char szFileTitle[cchSzMaxFile];
char *szFileTemp = szFileTempCore;
#endif

#ifdef WCLI
WI wi = {
  (HINSTANCE)NULL, (HWND)NULL, (HWND)NULL, hdcNil, (HPEN)NULL, (HBRUSH)NULL,
  0, 0, fFalse, fFalse, fFalse};
#endif

/* Color tables for Astrolog's graphics palette. */

CONST KV rgbbmpDef[cColor] = {
  0x000000, 0x00007F, 0x007F00, 0x007F7F,
  0x7F0000, 0x7F007F, 0x7F7F00, 0xBFBFBF,
  0x7F7F7F, 0x0000FF, 0x00FF00, 0x00FFFF,
  0xFF0000, 0xFF00FF, 0xFFFF00, 0xFFFFFF};
KV rgbbmp[cColor];
#ifdef X11
KV rgbind[cColor], fg, bg;
#endif
#ifdef WIN
CONST int ikPalette[cColor] =
  {-0, -1, 1, 4, 6, 3, -8, 5, -3, -2, -4, -5, -7, 2, 7, -6};
CONST int rgcmdMode[gMax] = {0,
  cmdChartList, cmdChartWheel, cmdChartGrid, cmdChartHorizon, cmdChartOrbit,
  cmdChartSector, cmdChartCalendar, cmdChartInfluence, cmdChartEsoteric,
  cmdChartAstroGraph, cmdChartEphemeris, cmdTransit, cmdTransit,
  cmdChartSphere, cmdChartMap, cmdChartGlobe, cmdChartPolar,
  0/*cmdRelBiorhythm*/, cmdChartAspect, cmdChartMidpoint, cmdChartArabic,
  cmdChartRising, cmdTransit, cmdTransit, cmdTransit, cmdTransit,
  cmdHelpSign, cmdHelpObject, cmdHelpAspect, cmdHelpConstellation,
  cmdHelpPlanetInfo, cmdHelpRay, cmdHelpMeaning, cmdHelpSwitch,
  cmdHelpObscure, cmdHelpKeystroke, cmdHelpCredit};
#endif
char *szWheelX[4+1] = {NULL, NULL, NULL, NULL, NULL};

/* These are the actual color arrays and variables used by the program.      */
/* Technically, Astrolog always assumes we are drawning on a color terminal. */
/* For B/W graphics, all the values below are filled with black or white.    */

KI kMainB[9], kRainbowB[cRainbow+1], kElemB[cElem], kAspB[cAspect+1],
  kObjB[objMax], kRayB[cRay+2];

/* Some physical X window variables dealing with the window itself. */

#ifdef X11
XSizeHints hint;
char xkey[10];
#endif


/*
******************************************************************************
** Graphics Table Data.
******************************************************************************
*/

#ifdef VECTOR
//                                  ESMMVMJSUNPccpjvnslpveA23I56D89M12
CONST char szObjectFont[oNorm+1] = ";QRSTUVWXYZ     <> ?  a  c     b  ";
//                                    C OSTSisssqbssnbbtqPC
CONST char szAspectFont[cAspect2+1] = "!\"#$'&%()+*         ";
#endif

CONST char *szDrawSign[cSign+2] = {"",
  "ND4HU2HLGDFBR6EUHLGD2G",                /* Aries        */
  "BL3D2F2R2E2U2H2NE2L2NH2G2",             /* Taurus       */
  "BLU3LHBR7GLNL3D6NL3RFBL7ERU3",          /* Gemini       */
  "BFNDERFDGLNHGL2HLHBU3NEDFREUHNLER2FRF", /* Cancer       */
  "BF4H2UEU2H2L2G2D2FDGH",                 /* Leo          */
  "BF4BLH2U3E2D5G2BU5U2HGND6HGND6H",       /* Virgo        */
  "BGNL3HUER2FDGR3BD2L8",                  /* Libra        */
  "BH4FND6EFND6EFD6FREU",                  /* Scorpio      */
  "BG4E3NH2NF2E5NL2D2",                    /* Sagittarius  */
  "BH3BLED4FND2EU2EUFNDERFDGLNHF2D2G",     /* Capricorn #1 */
  "BG4EUEDFDEUEDFDEUEBU5GDGUHUGDGUHUGDG",  /* Aquarius     */
  "NL4NR4BH4F2D4G2BR8H2U4E2",              /* Pisces       */
  "BH4RFR2ER3G3D2GDFR2EU2HL3G2DG"};        /* Capricorn #2 */

CONST char *szDrawSign2[cSign+2] = {"",
  "BD8U7HU3HU2H2L2G2D2F2BR12E2U2H2L2G2D2GD3G",     /* Aries  */
  "BH6BU2FDFRFNR4GLGDGD4FDFRFR4EREUEU4HUHLHEREUE", /* Taurus */
  "BL3U6LHLHBR14GLGLNL6D12NL6RFRFBL14ERERU6",      /* Gemini */
  "BF5NLRE2U2H2L2G2D2F2G2L4HL2H3BE6NH2D2G2L2H2U2E2R2E2R4FR2F3", /* Cancer */
  "BF8H4U2E2U4HUHLHL4GLGDGD4FDFD2GL2HU",           /* Leo   */
  "BF8BL2H3UHU5E4D9GDG3BU10U4H2G2ND12H2G2ND12H2",  /* Virgo */
  "", /* Libra */
  "BH8F2ND12E2F2ND12E2F2D12F2RE2U3NGF", /* Scorpio */
  "", /* Sagittarius  */
  "BH6BL2E2D4FD4FND4EU2EUEU2EUF2ND2E2R2F2D2G2L2NH2F4D4G2", /* Capricorn #1 */
  "BG8EUE2UEDFD2FDEUE2UEDFD2FDEUE2UEBU10GDG2DGUHU2HUGDG2DGUHU2HUGDG2DG",
    /* Aquarius */
  "NL8NR8BH8F3DFD6GDG3BR16H3UHU6EUE3",             /* Pisces */
  "BH8RFRFR4ER2ER4G5DGD2GDGD2F2R4E2U4H2L6G4DGDG"}; /* Capricorn #2 */

CONST char *szDrawSign3[cSign+2] = {"",
  "BD12U10HU4HU2HU2H3L3G3D3F3BR18E3U3H3L3G3D2GD2GD4",     /* Aries  */
  "BL9D6FDF3RFR6ERE3UEU6HUH3LHNL6ERE3UEBL18FDF3RFGLG3DG", /* Taurus */
  "BL4U9L2HLH2BR21G2LGL2NL9D18NL9R2FRF2BL21E2RER2U9",     /* Gemini */
  "BF3ND3E3R3F3D3G3L3NH3G2LGL6HLHLH4BU9NE3D3F3R3E3U3H3NL3E2RER6FRFRF4",
    /* Cancer */
  "BF12H6U3E2UEU5HUH3LHL6GLG3DGD5FDFDFD3G2L2H2U",        /* Leo   */
  "BF12BL3H5UHU8E6D14GDG5BU15U7H2L2G2ND19H2L2G2ND19H2L", /* Virgo */
  "", /* Libra        */
  "BH12RF2ND19E2R2F2ND19E2R2F2D20F2R3E2U5NG2F2", /* Scorpio */
  "", /* Sagittarius  */
  "BH9BL3E3D4FD4FD4FND6EU2EUEU2EUEU2EUF3ND3E3R3F3D3G3L3NH3F5DFD4GDG2",
    /* Capricorn #1 */
  "BG12E2UEUEUE2D2FDFDFD2E2UEUEUE2D2FDFDFD2E2UEUEUE2BU15"
    "G2DGDGDG2U2HUHUHU2G2DGDGDG2U2HUHUHU2G2DGDGDG2", /* Aquarius */
  "NL12NR12BH12F4DFDFD8GDGDG4BR24H4UHUHU8EUEUE4", /* Pisces */
  "BH12RFRFR2FR6ER2ER2ER4G8DGD3GDGDGD3F3R6E3U6H3L8GLG4DGDGDG2"};
    /* Capricorn #2 */

CONST char *szDrawObjectDef[objMax+5] = {
  "ND4NL4NR4U4LGLDGD2FDRFR2ERUEU2HULHL",    /* Earth   */
  "U0BH3DGD2FDRFR2ERUEU2HULHL2GL",          /* Sun     */
  "BG3E2U2H2ER2FRDFD2GDLGL2H",              /* Moon    */
  "BD4UNL2NR2U2REU2HNEL2NHGD2FR",           /* Mercury */
  "LHU2ER2FD2GLD2NL2NR2D2",                 /* Venus   */
  "HLG2DF2RE2UHE4ND2L2",                    /* Mars    */
  "BH3RFDGDGDR5NDNR2U6E",                   /* Jupiter */
  "BH3R2NUNR2D4ND2E2RFDGDF",                /* Saturn  */
  "BD4NEHURBFULU3NUNR2L2NU2DGBU5NFBR6GD3F", /* Uranus #1 */
  "BD4U2NL2NR2U6NFNGBL3NGD2F2R2E2U2F",      /* Neptune   */
  "D2NL2NR2D2BU8GFEHBL3D2F2R2E2U2",         /* Pluto  #1 */
  "BG2LDFEULU3NURFRFBU5GLGLU2",             /* Chiron          */
  "BD4UNL3NR3U2RE2UH2L2G",                  /* Ceres           */
  "BD4UNL3NR3UE2HUHNUGDGF2",                /* Pallas Athena   */
  "BD4UNL2NR2U4NL4NR4NE3NF3NG3NH3U3",       /* Juno            */
  "BU4DBG3NLFDF2E2UERBH2GDGHUH",            /* Vesta           */
  "BG2LGFEU2HU2E2R2F2D2GD2FEHL",            /* North Node      */
  "BH2LHEFD2GD2F2R2E2U2HU2EFGL",            /* South Node      */
  "BG4E8BG2FD2G2L2H2U2E2R2F",               /* Lilith #1       */
  "NE2NF2NG2H2GD2F2R2E2U2H2L2G",            /* Part of Fortune */
  "U2NHNEBD4NGNFU2L2NHNGR4NEF",             /* Vertex          */
  "BH4NR3D4NR2D4R3BR2U8R2FD2GL2",           /* East Point      */
  "BG4U4NR2U3EFD7BR2NURU2HU2RDBR3ULD5RU",   /* Ascendant  */
  "BH3ER4FD2GLGLG2DR6",                     /* 2nd Cusp   */
  "BH3ER4FD2GNL3FD2GL4H",                   /* 3rd Cusp   */
  "BH4R2NR2D8NL2R2BR4NUL2U8R2D",            /* Nadir      */
  "BG3FR4EU2HL5U4R6",                       /* 5th Cusp   */
  "BE3HL4GD6FR4EU2HL4G",                    /* 6th Cusp   */
  "BH4D8REU6HLBF7DRU2HU2RDBG4NRU3NRU2R",    /* Descendant */
  "BL2GD2FR4EU2HNL4EU2HL4GD2F",             /* 8th Cusp   */
  "BG3FR4EU6HL4GD2FR4E",                    /* 9th Cusp   */
  "BG4U8F2ND6E2D8BR4NUL2U8R2D",             /* Midheaven  */
  "BH3ED8NLRBR2RNRU8G",                     /* 11th Cusp  */
  "BG4RNRU8GBR4ER2FD2GLG2D2R4",             /* 12th Cusp  */
  "NU4D4NH3E3",                             /* Vulcan   */
  "BH4BRFDG2DR8BG3UNL2NR2U5LUEFDL",         /* Cupido   */
  "BENUNL2NR2D3ND2NR2L2H2U2E2R4",           /* Hades    */
  "BU4NG2NF2D7NDBLHLBR6LGL2GLBR6LHL",       /* Zeus     */
  "BU2D3ND3NR2L2BH2UE2R4F2D",               /* Kronos   */
  "U3NLR2NRD3NL2NR2D4NRL2NLU4L4UEUH",       /* Apollon  */
  "BUNU2NL2NR2D2ND3LHU2ENHR2NEFD2GL",       /* Admetos  */
  "G2DGR6HUH2U4NG2F2",                      /* Vulcanus */
  "ND4U4BL3DF2R2E2UBD8UH2L2G2D",            /* Poseidon */
  "T", "T", "T", "T", "T", "T", "T", "T",   /* Stars    */
  "T", "T", "T", "T", "T", "T", "T", "T",
  "T", "T", "T", "T", "T", "T", "T", "T",
  "T", "T", "T", "T", "T", "T", "T", "T",
  "T", "T", "T", "T", "T", "T", "T", "T",
  "T", "T", "T", "T", "T", "T", "T",
  "BD2D0BU6NG2NF2D4LGD2FR2EU2HL",           /* Uranus #2 */
  "BL3R5EU2HL5D8R5",                        /* Pluto  #2 */
  "UERHL2G2D2F2R2ELHU",                     /* Lilith #2 */
  "ND4U4NG3F3",                             /* Pluto  #3 */
  };
CONST char *szDrawObject[objMax+5];

CONST char *szDrawObjectDef2[objMax+5] = {
  "ND8NL8NR8U8L2GLG3DGD4FDF3RFR4ERE3UEU4HUH3LHL2", /* Earth */
  "U0BU8L2GLG3DGD4FDF3RFR4ERE3UEU4HUH3LHL2",       /* Sun   */
  "BG6E3UEU2HUH3E2R4FRF3DFD4GDG3LGL4H2",           /* Moon  */
  "", /* Mercury */
  "", /* Venus   */
  "BELHL4G3D4F3R4E3U4HUE7ND5L5",      /* Mars    */
  "BH6BRRF2D2GDGDGDGDR10ND2NR4U12E2", /* Jupiter */
  "", /* Saturn  */
  "BD4LGD2FR2EU2HLU6NU2NR4L4NU4D2G2BU10NF2BR12G2D6F2",     /* Uranus #1 */
  "BD8U4NL4NR4U12NF2NG2BL6DNFNGD3FDFRFR4EREUEU3NGNFU",     /* Neptune   */
  "D4NL4NR4D4BU16LGD2FR2EU2HLBL6D4FDFRFR4EREUEU4",         /* Pluto  #1 */
  "BG4LGD2FR2EU2HLU7RF2RF2RFBU10GLG2LG2BLU5",     /* Chiron        */
  "BD8U2NL6NR6U4R3E3U4H3L4G2",                    /* Ceres         */
  "BD8U2NL6NR6U2E4HUHUHUHNUGDGDGDGF4",            /* Pallas Athena */
  "BD8U2NL4NR4U8NL7NR7NE5NF5NG5NH5U6",            /* Juno          */
  "BU8D3BG5NL3DF2DF2DFEUE2UE2UR3BH4GDG2DGHUH2UH", /* Vesta         */
  "BG4BDHL2GD2FR2EU5H2U4E4R4F4D4G2D5FR2EU2HL2G",  /* North Node    */
  "BH4BUGL2HU2ER2FD5G2D4F4R4E4U4H2U5ER2FD2GL2H",  /* South Node    */
  "", /* Lilith #1       */
  "", /* Part of Fortune */
  "", /* Vertex          */
  "", /* East Point      */
  "BG8U8NR4U6E2F2D14BR4NHREU3HLHU3ERFBR6HLGD8FRE", /* Ascendant */
  "", /* 2nd Cusp  */
  "", /* 3rd Cusp  */
  "BH8R4NR4D16NL4R4BR8BUNUGL3HU14ER3FD", /* Nadir */
  "", /* 5th Cusp  */
  "", /* 6th Cusp  */
  "BH8D16R2E2U12H2L2BF14BGFREU3HLHU3ERFBG9NR3U5NR3U5R3", /* Descendant */
  "", /* 8th Cusp  */
  "", /* 9th Cusp  */
  "BG8U16F4ND12E4D16BR8BUNUGL3HU14ER3FD", /* Midheaven */
  "", /* 11th Cusp */
  "", /* 12th Cusp */
  "", /* Vulcan    */
  "BH8BR2F2D2G2DG2DR16BG6U2NL4NR4U10LHU2ER2FD2GL", /* Cupido    */
  "", /* Hades     */
  "BU8NG4NF4D14ND2BLHL2HLBR12LGL2GL2GL2GLBR12LHL2HL", /* Zeus      */
  "", /* Kronos    */
  "U6NL2R4NR2D6NL4NR4D8NR2L4NL2U8L8UE2U3H2", /* Apollon   */
  "", /* Admetos   */
  "G2DG2DG2R12H2UH2UH2U8NG4F4", /* Vulcanus  */
  "", /* Poseidon  */
  "", "", "", "", "", "", "", "", "", "",   /* Stars    */
  "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "",
  "", /* Uranus #2 */
  "", /* Pluto  #2 */
  "", /* Lilith #2 */
  "", /* Pluto  #3 */
  };
CONST char *szDrawObject2[objMax+5];

CONST char *szDrawHouse[cSign+1] = {"",
  "BD2NLNRU4L", "BHBUR2D2L2D2R2", "BHBUR2D2NL2D2L2",
  "BHBUD2R2NU2D2", "BEBUL2D2R2D2L2", "NLRD2L2U4R2",
  "BHBUR2DG2D", "NRLU2R2D4L2U2", "NRLU2R2D4L2",
  "BH2NLD4NLRBR2U4R2D4L2", "BH2NLD4NLRBR2RNRU4L", "BH2NLD4NLRBR2NR2U2R2U2L2"};

CONST char *szDrawHouse2[cSign+1] = {"",
  "BD4NL2NR2U8G2", "BH2BUER2FD2G4DR4", "BH2BUER2FD2GNLFD2GL2H",
  "BH2BU2D4R3NU4NRD4", "BE2BU2L4D4R3FD2GL2H", "NL2RFD2GL2HU6ER2F",
  "", "NRLHU2ER2FD2GFD2GL2HU2E", "NR2LHU2ER2FD6GL2H",
  "BH4NG2D8NL2R2BR5HU6ER2FD6GL2", "BH4NG2D8NL2R2BR4R2NR2U8G2",
  "BH4NG2D8NL2R2BR4NR4UE4U2HL2G"};

CONST char *szDrawHouse3[cSign+1] = {"",
  "BD6NL3NR3U12G3", "BH3BU2ER4FD4G6DR6", "BH3BU2ER4FD3G2NL2F2D3GL4H",
  "BH3BU3D6R5NU6NRD6", "BE3BU3L6D6R5FD4GL4H", "NL3R2FD4GL4HU10ER4F",
  "", "NR2L2HU4ER4FD4GFD4GL4HU4E", "NR3L2HU4ER4FD10GL4H",
  "BH6NG3D12NL3R3BR7HU10ER4FD10GL4", "BH6NG3D12NL3R3BR6R3NR3U12G3",
  "BH6NG3D12NL3R3BR6NR6UE6U4HL4G"};

CONST char *szDrawAspectDef[cAspect2+1] = {"",
  "HLG2DF2RE2UHE4",                        /* Conjunction      */
  "BGL2GDFREU2E2U2ERFDGL2",                /* Opposition       */
  "BH4R8D8L8U8",                           /* Square           */
  "BU4GDGDGDGDR8UHUHUHUH",                 /* Trine            */
  "BLNH3NG3RNU4ND4RNE3F3",                 /* Sextile          */
  "BG4EUEUEUEUNL4NR4BDFDFDFDF",            /* Inconjunct       */
  "BH4FDFDFDFDNL4NR4BUEUEUEUE",            /* Semisextile      */
  "BE4G8R8",                               /* Semisquare       */
  "BD2L3U6R6D6L3D2R2",                     /* Sesquiquadrature */
  "F4BU3U2HULHL2GLDGD2FDRFR2E3",           /* Quintile         */
  "BD2U3NR3NU3L3BD5R6",                    /* Biquintile       */
  "BU2D3NR3ND3L3BU5R6",                    /* Semiquintile     */
  "BH3R6G6",                               /* Septile      */
  "BR3L5HUER4FD4GL4H",                     /* Novile       */
  "BF2UHL2GFR3DGL3BE6LNLU2NRLBL4LNLD2NLR", /* Binovile     */
  "BL2R4G4BE6LNLU2NRLBL4LNLD2NLR",         /* Biseptile    */
  "BL2R4G4BE6L7NLU2NLR3ND2R3ND2R",         /* Triseptile   */
  "BF2UHL2GFR3DGL3BU6LNLU2NLRBR2F2E2",     /* Quatronovile */
  "BU4BLD8BR2U8",                          /* Parallel       */
  "BU4BLD8BR2U8BF3BLL6BD2R6"};             /* Contraparallel */
CONST char *szDrawAspect[cAspect2+1];

CONST char *szDrawAspectDef2[cAspect2+1] = {"",
  "BELHL4G3D4F3R4E3U4HUE7",                /* Conjunction */
  "BG3HL2G2D2F2R2E2U2HE6HU2E2R2F2D2G2L2H", /* Opposition  */
  "", /* Square           */
  "BU8GDGDGDGDGDGDGDGDR16UHUHUHUHUHUHUHUH",     /* Trine       */
  "BU8D16BL8BU2E3RE3R2E3RE3BL16F3RF3R2F3RF3",   /* Sextile     */
  "BG8EUEUEUEUEUEUEUEUNL8NR8BDFDFDFDFDFDFDFDF", /* Inconjunct  */
  "BH8FDFDFDFDFDFDFDFDNL8NR8BUEUEUEUEUEUEUEUE", /* Semisextile */
  "", /* Semisquare       */
  "", /* Sesquiquadrature */
  "BFF7BU6U4HUH3LHL4GLG3DGD4FDF3RFR4E6", /* Quintile */
  "", /* Biquintile   */
  "", /* Semiquintile */
  "", /* Septile      */
  "", /* Novile       */
  "", /* Binovile     */
  "", /* Biseptile    */
  "", /* Triseptile   */
  "", /* Quatronovile */
  "", /* Parallel        */
  ""}; /* Contraparallel */
CONST char *szDrawAspect2[cAspect2+1];

CONST char *szDrawCh[256-32] = {"",
  "BR2D4BD2D0", "BRD2BR2U2", "BD2R4BD2L4BFU4BR2D4", "BR2D6BENL3EHL2HER3",
  "RDLNUBR4G4BR4DLUR", "BD2NF4UEFDG2DFRE2", "BR2DG", "BR3G2D2F2", "BRF2D2G2",
  "BD2FNGRNU2ND2RNEF", "BD3R2NU2ND2R2", "BD5BR2DG", "BD3R4", "BD6BRRULD",
  "BD5E4", /* Special Characters */

  "BDD4NE4FR2EU4HL2G", "BFED6NLR", "BDER2FDG4R4", "BDER2FDGNLFDGL2H",
  "D3R3NU3ND3R", "NR4D3R3FDGL2H", "BR3NFL2GD4FR2EUHL3", "R4DG4D",
  "BDDFNR2GDFR2EUHEUHL2G", "BD5FR2EU4HL2GDFR3", /* Numbers */

  "BR2BD2D0BD2D0", "BR2BD2D0BD2G", "BR3G3F3", "BD2R4BD2L4", "BRF3G3",
  "BDER2FDGLDBD2D0", "BF2DFEU2HL2GD4FR2", /* Special Characters */

  "BD6U4E2F2D2NL4D2", "D6R3EUHNL3EUHL3", "BR3NFL2GD4FR2E", "D6R2E2U2H2L2",
  "NR4D3NR3D3R4", "NR4D3NR3D3", "BR3NFL2GD4FR2EU2L2", "D3ND3R4NU3D3",
  "BRRNRD6NLR", "BD4DFR2EU5", "D3ND3RNE3F3", "D6R4", "ND6F2NDE2D6",
  "ND6F4ND2U4", "BDD4FR2EU4HL2G", "R3FDGL3NU3D3", "BDD4FRENHNFEU3HL2G",
  "ND6R3FDGL2NLF3", "BR3NFL2GDFR2FDGL2H", "R2NR2D6", "D5FR2EU5",
  "D2FDFNDEUEU2", "D6E2NUF2U6", "DF4DBL4UE4U", "D2FRND3REU2",
  "R4DG4DR4", /* Upper Case Letters */

  "BR3L2D6R2", "BDF4", "BRR2D6L2", "BD2E2F2", "BD6R4", "BR2DF", /* Symbols */

  "BF4G2LHU2ER2FD3", "D5NDFR2EU2HL2G", "BF4BUHL2GD2FR2E", "BR4D5NDGL2HU2ER2F",
  "BD4R4UHL2GD2FR3", "BD3RNR3ND3U2ERF", "BD8R3EU4HL2GD2FR2E", "D3ND3ER2FD3",
  "BR2D0BD2D4", "BR2D0BD2D5GLH", "D4ND2REREBD4HLH", "BR2D6",
  "BD2DND3EFNDEFD3", "BD2DND3ER2FD3", "BD3D2FR2EU2HL2G", "BD2DND5ER2FD2GL2H",
  "BR4BD8U5HL2GD2FR2E", "BD2DND3ER2F", "BD6R3EHL2HER3", "BR2D2NL2NR2D4",
  "BD2D3FRE2NU2D2", "BD2DFDFEUEU", "BD2D3FENUFEU3", "BD2F2NG2NE2F2",
  "BD2D3FR2ENU3D2GL3", "BD2R4G4R4", /* Lower Case Letters */

  "BR3GDGFDF", "BR2D2BD2D2", "BRFDFGDG", "BFEFE", "", /* Symbols */

  "BR3NFLGDNRNLD2NRNLDFRE", "BR2FGHE", "BR2BD4DG", "BR4LGD2NRNLD2GL",
  "BD6EBFE", "BD6D0BR2D0BR2D0", "BR2DNL2NR2D4", "BR2DNL2NR2D4NL2NR2D",
  "BR2NGF", "RDLNUBR4G2L2BD2RDLUBR3RDLU", "BD5FR2EUHL2HUER2NFBU2GH", "BF2FG",
  "BRNRGD4FRNR2U3NR2U3R2", "", "R4DG4DR4BU6BHHG", "", /* 80-8F */

  "", "BR3GD", "BR2DG", "BFNDEBFNED", "BD2EUBR2DG", "BD3BRRDLU", "BD3BRR2",
  "BD3R4", "BFEFE", "RND2RBRND2FNDED2", "BD6R3EHL2HENR3BU2FE", "BF2GF",
  "BD4NED2FRNR2U2R2UHLNLD2", "", "BD6NR4E4L4BUBEFE",
  "D2FRND3REU2BUBHD0BL2D0", /* 90-9F */

  "", "BR2D0BD2D4", "BR2DND6NLRFBL4NED3FR2E", "BD6NR4EU2NLNR2U2ERF",
  "BDFEFNEFGNFGHNGHE", "F2NE2DNL2NR2D2NL2NR2D", "BR2D2BD2D2",
  "BD6R3EHNL2EHL2GFBU2HER3", "BRD0BR2D0", "BD2ER2FD2GL2HU2BR3LGFR",
  "BF4U2NUG2LHU2ER2F", "BD3NFEBR3GF", "BD2R4D", "BD3BRR2",
  "BD2ER2FD2GL2HU2BRND2R2DLNLF", "BRR2", /* A0-AF */

  "BR2FGHE", "BR2D2NL2NR2D2BG2R4", "BRR2DL2DR2", "BRR2DNLDL2", "BR3G",
  "BD3D2ND3FRE2NUD2", "BR4ND6L2ND6LGDFR", "BR2BD3D0", "BR2BD5DL", "BRRD2NLR",
  "BDD2FR2EU2HL2G", "BD2FGBR3EH", "RND2BR3BDG4BDBR4UNULU",
  "RND2BR3BDG4BDBR4LURUL", "RD2LBURBR3G4BDBR4UNULU",
  "BR2D0BD2DLGDFR2E", /* B0-BF */

  "BD6U4E2F2D2NL4D2BU7BLH", "BD6U4E2F2D2NL4D2BU7BL3E",
  "BD6U4E2F2D2NL4D2BU7BLHG", "BD6U4E2F2D2NL4D2BU8GHG",
  "BD6U4E2F2D2NL4D2BU8BLD0BL2D0", "BD6U4E2F2D2NL4D2BU8BL2D0",
  "BD6U2NR2U2E2NR2D3NR2D3R2", "BR3NFL2GD4FR2EBG2DL", "NR4D3NR3D3R4BU7BLH",
  "NR4D3NR3D3R4BU7BL3E", "NR4D3NR3D3R4BU7BLHG", "NR4D3NR3D3R4BU8BLD0BL2D0",
  "BRRNRD6NLRBU7H", "BRRNRD6NLRBU7BL2E", "BRRNRD6NLRBU7HG",
  "BRRNRD6NLRBU8D0BL2D0", /* C0-CF */

  "BD3RNRD3RE2U2H2LD3", "ND6F4ND2U4BU2GHG", "BRGD4FR2EU4HNL2BUH",
  "BRGD4FR2EU4HL2BUE", "BRGD4FR2EU4HL2BUEF", "BRGD4FR2EU4HL2BUEFE",
  "BRGD4FR2EU4HL2BU2D0BR2D0", "BDF2NE2NG2F2", "BRGD4FR2EU4HNL2BRGDG2DG",
  "D5FR2EU5BHBLH", "D5FR2EU5BHBLE", "D5FR2EU5BHHG", "D5FR2EU5BHBUD0BL2D0",
  "D2FRND3REU2BHBLE", "D5NDR3EU2HL3", "BD6U5ERFDGF2GL", /* D0-DF */

  "BF4G2LHU2ER2FD3BU6BLH", "BF4G2LHU2ER2FD3BU6BL2E", "BF4G2LHU2ER2FD3BU6BLHG",
  "BF4G2LHU2ER2FD3BU7GHG", "BF4G2LHU2ER2FD3BU6BLD0BL2D0",
  "BF4G2LHU2ER2FD3BU6BL2D0", "BD2R3FDL3GDR2NR2U4", "BF4BUHL2GD2FR2EBG3RU2",
  "BD4R4UHL2GD2FR3BU5BL2H", "BD4R4UHL2GD2FR3BU6BLG", "BD4R4UHL2GD2FR3BU5BLHG",
  "BD4R4UHL2GD2FR3BU6BLD0BL2D0", "BRFBD2D3", "BR3GBD2D3", "BFENFBD3D3",
  "BRD0BR2D0BLBD2D4", /* E0-EF */

  "BR3DNLNRD4NDGLHUERF", "BD2DND3ER2FD3BU6GHG", "BD3D2FR2EU2HL2NGBEH",
  "BD3D2FR2EU2HL2NGBEE", "BD3D2FR2EU2HL2NGBUEF", "BD3D2FR2EU2HL2NGBUEFE",
  "BD3D2FR2EU2HL2NGBU2D0BR2D0", "BD3R4BH2D0BD4D0", "BD3D2FR2EU2HL2GD2BDE4",
  "BD2D3FRE2ND2U2BHBLH", "BD2D3FRE2ND2U2BHBLE", "BD2D3FRE2ND2U2BHHG",
  "BD2D3FRE2ND2U2BHBUD0BL2D0", "BD8R3EU2NU3GL2HU3BEBRE", "D5ND3FR2EU2HL2G",
  "BD8R3EU2NU3GL2HU3BEBUD0BR2D0"}; /* F0-FF */

CONST char *szWorldData[62*3] = {
"-031+70",
"LLRRHLLLLDULLGLLLDULGLLLGLRREDEGGLGGLGLGLLGDRLDRLFFRRERFDFRRREUEEHLUERERUERR\
FGLGLDDFRRRRREFRLGLLLLLGEFDLHGDDLGHLGLLHGLHURDLRRELLLRHUGLDFDLGLLFHGGLGLLLDLL\
LDRRFFDDGLLLLLLGDFGDDRRFRERREEUEREUEFRRERRFFFRFRDDLLLLRFRUREURULHLHHHEF",
"5EUROPE",
"+006+50", "RRERRRRUELLUHHLLREULLELLDGHDUFDEGRDRRLFDLLRGRRGGL", "5ENGLAND",
"+008+55", "GLFGRRREUULL", "5IRELAND",
"+023+64", "RRFRERRREHLLLLLGHLLRFLLRFL", "5ICELAND",
"-011+80", "DDURFRERLGRRLLFRRREEFRRRLHGELLLHRRFRRRRERLLLLLLLLLLLDHGULLL",
"5SVALBARD",
"-014+45",
"FRFRFDDFRDRRLLFRURFHHUERRRRRHUUEERRRRGRDERRLHLRRERRGGRFRFFGLLLLHLLLLGLLDLLLF\
GRFFRERFRERDDDGDGLLDFFEUDDFFDFFDDFFFDFDDDRRERRERRRUERRERURUEEHHLHUGGLLLUUGUHU\
HURRFFRFRRRDRRFRRRRRRRF",
"5MIDDLE EAST",
"-009+41", "DDRUULEUGD", "5SARDINIA",
"-024+35", "RRLL", "5CRETE",
"-032+35", "RRLL", "5CYPRUS",
"-052+37", "LLHUURHUHUHERERRRDDLLLFFDDURFLLDFDDL", "0CASPAIN SEA",
"-060+44", "LLUEERDFLDL", "0ARAL SEA",
"-068+24",
"FRGFRREDDDDDFDFDDFDDFERUEUUUUEEEEEREURRREFDFRDDDDRREFDDFDDGDDRFDDFDFFRUHUUHH\
HULUEUUURDRFDFRDEEREUUUHHHUUEERRDDEURRERREREEEUEULLREUHUHLEERRHLGLULUREERDLDR\
ERRFGRFDGRRREUHHUREUE",
"6ASIA S",
"-140+36",
"DEUUEUHURREREEGLLHHDDGLDRGDDGGLGLLLGGLDLRDFEUHRRGEERDLLRGLRERRERRE",
"6JAPAN",
"-121+25", "GDFUEUL", "6TAIWAN",
"-080+10", "DDDDREUHH", "6SRI LANKA",
"-121+18", "LDDDRDDRHRRFFDDDLLEHDULRHDFDDGERDDREUUULUUHHLHEUUL",
"2PHILIPPINES",
"-131+43",
"EFREEREEEUUUEUHLLUDLULEERERERRRRRRERRFLRRRRLUERERRRDRERURRGDLGLGLGLGGDDFDFEU\
RRUERUURULEEREDERRFRERERRRERRHLHLRRRREURDRRFRFRUURRHLLLDHHLLHLLHLLLLLLLDLLHRL\
LLLLLLGHULLLLLLLLLLULLLGL",
"6SIBERIA",
"-145+71",
"RELLRHLLLLGDHGHLLLLGLLHUHLLLLLDLLLLHLLLLLDULUDLGLLLLRRERERRRELHLLLLLLLELLLLG\
DLLLLLUDLLLLLGLLLDLLLLLLLDFRDDHELLLLLLDRRLLHUDLGFGRRRRFRLHLLDGLGLLHRRREUHUUUL\
LGGLDRFGHLLLHLLLLRFGHLGLLLULGLLLGLLHRHLDDDLLLLDLLLFLLHUHLRRFRRRREHLLHLLLHLLL",
"6RUSSIA",
"-143+54", "GDDDDDDDEDUUURUUHUU", "6SAKHALIN",
"-180+72", "GRRRRULLL", "6WRANGEL I.",
"-137+76", "DRRRRRRRELLLLLLLL", "6SIBERIAN I.",
"-091+80", "FERDRRRRRRULLLLLRRULLLLGL", "6SEVERNAYA",
"-101+79", "GRRRRELLLL", "6ZEMLYA",
"-068+77", "LLGLLLLLLGLLGGLGLRFRRRRLHERERERRRERRRREL", "6NOVAYA",
"+123+49",
"FGULLFDDDGFDDDFFDFRFRFDFFFDLFFRDFFEHHHHUHHUFRDFFFRDFFFDFGFRFRFRRFRRRRFFRRFRF\
FDRFFRFEUUGLHHUUEUHLLLLLEUUEULLLGDLLGLHHUHUUUEHEERERRFRRHRREFRRFDFDFEUUHUUUEE\
RERUUUHFDEUHFEURRRELUERRE",
"4NORTH AMERICA S",
"+113+42", "FH", "0SALT LAKE",
"+156+20", "DRULHLHL", "4HAWAII",
"+085+22", "RERFRRFRGRRRRHLHLHLLLLLG", "4CUBA",
"+070+18", "RRHHLLLFLLLFRRRRRR", "4HAITI",
"+078+18", "RRHLLF", "4JAMAICA",
"+066+18", "ELLDR", "4PUERTO RICO",
"+078+24", "UD", "4NASSAU",
"+067+45",
"REFLGDERERREHDLLLHUELLLGLGLREEERRRRRRREERRGGDGRRRFEFUUHLLLEUUHHGLRELLHHUHHHD\
GLGHHULLHLLLLLDFGFDDGLLFDDGHHUULLLLHLLHLLLUHUUEREEREERRRREUUHLLLDDGHULLLHLUHL\
GDRFGGULLLLLLLLLHLLGFLHLLLLLRHLLLLLHLLLLLLHGLLLLGUGLLLHLL",
"4CANADA",
"+088+49",
"LGLGRRRRRRRFLLLGRGDDREUURUFRGRFGFERERREEREERLGGLGLLLGRLLGLEUERHLLLHULHL",
"0GREAT LAKES",
"+117+61", "REHRFRRERGLGLLLL", "0SLAVE LAKE",
"+125+66", "RRERRRGREDLFHGLLLERLLLL", "0BEAR LAKE",
"+097+50", "UULHURFDFG", "0LAKE WINNIPEG",
"+090+72",
"FRRLLFRRRRRRRRRRFRRGLLGRREEFRFLGLFLLLLFRERFRFRRFRRHLHFRRRUHLHRRFRURELLHLLLHR\
RHLHLHGHLHLLGLLEHFRRRHLLLLLLGLDFHLUELLGG",
"4BAFFIN I.",
"+125+72",
"RFRREERRRLLGFFRRRRRLLLLLFRRRRRRRREFRRRRHRRLHLHHLRRULGLFLHLDLLULLLLHLLLLLLLDG",
"4VICTORIA I.",
"+141+70",
"LLLLLLLLHGLHLLLHGLLGLLGLLDRRFRRDLLLULGLLFRRRRRRDLGLLGFDRRRDRRRRRGGGLLGLLGGLL\
RRERERRRERREERRELEERRRLLGDRERRURRFRRRRRFRRFUDRUDDHFDURDURLURDDLFRULURDHFFRGFE\
GRFFRFRFLHLHLFFRFE",
"4ALASKA",
"+045+60",
"REUEREUERRRRERERRRERRRRERLLLLLLHRRRGERHFRRRRHLUDLLHLRERFRERLEUHRRHLEERLLURRR\
RRRRRELLLLLLLLLLGLLLRERHGLRELLLLLLLELLLLLLLLLLGLLLLLLGLLLLLLGLULLLLLLLFRLLLLL\
GLRRRGLLLLLLLGRRRRRRRGLLLLRRFRRRRRRRRRRFDFDLFREFRDLLLDERRFGLLGFFDRFFFRRRF",
"4GREENLAND",
"+080+10",
"DRFDFDDGGGDDGRDGDDFFDFDFFDFFRFFFDDDDDDGDDDDGDDDDGDGFGDDDEUDDDGUDDLDRGDDDFDFR\
FRRFERRLHLUHUURUEELHEREURULURREURREREUHUUDFRREEEEEUEUUEERERRREUEUEUUUUUEEEEUU\
UHLHLHLLLLHLHLGEHLGEUHUUHLHLLLHHLHULEDLLELLGHLLHLGDDHUELLGLGDGHHL",
"3SOUTH AMERICA",
"+060-51", "LDRRELL", "3FALKLAND ISLANDS",
"+092+00", "FUL", "3GALAPAGOS I.",
"-032+32",
"LLGLHLLLLHLGDGHLLHHLLHLEUULLLLLLLLLGLGLLLLHDGLGDGDGGLDGGGDGDFDDDDGDDFFFFDFRF\
FRRRRRRRRERERRFFRRFFDDDGDFFFDFDDDFDGDGDDDFDFDFDDDFDFDFDDFFERRRRREEEEEEEUUEREU\
UHUEEEREEUUUUHUUUHUEUEEEEEREEUEUEEUUULLLLGLLHUHHLHUHHUUHHUUHUHHUU",
"1AFRICA",
"-049-12", "DGGGLGDDDDGDDFFREUEUEUUUEUUUUH", "1MADAGASCAR",
"-032+00", "DDDREUELLL", "0LAKE VICTORIA",
"-014+14", "LRFLU", "0LAKE CHAD",
"-124-16",
"LGDGGLGLLGLDDDGFDDFDFDGFRRRERRRRURERRRRRRRFFFEEDDRFDFRFREFRERRUUEUEEUUUUUUUH\
HHHHHHUUHHHUULDDDDGDGHLHLHEUELLLHLFLLULDRGDDLLHLGG",
"2AUSTRALIA",
"-173-35", "FFDGFDREURULHHHL", "2NEW ZEALAND N",
"-174-41", "LLDGLGLGGRFREEUREEU", "2NEW ZEALAND S",
"-145-41", "DFRRUUUDLLL", "2TASMANIA",
"-178-17", "GRRURUGDH", "2FIJI",
"-130+00", "FRFRLGFEFRFRFDGRRFRRUERFFFRRRLHHHHRHLHHLHLLHGGLHUHLGH",
"2NEW GUINEA",
"-115-04", "RUUEEURHUUEHHGGGGLLDDHLDDFDDRRDERF", "2BORNEO",
"-095+06", "DFFFFFFDFFFFRUUUHFRHLHLUHHHHHLLH", "2SUMATRA",
"-106-06", "GRFRRRRRRFRRHLHLLLLLHL", "2JAVA",
"-120+00", "DGDDRDFHUEDFRHUHREFHLGHURRRRELLLLG", "2CELEBES",
"+000-70",
"ULDLLLLLLLLGLLGLLLGLLGLLLLGLGLLGLLLLGLLLLLHLGLLLLLHLLLLLHLLLLHLLUERLEUUUUUUE\
ERRRULLGLLLLGLGGLLLDRUDRDLGHLLGLLFGRRFLLLLLLLDHLLLLHLLLLLGLLLLHLLLLLLLGRFDLLL\
ULLLGHLLLLLLLLLLHGHLLGLLLLLLLGLLLLLLLLLLLGLLLGLLLLLLLLGLLLLLLLLLLLLLLLLLLLLL",
"7ANTARCTICA W",
"+180-78",
"LLLLLLLHLLGHLLGHLUEERRERREHLLLLHLLLLLLHLLLLLLLLLLLHLHLLLLLHLLULDLLLLLDLLHLLL\
LGHFLLLLLHLLLLLLGLHLLHLGLLLLHLGLLGLLLULLLGLLHDFLLLGLGLLLELLLLHLLLLLLLLLLHLLLH\
LLLLGGHGHGLLLGLDLLLLHLLGHGLLLLLLLLLLLLLLHLGLLLLLLLLLLLLLL",
"7ANTARCTICA E",
"", "", ""};

#ifdef CONSTEL
CONST char *szDrawConstel[cCnstl+1] = {"",
"550210+51DDd3r8d2Rr7d2Rr3Dd5l2d3r10uru6rUu2Rr2ur4u2RrUUu3Ll7d2l3DdLl5d2Lu2l4\
Uul8Dd2Ll3Uul7", /* Andromeda */
"660913-25d2Ll5Dl5d2l4d4LlDRRr8Uu5l6", /* Antila */
"561804-68DDd3RRRRRr2Uu9LLLLl3Uu2Ll4", /* Apus */
"362213+02Dd3Ll14DDd5RRrUUur7Dd4Rr6UUu2Ll9ul3dLl13", /* Aquarius */
"562003+16Ddl3d7l3Dd9r7Dd2RRUu6r5Uu2l4u4r3Uu2l3u7Lld2l13dLl3", /* Aquila */
"641803-45Dd7Rr5Dd8Ru3rur2u3r3UUu5LLl3", /* Ara */
"560307+31DDrd9RRr3Uu6Llu2l7UuLl7", /* Aries */
"650604+56d2l6Dl4d6Ll7Dd5Rr5Dd2RuRr2Ur4u6l3UUu3Ll3u3Ll4", /* Auriga */
"431504+55d2l8DDdr4d7r4Dd5rDDd2RRr6UUu8LlUlUUu5Ll4", /* Bootes */
"560501-27DDd3Rr2d3r5d3r4Uu3l5Ul2u3Ll", /* Caelum */
"751407+86DdRr6d3RRr6u3Rr2UuRr9Dd7Rr12DDRr12u2Rr11Dd4Rr12d3RRr8u2r2u2rUu8l6Uu\
7lULLl6u5LLLl5uLLLLLLl7", /* Camelopardalis */
"550906+33DDDd3Rr14URrULu8l2Uu3Ll6", /* Cancer */
"551309+52Dd2Ll2DDrd2Rr9UuRr8u2r5Uu4lUu2Ll9", /* Canes Venatici */
"550707-11DDd3Rr12UUu9Ll7", /* Canis Major */
"660714+13DLld3l2DRr10ur3Uu2l7ul5", /* Canis Minor */
"562114-09DDd5r7d2Rr13UUul7Dd4Ll7Uul7", /* Capricornus */
"360804-51d2l4d2l5d2LLLl5DDd5RRRUu6RRr2Uu2r4u3r6u2r2u2LLl4", /* Carina */
"440310+77Dd2r6Dd3Rr6u2Rdr3d3r4d4r4DdRd2r10u2Rr5Uur3u2RrULl4u4l6u3Ll6Uu7LLLl1\
0", /* Cassiopeia */
"551501-30Dd3Rr11Dd5l6Dd4RRrUu5RrDd4r9Uu3r4UUu4Ll5u2l5ULLLl", /* Centaurus */
"850805+88d3RRRr9DRRr4d3RRRr9Dd3Rr5d3r6DRrd3r7dr3d3r3u2RRr5UurdRu2l7u5l3Uu5r9\
Uul12u5LLl14u2LLLLLLLLLLl5", /* Cepheus */
"560306+11DDdRr4DDd4Rr4dRRrUUu4Ll7Uu3LLl2UuLl6", /* Cetus */
"561313-75Dd3RRRRRRr6Uu5LLLLLLl13", /* Chamaeleon */
"341507-55Ddr2d3r3d4RrDdRr2Uu5r2uLl10Uu5Ll7", /* Circinus */
"660603-27Dd3l7Dd3Rr14UUu3Ll3", /* Columba */
"561207+33d2Ll6Dd2l3Dd6RrdRUu8l2Uu3l5", /* Coma Berenices */
"561905-37Dd5RRUu3Ll5", /* Corona Australis */
"451606+39Dd4r2dRr11Uu2l4u7Ll6", /* Corona Borealis */
"551214-12Dd3r4d2RrUu8Ll14", /* Corvus */
"551114-07DDd5RrUrUu3Ll14", /* Crater */
"561214-55Dd4RrUu5Ll14", /* Crux */
"552010+61Dd5LLlDd5RdrDd4r2Dd2RuRr4d2r6Ulu7lUu4r4u4rUu6l5u2l5ULl9ul",
/* Cygnus */
"542010+21DLl2d8RrDd4rd4r7u7r3Uu6l2Uul5", /* Delphinus */
"570408-49Dd4Lld3l7DdLd3l8DRRr6Uur3u3r5u3Rr2u2Ll2Uul6", /* Dorado */
"352013+86d5r12Dd5l9Dd3r3d5RDr3d2r5d2r5Dd2Rr10UuRr14uRRr3ur8u2Rr8Uu2Rr7uRr14u\
3Rr8Uu3RRr9UuLl13DdLl9d3LLlDdLd4Ll10ULl8u5Ll7Ul7u6LLLl13", /* Draco */
"562107+13Dd8Rr2u4lUu2Ll3ul4", /* Equuleus */
"430411+00d4Ll3DdRd3rDd7r3Dr2d7r5DRrd4r7d2r6d3Rr4Ddr4d3r4d4Rr8u5l3u2l4Uu2Ll6U\
Llul7u4l4Uu6Rr4UUu9Ll10ULl11", /* Eridanus */
"550312-24Dd5r4d4r7DRRr4Uu6LLl12", /* Fornax */
"560713+35d2Ll2Dd2RDrd7r5dr7Dru2Rr9u6rUu2Ru6l10Uu5Ll13", /* Gemini */
"442307-36DDd6Rr13UuRr8Uu4LLl7", /* Grus */
"551805+51Dd2rDDl3d4l7Dd8r8u2Rr9dRr2Dd6r10Uu6RUu2l2u3l2ul2Uu9Rr3UUu2LLldLl5",
/* Hercules */
"770404-40d9r2DdRr2d2r5d4r4Dd7Rr12Uu6l4u3l4UuLlu3l6u2l7ULl4", /* Horologium */
"760910+07DDd2Ll13Dld5LLl10u2LLl6d2LlDRRRr5d4r5d2RRru4r4Ur5u3Rr2u2r10URr5u2r3\
u7r4UUu7Ll10", /* Hydra */
"560203-58Dd7LLl9Dd5Rr8Dd2RRRr14Uu6l12dLl6UUu2Ll3", /* Hydrus */
"742107-45d4Ll2DDd7Ll7Dd4RRr8UURr8Uu5Ll7", /* Indus */
"562214+57DDd5r13uRrUu4LulUu3l3u3l3ul7", /* Lacerta */
"551200+28DDr6Dd7Rr2Uu7Rr9UUUu3LDd2l9d5l4u2Ll2u3L", /* Leo */
"451004+41Ddl9d6Ll2Dd5Rr2d2r4u5RUu3r9u6l5UuLl4", /* Leo Minor */
"550603-11Dd7RRrUu6Lu3Ll3", /* Lepus */
"551600-04DDdr3Dr11u5Rr9UUu2l6u7Ll3d3L", /* Libra */
"331602-30Dd2Rr3d6r5Dd4r4dRr11Uu7LlUULl2", /* Lupus */
"640703+62DLl9Dd4Ll5d5l6Ddr5d6RRr2u2r6Uu4RrUr4u4r6Uu2Ll3", /* Lynx */
"551903+48d4l4Dd3rDrd5Rrur7Ur3Uu8Ll3", /* Lyra */
"560608-70d5Ll9Dd5RRRRr8Uu5Ll9ULLl8", /* Mensa */
"552107-27DDd5Rr8UUu3Ll7", /* Microscopium */
"470701+12Dld9l3DLl3DdRRRrUu6Ll5UUlu2Ll", /* Monoceros */
"561311-64dl2Dd5RRr10Uu6LLl11", /* Musca */
"561609-42DDRr8u5r4ul4Uu2l5u6Ll9", /* Norma */
"270000-74lDd2LLLl7d3LLLLl9u2LLLLLLLLLLLl4Uu6LLLLLL", /* Octans */
"641806+14d2l5Dd4r5d2l2dr2DRrd4LlDRr3d2rUr7d6l7DDRr2u5r7UlurUu2Ru4l6Uu4l7Uu3L\
l6uLl6", /* Ophiuchus */
"560600+23dl5Dd2lDrDd4RrDdr11Uu6Rr4UrUu5Llul5dl4d3l3u6rUu3L", /* Orion */
"552007-57DLl7Dd4RRRr11Uu2Rr5Uu3LLLl7", /* Pavo */
"552201+36dLl9d2l4dLl2Ddld6lDd7r2d2RrDd2Rr2d6Rr3ur3dr2Uu3r4drUl3u4l3u4l4Uu6Ll\
", /* Pegasus */
"460209+59d2Ll5d2l2d2Ll12DDd4r3d5RRr2u3r2u3rUUur7Dd2Rr3Uur4u4l4u3Lul9",
/* Perseus */
"650206-40d8RrDdr4d2r3d5RRr8UULLLl6", /* Phoenix */
"640601-43Dd3l2d2l6d3l4Dd4Ru3r7Uu3r7u3Rr7Uu4l5u3LLl", /* Pictor */
"450108+33Ddl4Dd9Ll2Dd7RRr8Dd6Rru3Rr2Uu8Ll14UuLl2u2l2Uu2l10u3r2Uu3Ll8",
/* Pisces */
"562302-25Dd6RRr8Uu5LLl2", /* Piscis Austrinus */
"570807-11DDDd3r6DdRRr14Uu7l9Uu7Ll7UUu9Ll7", /* Puppis */
"560810-18DLl3d5l4Dd7Rr8UUu2l3", /* Pyxis */
"560401-53d3l5d3l3Dd7Rr11Uu3l4u4Ll", /* Reticulum */
"452005+22Dd4Rrur13u2RrUu2Ll5Dl9u2Ll5", /* Sagitta */
"552002-12Dd7l5DDd5Rr10Uu3RRUr4Uu4LLu4Ll2", /* Sagittarius */
"471606-08Dd9lDrd5l7DLLDd5Rr6u3r7UURr3u9LUUu2l6", /* Scorpius */
"560111-25DDRRr8u4r5Uu5LLl11", /* Sculptor */
"551900-04Dd6r9Uu6L", /* Scutum */
"861814+06d4r4Dd4r4Dd6Rr11Ul7d2lu2Llu6RrULl6u3l2ur2u2l8bRbRbRbUbUd4l3Dd6l3Dd4\
Rr12UUUu5Ll2d3RD", /* Serpens */
"551013+07DDd2Rr5UUu7Ll13", /* Sextans */
"640600+29d6r3Dd2ld6r3u3r4ur5dRr5DDRr5dr4UUUlUuLl9Dl4dLL", /* Taurus */
"552007-45Dd7RRr12Uu5LLl7", /* Telescopium */
"560211+37d3l2d3r5Dd2r7d2Rr3u3r4Uu5Ll2u2l9", /* Triangulum */
"561609-60dl3d3l2dLd3l3DdRRRrUu2Ll2u4l3u3l2ULl9", /* Triangulum Australe */
"360106-58DDd5Rr2uRr8Uu3Rr13Uu4Ll7d2LLl6", /* Tucana */
"641107+73Dd4Lld3Ll8dLl7Dd5r5Dd2Rr6Uu2Rr12Dd6rDDd2Rr13Uu3Rr2u6r9UuRr10u5Rr6UU\
r6Uu3LLLl7", /* Ursa Major */
"342200+86RRRRrDr7d5Rr7DRr5d5RRu4r14Uu6l8u3Ll7Uu6RRRRRRr10u2RRRRRRRRRRrd2R",
/* Ursa Minor */
"560907-37DLLlDd7RRRr2u2r5u2r4u2r3Uu7l6Uu3Ll7", /* Vela */
"551309+14Dd2LLl3DdRr3d7r6DDd3RRrUu8RrUu3r5UULu3l14uLl9", /* Virgo */
"560900-64Dd5RRr7Uu6LLL", /* Volans */
"462100+29dl8d4r3DRr5ur5uRrDr9u2Rru3Ll5u2l6u2LL"}; /* Vulpecula */
#endif /* CONSTEL */
#endif /* GRAPH */

/* xdata.cpp */
