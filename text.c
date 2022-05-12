/*
 * Hocoslamfy, text rendering code file
 * Copyright (C) 2013 Nebuleon Fumika <nebuleon@gcw-zero.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "text.h"
#include "unifont.h"

void InitializeText(bool* Continue, bool* Error)
{
}

void FinalizeText()
{
}

static uint32_t CutString(const char* String, const uint32_t MaxWidth, struct StringCut* Cuts, uint32_t CutsAllocated, const int fontHeight)
{
	uint32_t Cut = 0;
	uint32_t CutStart = 0, Cur = 0, CutWidth = 0;
	uint32_t LastSpace = -1;
	bool SpaceInCut = false;
	const unsigned char* fontWidth = _font_width;
	if (fontHeight != 16)
	{
		fontWidth = _font_width_small;
	}

	while (String[Cur] != '\0')
	{
		if (String[Cur] != '\n')
		{
			if (String[Cur] == ' ')
			{
				LastSpace = Cur;
				SpaceInCut = true;
			}
			CutWidth += fontWidth[(uint8_t) String[Cur]];
		}

		if (String[Cur] == '\n' || CutWidth > MaxWidth)
		{
			if (Cut < CutsAllocated)
				Cuts[Cut].Start = CutStart;
			if (String[Cur] == '\n')
			{
				if (Cut < CutsAllocated)
					Cuts[Cut].End = Cur;
			}
			else if (CutWidth > MaxWidth)
			{
				if (SpaceInCut)
				{
					if (Cut < CutsAllocated)
						Cuts[Cut].End = LastSpace;
					Cur = LastSpace;
				}
				else
				{
					if (Cut < CutsAllocated)
						Cuts[Cut].End = Cur;
					Cur--; // Next iteration redoes this character
				}
			}
			CutStart = Cur + 1;
			CutWidth = 0;
			SpaceInCut = false;
			Cut++;
		}
		Cur++;
	}

	if (Cut < CutsAllocated)
	{
		Cuts[Cut].Start = CutStart;
		Cuts[Cut].End = Cur;
	}
	return Cut + 1;
}

uint32_t GetSectionRenderedWidth(const char* String, const uint32_t Start, const uint32_t End, const int fontHeight)
{
	uint32_t Result = 0, i;
	const unsigned char* fontWidth = _font_width;
	if (fontHeight != 16)
	{
		fontWidth = _font_width_small;
	}

	for (i = Start; i < End; i++)
	{
		Result += fontWidth[(uint8_t) String[i]];
	}
	return Result;
}

#if SCREEN_BPP == 16
void PrintString(const char* String, uint16_t TextColor,
	void* Dest, uint32_t DestPitch, uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height,
	enum HorizontalAlignment HorizontalAlignment, enum VerticalAlignment VerticalAlignment,
	const int fontHeight)
#else
void PrintString(const char* String, uint32_t TextColor,
	void* Dest, uint32_t DestPitch, uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height,
	enum HorizontalAlignment HorizontalAlignment, enum VerticalAlignment VerticalAlignment,
	const int fontHeight)
#endif
{
	const unsigned char* fontWidth = _font_width;
	const unsigned short* fontBits = _font_bits;
	if (fontHeight != 16)
	{
		fontWidth = _font_width_small;
		fontBits = _font_bits_small;
	}

	struct StringCut* Cuts = malloc((Height / fontHeight) * sizeof(struct StringCut));
	uint32_t CutCount = CutString(String, Width, Cuts, Height / fontHeight, fontHeight), Cut;
	if (CutCount > Height / fontHeight)
	{
		CutCount = Height / fontHeight;
	}

	for (Cut = 0; Cut < CutCount; Cut++)
	{
		uint32_t TextWidth = GetSectionRenderedWidth(String, Cuts[Cut].Start, Cuts[Cut].End, fontHeight);
		uint32_t LineX, LineY;
		switch (HorizontalAlignment)
		{
			case LEFT:   LineX = X;                           break;
			case CENTER: LineX = X + (Width - TextWidth) / 2; break;
			case RIGHT:  LineX = (X + Width) - TextWidth;     break;
			default:     LineX = 0; /* shouldn't happen */    break;
		}
		switch (VerticalAlignment)
		{
			case TOP:
				LineY = Y + Cut * fontHeight;
				break;
			case MIDDLE:
				LineY = Y + (Height - CutCount * fontHeight) / 2 + Cut * fontHeight;
				break;
			case BOTTOM:
				LineY = (Y + Height) - (CutCount - Cut) * fontHeight;
				break;
			default:
				LineY = 0; /* shouldn't happen */
				break;
		}

		uint32_t Cur;
		for (Cur = Cuts[Cut].Start; Cur < Cuts[Cut].End; Cur++)
		{
			uint32_t glyph_offset = (uint32_t) String[Cur] * fontHeight;
			uint32_t glyph_width = fontWidth[(uint8_t) String[Cur]];
			uint32_t glyph_column, glyph_row;
			uint16_t current_halfword;

			for(glyph_row = 0; glyph_row < fontHeight; glyph_row++, glyph_offset++)
			{
				current_halfword = fontBits[glyph_offset];
				for (glyph_column = 0; glyph_column < glyph_width; glyph_column++)
				{
					if ((current_halfword >> (15 - glyph_column)) & 0x01)
					{
#if SCREEN_BPP == 16
							*(uint16_t*) ((uint8_t*) Dest + (LineY + glyph_row) * DestPitch + (LineX + glyph_column) * sizeof(uint16_t)) = TextColor;
#else
							*(uint32_t*) ((uint8_t*) Dest + (LineY + glyph_row) * DestPitch + (LineX + glyph_column) * sizeof(uint32_t)) = TextColor;
#endif
					}
						
				}
			}

			LineX += glyph_width;
		}
	}

	free(Cuts);
}

uint32_t GetRenderedWidthBySize(const char* str, const int fontHeight)
{
	const unsigned char* fontWidth = _font_width;
	if (fontHeight != 16)
	{
		fontWidth = _font_width_small;
	}

	struct StringCut* Cuts = malloc(sizeof(struct StringCut));
	uint32_t CutCount = CutString(str, UINT32_MAX, Cuts, 1, fontHeight);
	if (CutCount > 1)
	{
		Cuts = realloc(Cuts, CutCount * sizeof(struct StringCut));
		CutString(str, UINT32_MAX, Cuts, CutCount, fontHeight);
	}

	uint32_t Result = 0, LineWidth, Cut;
	for (Cut = 0; Cut < CutCount; Cut++)
	{
		LineWidth = 0;
		uint32_t Cur;
		for (Cur = Cuts[Cut].Start; Cur < Cuts[Cut].End; Cur++)
		{
			LineWidth += fontWidth[(uint8_t) str[Cur]];
		}
		if (LineWidth > Result)
			Result = LineWidth;
	}

	free(Cuts);

	return Result;
}

uint32_t GetRenderedWidth(const char* str)
{
	return GetRenderedWidthBySize(str, _font_height);
}

uint32_t GetRenderedHeightBySize(const char* str, const int fontHeight)
{
	return CutString(str, UINT32_MAX, NULL, 0, fontHeight) * fontHeight;
}

uint32_t GetRenderedHeight(const char* str)
{
	return GetRenderedHeightBySize(str, _font_height);
}

#if SCREEN_BPP == 16
void PrintSizedStringOutline(const char* String, uint16_t TextColor, uint16_t OutlineColor,
	void* Dest, uint32_t DestPitch, uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height,
	enum HorizontalAlignment HorizontalAlignment, enum VerticalAlignment VerticalAlignment,
	const int fontHeight)
#else
void PrintSizedStringOutline(const char* String, uint32_t TextColor, uint32_t OutlineColor,
	void* Dest, uint32_t DestPitch, uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height,
	enum HorizontalAlignment HorizontalAlignment, enum VerticalAlignment VerticalAlignment,
	const int fontHeight)
#endif
{
	uint32_t sx, sy;
	for (sx = 0; sx <= 2; sx++)
		for (sy = 0; sy <= 2; sy++)
			if (!(sx == 1 && sy == 1))
				PrintString(String, OutlineColor, Dest, DestPitch, X + sx, Y + sy, Width - 2, Height - 2, HorizontalAlignment, VerticalAlignment, fontHeight);
				
	PrintString(String, TextColor, Dest, DestPitch, X + 1, Y + 1, Width - 2, Height - 2, HorizontalAlignment, VerticalAlignment, fontHeight);
}

#if SCREEN_BPP == 16
void PrintStringOutline(const char* String, uint16_t TextColor, uint16_t OutlineColor,
	void* Dest, uint32_t DestPitch, uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height,
	enum HorizontalAlignment HorizontalAlignment, enum VerticalAlignment VerticalAlignment)
#else
void PrintStringOutline(const char* String, uint32_t TextColor, uint32_t OutlineColor,
	void* Dest, uint32_t DestPitch, uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height,
	enum HorizontalAlignment HorizontalAlignment, enum VerticalAlignment VerticalAlignment)
#endif
{
	PrintSizedStringOutline(String, TextColor, OutlineColor, Dest, DestPitch, X, Y, Width, Height, HorizontalAlignment, VerticalAlignment, _font_height);
}

#if SCREEN_BPP == 16
void PrintSmallStringOutline(const char* String, uint16_t TextColor, uint16_t OutlineColor,
	void* Dest, uint32_t DestPitch, uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height,
	enum HorizontalAlignment HorizontalAlignment, enum VerticalAlignment VerticalAlignment)
#else
void PrintSmallStringOutline(const char* String, uint32_t TextColor, uint32_t OutlineColor,
	void* Dest, uint32_t DestPitch, uint32_t X, uint32_t Y, uint32_t Width, uint32_t Height,
	enum HorizontalAlignment HorizontalAlignment, enum VerticalAlignment VerticalAlignment)
#endif
{
	PrintSizedStringOutline(String, TextColor, OutlineColor, Dest, DestPitch, X, Y, Width, Height, HorizontalAlignment, VerticalAlignment, _font_height_small);
}