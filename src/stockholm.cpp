// *******************************************************************************************
// This file is a part of MSAC software distributed under GNU GPL 3 licence.
// The homepage of the MSAC project is http://sun.aei.polsl.pl/msac
//
// Author: Sebastian Deorowicz
// Version: 1.0
// Date   : 2017-12-27
// *******************************************************************************************

#include "stockholm.h"
#include <iostream>
#include "defs.h"

using namespace std;

// *******************************************************************************************
// CStockholmFile
// *******************************************************************************************


// *******************************************************************************************
// Open Sockholm file for reading
bool CStockholmFile::OpenForReading(string file_name)
{
	if (in)
	{
		cerr << "Currently some Stockholm file is already open\n";
		return false;
	}

	in = new CInFile();
	
	if(!in->Open(file_name))
	{
		cerr << "Cannot open " << file_name << " file for reading\n";
		return false;
	}

	return true;
}

// *******************************************************************************************
// Open Stockholm file for writing
bool CStockholmFile::OpenForWriting(string file_name)
{
	if (out)
	{
		cerr << "Currently some Stockholm file is already open\n";
		return false;
	}

	out = new COutFile();

	if (!out->Open(file_name))
	{
		cerr << "Cannot open " << file_name << " file for writing\n";
		return false;
	}

	return true;
}

// *******************************************************************************************
// Close Stockholm file
bool CStockholmFile::Close()
{
	if (!in && !out)
	{
		cerr << "Currently a Stockholm file is not open\n";
		return false;
	}

	if (in)
	{
		delete in;
		in = nullptr;
	}

	if (out)
	{
		delete out;
		out = nullptr;
	}

	return true;
}

// *******************************************************************************************
// Return end-of-flag 
bool CStockholmFile::Eof()
{
	return in && in->Eof();
}

// *******************************************************************************************
// Return position in Stockholm file
size_t CStockholmFile::GetPos()
{
	if (in)
		return in->GetPos();
	else
		return 0;
}

// *******************************************************************************************
// Return single family
bool CStockholmFile::GetSequences(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences)
{
	vector<uint8_t> vu;
	vector<uint8_t> eof_marker = { '/', '/' };				// End of family marker

	v_meta.clear();
	v_offsets.clear();
	v_names.clear();
	v_sequences.clear();

	int line_no = 0;
	int last_meta_line_no = 0;

	while (read_line(vu))
	{
		line_no++;

		if (vu == eof_marker || vu.empty())
			break;

		if (vu[0] == '#')
		{
			v_meta.push_back(vu);
			if (line_no - last_meta_line_no > 1 || !v_sequences.empty())
				v_offsets.push_back(line_no - last_meta_line_no - 1);
			last_meta_line_no = line_no;
		}
		else
		{
			// Split sequence line into sequence name and sequence symbols
			v_names.push_back("");
			v_sequences.push_back("");
			
			auto s = &(v_names.back());
			int line_mode = 0;		// 0 - initial letters/digits (seq. name), 1 - white space characters, 2 - sequence symbols

			for (auto c : vu)
			{
				if (line_mode == 0 && (c == ' ' || c == '\t'))
					line_mode = 1;
				else if (line_mode == 1 && (c != ' ' && c != '\t'))
				{
					line_mode = 2;
					s = &(v_sequences.back());
				}

				s->push_back((char)c);
			}
		}
	}

	return !v_sequences.empty() || !v_meta.empty();
}

// *******************************************************************************************
// Put single file 
bool CStockholmFile::PutSequences(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences)
{
	uint32_t no_leading_meta = (uint32_t) (v_meta.size() - v_offsets.size());
	uint32_t i_meta, i_seq, i_offset;

	const char *eof_marker = "//\n";

	for (i_meta = 0; i_meta < no_leading_meta; ++i_meta)
	{
		out->Write((char*) (v_meta[i_meta].data()), v_meta[i_meta].size());
		out->Put('\n');
	}

	uint32_t cur_offset = ~(0u);
	i_offset = 0;

	if (!v_offsets.empty())
		cur_offset = v_offsets[i_offset++];

	for (i_seq = 0; i_seq < v_sequences.size(); ++i_seq)
	{
		while (cur_offset == 0)
		{
			out->Write((char*)(v_meta[i_meta].data()), v_meta[i_meta].size());
			out->Put('\n');
			i_meta++;
			
			if (i_offset < v_offsets.size())
				cur_offset = v_offsets[i_offset++];
			else
				cur_offset = ~(0u);
		}

		out->Write((char*) (v_names[i_seq].data()), v_names[i_seq].size());
		out->Write((char*)(v_sequences[i_seq].data()), v_sequences[i_seq].size());
		out->Put('\n');
		--cur_offset;
	}

	while (cur_offset == 0)
	{
		out->Write((char*)(v_meta[i_meta].data()), v_meta[i_meta].size());
		out->Put('\n');
		i_meta++;

		if (i_offset < v_offsets.size())
			cur_offset = v_offsets[i_offset++];
		else
			cur_offset = ~(0u);
	}

	out->Write(eof_marker, 3);

	return true;
}

// *******************************************************************************************
// Read single line
bool CStockholmFile::read_line(vector<uint8_t> &vu)
{
	vu.clear();

	while (!in->Eof())
	{
		int c = in->Get();
		if (c != '\n' && c != '\r')
			vu.push_back((uint8_t)c);
		else if (!vu.empty() || in->Eof())
			break;
	}

	return !vu.empty();
}


// *******************************************************************************************
// CCompressedStockholmFile
// *******************************************************************************************

// *******************************************************************************************
// Open MSAC-compressed Stockholm file for reading
bool CCompressedStockholmFile::OpenForReading(string file_name)
{
	if (f)
	{
		cerr << "Currently some compressed Stockholm file is already open\n";
		return false;
	}

	f = fopen(file_name.c_str(), "rb");
	if (!f)
	{
		cerr << "Cannot open " << file_name << " file for reading\n";
		return false;
	}

	setvbuf(f, NULL, _IOFBF, 128 << 20);

	return true;
}

// *******************************************************************************************
// Open MSAC-compressed Stockholm file for writing
bool CCompressedStockholmFile::OpenForWriting(string file_name)
{
	if (f)
	{
		cerr << "Currently some Stockholm file is already open\n";
		return false;
	}

	f = fopen(file_name.c_str(), "wb");
	if (!f)
	{
		cerr << "Cannot open " << file_name << " file for writing\n";
		return false;
	}

	setvbuf(f, NULL, _IOFBF, 128 << 20);

	return true;
}

// *******************************************************************************************
// Close MSAC-compressed file
bool CCompressedStockholmFile::Close()
{
	if (!f)
	{
		cerr << "Currently a Stockholm file is not open\n";
		return false;
	}

	fclose(f);
	f = nullptr;

	return true;
}

// *******************************************************************************************
// Return end-of-file 
bool CCompressedStockholmFile::Eof()
{
	return feof(f);
}

// *******************************************************************************************
// Store compressed block
bool CCompressedStockholmFile::Store(vector<uint8_t> &v_compressed_data)
{
	if (!f)
		return false;

	size_t size = v_compressed_data.size();

	if (fwrite(&size, sizeof(size_t), 1, f) != 1)
		return false;

	return fwrite(v_compressed_data.data(), 1, size, f) == size;
}

// *******************************************************************************************
// Load compressed block
bool CCompressedStockholmFile::Load(vector<uint8_t> &v_compressed_data)
{
	if (!f)
		return false;

	size_t size;

	fread(&size, sizeof(size_t), 1, f);

	if (feof(f))
		return false;

	v_compressed_data.resize(size);
	return fread(v_compressed_data.data(), 1, size, f) == size;
}

// EOF