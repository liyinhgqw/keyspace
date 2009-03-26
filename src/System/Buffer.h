#ifndef BUFFER_H
#define BUFFER_H

#include <stdarg.h>
#include "Common.h"

class ByteString
{
public:
	int		size;
	int		length;
	char*	buffer;
	
	ByteString() { Init(); }
	
	ByteString(int size_, int length_, char* buffer_)
		: size(size_), length(length_), buffer(buffer_) {}
	
	void Init() { size = 0; length = 0; buffer = 0; }
	
	bool Set(const char* str)
	{
		if (buffer == NULL)
			ASSERT_FAIL();
		
		int len = strlen(str);
		return Set(str, len);
	}
	
	bool Set(const char* str, int len)
	{
		if (buffer == NULL)
			ASSERT_FAIL();
	
		if (len > size)
			return false;
		
		memcpy(buffer, str, len);
		
		length = len;
		
		return true;		
	}
	
	bool operator==(const ByteString& other)
	{
		return MEMCMP(buffer, length, other.buffer, other.length);
	}
	
	bool Advance(unsigned n)
	{
		if (length < n)
			return false;
		
		buffer += n;
		length -= n;
		size   -= n;
		
		return true;
	}
	
	void Clear() { length = 0; }
	
	bool Printf(const char* fmt, ...)
	{
		va_list ap;
		
		va_start(ap, fmt);
		length = vsnprintf(buffer, size, fmt, ap);
		va_end(ap);
		
		if (length > size)
		{
			length = size;
			return false;
		}
		
		return true;
	}
};

template<int n>
class ByteArray : public ByteString
{
public:
	char	data[n];
	
	ByteArray() { size = n; length = 0; buffer = data; }
	
	ByteArray(const char* str)
	{
		size = n;
		length = strlen(str);
		memcpy(data, str, length);
		buffer = data;
	}
	
	bool Set(const char* str) { return ByteString::Set(str); }

	bool Set(const char* str, int len) { return ByteString::Set(str, len); }
							
	bool Set(ByteString bs)
	{
		if (bs.length > size)
			return false;
		
		memcpy(buffer, bs.buffer, bs.length);
		
		length = bs.length;
		
		return true;
	}
};

template<int n>
class DynArray : public ByteString
{
public:
	char data[n];
	enum { GRAN = 32 };
	DynArray()
	{
		size = n;
		length = 0;
		buffer = data;
	}
	
	~DynArray()
	{
		if (buffer != data)
			delete[] buffer;
	}
	
	bool Append(const char *str, int len)
	{
		if (length + len > size)
			Reallocate(length + len, true);

		memcpy(data + length, str, len);
		length += len;
		
		return true;
	}
	
	void Reallocate(int newsize, bool keepold)
	{
		char *newbuffer;
		
		size = size + GRAN - 1;
		size -= size % GRAN;
		
		newbuffer = new char[size];
		
		if (keepold)
			memcpy(newbuffer, buffer, length);

		if (buffer != data)
			delete[] buffer;
		
		buffer = newbuffer;
	}
};

#endif
