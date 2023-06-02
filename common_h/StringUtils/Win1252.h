//============================================================================================
// Makeev Sergey, 2008
//============================================================================================


/*
 * CP1252

 Afrikaans (af), Albanian (sq), Basque (eu), Catalan (ca), Danish (da), Dutch (nl),
 *English (en), Faroese (fo), Finnish (fi), *French (fr), Galician (gl), *German (de), Icelandic (is)
 Irish (ga), *Italian (it), Norwegian (no), Portuguese (pt), Scottish (gd), *Spanish (es), Swedish (sv)

 */

static const byte cp1252_page01[72] = {
  0x00, 0x00, 0x8c, 0x9c, 0x00, 0x00, 0x00, 0x00, /* 0x50-0x57 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x58-0x5f */
  0x8a, 0x9a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x60-0x67 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x68-0x6f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */
  0x9f, 0x00, 0x00, 0x00, 0x00, 0x8e, 0x9e, 0x00, /* 0x78-0x7f */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80-0x87 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */
  0x00, 0x00, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */
};
static const byte cp1252_page02[32] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x88, 0x00, /* 0xc0-0xc7 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd0-0xd7 */
  0x00, 0x00, 0x00, 0x00, 0x98, 0x00, 0x00, 0x00, /* 0xd8-0xdf */
};
static const byte cp1252_page20[48] = {
  0x00, 0x00, 0x00, 0x96, 0x97, 0x00, 0x00, 0x00, /* 0x10-0x17 */
  0x91, 0x92, 0x82, 0x00, 0x93, 0x94, 0x84, 0x00, /* 0x18-0x1f */
  0x86, 0x87, 0x95, 0x00, 0x00, 0x00, 0x85, 0x00, /* 0x20-0x27 */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28-0x2f */
  0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x37 */
  0x00, 0x8b, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x38-0x3f */
};





byte UnicodeToCP1252 (word unicodeChar)
{
  if (unicodeChar < 0x0080)
	{
    return (byte)(unicodeChar & 0xFF);
  }
	
	if (unicodeChar >= 0x00a0 && unicodeChar < 0x0100)
	{
    return (byte)(unicodeChar & 0xFF);
	}

	if (unicodeChar >= 0x0150 && unicodeChar < 0x0198)
	{
    return cp1252_page01[unicodeChar - 0x0150];
	}

	if (unicodeChar >= 0x02c0 && unicodeChar < 0x02e0)
	{
    return cp1252_page02[unicodeChar - 0x02c0];
	}

	if (unicodeChar >= 0x2010 && unicodeChar < 0x2040)
	{
    return cp1252_page20[unicodeChar - 0x2010];
	}

	if (unicodeChar == 0x20ac)
	{
    return 0x80;
	}

	if (unicodeChar == 0x2122)
	{
    return 0x99;
	}

	//Non CP1252 char
  return 0x3F;
}


int ConvertUnicodeToCP1252(LPCWSTR _input, char * _out, int _outSize)
{
	int inOffset = 0;
	int outOffset = 0;

	for (;;)
	{
		WCHAR ch = _input[inOffset];
		_out[outOffset] = UnicodeToCP1252(ch);

		inOffset++;
		outOffset++;

		if (ch == 0 || outOffset == _outSize)
		{
			break;
		}
	}

	return outOffset;
}