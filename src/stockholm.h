#pragma once
// *******************************************************************************************
// This file is a part of MSAC software distributed under GNU GPL 3 licence.
// The homepage of the MSAC project is http://sun.aei.polsl.pl/msac
//
// Author: Sebastian Deorowicz
// Version: 1.0
// Date   : 2017-12-27
// *******************************************************************************************

#include <string>
#include <vector>
#include <cstdio>
#include "defs.h"

using namespace std;

// *******************************************************************************************
//
// *******************************************************************************************
class CStockholmFile
{
//	FILE *f;
	CInFile *in;
	COutFile *out;
	bool mode_reading;

	bool read_line(vector<uint8_t> &vu);

public:
	CStockholmFile() : in(nullptr), out(nullptr)
	{};

	~CStockholmFile()
	{
		if (in)
			delete in;
		if (out)
			delete out;
	}

	bool OpenForReading(string file_name);
	bool OpenForWriting(string file_name);
	bool Close();

	bool Eof();
	size_t GetPos();

	bool GetSequences(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences);
	bool PutSequences(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences);
};

// *******************************************************************************************
//
// *******************************************************************************************
class CCompressedStockholmFile
{
	FILE *f;

public:
	CCompressedStockholmFile() : f(nullptr)
	{};

	~CCompressedStockholmFile()
	{
		if (f)
			fclose(f);
	}

	bool OpenForReading(string file_name);
	bool OpenForWriting(string file_name);
	bool Close();

	bool Store(vector<uint8_t> &v_compressed_data);
	bool Load(vector<uint8_t> &v_compressed_data);

	bool Eof();
};

// EOF
