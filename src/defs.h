#pragma once
// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include "libs/zlib.h"

using namespace std;

#ifndef _WIN32
#define my_fseek	fseek
#define my_ftell	ftell
#else
#define my_fseek	_fseeki64
#define my_ftell	_ftelli64
#endif

//#define EXPERIMENTAL_MODE

const vector<uint8_t> GF_ID{ '#', '=', 'G', 'F', ' ', 'I', 'D' };
const vector<uint8_t> GF_AC{ '#', '=', 'G', 'F', ' ', 'A', 'C' };

enum class stage_mode_t {forward, reverse, copy_forward, copy_reverse};

// *******************************************************************************************
struct stockholm_family_desc_t {
	size_t n_sequences;
	size_t n_columns;
	size_t raw_size;
	size_t compressed_size;
	size_t compressed_data_ptr;
	string ID;
	string AC;

	stockholm_family_desc_t() : n_sequences(0), n_columns(0), raw_size(0), compressed_size(0), compressed_data_ptr(0)
	{};

	stockholm_family_desc_t(size_t _n_sequences, size_t _n_columns, size_t _raw_size, size_t _compressed_size,
		size_t _compressed_data_ptr, string _ID, string _AC) :
		n_sequences(_n_sequences),
		n_columns(_n_columns),
		raw_size(_raw_size),
		compressed_size(_compressed_size),
		compressed_data_ptr(_compressed_data_ptr),
		ID(_ID),
		AC(_AC)
	{};
};

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
	gzFile_s *gz_f;
	char *buffer;
	size_t buffer_pos;
	size_t buffer_filled;
	bool gz_used;

	size_t file_size;
	size_t before_buffer_bytes;

public:
	CInFile() : f(nullptr), gz_f(nullptr), buffer(nullptr), gz_used(false)
	{};

	~CInFile()
	{
		if (f)
			fclose(f);
		if (gz_f)
			gzclose(gz_f);
		if (buffer)
			delete[] buffer;
	}

	bool Open(string file_name)
	{
		if (f || gz_f)
			return false;

		gz_used = (file_name.size() > 3 && file_name.substr(file_name.size() - 3, 3) == ".gz");

		if (gz_used)
		{
			gz_f = gzopen(file_name.c_str(), "rb");
			if (!gz_f)
				return false;
			file_size = 0;
			gzbuffer(gz_f, 32 << 20);
		}
		else
		{
			f = fopen(file_name.c_str(), "rb");
			if (!f)
				return false;

			my_fseek(f, 0, SEEK_END);
			file_size = my_ftell(f);
			my_fseek(f, 0, SEEK_SET);
		}

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

		if (gz_f)
		{
			gzclose(gz_f);
			gz_f = nullptr;
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

		if (gz_used)
		{
			if (gzeof(gz_f))
				return EOF;
		}
		else
		{
			if (feof(f))
				return EOF;
		}

		before_buffer_bytes += buffer_pos;

		if(gz_used)
			buffer_filled = gzread(gz_f, buffer, BUFFER_SIZE);
		else
			buffer_filled = fread(buffer, 1, BUFFER_SIZE, f);

		if (buffer_filled == 0)
			return EOF;

		buffer_pos = 0;
		return buffer[buffer_pos++];
	}

	bool Eof()
	{
		if(gz_used)
			return (buffer_pos == buffer_filled) && gzeof(gz_f);
		else
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
