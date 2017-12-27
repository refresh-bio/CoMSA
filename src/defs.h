#pragma once
// *******************************************************************************************
// This file is a part of MSAC software distributed under GNU GPL 3 licence.
// The homepage of the MSAC project is http://sun.aei.polsl.pl/msac
//
// Author: Sebastian Deorowicz
// Version: 1.0
// Date   : 2017-12-27
// *******************************************************************************************

#include <vector>
#include <cstdint>
#include <cstring>

using namespace std;

#ifndef _WIN32
#define my_fseek	fseek
#define my_ftell	ftell
#else
#define my_fseek	_fseeki64
#define my_ftell	_ftelli64
#endif

// *******************************************************************************************
// Class for storage of range coder compressed data
class CVectorIOStream
{
	vector<uint8_t> &v;
	size_t read_pos;

public:
	CVectorIOStream(vector<uint8_t> &_v) : v(_v), read_pos(0)
	{}

	void RestartRead()
	{
		read_pos = 0;
	}

	bool Eof()
	{
		return read_pos >= v.size();
	}

	uint8_t GetByte()
	{
		return v[read_pos++];
	}

	void PutByte(uint8_t x)
	{
		v.push_back(x);
	}
};

// *******************************************************************************************
// Buffered input file
class CInFile
{
	const int BUFFER_SIZE = 128 << 20;

	FILE *f;
	char *buffer;
	size_t buffer_pos;
	size_t buffer_filled;

	size_t file_size;
	size_t before_buffer_bytes;

public:
	CInFile() : f(nullptr), buffer(nullptr)
	{};

	~CInFile()
	{
		if (f)
			fclose(f);
		if (buffer)
			delete[] buffer;
	}

	bool Open(string file_name)
	{
		if (f)
			return false;

		f = fopen(file_name.c_str(), "rb");
		if (!f)
			return false;

		my_fseek(f, 0, SEEK_END);
		file_size = my_ftell(f);
		my_fseek(f, 0, SEEK_SET);
		before_buffer_bytes = 0;

		buffer = new char[BUFFER_SIZE];
		buffer_pos = 0;
		buffer_filled = 0;

		return true;
	}

	bool Close()
	{
		if (f)
		{
			fclose(f);
			f = nullptr;
		}
		if (buffer)
		{
			delete[] buffer;
			buffer = nullptr;
		}

		return true;
	}

	int Get()
	{
		if (buffer_pos < buffer_filled)
			return buffer[buffer_pos++];

		if (feof(f))
			return EOF;

		before_buffer_bytes += buffer_pos;

		buffer_filled = fread(buffer, 1, BUFFER_SIZE, f);
		if (buffer_filled == 0)
			return EOF;

		buffer_pos = 0;
		return buffer[buffer_pos++];
	}

	bool Eof()
	{
		return (buffer_pos == buffer_filled) && feof(f);
	}

	size_t FileSize()
	{
		if (f)
			return file_size;
		else
			return 0;
	}

	size_t GetPos()
	{
		return before_buffer_bytes + buffer_pos;
	}
};

// *******************************************************************************************
// Buffered output file
class COutFile
{
	const size_t BUFFER_SIZE = 128 << 20;

	FILE *f;
	char *buffer;
	size_t buffer_pos;
	bool success;

public:
	COutFile() : f(nullptr), buffer(nullptr)
	{};

	~COutFile()
	{
		if (f)
			Close();
		if (buffer)
			delete[] buffer;
	}

	bool Open(string file_name)
	{
		if (f)
			return false;

		f = fopen(file_name.c_str(), "wb");
		if (!f)
			return false;

		buffer = new char[BUFFER_SIZE];
		buffer_pos = 0;
		success = true;

		return true;
	}

	bool Close()
	{
		if (buffer_pos)
			success &= fwrite(buffer, 1, buffer_pos, f) == buffer_pos;

		if (f)
		{
			fclose(f);
			f = nullptr;
		}
		if (buffer)
		{
			delete[] buffer;
			buffer = nullptr;
		}

		return success;
	}

	void Put(char c)
	{
		if (buffer_pos == BUFFER_SIZE)
		{
			success &= fwrite(buffer, 1, BUFFER_SIZE, f) == BUFFER_SIZE;
			buffer_pos = 0;
		}

		buffer[buffer_pos++] = c;
	}

	void Write(const char *p, size_t n)
	{
		char *q = (char *)p;

		while (buffer_pos + n > BUFFER_SIZE)
		{
			size_t small_n = BUFFER_SIZE - buffer_pos;
			memcpy(buffer + buffer_pos, q, small_n);
			success &= fwrite(buffer, 1, BUFFER_SIZE, f) == BUFFER_SIZE;

			buffer_pos = 0;
			n -= small_n;
			q += small_n;
		}

		memcpy(buffer+buffer_pos, q, n);
		buffer_pos += n;
	}

	void Write(string &s)
	{
		Write((char*)s.c_str(), s.size());
	}

	void Write(string &s, size_t start_pos, size_t len)
	{
		Write((char*)s.c_str() + start_pos, len);
	}
};

// EOF
