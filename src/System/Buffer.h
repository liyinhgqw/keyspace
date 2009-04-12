#ifndef BUFFER_H
#define BUFFER_H

#include "Common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

class ByteString
{
public:
	int		size;
	int		length;
	char*	buffer;
	
	ByteString() { Init(); }
	
	ByteString(int size_, int length_, char* buffer_)
		: size(size_), length(length_), buffer(buffer_) {}
	
	// FIXME this should be virtual, but then KeyspaceOp will
	// call the destructor when returning to the client...
	virtual ~ByteString() {}
	
	virtual void Init() { size = 0; length = 0; buffer = 0; }

	ByteString& operator=(const ByteString &bs)
	{
		size = bs.size;
		length = bs.length;
		buffer = bs.buffer;
		
		return *this;
	}
	
	virtual bool Set(const char* str)
	{
		if (buffer == NULL)
			ASSERT_FAIL();
		
		int len = strlen(str);
		return Set(str, len);
	}
	
	virtual bool Set(const char* str, int len)
	{
		if (buffer == NULL)
			ASSERT_FAIL();
	
		if (len > size)
			return false;
		
		memcpy(buffer, str, len);
		
		length = len;
		
		return true;		
	}
	
	bool Set(ByteString other)
	{
		if (other.length > size)
			return false;
		
		memcpy(buffer, other.buffer, other.length);
		
		length = other.length;
		
		return true;
	}

	bool operator==(const ByteString& other)
	{
		return MEMCMP(buffer, length, other.buffer, other.length);
	}
	
	bool Advance(unsigned n)
	{
		if (length < (int) n)
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
	
	int Remaining() const
	{
		return size - length;
	}
};

class ByteBuffer : public ByteString
{
public:
	ByteBuffer()
	{
		buffer = NULL;
		size = 0;
		length = 0;
	}
	
	~ByteBuffer()
	{
		Free();
	}
	
	bool Allocate(int size_)
	{
		if (buffer != NULL)
			ASSERT_FAIL();
			
		buffer = (char*) Alloc(size_);
		if (buffer == NULL)
			return false;
		size = size_;
		length = 0;
		return true;
	}

	bool Reallocate(int size_)
	{
		if (size_ <= size)
		{
			length = 0;
			return true;
		}
		
		if (buffer != NULL)
			free(buffer);
		
		buffer = (char*) Alloc(size_);
		if (buffer == NULL)
			return false;
		size = size_;
		length = 0;
		return true;
	}
	
	void Free()
	{
		if (buffer != NULL)
			free(buffer);
		buffer = NULL;
		size = 0;
		length = 0;
	}
	
	void Init()
	{
		length = 0;
	}
	
private:
	ByteString& operator=(const ByteString&) { return *this; } // we would loose our pointer
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

	bool Set(ByteString bs)	{ return ByteString::Set(bs); }
};

template<int n>
class DynArray : public ByteString
{
public:
	char		data[n];
	DynArray*	next;
	
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
	
	void Init()
	{
		length = 0;
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
		
		newsize = newsize + GRAN - 1;
		newsize -= newsize % GRAN;
		
		newbuffer = new char[newsize];
		
		if (keepold && length > 0)
			memcpy(newbuffer, buffer, length);

		if (buffer != data)
			delete[] buffer;
		
		buffer = newbuffer;
	}
};

#endif
