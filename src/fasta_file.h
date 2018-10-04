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

	vector<string> v_sequences;
	vector<string> v_names;
	int wrap_width;
	bool store_sequences_only;

	bool read_name(string &str);
	bool read_sequence(string &str);

public:
	CFastaFile() : in(nullptr), out(nullptr) 
	{
	};

	~CFastaFile()
	{
		if (in)
			delete in;
		if (out)
			delete out;
	}

	bool ReadFile(string file_name);
	bool SaveFile(string file_name);

	bool GetSequences(vector<string> &_v_names, vector<string> &_v_sequences);
	bool PutSequences(vector<string> &_v_names, vector<string> &_v_sequences, int _wrap_width, bool _store_sequences_only);
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
