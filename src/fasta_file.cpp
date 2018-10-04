// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdio>
#include "fasta_file.h"
#include "defs.h"

// *******************************************************************************************
// CFastaFile
// *******************************************************************************************


// *******************************************************************************************
// Read FASTA file into memory
bool CFastaFile::ReadFile(string file_name)
{
	v_names.clear();
	v_sequences.clear();

	in = new CInFile;

	if (!in->Open(file_name))
		return false;

	string name;
	string sequence;
	string tmp;

	int test_symbol = 0;

	if (!in->Eof())
		test_symbol = in->Get();

	if (test_symbol != '>')
		return false;

	while (!in->Eof())
	{
		// Read name
		if (!read_name(name))
			break;

		// Read sequence
		if (!read_sequence(sequence))
			break;

		v_names.emplace_back(name);
		v_sequences.emplace_back(sequence);
	}

	delete in;
	in = nullptr;

	return true;
}

// *******************************************************************************************
// Save sequences to FASTA file
bool CFastaFile::SaveFile(string file_name)
{
	out = new COutFile();
	if (!out->Open(file_name))
		return false;

	size_t n_seq = std::max(v_names.size(), v_sequences.size());

	if (!n_seq)
		return true;

	size_t seq_size = v_sequences.front().size();

	for (size_t i = 0; i < n_seq; ++i)
	{
		out->Write(v_names[i]);
		out->Put('\n');

		// remove any gaps if requested
		if (store_sequences_only)
			v_sequences[i].erase(remove_if(v_sequences[i].begin(), v_sequences[i].end(), [](char c) {return c < 'A' || c > 'z'; }), v_sequences[i].end());

		if (wrap_width == 0)
		{
			out->Write(v_sequences[i]);
			out->Put('\n');
		}
		else
		{
			size_t cur_pos = 0;

			while (cur_pos < seq_size)
			{
				size_t end_pos = cur_pos + wrap_width;
				if (end_pos > seq_size)
					end_pos = seq_size;

				out->Write(v_sequences[i], cur_pos, end_pos - cur_pos);
				out->Put('\n');

				cur_pos = end_pos;
			}
		}
	}

	out->Close();
	delete out;
	out = nullptr;

	return true;
}

// *******************************************************************************************
// Return sequences read from FASTA file
bool CFastaFile::GetSequences(vector<string> &_v_names, vector<string> &_v_sequences)
{
	_v_names = std::move(v_names);
	_v_sequences = std::move(v_sequences);

	return !v_sequences.empty();
}

// *******************************************************************************************
// Pass sequences to store
bool CFastaFile::PutSequences(vector<string> &_v_names, vector<string> &_v_sequences, int _wrap_width, bool _store_sequences_only)
{
	v_names = std::move(_v_names);
	v_sequences = std::move(_v_sequences);
	wrap_width = _wrap_width;
	store_sequences_only = _store_sequences_only;

	return !v_sequences.empty();
}

// *******************************************************************************************
// Read id of a sequence
bool CFastaFile::read_name(string &str)
{
	const int SMALL_BUF_SIZE = 1024;
	char small_buf[SMALL_BUF_SIZE];
	int small_buf_pos = 0;

	str.clear();
	str.push_back('>');

	while (!in->Eof())
	{
		int c = in->Get();
		if (in->Eof())
			break;

		if (c != '\n' && c != '\r')
			small_buf[small_buf_pos++] = (char) c;
		else if (small_buf_pos > 0)
			break;

		if (small_buf_pos == SMALL_BUF_SIZE)
		{
			str.insert(str.end(), small_buf, small_buf + small_buf_pos);
			small_buf_pos = 0;
		}
	}

	str.insert(str.end(), small_buf, small_buf + small_buf_pos);

	return str.size() > 1;
}

// *******************************************************************************************
// Read complete sequence
bool CFastaFile::read_sequence(string &str)
{
	const int SMALL_BUF_SIZE = 1024;
	char small_buf[SMALL_BUF_SIZE];
	int small_buf_pos = 0;

	str.clear();
	
	while (!in->Eof())
	{
		int c = in->Get();
		if (in->Eof())
			break;
		if (c == '>')
		{
			break;
		}

		if (c != '\n' && c != '\r')
			small_buf[small_buf_pos++] = (char)c;

		if (small_buf_pos == SMALL_BUF_SIZE)
		{
			str.insert(str.end(), small_buf, small_buf + small_buf_pos);
			small_buf_pos = 0;
		}
	}

	str.insert(str.end(), small_buf, small_buf + small_buf_pos);

	return !str.empty();
}


// *******************************************************************************************
// CCompressedFastaFile
// *******************************************************************************************

// *******************************************************************************************
// Save compressed MSAC file
bool CCompressedFastaFile::Save(string file_name, vector<uint8_t> &v_compressed_data)
{
	FILE *out = fopen(file_name.c_str(), "wb");
	if (!out)
	{
		cerr << "Cannot open " << file_name << " file for writing\n";
		return false;
	}

	fwrite(v_compressed_data.data(), 1, v_compressed_data.size(), out);
	fclose(out);

	return true;
}

// *******************************************************************************************
// Load compressed MSAC file
bool CCompressedFastaFile::Load(string file_name, vector<uint8_t> &v_compressed_data)
{
	FILE *in = fopen(file_name.c_str(), "rb");
	if (!in)
	{
		cerr << "Cannot open " << file_name << " file for reading\n";
		return false;
	}

	my_fseek(in, 0, SEEK_END);
	size_t in_size = my_ftell(in);		
	my_fseek(in, 0, SEEK_SET);

	v_compressed_data.resize(in_size);
	fread(v_compressed_data.data(), 1, in_size, in);
	fclose(in);

	return true;
}

// EOF
