#pragma once
// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
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
	CInFile *in;
	COutFile *out;
	bool mode_reading;

	bool read_line(vector<uint8_t> &vu);
	string trim(string s);

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

	bool GetSequences(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences,
		string &ID, string &AC);
	bool PutSequences(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences, 
		int wrap_width, bool store_sequences_only);
};

// *******************************************************************************************
//
// *******************************************************************************************
class CCompressedStockholmFile
{
	FILE *f;

	size_t file_pos;
	size_t logical_file_size;

	vector<stockholm_family_desc_t> v_fam_desc;

	size_t store_uint(size_t x, bool fixed_size = false);
	size_t load_uint(size_t &x, bool fixed_size = false);
	void preload_family_descriptions(size_t footer_size);

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
	bool StoreFamilyDescriptions(vector<stockholm_family_desc_t> &_v_fam_desc);
	bool Load(vector<uint8_t> &v_compressed_data);
	bool LoadFamilyDescriptions(vector<stockholm_family_desc_t> &_v_fam_desc);

	size_t GetPos();
	bool SetPos(size_t pos);

	bool Eof();
};

// EOF
