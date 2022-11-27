#ifndef _string_hash_h_
#define _string_hash_h_


inline unsigned long StringHash(const char * str, unsigned long & len)
{
	len = 0;
	if (!str) return 0;

	unsigned int hash = 1315423911;
	const char * __restrict char_ptr = str;

	while (*char_ptr)
	{
		char c = *char_ptr++;
		hash ^= ((hash << 5) + c + (hash >> 2));
	}
	len = (unsigned long)(char_ptr - str);

	return hash;
}

inline unsigned long StringHash(const char * str)
{
	unsigned long len;
	return StringHash(str, len);
}

inline unsigned long StringHashNoCase(const char * str, unsigned long & len)
{
	len = 0;
	if (!str) return 0;

	unsigned int hash = 1315423911;
	const char * __restrict char_ptr = str;

	while(*char_ptr != '\0')
	{
		char c = *char_ptr++;
		if(c <= 'Z' && c >= 'A') c += 'a' - 'A';

		hash ^= ((hash << 5) + c + (hash >> 2));
	}
	len = (unsigned long)(char_ptr - str);
	return hash;
}

inline unsigned long StringHashNoCase(const char * str)
{
	unsigned long len;
	return StringHashNoCase(str, len);
}


#endif

