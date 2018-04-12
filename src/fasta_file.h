#pragma once
// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.1
// Date   : 2018-04-12
// *******************************************************************************************

#include <vector>
#include <string>
#include <cstdio>
#include "defs.h"

using namespace std;

// *******************************************************************************************
//
// *******************************************************************************************
class CFastaFile
{
	CInFile *in;
	COutFile *out;
	FILE *f;

	vector<string> v_sequences;
	vector<string> v_names;
	int wrap_width;

	bool read_name(string &str);
	bool read_sequence(string &str);

public:
	CFastaFile() : in(nullptr), out(nullptr), f(nullptr)
	{
	};

	~CFastaFile()
	{
		if (in)
			delete in;
		if (out)
			delete out;
		if (f)
			fclose(f);
	}

	bool ReadFile(string file_name);
	bool SaveFile(string file_name);

	bool GetSequences(vector<string> &_v_names, vector<string> &_v_sequences);
	bool PutSequences(vector<string> &_v_names, vector<string> &_v_sequences, int _wrap_width);
};

// *******************************************************************************************
//
// *******************************************************************************************
class CCompressedFastaFile
{
public:
	static bool Save(string file_name, vector<uint8_t> &v_compressed_data);
	static bool Load(string file_name, vector<uint8_t> &v_compressed_data);
};

// EOF
