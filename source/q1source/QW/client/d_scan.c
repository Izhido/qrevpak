/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// d_scan.c
//
// Portable C scan-level rasterization code, all pixel depths.

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for the software renderer builds (part 1):
#if !defined(GXQUAKE) && !defined(GLQUAKE)
// <<< FIX

#include "quakedef.h"
#include "r_local.h"
#include "d_local.h"

unsigned char	*r_turb_pbase, *r_turb_pdest;
fixed16_t		r_turb_s, r_turb_t, r_turb_sstep, r_turb_tstep;
int				*r_turb_turb;
int				r_turb_spancount;

void D_DrawTurbulent8Span (void);


/*
=============
D_WarpScreen

// this performs a slight compression of the screen at the same time as
// the sine warp, to keep the edges from wrapping
=============
*/
void D_WarpScreen (void)
{
	int		w, h;
	int		u,v;
	byte	*dest;
	int		*turb;
	int		*col;
	byte	**row;
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Allocating in big stack. Stack in this device is pretty small:
	//byte	*rowptr[1024];
	//int		column[1280];
	byte** rowptr = Sys_BigStackAlloc(sizeof(byte*) * 1024, "D_WarpScreen");
	int*		column = Sys_BigStackAlloc(sizeof(int) * 1280, "D_WarpScreen");
// <<< FIX

	float	wratio, hratio;

	w = r_refdef.vrect.width;
	h = r_refdef.vrect.height;

	wratio = w / (float)scr_vrect.width;
	hratio = h / (float)scr_vrect.height;

	for (v=0 ; v<scr_vrect.height+AMP2*2 ; v++)
	{
		rowptr[v] = d_viewbuffer + (r_refdef.vrect.y * screenwidth) +
				 (screenwidth * (int)((float)v * hratio * h / (h + AMP2 * 2)));
	}

	for (u=0 ; u<scr_vrect.width+AMP2*2 ; u++)
	{
		column[u] = r_refdef.vrect.x +
				(int)((float)u * wratio * w / (w + AMP2 * 2));
	}

	turb = intsintable + ((int)(cl.time*SPEED)&(CYCLE-1));
	dest = vid.buffer + scr_vrect.y * vid.rowbytes + scr_vrect.x;

	for (v=0 ; v<scr_vrect.height ; v++, dest += vid.rowbytes)
	{
		col = &column[turb[v]];
		row = &rowptr[v];
		for (u=0 ; u<scr_vrect.width ; u+=4)
		{
			dest[u+0] = row[turb[u+0]][col[u+0]];
			dest[u+1] = row[turb[u+1]][col[u+1]];
			dest[u+2] = row[turb[u+2]][col[u+2]];
			dest[u+3] = row[turb[u+3]][col[u+3]];
		}
	}

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Deallocating from previous fix:
	Sys_BigStackFree(sizeof(byte*) * 1024 + sizeof(int) * 1280, "D_WarpScreen");
// <<< FIX
}


#if	!id386

/*
=============
D_DrawTurbulent8Span
=============
*/
void D_DrawTurbulent8Span (void)
{
	int		sturb, tturb;

	do
	{
		sturb = ((r_turb_s + r_turb_turb[(r_turb_t>>16)&(CYCLE-1)])>>16)&63;
		tturb = ((r_turb_t + r_turb_turb[(r_turb_s>>16)&(CYCLE-1)])>>16)&63;
		*r_turb_pdest++ = *(r_turb_pbase + (tturb<<6) + sturb);
		r_turb_s += r_turb_sstep;
		r_turb_t += r_turb_tstep;
	} while (--r_turb_spancount > 0);
}

#endif	// !id386

/*
=============
Turbulent8
=============
*/
void Turbulent8 (espan_t *pspan)
{
	int				count;
	fixed16_t		snext, tnext;
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// These are not needed anymore in here:
	float			/*sdivz, tdivz, */zi, z, du, dv, spancountminus1;
	float			/*sdivz16stepu, tdivz16stepu, */zi16stepu;
// <<< FIX
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New fixed16_t variables mirroring the ones above:
	fixed16_t		sdivz16stepu16, tdivz16stepu16/*, zi8stepu16*/;
// <<< FIX
	
	r_turb_turb = sintable + ((int)(cl.time*SPEED)&(CYCLE-1));

	r_turb_sstep = 0;	// keep compiler happy
	r_turb_tstep = 0;	// ditto

	r_turb_pbase = (unsigned char *)cacheblock;

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// These are not needed anymore in here:
	//sdivz16stepu = d_sdivzstepu * 16;
	//tdivz16stepu = d_tdivzstepu * 16;
// <<< FIX
	zi16stepu = d_zistepu * 16;

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Calculating fixed16_t versions of the above:
	sdivz16stepu16 = d_sdivzstepu16 << 4;
	tdivz16stepu16 = d_tdivzstepu16 << 4;
	//zi8stepu16 = d_zistepu * 0x80000;
// <<< FIX

	do
	{
		r_turb_pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Removing as much floating-point operations as possible:
		//du = (float)pspan->u;
		//dv = (float)pspan->v;

		//sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		//tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		//zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		//z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		//r_turb_s = (int)(sdivz * z) + sadjust;
		//if (r_turb_s > bbextents)
		//	r_turb_s = bbextents;
		//else if (r_turb_s < 0)
		//	r_turb_s = 0;

		//r_turb_t = (int)(tdivz * z) + tadjust;
		//if (r_turb_t > bbextentt)
		//	r_turb_t = bbextentt;
		//else if (r_turb_t < 0)
		//	r_turb_t = 0;
		du = (float)pspan->u;
		dv = (float)pspan->v;
		fixed16_t du16 = pspan->u;
		fixed16_t dv16 = pspan->v;

		fixed16_t sdivz16 = d_sdivzorigin16 + dv16*d_sdivzstepv16 + du16*d_sdivzstepu16;
		fixed16_t tdivz16 = d_tdivzorigin16 + dv16*d_tdivzstepv16 + du16*d_tdivzstepu16;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)1.0 / zi;	// prescale to 16.16 fixed-point

		r_turb_s = (int)(sdivz16 * z) + sadjust;
		if (r_turb_s > bbextents)
			r_turb_s = bbextents;
		else if (r_turb_s < 0)
			r_turb_s = 0;

		r_turb_t = (int)(tdivz16 * z) + tadjust;
		if (r_turb_t > bbextentt)
			r_turb_t = bbextentt;
		else if (r_turb_t < 0)
			r_turb_t = 0;
// <<< FIX

		do
		{
		// calculate s and t at the far end of the span
			if (count >= 16)
				r_turb_spancount = 16;
			else
				r_turb_spancount = count;

			count -= r_turb_spancount;

			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Adjusting for the fix above:
				//sdivz += sdivz16stepu;
				//tdivz += tdivz16stepu;
				//zi += zi16stepu;
				//z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				//snext = (int)(sdivz * z) + sadjust;
				//if (snext > bbextents)
				//	snext = bbextents;
				//else if (snext < 16)
				//	snext = 16;	// prevent round-off error on <0 steps from
				//				//  from causing overstepping & running off the
				//				//  edge of the texture

				//tnext = (int)(tdivz * z) + tadjust;
				//if (tnext > bbextentt)
				//	tnext = bbextentt;
				//else if (tnext < 16)
				//	tnext = 16;	// guard against round-off error on <0 steps
				sdivz16 += sdivz16stepu16;
				tdivz16 += tdivz16stepu16;
				zi += zi16stepu;
				z = (float)1.0 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz16 * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz16 * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;	// guard against round-off error on <0 steps
// <<< FIX

				r_turb_sstep = (snext - r_turb_s) >> 4;
				r_turb_tstep = (tnext - r_turb_t) >> 4;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Adjusting for the fix above:
				//spancountminus1 = (float)(r_turb_spancount - 1);
				//sdivz += d_sdivzstepu * spancountminus1;
				//tdivz += d_tdivzstepu * spancountminus1;
				//zi += d_zistepu * spancountminus1;
				//z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				//snext = (int)(sdivz * z) + sadjust;
				//if (snext > bbextents)
				//	snext = bbextents;
				//else if (snext < 16)
				//	snext = 16;	// prevent round-off error on <0 steps from
				//				//  from causing overstepping & running off the
				//				//  edge of the texture

				//tnext = (int)(tdivz * z) + tadjust;
				//if (tnext > bbextentt)
				//	tnext = bbextentt;
				//else if (tnext < 16)
				//	tnext = 16;	// guard against round-off error on <0 steps
				spancountminus1 = (float)(r_turb_spancount - 1);
				fixed16_t spancountminus1_16 = r_turb_spancount - 1;
				sdivz16 += d_sdivzstepu16 * spancountminus1_16;
				tdivz16 += d_tdivzstepu16 * spancountminus1_16;
				zi += d_zistepu * spancountminus1;
				z = (float)1.0 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz16 * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 16)
					snext = 16;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz16 * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 16)
					tnext = 16;	// guard against round-off error on <0 steps
// <<< FIX

				if (r_turb_spancount > 1)
				{
					r_turb_sstep = (snext - r_turb_s) / (r_turb_spancount - 1);
					r_turb_tstep = (tnext - r_turb_t) / (r_turb_spancount - 1);
				}
			}

			r_turb_s = r_turb_s & ((CYCLE<<16)-1);
			r_turb_t = r_turb_t & ((CYCLE<<16)-1);

			D_DrawTurbulent8Span ();

			r_turb_s = snext;
			r_turb_t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}


#if	!id386

/*
=============
D_DrawSpans8
=============
*/
void D_DrawSpans8 (espan_t *pspan)
{
	int				count, spancount;
	unsigned char	*pbase, *pdest;
	fixed16_t		s, t, snext, tnext, sstep, tstep;
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// These are not needed anymore in here:
	float			/*sdivz, tdivz, */zi, z, du, dv, spancountminus1;
	float			/*sdivz8stepu, tdivz8stepu, */zi8stepu;
// <<< FIX
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New fixed16_t variables mirroring the ones above:
	fixed16_t		sdivz8stepu16, tdivz8stepu16/*, zi8stepu16*/;
// <<< FIX
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// New variables to track t changes:
	int tcheck;
	int twhole;
	int prev_twhole;
	int tspan;

	prev_twhole = 0;
	tspan = 0;
// <<< FIX

	sstep = 0;	// keep compiler happy
	tstep = 0;	// ditto

	pbase = (unsigned char *)cacheblock;

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// These are not needed anymore in here:
	//sdivz8stepu = d_sdivzstepu * 8;
	//tdivz8stepu = d_tdivzstepu * 8;
// <<< FIX
	zi8stepu = d_zistepu * 8;
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Calculating fixed16_t versions of the above:
	sdivz8stepu16 = d_sdivzstepu16 << 3;
	tdivz8stepu16 = d_tdivzstepu16 << 3;
	//zi8stepu16 = d_zistepu * 0x80000;
// <<< FIX

	do
	{
		pdest = (unsigned char *)((byte *)d_viewbuffer +
				(screenwidth * pspan->v) + pspan->u);

		count = pspan->count;

	// calculate the initial s/z, t/z, 1/z, s, and t and clamp
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Removing as much floating-point operations as possible:
		//du = (float)pspan->u;
		//dv = (float)pspan->v;

		//sdivz = d_sdivzorigin + dv*d_sdivzstepv + du*d_sdivzstepu;
		//tdivz = d_tdivzorigin + dv*d_tdivzstepv + du*d_tdivzstepu;
		//zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		//z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

		//s = (int)(sdivz * z) + sadjust;
		//if (s > bbextents)
		//	s = bbextents;
		//else if (s < 0)
		//	s = 0;

		//t = (int)(tdivz * z) + tadjust;
		//if (t > bbextentt)
		//	t = bbextentt;
		//else if (t < 0)
		//	t = 0;
		du = (float)pspan->u;
		dv = (float)pspan->v;
		fixed16_t du16 = pspan->u;
		fixed16_t dv16 = pspan->v;

		fixed16_t sdivz16 = d_sdivzorigin16 + dv16*d_sdivzstepv16 + du16*d_sdivzstepu16;
		fixed16_t tdivz16 = d_tdivzorigin16 + dv16*d_tdivzstepv16 + du16*d_tdivzstepu16;
		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
		z = (float)1.0 / zi;	// prescale to 16.16 fixed-point

		s = (int)(sdivz16 * z) + sadjust;
		if (s > bbextents)
			s = bbextents;
		else if (s < 0)
			s = 0;

		t = (int)(tdivz16 * z) + tadjust;
		if (t > bbextentt)
			t = bbextentt;
		else if (t < 0)
			t = 0;
// <<< FIX

		do
		{
		// calculate s and t at the far end of the span
			if (count >= 8)
				spancount = 8;
			else
				spancount = count;

			count -= spancount;

			if (count)
			{
			// calculate s/z, t/z, zi->fixed s and t at far end of span,
			// calculate s and t steps across span by shifting
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Adjusting for the fix above:
				//sdivz += sdivz8stepu;
				//tdivz += tdivz8stepu;
				//zi += zi8stepu;
				//z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point

				//snext = (int)(sdivz * z) + sadjust;
				//if (snext > bbextents)
				//	snext = bbextents;
				//else if (snext < 8)
				//	snext = 8;	// prevent round-off error on <0 steps from
				//				//  from causing overstepping & running off the
				//				//  edge of the texture

				//tnext = (int)(tdivz * z) + tadjust;
				//if (tnext > bbextentt)
				//	tnext = bbextentt;
				//else if (tnext < 8)
				//	tnext = 8;	// guard against round-off error on <0 steps
				sdivz16 += sdivz8stepu16;
				tdivz16 += tdivz8stepu16;
				zi += zi8stepu;
				z = (float)1.0 / zi;	// prescale to 16.16 fixed-point

				snext = (int)(sdivz16 * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz16 * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
// <<< FIX

				sstep = (snext - s) >> 3;
				tstep = (tnext - t) >> 3;
			}
			else
			{
			// calculate s/z, t/z, zi->fixed s and t at last pixel in span (so
			// can't step off polygon), clamp, calculate s and t steps across
			// span by division, biasing steps low so we don't run off the
			// texture
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Adjusting for the fix above:
				//spancountminus1 = (float)(spancount - 1);
				//sdivz += d_sdivzstepu * spancountminus1;
				//tdivz += d_tdivzstepu * spancountminus1;
				//zi += d_zistepu * spancountminus1;
				//z = (float)0x10000 / zi;	// prescale to 16.16 fixed-point
				//snext = (int)(sdivz * z) + sadjust;
				//if (snext > bbextents)
				//	snext = bbextents;
				//else if (snext < 8)
				//	snext = 8;	// prevent round-off error on <0 steps from
				//				//  from causing overstepping & running off the
				//				//  edge of the texture

				//tnext = (int)(tdivz * z) + tadjust;
				//if (tnext > bbextentt)
				//	tnext = bbextentt;
				//else if (tnext < 8)
				//	tnext = 8;	// guard against round-off error on <0 steps
				spancountminus1 = (float)(spancount - 1);
				fixed16_t spancountminus1_16 = spancount - 1;
				sdivz16 += d_sdivzstepu16 * spancountminus1_16;
				tdivz16 += d_tdivzstepu16 * spancountminus1_16;
				zi += d_zistepu * spancountminus1;
				z = (float)1.0 / zi;	// prescale to 16.16 fixed-point
				snext = (int)(sdivz16 * z) + sadjust;
				if (snext > bbextents)
					snext = bbextents;
				else if (snext < 8)
					snext = 8;	// prevent round-off error on <0 steps from
								//  from causing overstepping & running off the
								//  edge of the texture

				tnext = (int)(tdivz16 * z) + tadjust;
				if (tnext > bbextentt)
					tnext = bbextentt;
				else if (tnext < 8)
					tnext = 8;	// guard against round-off error on <0 steps
// <<< FIX

				if (spancount > 1)
				{
					sstep = (snext - s) / (spancount - 1);
					tstep = (tnext - t) / (spancount - 1);
				}
			}

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Switching between span drawing routines (part 1):
			tcheck = ((t + spancount * tstep) >> 16) - (t >> 16);
			if(tcheck == 0)
			{
				tspan = (t >> 16) * cachewidth;
				do
				{
					*pdest++ = *(pbase + (s >> 16) + tspan);
					s += sstep;
				} while (--spancount > 0);
			} else 
			{
				if(tcheck < 0)
				{
					tcheck = -tcheck;
				};
				tcheck = tcheck << 2;
				if(tcheck < spancount)
				{
					do
					{
						twhole = (t >> 16);
						if(prev_twhole != twhole)
						{
							prev_twhole = twhole;
							tspan = twhole * cachewidth;
						};
						*pdest++ = *(pbase + (s >> 16) + tspan);
						s += sstep;
						t += tstep;
					} while (--spancount > 0);
				} else
				{
// <<< FIX
			do
			{
				*pdest++ = *(pbase + (s >> 16) + (t >> 16) * cachewidth);
				s += sstep;
				t += tstep;
			} while (--spancount > 0);

// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Switching between span drawing routines (part 2):
				};
			};
// <<< FIX
			s = snext;
			t = tnext;

		} while (count > 0);

	} while ((pspan = pspan->pnext) != NULL);
}

#endif


#if	!id386

/*
=============
D_DrawZSpans
=============
*/
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Replacing function, to remove local floating-point variables (part 1):
#if 0
// <<< FIX
void D_DrawZSpans (espan_t *pspan)
{
	int				count, doublecount, izistep;
	int				izi;
	short			*pdest;
	unsigned		ltemp;
	double			zi;
	float			du, dv;

// FIXME: check for clamping/range problems
// we count on FP exceptions being turned off to avoid range problems
	izistep = (int)(d_zistepu * 0x8000 * 0x10000);

	do
	{
		pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

	// calculate the initial 1/z
		du = (float)pspan->u;
		dv = (float)pspan->v;

		zi = d_ziorigin + dv*d_zistepv + du*d_zistepu;
	// we count on FP exceptions being turned off to avoid range problems
		izi = (int)(zi * 0x8000 * 0x10000);

		if ((long)pdest & 0x02)
		{
			*pdest++ = (short)(izi >> 16);
			izi += izistep;
			count--;
		}

		if ((doublecount = count >> 1) > 0)
		{
			do
			{
				ltemp = izi >> 16;
				izi += izistep;
				ltemp |= izi & 0xFFFF0000;
				izi += izistep;
				*(int *)pdest = ltemp;
				pdest += 2;
			} while (--doublecount > 0);
		}

		if (count & 1)
			*pdest = (short)(izi >> 16);

	} while ((pspan = pspan->pnext) != NULL);
}
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Replacing function, to remove local floating-point variables (part 2):
#endif
void D_DrawZSpans (espan_t *pspan)
{
	int				count, doublecount, izistep;
	int				izi;
	short			*pdest;
	unsigned		ltemp;
	int			du, dv;
	int ziorigin;
	int zistepv;

// FIXME: check for clamping/range problems
// we count on FP exceptions being turned off to avoid range problems
	izistep = d_zistepu * 0x80000000;
	ziorigin = d_ziorigin * 0x80000000;
	zistepv = d_zistepv * 0x80000000;
	do
	{
		pdest = d_pzbuffer + (d_zwidth * pspan->v) + pspan->u;

		count = pspan->count;

	// calculate the initial 1/z
		du = pspan->u;
		dv = pspan->v;

		izi = ziorigin + dv*zistepv + du*izistep;
	// we count on FP exceptions being turned off to avoid range problems

		if ((long)pdest & 0x02)
		{
			*pdest++ = (short)(izi >> 16);
			izi += izistep;
			count--;
		}

		if ((doublecount = count >> 1) > 0)
		{
			do
			{
				ltemp = izi >> 16;
				izi += izistep;
				ltemp |= izi & 0xFFFF0000;
				izi += izistep;
				*(int *)pdest = ltemp;
				pdest += 2;
			} while (--doublecount > 0);
		}

		if (count & 1)
			*pdest = (short)(izi >> 16);

	} while ((pspan = pspan->pnext) != NULL);
}
// <<< FIX

#endif
// >>> FIX: For Nintendo Wii using devkitPPC / libogc
// Include only for the software renderer builds (part 2):
#endif
// <<< FIX

