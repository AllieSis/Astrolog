/*
** Astrolog (Version 7.00) File: xcharts1.cpp
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
** Single Chart Graphics Routines.
******************************************************************************
*/

/* Draw a wheel chart, in which the 12 signs and houses are delineated, and  */
/* the planets are inserted in their proper places. This is the default      */
/* graphics chart to generate, as is done when the -v or -w (or no) switches */
/* are included with -X. Draw the aspects in the middle of chart, too.       */

void XChartWheel()
{
  real xsign[cSign+1], xhouse[cSign+1], xplanet[objMax], symbol[objMax];
  int cx, cy, i, j;
  real unitx, unity;

  /* Set up variables and temporarily automatically decrease the horizontal */
  /* chart size to leave room for the sidebar if that mode is in effect.    */

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;
  gi.rAsc = gs.objLeft ? planet[NAbs(gs.objLeft)-1] +
    rDegQuad*(gs.objLeft < 0) : chouse[1];
  if (us.fVedic)
    gi.rAsc = gs.objLeft ? (gs.objLeft < 0 ? 120.0 : -60.0)-gi.rAsc : 0.0;

  /* Fill out arrays with the angular degree on the circle of where to */
  /* place each object, cusp, and sign glyph based on the chart mode.  */

  if (gi.nMode == gWheel) {
    for (i = 1; i <= cSign; i++)
      xhouse[i] = PZ(chouse[i]);
  } else {
    gi.rAsc -= chouse[1];
    for (i = 1; i <= cSign; i++)
      xhouse[i] = PZ(ZFromS(i));
  }
  for (i = 1; i <= cSign; i++)
    xsign[i] = PZ(HousePlaceInX(ZFromS(i), 0.0));
  for (i = 0; i <= cObj; i++)
    xplanet[i] = PZ(HousePlaceInX(planet[i], planetalt[i]));

  /* Go draw the outer sign and house rings. */

  DrawWheel(xsign, xhouse, cx, cy, unitx, unity,
    0.65, 0.70, 0.75, 0.80, 0.875);

  for (i = 0; i <= cObj; i++)    /* Figure out where to put planet glyphs. */
    symbol[i] = xplanet[i];
  FillSymbolRing(symbol, 1.0);

  /* For each planet, draw a small dot indicating where it is, and then */
  /* a line from that point to the planet's glyph.                      */

  DrawSymbolRing(symbol, xplanet, ret, cx, cy, unitx, unity,
    0.50, 0.52, 0.56, 0.60);

  /* Draw lines connecting planets which have aspects between them. */

  if (!gs.fEquator) {               /* Don't draw aspects in equator mode. */
    if (!FCreateGrid(fFalse))
      return;
    for (j = cObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j))
          DrawAspectLine(i, j, cx, cy, xplanet[i], xplanet[j], unitx, unity,
            0.48);
  }

  /* Go draw sidebar with chart information and positions if need be. */

  DrawSidebar();
}


/* Draw an astro-graph chart on a map of the world, i.e. the draw the     */
/* Ascendant, Descendant, Midheaven, and Nadir lines corresponding to the */
/* time in the chart. This chart is done when the -L switch is combined   */
/* with the -X switch.                                                    */

void XChartAstroGraph()
{
  real planet1[objMax], planet2[objMax],
    end1[cObj*2+2], end2[cObj*2+2],
    symbol1[cObj*2+2], symbol2[cObj*2+2],
    lon = Lon, longm, x, y, z, ad, oa, am, od, dm, lat;
  int unit = gi.nScale, fVector, lat1 = -60, lat2 = 75, y1, y2, xold1, xold2,
    xmid, i, j, k, l;

  /* Erase top and bottom parts of map. We don't draw the astro-graph lines */
  /* above certain latitudes, and this gives us room for glyph labels, too. */

  y1 = (90-lat1)*gi.nScale;
  y2 = (90-lat2)*gi.nScale;
  DrawColor(gi.kiOff);
  DrawBlock(0, 1, gs.xWin-1, y2-1);
  DrawBlock(0, y1+1, gs.xWin-1, gs.yWin-2);
  DrawColor(gi.kiLite);
  i = gs.yWin/2;
  if (gs.fEquator)
    DrawDash(0, i, gs.xWin-2, i, 4);    /* Draw equator. */
  DrawColor(gi.kiOn);
  DrawLine(1, y2, gs.xWin-2, y2);
  DrawLine(1, y1, gs.xWin-2, y1);
  for (i = 0; i <= cObj*2+1; i++)
    end1[i] = end2[i] = -rLarge;

  /* Draw small hatches every 5 degrees along edges of world map. */

  DrawColor(gi.kiLite);
  for (i = lat1+5; i < lat2; i += 5) {
    j = (90-i)*gi.nScale;
    k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
    DrawLine(1, j, k, j);
    DrawLine(gs.xWin-2, j, gs.xWin-1-k, j);
  }
  for (i = -nDegHalf+5; i < nDegHalf; i += 5) {
    j = (nDegHalf-i)*gi.nScale;
    k = (2+(i%10 == 0)+2*(i%30 == 0)+(i%90 == 0))*gi.nScaleT;
    DrawLine(j, y2+1, j, y2+k);
    DrawLine(j, y1-1, j, y1-k);
  }
  if (us.fLatitudeCross) {
    DrawColor(kPurpleB);
    i = (int)((rDegQuad - Lat)*(real)gi.nScale);
    DrawLine(0, i, gs.xWin-1, i);
  }

  /* Calculate zenith locations of each planet. */

  for (i = 0; i <= cObj; i++) if (!ignore[i] || i == oMC) {
    planet1[i] = Tropical(i == oMC ? is.MC : planet[i]);
    planet2[i] = planetalt[i];
    EclToEqu(&planet1[i], &planet2[i]);
  }

  /* Draw the Midheaven lines and zenith location markings. */

  if (lon < 0.0)
    lon += rDegMax;
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    x = planet1[oMC]-planet1[i];
    if (x < 0.0)
      x += rDegMax;
    if (x > rDegHalf)
      x -= rDegMax;
    z = lon + x;
    if (z > rDegHalf)
      z -= rDegMax;
    j = (int)(Mod(rDegHalf-z+gs.rRot)*(real)gi.nScale);
    if (!ignorez[arMC]) {
      DrawColor(kElemB[eEar]);
      DrawLine(j, y1+unit*4, j, y2-unit*1);
    }
    end2[i*2] = (real)j;
    y = planet2[i];
    k = (int)((rDegQuad-y)*(real)gi.nScale);
    if (FBetween((int)y, lat1, lat2) && !ignorez[arMC]) {
      DrawColor(gi.kiLite);
      DrawBlock(j-gi.nScaleT, k-gi.nScaleT, j+gi.nScaleT, k+gi.nScaleT);
      DrawColor(gi.kiOff);
      DrawBlock(j, k, j, k);
    }

    /* Draw Nadir lines assuming we aren't in bonus chart mode. */

    if (!gs.fAlt && !ignorez[arIC]) {
      j += 180*gi.nScale;
      if (j > gs.xWin-2)
        j -= (gs.xWin-2);
      end1[i*2] = (real)j;
      DrawColor(kElemB[eWat]);
      DrawLine(j, y1+unit*2, j, y2-unit*2);
    }
  }

  /* Now, normally, unless we are in bonus chart mode, we will go on to draw */
  /* the Ascendant and Descendant lines here.                                */

  longm = Mod(planet1[oMC] + lon);
  if (!gs.fAlt && (!ignorez[arAsc] || !ignorez[arDes]))
  for (i = 1; i <= cObj; i++) if (FProper(i)) {
    xold1 = xold2 = nNegative;

    /* Hack: Normally we draw the Ascendant and Descendant line segments */
    /* simultaneously. However, for the PostScript and metafile vector   */
    /* graphics, this will cause the file to get inordinately large due  */
    /* to the constant thrashing between the Asc and Desc colors. Hence  */
    /* for these charts only, we'll do two passes for Asc and Desc.      */
    fVector = (gs.ft == ftPS || gs.ft == ftWmf);
    for (l = 0; l <= fVector; l++)

    for (lat = (real)lat1; lat <= (real)lat2;
      lat += 1.0/(real)(gi.nScale/gi.nScaleT)) {

      /* First compute and draw the current segment of Ascendant line. */

      j = (int)((rDegQuad-lat)*(real)gi.nScale);
      ad = RTanD(planet2[i])*RTanD(lat);
      if (ad*ad > 1.0)
        ad = rLarge;
      else {
        ad = RAsin(ad);
        oa = planet1[i] - DFromR(ad);
        if (oa < 0.0)
          oa += rDegMax;
        am = oa - rDegQuad;
        if (am < 0.0)
          am += rDegMax;
        z = longm-am;
        if (z < 0.0)
          z += rDegMax;
        if (z > rDegHalf)
          z -= rDegMax;
        k = (int)(Mod(rDegHalf-z+gs.rRot)*(real)gi.nScale);
        if (!fVector || !l) {
          if (!ignorez[arAsc]) {
            DrawColor(kElemB[eFir]);
            DrawWrap(xold1, j+gi.nScaleT, k, j, 1, gs.xWin-2);
          }
          if (lat == (real)lat1) {                          /* Line segment */
            if (!ignorez[arAsc])                            /* pointing to  */
              DrawLine(k, y1, k, y1+unit*4);                /* Ascendant.   */
            end2[i*2+1] = (real)k;
          }
        } else if (lat == (real)lat1)
          end2[i*2+1] = (real)k;
        xold1 = k;
      }

      /* The curving Ascendant and Descendant lines actually touch at low or */
      /* high latitudes. Sometimes when we start out, a particular planet's  */
      /* lines haven't appeared yet, i.e. we are scanning at a latitude      */
      /* where our planet's lines don't exist. If this is the case, then     */
      /* when they finally do start, draw a thin horizontal line connecting  */
      /* the Ascendant and Descendant lines so they don't just start in      */
      /* space. Note that these connected lines aren't labeled with glyphs.  */

      if (ad == rLarge) {
        if (xold1 >= 0) {
          if ((!fVector || !l) && !ignorez[arAsc] && !ignorez[arDes]) {
            xmid = (xold1+xold2)/2;
            if (NAbs(xold2-xold1) > (gs.xWin >> 1)) {
              xmid += (gs.xWin >> 1);
              if (xmid >= gs.xWin)
                xmid -= gs.xWin;
            }
            DrawColor(kElemB[eFir]);
            DrawWrap(xold1, j+1, xmid, j+1, 1, gs.xWin-2);
            DrawColor(kElemB[eAir]);
            DrawWrap(xmid, j+1, xold2, j+1, 1, gs.xWin-2);
          }
          lat = rDegQuad;
        }
      } else {

        /* Then compute and draw corresponding segment of Descendant line. */

        od = planet1[i] + DFromR(ad);
        dm = od + rDegQuad;
        z = longm-dm;
        if (z < 0.0)
          z += rDegMax;
        if (z > rDegHalf)
          z -= rDegMax;
        k = (int)(Mod(rDegHalf-z+gs.rRot)*(real)gi.nScale);
        if (xold2 < 0 && lat > (real)lat1 && (!fVector || l) &&
          !ignorez[arDes]) {
          xmid = (xold1+k)/2;
          if (NAbs(k-xold1) > (gs.xWin >> 1)) {
            xmid += (gs.xWin >> 1);
            if (xmid >= gs.xWin)
              xmid -= gs.xWin;
          }
          DrawColor(kElemB[eFir]);
          DrawWrap(xold1, j, xmid, j, 1, gs.xWin-2);
          DrawColor(kElemB[eAir]);
          DrawWrap(xmid, j, k, j, 1, gs.xWin-2);
        }
        if ((!fVector || l) && !ignorez[arDes]) {
          DrawColor(kElemB[eAir]);
          DrawWrap(xold2, j+gi.nScaleT, k, j, 1, gs.xWin-2);
          if (lat == (real)lat1)                            /* Line segment */
            DrawLine(k, y1, k, y1+unit*2);                  /* pointing to  */
        }                                                   /* Descendant.  */
        xold2 = k;
      }
    }

    /* Draw segments pointing to top of Ascendant and Descendant lines. */

    if (ad != rLarge) {
      if (!ignorez[arAsc]) {
        DrawColor(kElemB[eFir]);
        DrawLine(xold1, y2, xold1, y2-unit*1);
      }
      if (!ignorez[arDes]) {
        DrawColor(kElemB[eAir]);
        DrawLine(k, y2, k, y2-unit*2);
      }
      end1[i*2+1] = (real)k;
    }
  }

  /* Plot chart location. */

  DrawColor(kMagentaB);
  i = (int)(Mod(rDegHalf - Lon + gs.rRot)*(real)gi.nScale);
  j = (int)((rDegQuad - Lat)*(real)gi.nScale);
  if (us.fLatitudeCross)
    DrawSpot(i, j);
  else
    DrawPoint(i, j);

  /* Determine where to draw the planet glyphs. We have four sets of each    */
  /* planet - each planet's glyph appearing in the chart up to four times -  */
  /* one for each type of line. The Midheaven and Ascendant lines are always */
  /* labeled at the bottom of the chart, while the Nadir and Descendant      */
  /* lines at the top. Therefore we need to place two sets of glyphs, twice. */

  for (i = 0; i <= cObj*2+1; i++) {
    symbol1[i] = end1[i];
    symbol2[i] = end2[i];
  }
  FillSymbolLine(symbol1);
  FillSymbolLine(symbol2);

  /* Now actually draw the planet glyphs. */

  for (i = 0; i <= cObj*2+1; i++) {
    j = i >> 1;
    if (FProper(j)) {
      if ((gi.xTurtle = (int)symbol1[i]) > 0 && gs.fLabel &&
        !ignorez[FOdd(i) ? arDes : arIC]) {
        DrawColor(ret[j] < 0.0 ? gi.kiGray : gi.kiOn);
        DrawDash((int)end1[i], y2-unit*2, (int)symbol1[i], y2-unit*4,
          (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
        DrawObject(j, gi.xTurtle, y2-unit*10);
      }
      if ((gi.xTurtle = (int)symbol2[i]) > 0 && gs.fLabel &&
        !ignorez[FOdd(i) ? arAsc : arMC]) {
        DrawColor(ret[j] < 0.0 ? gi.kiGray : gi.kiOn);
        DrawDash((int)end2[i], y1+unit*4, (int)symbol2[i], y1+unit*8,
          (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
        DrawObject(j, gi.xTurtle, y1+unit*14);
        k = FOdd(i) ? oAsc : oMC;
        l = kObjB[k]; kObjB[k] = kObjB[j];
        DrawObject(k, (int)symbol2[i], y1+unit*24-gi.nScaleT);
        kObjB[k] = l;
      }
    }
  }
}


/* Draw an aspect and midpoint grid in the window, with planets labeled down */
/* the diagonal. This chart is done when the -g switch is combined with the  */
/* -X switch. The chart always has a certain number of cells; hence based    */
/* how the restrictions are set up, there may be blank columns and rows,     */
/* or else only the first number of unrestricted objects will be included.   */

void XChartGrid()
{
  char sz[cchSzDef], szT[cchSzDef];
  int nScale, unit, siz, x, y, i, j, k, l;
  KI c;

  nScale = gi.nScale/gi.nScaleT;
  unit = CELLSIZE*gi.nScale; siz = gi.nGridCell*unit;
  sprintf(szT, "");
  i = us.fSmartCusp; us.fSmartCusp = fFalse;
  if (!FCreateGrid(gs.fAlt))
    return;
  us.fSmartCusp = i;

  /* Loop through each cell in each row and column of grid. */

  for (y = 1, j = oEar-1; y <= gi.nGridCell; y++) {
    do {
      j++;
    } while (!FProper(j) && j <= cObj);
    DrawColor(gi.kiGray);
    DrawDash(0, y*unit, siz, y*unit, !gs.fColor);
    DrawDash(y*unit, 0, y*unit, siz, !gs.fColor);
    if (j <= cObj) for (x = 1, i = oEar-1; x <= gi.nGridCell; x++) {
      do {
        i++;
      } while (!FProper(i) && i <= cObj);
      if (i <= cObj) {
        gi.xTurtle = x*unit-unit/2;
        gi.yTurtle = y*unit-unit/2 - (nScale > 2 ? 5*gi.nScaleT : 0);
        k = grid->n[i][j];

        /* If this is an aspect cell, draw glyph of aspect in effect. */

        if (gs.fAlt ? x > y : x < y) {
          if (k) {
            DrawColor(c = kAspB[k]);
            DrawAspect(k, gi.xTurtle, gi.yTurtle);
          }

        /* If this is a midpoint cell, draw glyph of sign of midpoint. */

        } else if (gs.fAlt ? x < y : x > y) {
          DrawColor(c = kSignB(grid->n[i][j]));
          DrawSign(grid->n[i][j], gi.xTurtle, gi.yTurtle);

        /* For cells on main diagonal, draw glyph of planet. */

        } else {
          if (gs.fLabelAsp) {
            DrawColor(kDkBlueB);
            DrawBlock((x-1)*unit+1, (y-1)*unit+1, x*unit-1, y*unit-1);
          }
          DrawColor(gi.kiLite);
          DrawEdge((x-1)*unit, (y-1)*unit, x*unit, y*unit);
          DrawObject(i, gi.xTurtle, gi.yTurtle);
        }

        /* When the scale size is 300+, we can print text in each cell: */

        if (nScale > 2 && gs.fLabel) {
          l = NAbs(grid->v[i][j]); k = l / 60; l %= 60;
          if (nScale > 3 && is.fSeconds)
            sprintf(szT, "%s%02d", x == y ? "'" : "", l);

          /* For the aspect portion, print the orb in degrees and minutes. */

          if (gs.fAlt ? x > y : x < y) {
            if (grid->n[i][j]) {
              sprintf(sz, "%c%d%c%02d'%s", grid->v[i][j] < 0 ?
                (us.fAppSep ? 'a' : '-') : (us.fAppSep ? 's' : '+'),
                k/60, chDeg2, k%60, szT);
              if (nScale == 3)
                sz[7] = chNull;
            } else
              sprintf(sz, "");

          /* For the midpoint portion, print the degrees and minutes. */

          } else if (gs.fAlt ? x < y : x > y)
            sprintf(sz, "%2d%c%02d'%s", k/60, chDeg2, k%60, szT);

          /* For the main diagonal, print degree and sign of each planet. */

          else {
            c = kSignB(grid->n[i][j]);
            sprintf(sz, "%.3s %02d%s", szSignName[grid->n[i][j]], k, szT);
          }
          DrawColor(c);
          DrawSz(sz, x*unit-unit/2, y*unit-3*gi.nScaleT, dtBottom);
        }
      }
    }
  }
}


/* Translate zodiac position to chart pixel coordinates representing local */
/* horizon position, for the rectangular -Z -X switch chart.               */

void PlotHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  lat = rDegQuad - lat;
  *xp = x1 + (int)((real)xs*lon/rDegMax + rRound);
  *yp = y1 + (int)((real)ys*lat/rDegHalf + rRound);
}

void LocToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Mod(rDegQuad - lon);
    PlotHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  } else {
    lon = rDegMax - lon;
    CoorXform(&lon, &lat, is.latMC - rDegQuad);
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  }
}

void EquToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToLocal(&lon, &lat, rDegQuad - is.latMC);
    lon = rDegMax - lon;
    LocToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  } else {
    EquToEcl(&lon, &lat);
    lon = Mod(Untropical(lon));
    EclToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  }
}

void EclToHorizon(real lon, real lat, int x1, int y1, int xs, int ys,
  int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Tropical(lon);
    EclToEqu(&lon, &lat);
    EquToHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
  } else
    PlotHorizon(lon, lat, x1, y1, xs, ys, xp, yp);
}


/* Draw the local horizon, and draw in the planets where they are at the */
/* time in question, as done when the -Z is combined with the -X switch. */

void XChartHorizon()
{
  int cx, cy, unit, x1, y1, x2, y2, xs, ys, xp, yp, i, j, k;
  real lonM, latM, lonH, latH;
  ObjDraw rgod[objMax];
  char sz[cchSzDef];
  flag fFlip = gs.fEcliptic && us.rHarmonic < 0.0;
#ifdef SWISS
  ES es, *pes1, *pes2;
  int xp2, yp2;
#endif

  unit = Max(12, 6*gi.nScale);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  xs = x2-x1; ys = y2-y1; cx = (x1+x2)/2; cy = (y1+y2)/2;

  /* Calculate the local horizon coordinates of each planet. First convert */
  /* zodiac position and declination to zenith longitude and latitude.     */

  lonM = Tropical(is.MC); latM = 0.0;
  EclToEqu(&lonM, &latM);
  latM = Lat;
  is.lonMC = lonM; is.latMC = latM;
  ClearB((pbyte)rgod, sizeof(rgod));
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    EclToHorizon(planet[i], planetalt[i], x1, y1, xs, ys,
      &rgod[i].x, &rgod[i].y);
    rgod[i].obj = i;
    rgod[i].kv = ~0;
    rgod[i].f = fTrue;
  }

  /* Draw Earth's equator. */

  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i <= nDegMax; i++) {
      EquToHorizon((real)i, 0.0, x1, y1, xs, ys, &xp, &yp);
      DrawPoint(xp, yp);
    }
  }

  /* Draw sign and house boundaries if 3D house setting is in effect. */

  if (us.fVedic) {
    /* Draw zodiac sign orange wedges. */
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    for (i = 0; i < nDegMax; i++) {
      if (gs.fColorSign && i%30 == 0)
        DrawColor(kSignB(i/30 + 1));
      EclToHorizon((real)i, 0.0, x1, y1, xs, ys, &xp, &yp);
      DrawPoint(xp, yp);
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        EclToHorizon((real)i, (real)j, x1, y1, xs, ys, &xp, &yp);
        DrawPoint(xp, yp);
      }
    }
    k = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        EclToHorizon((real)(i-1)*30.0+15.0, (real)j, x1, y1, xs, ys,
          &xp, &yp);
        if (gs.fColorSign)
          DrawColor(kSignB(i));
        DrawSign(!fFlip ? i : cSign+1 - i, xp, yp);
      }
    gi.nScale = k;
  }
  if (gs.fHouseExtra) {
    /* Draw house orange wedges. */
    if (!gs.fColorHouse)
      DrawColor(kDkGreenB);
    if (!us.fHouse3D) {
      for (j = 1 + !gs.fEcliptic; j <= cSign;
        j += (!gs.fEcliptic && j%3 == 0 ? 2 : 1)) {
        if (gs.fColorHouse)
          DrawColor(kSignB(j));
        for (i = 0; i < nDegHalf; i++) {
          lonH = (real)(i + !FBetween(j, sCan, sSag)*180); latH = 0.0;
          k = (j < sCan ? j-1 : (j < sCap ? j-7 : j-13))*30;
          CoorXform(&lonH, &latH, (real)k);
          LocToHorizon(Mod(lonH + rDegQuad), latH, x1, y1, xs, ys, &xp, &yp);
          DrawPoint(xp, yp);
        }
      }
      for (i = 1; i <= cSign; i++) {
        xp = FBetween(i, sCan, sSag) * nDegHalf;
        yp = i < sCan ? 15-30*i : (i < sCap ? 30*i-195 : 375-30*i);
        LocToHorizon((real)xp, (real)yp, x1, y1, xs, ys, &xp, &yp);
        if (gs.fColorHouse)
          DrawColor(kSignB(i));
        DrawHouse(i, xp, yp);
      }
    } else {
      for (i = 1; i <= cSign; i++) {
        if (gs.fColorHouse)
          DrawColor(kSignB(SFromZ(chouse[i])));
        for (j = -90; j <= 90; j++) {
          EclToHorizon(chouse[i], (real)j, x1, y1, xs, ys,
            &xp, &yp);
          DrawPoint(xp, yp);
        }
      }
      for (j = -75; j <= 75; j += 150)
        for (i = 1; i <= cSign; i++) {
          EclToHorizon(Midpoint(chouse[i], chouse[Mod12(i+1)]), (real)j,
            x1, y1, xs, ys, &xp, &yp);
          if (gs.fColorHouse)
            DrawColor(kSignB(i));
          DrawHouse(i, xp, yp);
        }
    }
  }

  /* Draw vertical lines dividing our rectangle into four areas. In our     */
  /* local space chart, the middle line represents due south, the left line */
  /* due east, the right line due west, and the edges due north. A fourth   */
  /* horizontal line divides that which is above and below the horizon.     */

  if (gs.fHouseExtra && !us.fHouse3D && !gs.fEcliptic) {
    DrawColor(gs.fColorHouse ? kSignB(sCap) : kDkGreenB);
    DrawDash(cx, y1, cx, cy, 1);
    DrawColor(gs.fColorHouse ? kSignB(sCan) : kDkGreenB);
    DrawDash(cx, cy, cx, y2, 1);
  }
  if (!(us.fVedic && gs.fEcliptic)) {
    DrawColor(gi.kiGray);
    if (!(gs.fHouseExtra && !us.fHouse3D && !gs.fEcliptic))
      DrawDash(cx, y1, cx, y2, 1);
    DrawDash((cx+x1)/2, y1, (cx+x1)/2, y2, 1);
    DrawDash((cx+x2)/2, y1, (cx+x2)/2, y2, 1);
  }
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);
  if (!(us.fVedic && gs.fEcliptic)) {
    if (gs.fHouseExtra && !us.fHouse3D) {
      if (gs.fColorHouse)
        DrawColor(kSignB(sAri));
      DrawDash(x1, cy, cx, cy, 1);
      if (gs.fColorHouse)
        DrawColor(kSignB(sLib));
      DrawDash(cx, cy, x2, cy, 1);
    } else
      DrawDash(x1, cy, x2, cy, 1);
  }

  /* Make a slightly smaller rectangle within the window to draw the planets */
  /* in. Make segments on all four edges marking 5 degree increments.        */

  DrawColor(gi.kiLite);
  for (i = 5; i < 180; i += 5) {
    j = y1+(int)((real)i*(real)ys/rDegHalf);
    k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
    DrawLine(x1+1, j, x1+1+k, j);
    DrawLine(x2-1, j, x2-1-k, j);
  }
  for (i = 0; i <= nDegMax; i += 5) {
    j = x1+(int)((real)i*(real)xs/rDegMax);
    if (i > 0 && i < nDegMax) {
      k = (2+(i%10 == 0)+2*(i%30 == 0))*gi.nScaleT;
      DrawLine(j, y1+1, j, y1+1+k);
      DrawLine(j, y2-1, j, y2-1-k);
    }
    if (i % 90 == 0) {
      k = !fFlip ? i : nDegMax-i;
      if (!gs.fEcliptic)
        sprintf(sz, "%c", *szDir[k/90 & 3]);
      else if (us.nDegForm == 0)
        sprintf(sz, "%3.3s", szSignName[Mod12((k / 90)*3 + 1)]);
      else if (us.nDegForm == 1)
        sprintf(sz, "%dh", k/15);
      else
        sprintf(sz, "%d", k);
      DrawSz(sz, j, y1-2*gi.nScaleT, dtBottom);
    }
  }

#ifdef SWISS
  /* Draw extra stars. */
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      EclToHorizon(es.lon, es.lat, x1, y1, xs, ys, &xp, &yp);
      DrawStar(xp, yp, &es);
    }
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      EclToHorizon(pes1->lon, pes1->lat, x1, y1, xs, ys, &xp, &yp);
      EclToHorizon(pes2->lon, pes2->lat, x1, y1, xs, ys, &xp2, &yp2);
      DrawWrap(xp, yp, xp2, yp2, x1, x2);
    }
  }

  /* Draw extra asteroids. */
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      EclToHorizon(es.lon, es.lat, x1, y1, xs, ys, &xp, &yp);
      DrawStar(xp, yp, &es);
    }
  }
#endif

  /* Draw planet glyphs, and spots for actual planet locations. */
  DrawObjects(rgod, objMax, 0);
}


/* Translate zodiac position to chart pixel coordinates representing local */
/* horizon position, for the circular -Z0 -X switch chart.                 */

void PlotHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  real s, x, y, rx, ry;

  rx = (real)pcr->xr; ry = (real)pcr->yr;
  s = (rDegQuad-lat)/rDegQuad;
  if (s > 1.0) {
    x = rx * (rSqr2 - 1.0);
    y = ry * (rSqr2 - 1.0);
    if (lon < 45.0 || lon >= rDegMax-45.0 ||
      FBetween(lon, rDegHalf-45.0, rDegHalf+45.0))
      s = 1.0 + (s - 1.0) * (((rx + x)/RAbs(RCosD(lon))-rx) / rx);
    else if (lon < 135.0 || lon >= rDegMax-135.0)
      s = 1.0 + (s - 1.0) * (((ry + y)/RAbs(RCosD(lon-90.0))-ry) / ry);
  }
  *xp = pcr->xc + (int)(rx*s*RCosD(rDegHalf+lon)+rRound);
  *yp = pcr->yc + (int)(ry*s*RSinD(rDegHalf+lon)+rRound);
}

void LocToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  if (!gs.fEcliptic)
    PlotHorizonSky(lon, lat, pcr, xp, yp);
  else {
    lon = rDegMax - lon;
    CoorXform(&lon, &lat, is.latMC - rDegQuad);
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToHorizonSky(lon, lat, pcr, xp, yp);
  }
}

void EquToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Mod(is.lonMC - lon + rDegQuad);
    EquToLocal(&lon, &lat, rDegQuad - is.latMC);
    lon = rDegMax - lon;
    LocToHorizonSky(lon, lat, pcr, xp, yp);
  } else {
    EquToEcl(&lon, &lat);
    lon = Mod(Untropical(lon));
    EclToHorizonSky(lon, lat, pcr, xp, yp);
  }
}

void EclToHorizonSky(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  if (!gs.fEcliptic) {
    lon = Tropical(lon);
    EclToEqu(&lon, &lat);
    EquToHorizonSky(lon, lat, pcr, xp, yp);
  } else
    PlotHorizonSky(lon, lat, pcr, xp, yp);
}


/* Draw the local horizon, and draw in the planets where they are at the  */
/* time in question. This chart is done when the -Z0 is combined with the */
/* -X switch. This is an identical function to XChartHorizon(); however,  */
/* that routine's chart is entered on the horizon and meridian. Here we   */
/* center the chart around the center of the sky straight up from the     */
/* local horizon, with the horizon itself being an encompassing circle.   */

void XChartHorizonSky()
{
  int cx, cy, rx, ry, unit, x1, y1, x2, y2, xs, ys, xp, yp, i, j, k;
  real lonM, latM, lonH, latH, s;
  CIRC cr;
  ObjDraw rgod[objMax];
#ifdef SWISS
  ES es, *pes1, *pes2;
  int xp2, yp2;
#endif

  unit = Max(12, 6*gi.nScale);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;
  xs = x2-x1; ys = y2-y1; cx = (x1+x2)/2; cy = (y1+y2)/2;
  rx = (int)((real)xs/2.0/rSqr2); ry = (int)((real)ys/2.0/rSqr2);
  cr.xc = cx; cr.yc = cy; cr.xr = rx; cr.yr = ry;

  /* Calculate the local horizon coordinates of each planet. First convert */
  /* zodiac position and declination to zenith longitude and latitude.     */

  lonM = Tropical(is.MC); latM = 0.0;
  EclToEqu(&lonM, &latM);
  latM = Lat;
  is.lonMC = lonM; is.latMC = latM;
  ClearB((pbyte)rgod, sizeof(rgod));
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    EclToHorizonSky(planet[i], planetalt[i], &cr, &rgod[i].x, &rgod[i].y);
    rgod[i].obj = i;
    rgod[i].kv = ~0;
    rgod[i].f = fTrue;
  }

  /* Draw Earth's equator. */

  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i <= nDegMax; i++) {
      EquToHorizonSky((real)i, 0.0, &cr, &xp, &yp);
      DrawPoint(xp, yp);
    }
  }

  /* Draw sign and house boundaries if 3D house setting is in effect. */

  if (us.fVedic) {
    /* Draw zodiac sign orange wedges. */
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    for (i = 0; i < nDegMax; i++) {
      if (gs.fColorSign && i%30 == 0)
        DrawColor(kSignB(i/30 + 1));
      EclToHorizonSky((real)i, 0.0, &cr, &xp, &yp);
      DrawPoint(xp, yp);
    }
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        EclToHorizonSky((real)i, (real)j, &cr, &xp, &yp);
        DrawPoint(xp, yp);
      }
    }
    k = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = -80; j <= 80; j += 160)
      for (i = 1; i <= cSign; i++) {
        EclToHorizonSky((real)(i-1)*30.0+15.0, (real)j, &cr, &xp, &yp);
        if (gs.fColorSign)
          DrawColor(kSignB(i));
        DrawSign(i, xp, yp);
      }
    gi.nScale = k;
  }
  if (gs.fHouseExtra) {
    /* Draw house orange wedges. */
    if (!gs.fColorHouse)
      DrawColor(kDkGreenB);
    if (!us.fHouse3D) {
      for (j = 1 + !gs.fEcliptic; j <= cSign;
        j += (!gs.fEcliptic && j%3 == 0 ? 2 : 1)) {
        if (gs.fColorHouse)
          DrawColor(kSignB(j));
        for (i = 0; i < nDegHalf; i++) {
          lonH = (real)(i + !FBetween(j, sCan, sSag)*180); latH = 0.0;
          k = (j < sCan ? j-1 : (j < sCap ? j-7 : j-13))*30;
          CoorXform(&lonH, &latH, (real)k);
          LocToHorizonSky(Mod(lonH + rDegQuad), latH, &cr, &xp, &yp);
          DrawPoint(xp, yp);
        }
      }
      for (i = 1; i <= cSign; i++) {
        xp = FBetween(i, sCan, sSag) * nDegHalf;
        yp = i < sCan ? 15-30*i : (i < sCap ? 30*i-195 : 375-30*i);
        LocToHorizonSky((real)xp, (real)yp, &cr, &xp, &yp);
        if (gs.fColorHouse)
          DrawColor(kSignB(i));
        DrawHouse(i, xp, yp);
      }
    } else {
      for (i = 1; i <= cSign; i++) {
        if (gs.fColorHouse)
          DrawColor(kSignB(SFromZ(chouse[i])));
        for (j = -90; j <= 90; j++) {
          EclToHorizonSky(chouse[i], (real)j, &cr, &xp, &yp);
          DrawPoint(xp, yp);
        }
      }
      for (j = -75; j <= 75; j += 150)
        for (i = 1; i <= cSign; i++) {
          EclToHorizonSky(Midpoint(chouse[i], chouse[Mod12(i+1)]), (real)j,
            &cr, &xp, &yp);
          if (gs.fColorHouse)
            DrawColor(kSignB(i));
          DrawHouse(i, xp, yp);
        }
    }
  }

  /* Draw a circle in window to indicate horizon line, lines dividing   */
  /* the window into quadrants to indicate n/s and w/e meridians, and   */
  /* segments on these lines and the edges marking 5 degree increments. */

  DrawColor(gi.kiGray);
  DrawDash(cx, y1, cx, y2, 1);
  DrawDash(x1, cy, x2, cy, 1);
  DrawColor(gi.kiLite);
  for (i = -125; i <= 125; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    s = 1.0/(rDegQuad*rSqr2);
    j = cy + (int)(s*ys/2*i);
    DrawLine(cx-k, j, cx+k, j);
    j = cx + (int)(s*xs/2*i);
    DrawLine(j, cy-k, j, cy+k);
  }
  for (i = 5; i < 55; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    s = 1.0/(rDegHalf-rDegQuad*rSqr2);
    j = (int)(s*ys/2*i);
    DrawLine(x1, y1+j, x1+k, y1+j);
    DrawLine(x1, y2-j, x1+k, y2-j);
    DrawLine(x2, y1+j, x2-k, y1+j);
    DrawLine(x2, y2-j, x2-k, y2-j);
    j = (int)(s*xs/2*i);
    DrawLine(x1+j, y1, x1+j, y1+k);
    DrawLine(x2-j, y1, x2-j, y1+k);
    DrawLine(x1+j, y2, x1+j, y2-k);
    DrawLine(x2-j, y2, x2-j, y2-k);
  }
  if (!gs.fEcliptic) {
    DrawSz("N", cx, y1-2*gi.nScaleT, dtBottom);
    DrawSz("E", x1/2, cy+2*gi.nScaleT, dtCent);
    DrawSz("W", (gs.xWin+x2)/2, cy+2*gi.nScaleT, dtCent);
    if (!gs.fText)
      DrawSz("S", cx, gs.yWin-3*gi.nScaleT, dtBottom);
  }
  DrawColor(gi.kiOn);
  DrawEdge(x1, y1, x2, y2);
  DrawCircle(cx, cy, rx, ry);
  for (i = 0; i < nDegMax; i += 5) {
    k = (2+(i/10*10 == i ? 1 : 0)+(i/30*30 == i ? 2 : 0))*gi.nScaleT;
    DrawLine(cx+(int)((rx-k)*RCosD((real)i)), cy+(int)((ry-k)*RSinD((real)i)),
      cx+(int)((rx+k)*RCosD((real)i)), cy+(int)((ry+k)*RSinD((real)i)));
  }

#ifdef SWISS
  /* Draw extra stars. */
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      EclToHorizonSky(es.lon, es.lat, &cr, &xp, &yp);
      DrawStar(xp, yp, &es);
    }
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      EclToHorizonSky(pes1->lon, pes1->lat, &cr, &xp, &yp);
      EclToHorizonSky(pes2->lon, pes2->lat, &cr, &xp2, &yp2);
      DrawLine(xp, yp, xp2, yp2);
    }
  }

  /* Draw extra asteroids. */
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      EclToHorizonSky(es.lon, es.lat, &cr, &xp, &yp);
      DrawStar(xp, yp, &es);
    }
  }
#endif

  /* Draw planet glyphs, and spots for actual planet locations. */
  DrawObjects(rgod, objMax, 0);
}


/* This is a subprocedure of XChartOrbit(). Adjust the coordinates of a    */
/* planet so its distance from the central body is on a logarithmic scale. */

void OrbitPlot(real *pxp, real *pyp, real *pzp, real sz, int obj, PT3R *space)
{
  real xp, yp, zp, rDist;

  /* Copy input parameters. Only care about Z-axis in 3D wireframe charts. */
  xp = *pxp; yp = *pyp;
  zp = pzp != NULL ? *pzp : 0.0;

  /* Things orbiting the Earth should be appropriately scaled too. */
  if (FGeo(obj)) {
    xp = space->x + (xp - space->x) * 80.0;
    yp = space->y + (yp - space->y) * 80.0;
    zp = space->z + (zp - space->z) * 80.0;
  }

  /* Compute distance, determine ratio, and recompute coordinates. */
  rDist = RLength3(xp, yp, zp);
  if (rDist < rSmall)
    return;
  rDist = (RLog(rDist / sz * 100.0 + 1.0) / rLog101) / (rDist / sz);
  xp *= rDist; yp *= rDist; zp *= rDist;

  /* Copy back to input parameters. */
  *pxp = xp; *pyp = yp;
  if (pzp != NULL)
    *pzp = zp;
}


/* This is a subprocedure of XChartOrbit(). Append the current set of      */
/* planet coordinates to an internal list, so trails feature can be drawn. */

void OrbitRecord()
{
  int i, j;

  /* Don't append coordinates to list if they haven't changed. */
  if (gs.cspace <= 0)
    return;
  for (i = 0; i <= oNorm; i++) {
    j = ((gi.ispace - 1 + gs.cspace) % gs.cspace)*oNorm1 + i;
    if (gi.rgspace[j].x != space[i].x || gi.rgspace[j].y != space[i].y ||
      gi.rgspace[j].z != space[i].z)
      break;
  }
  if (i > oNorm)
    return;

  /* Append latest set of coordinates to list. */
  j = gi.ispace*oNorm1;
  for (i = 0; i <= oNorm; i++) {
    gi.rgspace[j].x = space[i].x;
    gi.rgspace[j].y = space[i].y;
    gi.rgspace[j].z = space[i].z;
    j++;
  }
  gi.ispace = (gi.ispace + 1) % gs.cspace;
  if (gi.cspace < gs.cspace)
    gi.cspace++;
}


/* Draw a chart depicting an aerial view of the solar system in space, with */
/* all the planets drawn around the Sun, and the specified central planet   */
/* in the middle, as done when the -S is combined with the -X switch.       */

void XChartOrbit()
{
  int cx = gs.xWin / 2, cy = gs.yWin / 2, unit, x1, y1, x2, y2,
    i, j, k, l, nSav;
  real sx, sy, sz, xp, yp, xp2, yp2, xpEar = 0.0, ypEar = 0.0;
  ObjDraw rgod[objMax];
#ifdef SWISS
  ES es, *pes1, *pes2;
  int j2, k2;
#endif

  unit = Max(6*gi.nScale, 12*gs.fText*gi.nScaleText*gi.nScaleT);
  x1 = unit; y1 = unit; x2 = gs.xWin-1-unit; y2 = gs.yWin-1-unit;

  /* Determine the scale of the chart. For a scale size of 400, make the */
  /* graphic 1 AU in radius (just out to Earth's orbit). For 300, make   */
  /* the chart 6 AU in radius (enough for inner planets out to asteroid  */
  /* belt). For a scale of 200, make window 30 AU in radius (enough for  */
  /* planets out to Neptune). For scale of 100, make it 90 AU in radius  */
  /* (enough for all planets including the orbits of the Uranians.)      */

  i = gi.nScale/gi.nScaleT;
  sz = gs.rspace > 0.0 ? gs.rspace : (i <= 1 ? 90.0 : (i == 2 ? 30.0 :
    (i == 3 ? 6.0 : (gi.nScaleText <= 1 ? 1.0 : 0.006))));
  sx = (real)(cx-x1)/sz; sy = (real)(cy-y1)/sz;
  ClearB((pbyte)rgod, sizeof(rgod));
  for (i = 0; i <= cObj; i++) if (FProper(i)) {
    xp = space[i].x; yp = space[i].y;
    if (us.nStar > 0 || gs.fAllStar) {
      xp /= rLYToAU; yp /= rLYToAU;
    }
    if (us.fHouse3D)
      OrbitPlot(&xp, &yp, NULL, sz, i, &space[oEar]);
    rgod[i].x = cx-(int)(xp*sx); rgod[i].y = cy+(int)(yp*sy);
    rgod[i].obj = i;
    rgod[i].kv = ~0;
    rgod[i].f = fTrue;
  }

  /* Draw the 12 sign boundaries from the center body to edges of screen. */
  nSav = gi.nScale;
  gi.nScale = gi.nScaleText * gi.nScaleT;
  if (!gs.fHouseExtra) {
    i = us.objCenter != oSun ? oSun : oEar;
    if (!gs.fColorSign)
      DrawColor(gi.kiGray);
    for (i = 0; i < cSign; i++) {
      j = i+1;
      if (gs.fColorSign)
        DrawColor(kSignB(j));
      k = cx - 2*(int)((real)cx*RCosD((real)i*30.0));
      l = cy + 2*(int)((real)cy*RSinD((real)i*30.0));
      DrawClip(cx, cy, k, l, x1, y1, x2, y2, 1);

      /* Draw sign glyphs near edge of screen. */
      k = (j == sGem || j == sCap) ? cx - (cx - x1) * 27 / 100 :
        ((j == sCan || j == sSag) ? cx + (cx - x1) * 27 / 100 :
        (FBetween(j, sLeo, sSco) ? x2 - gi.nScale*8 : x1 + gi.nScale*8));
      l = (j == sPis || j == sLib) ? cy - (cy - y1) * 27 / 100 :
        ((j == sAri || j == sVir) ? cy + (cy - y1) * 27 / 100 :
        (FBetween(j, sTau, sLeo) ? y2 - gi.nScale*8 : y1 + gi.nScale*8));
      DrawSign(j, k, l);
    }
  }
  gi.nScale = nSav;

  /* Draw internal boundary. */
  DrawColor(gi.kiLite);
  DrawEdge(x1, y1, x2, y2);

  /* Draw orbital trails. */
  if (gs.cspace > 0) {
    if (gi.rgspace == NULL) {
      gi.rgspace = (PT3R *)PAllocate(sizeof(PT3R)*oNorm1*gs.cspace, "orbits");
      if (gi.rgspace == NULL)
        return;
    }
    OrbitRecord();
    for (i = 0; i < gi.cspace; i++) {
      l = (gi.ispace - gi.cspace + i + gs.cspace) % gs.cspace;
      for (j = 0; j <= oNorm; j++) if (FProper(j)) {
        k = l*oNorm1 + j;
        xp = gi.rgspace[k].x; yp = gi.rgspace[k].y;
        if (us.fHouse3D)
          OrbitPlot(&xp, &yp, NULL, sz, j, &gi.rgspace[k - j + oEar]);
        DrawColor(kObjB[j]);
        if (!gs.fLabelAsp)
          DrawPoint(cx-(int)(xp*sx), cy+(int)(yp*sy));
        else if (i > 0) {
          // -XA setting on means orbit trails are lines instead of just dots.
          k = ((gi.ispace - gi.cspace + i-1 + gs.cspace) % gs.cspace) *
            oNorm1 + j;
          xp2 = gi.rgspace[k].x; yp2 = gi.rgspace[k].y;
          if (us.fHouse3D)
            OrbitPlot(&xp2, &yp2, NULL, sz, j, &gi.rgspace[k - j + oEar]);
          DrawLine(cx-(int)(xp2*sx), cy+(int)(yp2*sy),
            cx-(int)(xp*sx), cy+(int)(yp*sy));
        }
      }
    }
  } else if (gs.cspace < 0) {
    // Negative -YXj setting means draw orbits (assume they're circular).
    for (i = 0; i <= oNorm; i++) if (FProper(i)) {
      l = !FGeo(i) ? oSun : oEar;
      if (i == l || FIgnore(l) || FIgnore2(i))
        continue;
      k = (int)RLength2((real)(rgod[i].x - rgod[l].x),
        (real)(rgod[i].y - rgod[l].y));
      if (us.objCenter == l && k > cx + cy)
        continue;
      DrawColor(kObjB[i]);
      DrawCircle(rgod[l].x, rgod[l].y, k, k);
    }
  }

  /* Draw lines connecting planets which have aspects between them. */
  if (!gs.fEquator && us.nAsp > 0) {
    if (!FCreateGrid(fFalse))
      return;
    nSav = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = oNorm; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j)) {
          DrawColor(kAspB[grid->n[i][j]]);
          DrawClip(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y, x1, y1, x2, y2,
            NAbs(grid->v[i][j]/(60*60*2)));
          if (gs.fLabelAsp) {
            k = (rgod[i].x + rgod[j].x) >> 1;
            l = (rgod[i].y + rgod[j].y) >> 1;
            if (FBetween(k, x1, x2) && FBetween(l, y1, y2))
              DrawAspect(grid->n[i][j], k, l);
          }
        }
    gi.nScale = nSav;
  }

#ifdef SWISS
  /* Draw extra stars. */
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      xp = es.space.x / rLYToAU; yp = es.space.y / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j = cx-(int)(xp*sx); k = cy+(int)(yp*sy);
      if (FInRect(j, k, x1, y1, x2, y2))
        DrawStar(j, k, &es);
    }
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      xp = pes1->space.x / rLYToAU; yp = pes1->space.y / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j = cx-(int)(xp*sx); k = cy+(int)(yp*sy);
      xp = pes2->space.x / rLYToAU; yp = pes2->space.y / rLYToAU;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j2 = cx-(int)(xp*sx); k2 = cy+(int)(yp*sy);
      if (FInRect(j, k, x1, y1, x2, y2) && FInRect(j2, k2, x1, y1, x2, y2))
        DrawLine(j, k, j2, k2);
    }
  }

  /* Draw extra asteroids. */
  if (!gs.fAllStar && gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      xp = es.space.x; yp = es.space.y;
      if (us.fHouse3D)
        OrbitPlot(&xp, &yp, NULL, sz, -1, NULL);
      j = cx-(int)(xp*sx); k = cy+(int)(yp*sy);
      if (FInRect(j, k, x1, y1, x2, y2))
        DrawStar(j, k, &es);
    }
  }
#endif

  /* Draw planet glyphs, and spots for actual planet locations. */
  DrawObjects(rgod, objMax, unit);
}


/* Draw a chart showing the 36 Gauquelin sectors, with all the planets    */
/* positioned in their appropriate sector (and at the correct fracton     */
/* across the sector) as done when the -l is combined with the -X switch. */

void XChartSector()
{
  real xplanet[objMax], symbol[objMax];
  byte ignoreSav[objMax], ignoreT[objMax];
  char sz[3];
  int cx, cy, i, j, k;
  real unitx, unity, px, py, temp;

  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;
  cx = gs.xWin/2 - 1; cy = gs.yWin/2 - 1;
  unitx = (real)cx; unity = (real)cy;

  /* Draw lines across the whole chart at the four angles. */

  DrawColor(gi.kiLite);
  DrawDash(cx+POINT1(unitx, 0.99, PX(0.0)),
           cy+POINT1(unity, 0.99, PY(0.0)),
           cx+POINT1(unitx, 0.99, PX(180.0)),
           cy+POINT1(unity, 0.99, PY(180.0)), !gs.fColor);
  DrawDash(cx+POINT1(unitx, 0.99, PX(90.0)),
           cy+POINT1(unity, 0.99, PY(90.0)),
           cx+POINT1(unitx, 0.99, PX(270.0)),
           cy+POINT1(unity, 0.99, PY(270.0)), !gs.fColor);

  /* Draw circles and radial lines delineating the 36 sectors. */

  DrawColor(gi.kiOn);
  for (i = 0; i < nDegMax; i += 10) {
    px = PX((real)i); py = PY((real)i);
    DrawLine(cx+POINT1(unitx, 0.81, px), cy+POINT1(unity, 0.81, py),
      cx+POINT2(unitx, 0.95, px), cy+POINT2(unity, 0.95, py));
  }
  DrawCircle(cx, cy, (int)(unitx*0.95+rRound), (int)(unity*0.95+rRound));
  DrawCircle(cx, cy, (int)(unitx*0.81+rRound), (int)(unity*0.81+rRound));

  /* Label the 36 sectors, with plus zones in red and normal in dark green. */

  k = pluszone[cSector];
  for (i = 1; i <= cSector; i++) {
    j = pluszone[i];
    DrawColor(j ? kRedB : kDkGreenB);
    sprintf(sz, "%d", i);
    DrawSz(sz, cx+POINT1(unitx, 0.88, PX((real)(i*10+175)))+
      (FBetween(i, 12, 19) ? -(gi.nScale/* *gi.nScaleT*/) : 0),
      cy+POINT1(unity, 0.88, PY((real)(i*10+175)))+(gi.nScale/* *gi.nScaleT*/),
      dtCent | dtScale);
    sprintf(sz, "%c", j ? '+' : '-');
    DrawSz(sz, cx+POINT1(unitx, 0.97, PX((real)(i*10+175))),
      cy+POINT1(unity, 0.97, PY((real)(i*10+175)))+gi.nScaleT*2, dtCent);
    if (j != k) {
      DrawColor(gi.kiGray);
      DrawDash(cx, cy, cx+POINT2(unitx, 0.81, PX((real)(i*10+170))),
        cy+POINT2(unity, 0.81, PY((real)(i*10+170))), 1);
    }
    k = j;
  }

  CopyRgb(ignore, ignoreSav, sizeof(ignore));
  CastSectors();    /* Go compute the planets' sector positions. */

  for (i = 0; i <= cObj; i++)    /* Figure out where to put planet glyphs. */
    symbol[i] = xplanet[i] = Mod(rDegHalf - planet[i]);
  FillSymbolRing(symbol, 1.0);

  /* For each planet, draw a small dot indicating where it is, and then */
  /* a line from that point to the planet's glyph.                      */

  for (i = cObj; i >= 0; i--) if (FProper(i)) {
    if (gs.fLabel) {
      temp = symbol[i];
      DrawColor(ret[i] < 0.0 ? gi.kiGray : gi.kiOn);
      DrawDash(cx+POINT1(unitx, 0.67, PX(xplanet[i])),
        cy+POINT1(unity, 0.67, PY(xplanet[i])),
        cx+POINT1(unitx, 0.71, PX(temp)),
        cy+POINT1(unity, 0.71, PY(temp)),
        (ret[i] < 0.0 ? 1 : 0) - gs.fColor);
      DrawObject(i, cx+POINT1(unitx, 0.75, PX(temp)),
        cy+POINT1(unity, 0.75, PY(temp)));
    } else
      DrawColor(kObjB[i]);
    if (gs.fHouseExtra)
      DrawSpot(cx+POINT1(unitx, 0.65, PX(xplanet[i])),
        cy+POINT1(unity, 0.65, PY(xplanet[i])));
    else
      DrawPoint(cx+POINT1(unitx, 0.65, PX(xplanet[i])),
        cy+POINT1(unity, 0.65, PY(xplanet[i])));
  }

  /* Draw lines connecting planets which have aspects between them. */

  if (!gs.fEquator) {               /* Don't draw aspects in equator mode. */
    if (!FCreateGrid(fFalse))
      return;
    for (j = cObj; j >= 1; j--)
      for (i = j-1; i >= 0; i--)
        if (grid->n[i][j] && FProper(i) && FProper(j))
          DrawAspectLine(i, j, cx, cy, xplanet[i], xplanet[j], unitx, unity,
            0.63);
  }

  CopyRgb(ignore, ignoreT, sizeof(ignore));
  cp1 = cp0;
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
  CastChart(fTrue);
  CopyRgb(ignoreT, ignore, sizeof(ignore));
  DrawSidebar();
  CopyRgb(ignoreSav, ignore, sizeof(ignore));
}


/* Draw an arrow from one point to another, a line with an arrowhead at the */
/* ending point. The size of the arrowhead is based on current scale size,  */
/* and the line segment is actually shorter and doesn't touch either        */
/* endpoint by the same amount. This is used by XChartDispositor() below.   */

void DrawArrow(int x1, int y1, int x2, int y2)
{
  real r, s, a;

  r = DFromR(Angle((real)(x2-x1), (real)(y2-y1)));
  s = (real)(gi.nScale*8);
  x1 += (int)(s*RCosD(r)); y1 += (int)(s*RSinD(r));    /* Shrink line by    */
  x2 -= (int)(s*RCosD(r)); y2 -= (int)(s*RSinD(r));    /* the scale amount. */
  s = (real)(gi.nScale)*4.5;
  DrawLine(x1, y1, x2, y2);                            /* Main segment. */
  for (a = -1.0; a <= 1.0; a += 2.0)
    DrawLine(x2, y2, x2 + (int)(s*RCosD(r + a*135.0)), /* The two arrow     */
      y2 + (int)(s*RSinD(r + a*135.0)));               /* head line pieces. */
}


/* Draw dispositor graphs for the 10 main planets, as done when the -j is   */
/* combined with the -X switch. Four graphs are drawn, one in each screen   */
/* quadrant. A dispositor graph may be based on the sign or house position, */
/* and the planets may be arranged in a hierarchy or a wheel format.        */

void XChartDispositor()
{
  int oDis[oNorm1], dLev[oNorm1], cLev[oNorm1], xo[oNorm1], yo[oNorm1],
    obj[oNorm1];
  real xCirc[oNorm1], yCirc[oNorm1];
  char sz[cchSzDef];
  int *rgRules, oNum, xLev, yLev, xSub, ySub, cx0, cy0, cx, cy, i, j, k;

  /* Determine rulership and object sets to use. */

  if (ignore7[rrStd] && ignore7[rrEso] && !ignore7[rrHie])
    rgRules = rgSignHie1;
  else if (ignore7[rrStd] && !ignore7[rrEso])
    rgRules = rgSignEso1;
  else
    rgRules = rules;
  oNum = 0;
  for (i = 0; i <= oNorm; i++)
    if (FThing(i) && (!FIgnore(i) || (rgRules == rules ?
      FBetween(i, oSun, oMain) : (FBetween(i, oEar, oMain) || i == oVul))))
      obj[++oNum] = i;

  /* Set up screen positions of the 10 planets for the wheel graphs. */

  cx0 = gs.xWin / 2; cy0 = gs.yWin / 2;
  for (i = 1; i <= oNum; i++) {
    j = (gs.fHouseExtra ? 270 : 180) - (i-1)*(us.fVedic ? -360 : 360)/oNum;
    xCirc[i] = (real)cx0*0.4*RCosD((real)j);
    yCirc[i] = (real)cy0*0.4*RSinD((real)j);
  }

  /* Loop over the two basic dispositor types: sign based and house based. */

  for (xSub = 0; xSub <= 1; xSub++) {
    cx = xSub * cx0 + cx0 / 2;

    /* For each planet, get its dispositor planet for current graph type. */

    for (i = 1; i <= oNum; i++) {
      k = obj[i];
      j = rgRules[xSub ? inhouse[k] : SFromZ(planet[k])];
      for (k = 1; k <= oNum; k++)
        if (obj[k] == j)
          break;
      if (k > oNum)
        k = 1;
      oDis[i] = k;
      dLev[i] = 1;
    }

    /* Determine the final dispositors (including mutual reception loops). */

    do {
      j = fFalse;
      for (i = 1; i <= oNum; i++)
        cLev[i] = fFalse;
      for (i = 1; i <= oNum; i++)
        if (dLev[i])
          cLev[oDis[i]] = fTrue;
      for (i = 1; i <= oNum; i++)     /* A planet isn't a final dispositor */
        if (dLev[i] && !cLev[i]) {    /* if nobody is pointing to it.      */
          dLev[i] = 0;
          j = fTrue;
        }
    } while (j);

    /* Determine the level of each planet, i.e. how many times you have to */
    /* jump to your dispositor before reaching a final, with finals == 1.  */

    do {
      j = fFalse;
      for (i = 1; i <= oNum; i++)
        if (!dLev[i]) {
          if (!dLev[oDis[i]])
            j = fTrue;
          else                              /* If my dispositor already has */
            dLev[i] = dLev[oDis[i]] + 1;    /* a level, mine is one more.   */
        }
    } while (j);

    /* Count the number of planets at each dispositor level. */

    for (i = 1; i <= oNum; i++)
      cLev[i] = 0;
    for (i = 1; i <= oNum; i++)
      cLev[dLev[i]]++;

    /* Count the number of levels total, and max planets on any one level. */

    xLev = yLev = 0;
    for (i = 1; i <= oNum; i++)
      if (cLev[i]) {
        yLev = i;
        if (cLev[i] > xLev)
          xLev = cLev[i];
      }

    /* Loop over our two dispositor display formats: hierarchy and wheel. */

    for (ySub = 0; ySub <= 1; ySub++) {
      cy = ySub * cy0 + cy0 / 2;
      sprintf(sz, "%s dispositor %s", xSub ? "House" : "Sign",
        ySub ? "wheel" : "hierarchy");
      DrawColor(gi.kiLite);
      DrawSz(sz, cx, ySub * cy0 + 3*gi.nScaleT, dtTop);

      if (ySub) {

        /* Draw a graph in wheel format. */

        for (i = 1; i <= oNum; i++) {
          k = obj[i];
          DrawObject(k, cx + (int)xCirc[i], cy + (int)yCirc[i]);
          j = oDis[i];
          if (j != i) {
            if (dLev[i] < 2)
              DrawColor(gi.kiOn);
            else
              DrawColor(kObjB[k]);
            DrawArrow(cx + (int)xCirc[i], cy + (int)yCirc[i],
              cx + (int)xCirc[j], cy + (int)yCirc[j]);
          }
          if (!gs.fAlt && (j == i || dLev[i] < 2)) {
            DrawColor(j == i ? gi.kiOn : gi.kiGray);
            DrawCircle(cx + (int)xCirc[i], cy + (int)yCirc[i],
              7*gi.nScale, 7*gi.nScale);
          }
        }
      } else {

        /* For level hierarchies, first figure out the screen coordinates    */
        /* for each planet, based on its level, total levels, and max width. */

        for (i = 1; i <= oNum; i++) {
          yo[i] = cy0*(dLev[i]*2-1)/(yLev*2);
          k = 0;
          for (j = 1; j < i; j++)
            if (dLev[i] == dLev[j])
              k = j;
          if (k)
            xo[i] = xo[k] + cx0/xLev;    /* One right of last one on level. */
          else
            xo[i] = cx - ((cx0/xLev)*(cLev[dLev[i]]-1)/2);
        }

        /* Draw graph in level hierarchy format. */

        for (i = 1; i <= oNum; i++) {
          k = obj[i];
          DrawObject(k, xo[i], yo[i]);
          j = oDis[i];
          if (j != i) {
            if (dLev[i] < 2) {
              if (NAbs(xo[i] - xo[j]) < cx0/xLev*3/2) {
                DrawColor(gi.kiOn);
                DrawArrow(xo[i], yo[i], xo[j], yo[j]);
              }
              DrawColor(gi.kiGray);
            } else {
              DrawColor(kObjB[k]);
              DrawArrow(xo[i], yo[i], xo[j], yo[j]);
            }
          } else
            DrawColor(gi.kiOn);
          if (!gs.fAlt && dLev[i] < 2)
            DrawCircle(xo[i], yo[i], 7*gi.nScale, 7*gi.nScale);
        }
      }
    }
  }

  /* Draw boundary lines between the four separate dispositor graphs. */

  if (gs.fBorder) {
    DrawColor(gi.kiLite);
    DrawBlock(cx0, 0, cx0, gs.yWin-1);
    DrawBlock(0, cy0, gs.xWin-1, cy0);
  }
}


/* Draw a chart showing a graphical ephemeris of Ray influences for the   */
/* given month or year, with the date on the vertical axis and each Ray   */
/* on the horizontal, as done when the -7 is combined with the -X switch. */

void XChartEsoteric()
{
  real rRay[cRay+2], rRaySav[cRay+2], power1[objMax], power2[objMax],
    power[oNorm+1];
  char sz[cchSzDef];
  int daytot, d = 1, day, mon, monsiz,
    x1, y1, x2, y2, xs, ys, m, n, u, v = 0, i, j, k;
  flag fYea;

  EnsureRay();
  fYea = (us.nEphemYears > 0);    /* Doing an entire year or just a month? */
  if (fYea) {
    daytot = DayInYear(Yea);
    day = 1; mon = 1; monsiz = 31;
  } else
    daytot = DayInMonth(Mon, Yea);
  x1 = (fYea ? 30 : 24) * gi.nScaleText * gi.nScaleT; y1 = 12 * gi.nScaleText;
  x2 = gs.xWin - x1; y2 = gs.yWin - y1;
  xs = x2 - x1; ys = y2 - y1;

  /* Label Rays along the top axis. */

  for (i = 1; i <= cRay+1; i++) {
    m = x1 + NMultDiv(xs, i-1, cRay+1);
    DrawColor(gi.kiGray);
    DrawDash(m, y1, m, y2, 2);
    if (i <= cRay)
      sprintf(sz, "Ray %d", i);
    else
      sprintf(sz, "Average");
    DrawColor(i <= cRay ? kRayB[i] : gi.kiOn);
    DrawSz(sz, x1 + xs*(i-1)/8, y1 - 3*gi.nScaleText,
      dtLeft | dtBottom | dtScale2);
  }

  /* Loop and display Ray influences for one day segment. */

  while (d <= daytot + 1) {
    n = v;
    if (gs.fLabel && (fYea ? (mon == Mon && day == 1) : (d == Day))) {
      if (fYea)
        v = y1 + NMultDiv(ys, d-2+Day, daytot);
      else
        v = y1 + NMultDiv(ys, (d-1)*24 + (int)Tim, daytot*24);
      DrawColor(kDkCyanB);
      DrawLine(x1, v, x2, v);       /* Marker line for specific day. */
    }
    v = y1 + NMultDiv(ys, d-1, daytot);
    if (!gs.fEquator && (!fYea || day == 1)) {
      DrawColor(gi.kiGray);
      DrawDash(x1, v, x2, v, 1);    /* Marker line for day or month. */
    }
    if (d > 1)
      for (i = 1; i <= cRay+1; i++)
        rRaySav[i] = rRay[i];
    ciCore = ciMain;
    if (fYea) {
      MM = mon; DD = day;
    } else
      DD = d;
    CastChart(fTrue);

    /* Compute Ray influences for current day. */

    for (i = 0; i <= cRay+1; i++)
      rRay[i] = 0.0;
    ComputeInfluence(power1, power2);
    for (i = 0; i <= oNorm; i++) {
      power[i] = power1[i] + power2[i];
      if (FIgnore(i))
        continue;
      k = SFromZ(planet[i]);
      for (j = 1; j <= cRay; j++)
        if (rgSignRay2[k][j]) {
          if (!gs.fAlt)
            rRay[j] += power[i];
          else
            rRay[j] += power[i] / (420 / rgSignRay2[k][j]);
        }
    }
    for (i = 0; i <= cRay; i++)
      rRay[cRay+1] += rRay[i] / 7.0;

    /* Draw a line segment for each Ray during this time section. */

    if (d > 1)
      for (i = 1; i <= cRay+1; i++) {
        k = x1 + (i-1)*xs/8;
        m = k + (int)((real)xs * rRaySav[i] / 8.0 / (real)gs.nRayWidth);
        u = k + (int)((real)xs * rRay[i]    / 8.0 / (real)gs.nRayWidth);
        DrawColor(i <= cRay ? kRayB[i] : gi.kiOn);
        DrawLine(m, n, u, v);
      }

    /* Label months or days in the month along the left and right edges. */

    if (d <= daytot && (!fYea || day == 1)) {
      if (fYea) {
        sprintf(sz, "%.3s", szMonth[mon]);
        i = (mon == Mon);
      } else {
        sprintf(sz, "%2d", d);
        i = (d == Day);
      }
      DrawColor(gs.fLabel && i ? gi.kiOn : gi.kiLite);
      i = v + gi.nScaleT;
      DrawSz(sz,      xFontT,              i, dtLeft | dtTop | dtScale2);
      DrawSz(sz, x2 + xFontT - gi.nScaleT, i, dtLeft | dtTop | dtScale2);
    }

    /* Now increment the day counter. For a month we always go up by one. */
    /* For a year we go up by four or until the end of the month reached. */

    if (fYea) {
      i = us.fSeconds ? 1 : 4;
      day += i;
      if (day > monsiz) {
        d += i-(day-monsiz-1);
        if (d <= daytot + 1) {
          mon++;
          monsiz = DayInMonth(mon, Yea);
          day = 1;
        }
      } else
        d += i;
    } else
      d++;
  }
  DrawColor(gi.kiLite);
  DrawEdge(x1, y1, x2, y2);

  ciCore = ciMain;    /* Recast original chart. */
  CastChart(fTrue);
}


/* Draw one aspect event within a box in a calendar. Called from        */
/* ChartInDaySearch() which computes aspect events, which is in turn    */
/* called from XChartCalendar() which draws the graphic calendar chart. */

void DrawCalendarAspect(InDayInfo *pid, int i, int iMax, int nVoid)
{
  int x1, y1, x2, y2, asp, x, y, z, s1, s2, s3, nT, k;
  char sz[4], *szTime;

  // Get pixel coordinates of the calendar and pixel size of each box.
  x1 = gi.rgzCalendar[pid->day*2];
  y1 = gi.rgzCalendar[pid->day*2 + 1];
  x2 = x1 + gi.rgzCalendar[0];
  y2 = y1 + gi.rgzCalendar[1];
  z = gi.nScaleT * 10;
  y = y2 - z*(iMax-i) + gi.nScaleT*3;
  if (y - gi.nScaleT*5 <= y1)
    return;
  x = x2 + gi.nScaleT*3;
  if (x - z*3 - gi.nScaleT*5 <= x1)
    return;

  // Get aspect event and time that it takes place.
  asp = pid->aspect;
  s1 = (int)pid->time/60;
  s2 = (int)pid->time-s1*60;
  s3 = us.fSeconds ? (int)(pid->time*60.0)-((s1*60+s2)*60) : -1;

  // Draw the aspect event itself.
  DrawColor(kObjB[pid->source]);
  DrawObject(pid->source, x - z*3, y);
  if (asp >= aCon) {
    DrawColor(kAspB[asp]);
    DrawAspect(pid->aspect, x - z*2, y);
  } else if (asp == aSig || asp == aHou) {
    DrawColor(gi.kiOn);
    DrawTurtle("NL4R4NG4H4", x - z*2, y);
  } else if (asp == aDir) {
    DrawColor(gi.kiOn);
    DrawTurtle("NR3L3HU2ER6FBD4NHD2GL6H", x - z*2, y);
    DrawTurtle(pid->dest ? "F4BL8U8R7FD2GL7" : "BG4U8R6F2D4G2L6", x - z, y);
  } else if (asp == aDeg) {
    DrawColor(gi.kiOn);
    DrawTurtle("BRUL2D2REFREU2H2L4G2D4F2R4", x - z*2, y);
  }
  if (asp >= aCon) {
    DrawColor(kObjB[pid->source]);
    DrawObject(pid->dest, x - z, y);
  } else if (asp == aSig || asp == aDeg) {
    nT = asp == aSig ? pid->dest : pid->dest / us.nSignDiv + 1;
    DrawColor(kSignB(nT));
    DrawSign(nT, x - z, y);
  }

  // Draw the time that the aspect event takes place (if room).
  szTime = SzTime(s1, s2, s3);
  nT = x - z*4 - CchSz(szTime)*xFont*gi.nScaleT;
  if (nT <= x1)
    return;
  DrawColor(gi.kiGray);
  DrawSz(szTime, nT, y+gi.nScaleT*2, dtLeft);

  // Draw extra information about the aspect event (if room).
  nT -= 4*xFont*gi.nScaleT;
  if (nT <= x1)
    return;
  k = kSignB(SFromZ(planet[oMoo]));
  if (pid->source == oSun && pid->dest == oMoo &&
    (asp == aCon || asp == aOpp)) {
    DrawColor(k);
    DrawSz(asp == aCon ? (nVoid < 0 ? "NEW" : "N+v") :
      (nVoid < 0 ? "FUL" : "F+v"), nT, y+gi.nScaleT*2, dtLeft);
  } else if (nVoid >= 0) {
    DrawColor(k);
    DrawSz("v/c", nT, y+gi.nScaleT*2, dtLeft);
  } else if (pid->source == oMoo && asp == aSig) {
    DrawColor(kSignB(pid->dest));
    sprintf(sz, "%.3s", szSignName[pid->dest]);
    DrawSz(sz, nT, y+gi.nScaleT*2, dtLeft);
  }
}


/* Draw a graphical calendar for a given month, with numbers in boxes,  */
/* scaled to fit within the given bounds. This is used for single month */
/* -K switch images and is called 12 times for a full year -Ky image.   */

void DrawCalendar(int mon, int x1, int y1, int x2, int y2)
{
  char sz[cchSzDef];
  int rgz[(31+1)*2], day, cday, dayHi, cweek, xunit, yunit, xs, ys, x0, y0,
    x, y, s, nSav;
  flag fSav;

  xs = x2 - x1; ys = y2 - y1;
  day = DayOfWeek(mon, 1, Yea);    /* Day of week of 1st of month.     */
  cday = DaysInMonth(mon, Yea);    /* Count of days in the month.      */
  dayHi = DayInMonth(mon, Yea);    /* Number of last day in the month. */
  cweek = us.fCalendarYear ? 6 : (day + cday + 6) / 7;   /* Week rows. */
  xunit = xs/8;                    /* Hor. pixel size of each day box. */
  yunit = ys/(cweek+2);            /* Ver. pixel size of each day box. */
  x0 = x1 + (xs - xunit*7) / 2;    /* Blank space to left of calendar. */
  y0 = y1 + yunit*3/2;             /* Blank space to top of calendar.  */

  /* Print the month and year in big letters at top of chart. */

  DrawColor(gi.kiOn);
  sprintf(sz, "%s, %d", szMonth[mon], Yea);
  s = gi.nScale;
  gi.nScale = Min((yunit*3/2-yFont*s) / yFont, xs/15/*CchSz(sz)*/ / xFont);
  gi.nScale = Max(gi.nScale-1, 1);
  DrawSz(sz, x1 + xs/2, y1 + (yunit*3/2-yFont*s)/2, dtCent | dtScale);

  /* Draw the grid of boxes for the days. */

  for (gi.nScale = s; gi.nScale > 0 && xunit / (xFont*gi.nScale) < 3;
    gi.nScale--)
    ;
  for (x = 0; x <= cWeek; x++) {

    /* Print days of week at top of each column (abbreviated if need be). */

    if (x < cWeek) {
      if (xunit / (xFont*gi.nScale) < 9)
        sprintf(sz, "%.3s", szDay[x]);
      else
        sprintf(sz, "%s", szDay[x]);
      DrawColor(kYellowB);
      DrawSz(sz, x0 + x*xunit + xunit/2, y0 - s*3, dtBottom | dtScale);
      DrawColor(kCyanB);
    }
    DrawLine(x0 + x*xunit, y0, x0 + x*xunit, y0 + cweek*yunit);
  }
  for (y = 0; y <= cweek; y++)
    DrawLine(x0, y0 + y*yunit, x0 + 7*xunit, y0 + y*yunit);

  /* Actually draw the day numbers in their appropriate boxes. */

  x = day; y = 0;
  for (day = 1; day <= dayHi; day = AddDay(mon, day, Yea, 1)) {
    rgz[day*2] = x0 + x*xunit;
    rgz[day*2+1] = y0 + y*yunit;
    sprintf(sz, gs.fText ? "%2d" : "%d", day);
    DrawColor(day == Day && mon == Mon && gs.fLabel ? kGreenB :
      (x <= 0 || x >= cWeek-1 ? kRedB : gi.kiLite));
    if (!gs.fAlt || gs.fLabelAsp)
      DrawSz(sz, x0 + x*xunit + s*2, y0 + y*yunit + s*4,
        dtLeft | dtTop | dtScale);
    else
      DrawSz(sz, x0 + x*xunit + xunit/2,
        y0 + y*yunit + yunit/2 + gi.nScale, dtCent | dtScale);
    if (++x >= cWeek) {
      x = 0;
      y++;
    }
  }

  /* Draw aspect events taking place within each day within the boxes. */

  if (gs.fLabelAsp) {
    gi.nScale = gi.nScaleT;
    rgz[0] = xunit; rgz[1] = yunit;
    gi.rgzCalendar = rgz;
    us.fInDayMonth = fTrue; us.fInDayYear = fFalse;
    nSav = Mon; Mon = mon;
    fSav = gs.fLabel; gs.fLabel = fTrue;
    ChartInDaySearch(is.fProgress);
    Mon = nSav; gs.fLabel = fSav;
    gi.rgzCalendar = NULL;
  }
  gi.nScale = s;
}


/* Draw a graphical calendar on the screen for the chart month or entire */
/* year, as done when the -K or -Ky is combined with the -X switch.      */

void XChartCalendar()
{
  int xs, ys, xunit, yunit, x1, y1, x, y;

  if (!us.fCalendarYear) {
    DrawCalendar(Mon, 0, 0, gs.xWin, gs.yWin);
    return;
  }

  /* Determine the best sized rectangle of months to draw the year in based */
  /* on the chart dimensions: Either do 6x2 months, or 4x3, 3x4, or 2x6.    */

  if (gs.xWin > gs.yWin) {
    if (gs.xWin > gs.yWin * 3) {
      xs = 6; ys = 2;
    } else {
      xs = 4; ys = 3;
    }
  } else {
    if (gs.yWin > gs.xWin * 2) {
      xs = 2; ys = 6;
    } else {
      xs = 3; ys = 4;
    }
  }
  xunit = gs.xWin / xs; yunit = gs.yWin / ys;
  x1 = (gs.xWin - xunit*xs) / 2;
  y1 = (gs.yWin - yunit*ys) / 2;
  for (y = 0; y < ys; y++)
    for (x = 0; x < xs; x++) {
      DrawCalendar(y * xs + x + 1, x1 + x*xunit, y1 + y*yunit,
        x1 + (x+1)*xunit, y1 + (y+1)*yunit);
    }
}


/* Translate to chart pixel coordinates, that indicate where to draw on a */
/* chart sphere, for the -XX switch chart. Inputs may be local horizon    */
/* altitude and azimuth coordinates, local horizon prime vertical, local  */
/* horizon meridian, zodiac position and latitude, or Earth coordinates.  */

flag FSphereLocal(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  if (gs.fEcliptic) {
    azi = Mod(azi - rDegQuad); neg(alt);
    CoorXform(&azi, &alt, is.latMC - rDegQuad);
    azi = Mod(is.lonMC - azi + rDegQuad);
    EquToEcl(&azi, &alt);
    azi = rDegMax - Untropical(azi); neg(alt);
  }
  azi = Mod(rDegQuad*3 - (azi + gs.rRot));
  if (gs.rTilt != 0.0)
    CoorXform(&azi, &alt, gs.rTilt);
  *xp = pcr->xc + (int)((real)pcr->xr * RCosD(azi) * RCosD(alt) - rRound);
  *yp = pcr->yc + (int)((real)pcr->yr * RSinD(alt) - rRound);
  return azi >= rDegHalf;
}

flag FSpherePrime(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  CoorXform(&azi, &alt, rDegQuad);
  return FSphereLocal(azi + rDegQuad, alt, pcr, xp, yp);
}

flag FSphereMeridian(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  azi = Mod(azi + rDegQuad);
  CoorXform(&azi, &alt, rDegQuad);
  return FSphereLocal(azi, alt, pcr, xp, yp);
}

flag FSphereZodiac(real lon, real lat, CONST CIRC *pcr, int *xp, int *yp)
{
  real lonT, latT;

  lonT = Tropical(lon); latT = lat;
  EclToEqu(&lonT, &latT);
  lonT = Mod(is.lonMC - lonT + rDegQuad);
  EquToLocal(&lonT, &latT, rDegQuad - is.latMC);
  return FSphereLocal(lonT + rDegQuad, -latT, pcr, xp, yp);
}

flag FSphereEarth(real azi, real alt, CONST CIRC *pcr, int *xp, int *yp)
{
  azi = Mod(azi + Lon + rDegQuad);
  CoorXform(&azi, &alt, rDegQuad - Lat);
  return FSphereLocal(azi + rDegQuad, -alt, pcr, xp, yp);
}


/* Draw a chart sphere (like a chart wheel but in 3D) as done with the -XX */
/* switch. This is similar to Astrolog's local horizon charts.             */

void XChartSphere()
{
  char sz[cchSzDef];
  int zGlyph, zGlyph2, zGlyphS, cChart, iChart, xo = 0, yo = 0, xp, yp,
    i, j, k, k2, nSav;
  flag fDir = !gs.fSouth, fAny = !gs.fAlt, fNoHorizon, f;
  real lonMC, latMC, rT;
  CIRC cr, cr2;
  CONST CP *pcp;
  CP cpSav;
  ObjDraw rgod[objMax];
#ifdef SWISS
  ES es, *pes1, *pes2;
#endif

  /* Initialize variables. */
  if (gs.fText && !us.fVelocity)
    gs.xWin -= xSideT;

  fNoHorizon = ignorez[0] && ignorez[1] && ignorez[2] && ignorez[3];
  zGlyph = 7*gi.nScale; zGlyph2 = 14*gi.nScale; zGlyphS = 9*gi.nScaleT;
  cr.xc = gs.xWin >> 1; cr.yc = gs.yWin >> 1;
  cr.xr = cr.xc - zGlyph; cr.yr = cr.yc - zGlyph;
  cr2 = cr;
  cr2.xr += zGlyph >> 1; cr2.yr += zGlyph >> 1;
  cChart = 1 +
    (us.nRel <= rcDual) + (us.nRel <= rcTriWheel) + (us.nRel <= rcQuadWheel);

  if (us.nRel < rcNone)
    CastChart(fTrue);
  lonMC = Tropical(is.MC); latMC = 0.0;
  EclToEqu(&lonMC, &latMC);
  latMC = Lat;
  is.lonMC = lonMC; is.latMC = latMC;

  /* Avoid default alignments of sphere that don't look as good. */
  if (us.fSmartSave && !gi.fDidSphere && gs.rRot == 0.0 && gs.rTilt == 0.0) {
    gs.rRot = 7.0;
    gs.rTilt = -7.0;
  }
  gi.fDidSphere = fTrue;

  /* Draw constellations. */
  if (gs.fConstel) {
    neg(gs.rTilt);
    DrawMap(fTrue, fTrue, gs.rRot);
    neg(gs.rTilt);
  }

  /* Draw horizon. */
  if (!fNoHorizon || (!gs.fHouseExtra && !us.fHouse3D)) {
    if (!gs.fColorHouse)
      DrawColor(gi.kiOn);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorHouse && (i == 0 || i == nDegHalf))
        DrawColor(kSignB(i ? sLib : sAri));
      f = FSphereLocal((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0) {
        DrawLine(xo, yo, xp, yp);
        k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
        for (j = -k; j <= k; j += (k << 1)) {
          FSphereLocal((real)i, (real)j / 2.0, &cr, &xo, &yo);
          DrawLine(xo, yo, xp, yp);
        }
      } else if (fAny)
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  /* Draw Earth's equator. */
  if (gs.fEquator) {
    DrawColor(kPurpleB);
    for (i = 0; i <= nDegMax; i++) {
      f = FSphereEarth((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0)
        DrawLine(xo, yo, xp, yp);
      else if (fAny && !FOdd(i))
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  /* Draw prime vertical. */
  if (!fNoHorizon) {
    if (!gs.fColorHouse)
      DrawColor(gi.kiGray);
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorHouse)
        DrawColor(kSignB((i-1)/30 + 1));
      f = FSpherePrime((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0) {
        DrawLine(xo, yo, xp, yp);
        k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
        for (j = -k; j <= k; j += (k << 1)) {
          FSpherePrime((real)i, (real)j / 2.0, &cr, &xo, &yo);
          DrawLine(xo, yo, xp, yp);
        }
      } else if (fAny && !FOdd(i))
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  /* Draw 3D house wedges and meridian. */
  if (!gs.fColorHouse)
    DrawColor(kDkGreenB);
  for (j = -1; j <= cSign; j++) {
    if (!(!gs.fHouseExtra && !us.fHouse3D) && !(j <= 0 && !fNoHorizon))
      continue;
    if ((j == sAri && chouse[j] == is.Asc) ||
        (j == sCap && chouse[j] == is.MC)  ||
        (j == sLib && chouse[j] == Mod(is.Asc + rDegHalf)) ||
        (j == sCan && chouse[j] == Mod(is.MC  + rDegHalf)))
      continue;
    if (gs.fColorHouse) {
      k = j > 0 ? j : (j < 0 ? sCan : sCap);
      DrawColor(kSignB(k));
    }
    rT = j > 0 ? chouse3[j] : (j < 0 ? rDegQuad : 270.0);
    for (i = -90; i <= 90; i++) {
      f = FSpherePrime(rT, (real)i, &cr, &xp, &yp) ^ fDir;
      if (f && i > -90) {
        DrawLine(xo, yo, xp, yp);
        if (j <= 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (k2 = -k; k2 <= k; k2 += (k << 1)) {
            FSphereMeridian((real)(j == 0 ? i+180 : 360-i), (real)k2 / 2.0,
              &cr, &xo, &yo);
            DrawLine(xo, yo, xp, yp);
          }
        }
      } else if (fAny && !FOdd(i))
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
  }

  /* Draw 2D house wedges. */
  if (!gs.fHouseExtra && us.fHouse3D)
    for (i = 1; i <= cSign; i++) {
      if (gs.fColorHouse)
        DrawColor(kSignB(i));
      for (j = -90; j <= 90; j++) {
        f = FSphereZodiac(chouse[i], (real)j, &cr, &xp, &yp) ^ fDir;
        if (f && j > -90)
          DrawLine(xo, yo, xp, yp);
        else if (fAny && !FOdd(i))
          DrawPoint(xp, yp);
        xo = xp; yo = yp;
      }
    }

  /* Draw sign wedges. */
  if (!us.fVedic) {
    if (!gs.fColorSign)
      DrawColor(kDkBlueB);
    /* Draw ecliptic circle. */
    for (i = 0; i <= nDegMax; i++) {
      if (gs.fColorSign)
        DrawColor(kSignB((i-1)/30 + 1));
      f = FSphereZodiac((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f && i > 0) {
        DrawLine(xo, yo, xp, yp);
        if (i % 30 != 0) {
          k = i % 10 == 0 ? 3 : (i % 5 == 0 ? 2 : 1);
          for (j = -k; j <= k; j += (k << 1)) {
            FSphereZodiac((real)i, (real)j / 2.0, &cr, &xo, &yo);
            DrawLine(xo, yo, xp, yp);
          }
        }
      } else if (fAny)
        DrawPoint(xp, yp);
      xo = xp; yo = yp;
    }
    /* Draw sign boundaries. */
    for (i = 0; i < nDegMax; i += 30) {
      if (gs.fColorSign)
        DrawColor(kSignB(i/30 + 1));
      for (j = -90; j <= 90; j++) {
        f = FSphereZodiac((real)i, (real)j, &cr, &xp, &yp) ^ fDir;
        if (f && j > -90)
          DrawLine(xo, yo, xp, yp);
        else if (fAny && !FOdd(i))
          DrawPoint(xp, yp);
        xo = xp; yo = yp;
      }
    }
  }

  /* Draw outer boundary. */
  DrawColor(gi.kiOn);
  DrawCircle(cr.xc, cr.yc, cr.xr, cr.yr);

  /* Label signs. */
  if (!us.fVedic) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = 78; j >= -78; j -= 156)
      for (i = 1; i <= cSign; i++) {
        f = FSphereZodiac((real)(i*30-15), (real)j, &cr, &xp, &yp) ^ fDir;
        if (f || fAny) {
          DrawColor(f ? (gs.fColorSign ? kSignB(i) : kDkBlueB) : gi.kiGray);
          DrawSign(i, xp, yp);
        }
      }
    gi.nScale = nSav;
  }

  /* Label houses. */
  if (!gs.fHouseExtra) {
    nSav = gi.nScale;
    gi.nScale = gi.nScaleText * gi.nScaleT;
    for (j = 82; j >= -82; j -= 164)
      for (i = 1; i <= cSign; i++) {
        if (!us.fHouse3D)
          f = FSpherePrime(Midpoint(chouse3[i], chouse3[Mod12(i+1)]), (real)j,
            &cr, &xp, &yp) ^ fDir;
        else
          f = FSphereZodiac(Midpoint(chouse[i], chouse[Mod12(i+1)]), (real)j,
            &cr, &xp, &yp) ^ fDir;
        if (f || fAny) {
          DrawColor(f ? (gs.fColorHouse ? kSignB(i) : kDkGreenB) : gi.kiGray);
          DrawHouse(i, xp, yp);
        }
      }
    gi.nScale = nSav;
  }

  /* Label directions. */
  if (!fNoHorizon) {
    k = zGlyph >> 1;
    for (i = 0; i < nDegMax; i += 90) {
      f = FSphereLocal((real)i, 0.0, &cr, &xp, &yp) ^ fDir;
      if (f || fAny) {
        j = i / 90;
        DrawColor(kObjB[oAsc + ((j + 3) & 3)*3]);
        if (!ignorez[(1 - j) & 3])
          DrawDash(cr.xc, cr.yc, xp, yp, f ? 0 : 2);
        if (gs.fColorHouse)
          DrawColor(f ? gi.kiOn : gi.kiGray);
        FSphereLocal((real)i, 0.0, &cr2, &xp, &yp);
        sprintf(sz, "%c", szDir[j][0]);
        DrawSz(sz, xp, yp + gi.nScale, dtCent);
      }
    }
    /* Label zenith and nadir points. */
    for (j = -90; j <= 90; j += nDegHalf) {
      f = FSphereLocal(0.0, (real)j, &cr2, &xp, &yp) ^ fDir;
      if (f || fAny) {
        DrawColor(gs.fColorHouse ? (f ? gi.kiOn : gi.kiGray) :
          kObjB[j <= 0 ? oMC : oNad]);
        sprintf(sz, "%c", j <= 0 ? 'Z' : 'N');
        DrawSz(sz, xp, yp + gi.nScale, dtCent);
      }
    }
  }

  /* Draw center point. */
  if (!fNoHorizon) {
    DrawColor(gi.kiOn);
    DrawSpot(cr.xc, cr.yc);
  }

#ifdef SWISS
  /* Draw extra stars. */
  if (gs.fAllStar) {
    DrawColor(gi.kiGray);
    SwissComputeStar(0.0, NULL);
    while (SwissComputeStar(is.T, &es)) {
      f = FSphereZodiac(es.lon, es.lat, &cr, &xp, &yp) ^ fDir;
      if (f)
        DrawStar(xp, yp, &es);
    }
    DrawColor(gi.kiLite);
    EnumStarsLines(fTrue, NULL, NULL);
    while (EnumStarsLines(fFalse, &pes1, &pes2)) {
      f  = FSphereZodiac(pes1->lon, pes1->lat, &cr, &xo, &yo) ^ fDir;
      f &= FSphereZodiac(pes2->lon, pes2->lat, &cr, &xp, &yp) ^ fDir;
      if (f)
        DrawLine(xo, yo, xp, yp);
    }
  }

  /* Draw extra asteroids. */
  if (gs.nAstLo > 0) {
    DrawColor(gi.kiGray);
    SwissComputeAsteroid(0.0, NULL, fTrue);
    while (SwissComputeAsteroid(is.T, &es, fTrue)) {
      f = FSphereZodiac(es.lon, es.lat, &cr, &xp, &yp) ^ fDir;
      if (f)
        DrawStar(xp, yp, &es);
    }
  }
#endif

  /* Determine set of planet data to use. */
  for (iChart = cChart; iChart >= 1; iChart--) {
    FProcessCommandLine(szWheelX[iChart]);
    if (iChart <= 1)
      pcp = rgpcp[us.nRel <= rcDual];
    else
      pcp = rgpcp[iChart];
    cpSav = cp0;
    cp0 = *pcp;

  /* Calculate planet coordinates. */
  for (i = 0; i <= cObj; i++) {
    f = FProper(i);
    if (f) {
      f = FSphereZodiac(planet[i], planetalt[i], &cr, &xp, &yp) ^ fDir;
      rgod[i].obj = i;
      rgod[i].x = xp; rgod[i].y = yp;
      rgod[i].kv = f ? ~0 : gi.kiGray;
      rgod[i].f = fAny || f;
    } else
      rgod[i].f = fFalse;
  }

  /* Draw lines connecting planets which have aspects between them. */
  if (!FCreateGrid(fFalse))
    return;
  nSav = gi.nScale;
  gi.nScale = gi.nScaleText * gi.nScaleT;
  for (j = cObj; j >= 1; j--)
    for (i = j-1; i >= 0; i--)
      if (grid->n[i][j] && FProper(i) && FProper(j) &&
        (fAny || (rgod[i].f && rgod[j].f))) {
        DrawColor(rgod[i].kv == ~0 && rgod[j].kv == ~0 ?
          kAspB[grid->n[i][j]] : gi.kiGray);
        DrawDash(rgod[i].x, rgod[i].y, rgod[j].x, rgod[j].y,
          NAbs(grid->v[i][j] / (60*60*2)) +
          ((rgod[i].kv != ~0) + (rgod[j].kv != ~0))*2);
        if (gs.fLabelAsp)
          DrawAspect(grid->n[i][j],
            (rgod[i].x + rgod[j].x) >> 1, (rgod[i].y + rgod[j].y) >> 1);
      }
  gi.nScale = nSav;

  /* Draw planet glyphs, and spots for actual planet locations. */
  DrawObjects(rgod, objMax, 0);

    cp0 = cpSav;
  } /* iChart */
  FProcessCommandLine(szWheelX[0]);

  DrawSidebar();
}
#endif /* GRAPH */

/* xcharts1.cpp */
