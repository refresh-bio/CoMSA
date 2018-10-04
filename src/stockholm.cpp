// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include "stockholm.h"
#include <algorithm>
#include <iostream>
#include <unordered_map>
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
//
string CStockholmFile::trim(string s)
{
	while (!s.empty())
	{
		if (s.front() == ' ' || s.front() == '\n' || s.front() == '\r')
			s.erase(s.begin());
		else
			break;
	}

	while (!s.empty())
	{
		if (s.back() == ' ' || s.back() == '\n' || s.back() == '\r')
			s.pop_back();
		else
			break;
	}

	return s;
}

// *******************************************************************************************
// Return single family
bool CStockholmFile::GetSequences(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, 
	vector<string> &v_sequences, string &ID, string &AC)
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

			if (equal(GF_ID.begin(), GF_ID.end(), vu.begin()))
				ID = trim(string(vu.begin() + GF_ID.size(), vu.end()));
			else if (equal(GF_AC.begin(), GF_AC.end(), vu.begin()))
				AC = trim(string(vu.begin() + GF_AC.size(), vu.end()));
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
bool CStockholmFile::PutSequences(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences, 
	int wrap_width, bool store_sequences_only)
{
	uint32_t no_leading_meta = (uint32_t) (v_meta.size() - v_offsets.size());
	uint32_t i_meta, i_seq, i_offset;

	const char *eof_marker = "//\n";

	string family_ID;
	string family_AC;
	unordered_map<string, string> m_seq_ac;

	if (store_sequences_only)
	{
		// Store just sequences with names - no metadata
		string s_ID("#=GF ID");
		string s_AC("#=GF AC");
		string s_GS("#=GS");
		string s_pAC(" AC ");
		string space(" ");

		for (auto &x : v_meta)
		{
			if (equal(s_ID.begin(), s_ID.end(), x.begin()))
				family_ID = string(find_end(x.begin(), x.end(), space.begin(), space.end())+1, x.end());
			else if (equal(s_AC.begin(), s_AC.end(), x.begin()))
				family_AC = string(find_end(x.begin(), x.end(), space.begin(), space.end())+1, x.end());
			else if (equal(s_GS.begin(), s_GS.end(), x.begin()))
			{
				string name(x.begin() + 5, find(x.begin() + 5, x.end(), ' '));
				auto p_AC = find_end(x.begin(), x.end(), s_pAC.begin(), s_pAC.end());
				if (p_AC != x.end())
				{
					auto q = find(p_AC + 4, x.end(), ' ');
					string protein_AC(p_AC + 4, q);

					m_seq_ac.insert(make_pair(name, protein_AC));
				}
			}
//			else if (regex_match(x.begin(), x.end(), mr, r_GS_AC))
//				m_seq_ac[string(mr[1].first, mr[1].second)] = string(mr[3].first, mr[3].second);
		}
		
		for (i_seq = 0; i_seq < v_sequences.size(); ++i_seq)
		{
			out->Put('>');
			for (auto c : v_names[i_seq])
				if (c == ' ')
					break;
				else
					out->Put(c);
			out->Put(' ');
			auto p = m_seq_ac.find(string(v_names[i_seq].begin(), find(v_names[i_seq].begin(), v_names[i_seq].end(), ' ')));
			if (p != m_seq_ac.end())
			{
				out->Write(p->second);
				out->Put(' ');
			}
			out->Write(family_AC);
			out->Put(';');
			out->Write(family_ID);
			out->Put(';');
			out->Put('\n');

			int seq_pos = 0;
			int next_wrap_pos = wrap_width ? wrap_width : -1;
			for (auto c : v_sequences[i_seq])
			{
				if (c >= 'A' && c <= 'Z')
				{
					if (seq_pos++ == next_wrap_pos)
					{
						out->Put('\n');
						next_wrap_pos += wrap_width;
					}
					out->Put(c);
				}
				else if (c >= 'a' && c <= 'z')
				{
					if (seq_pos++ == next_wrap_pos)
					{
						out->Put('\n');
						next_wrap_pos += wrap_width;
					}
					out->Put(c - 32);
				}
			}
			out->Put('\n');
		}
	}
	else
	{
		// Store in regular Stockholm file
		for (i_meta = 0; i_meta < no_leading_meta; ++i_meta)
		{
			out->Write((char*)(v_meta[i_meta].data()), v_meta[i_meta].size());
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

			out->Write((char*)(v_names[i_seq].data()), v_names[i_seq].size());
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
	}

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

	my_fseek(f, -(long long)(sizeof(size_t)), SEEK_END);
	logical_file_size = my_ftell(f);
	size_t footer_size;
	load_uint(footer_size, true);
	logical_file_size -= (long long) footer_size;

	my_fseek(f, - (long long) (footer_size + sizeof(size_t)), SEEK_END);

	// !!! Wczytanie opisu rodzin
	preload_family_descriptions(footer_size);

	my_fseek(f, 0, SEEK_SET);
	file_pos = 0;

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

	file_pos = 0;

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
	return file_pos >= logical_file_size;
}

// *******************************************************************************************
// Return current position of compressed Stockholm file
size_t CCompressedStockholmFile::GetPos()
{
	return file_pos;
}

// *******************************************************************************************
// Set the current position to the given value
bool CCompressedStockholmFile::SetPos(size_t pos)
{
	if (pos >= logical_file_size)
		return false;

	my_fseek(f, pos, SEEK_SET);
	file_pos = pos;

	return true;
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

	file_pos += sizeof(size_t) + size;

	return fwrite(v_compressed_data.data(), 1, size, f) == size;
}

// *******************************************************************************************
// Store single unsigned integer in the compressed stream
size_t CCompressedStockholmFile::store_uint(size_t x, bool fixed_size)
{
	uint8_t n_bytes = 0;

	if (fixed_size)
		n_bytes = sizeof(size_t);
	else
	{
		// Find no. of bytes necessary to store for x
		for (size_t t = x; t; ++n_bytes)
			t >>= 8;

		putc(n_bytes, f);
	}

	for (uint32_t i = 0; i < n_bytes; ++i)
	{
		putc(x & 0xff, f);
		x >>= 8;
	}

	return n_bytes + (fixed_size ? 0 : 1);
}

// *******************************************************************************************
// Load single unsigned integer from the compressed stream
size_t CCompressedStockholmFile::load_uint(size_t &x, bool fixed_size)
{
	uint32_t shift = 0;
	x = 0;

	uint32_t n_bytes;
	
	if (fixed_size)
		n_bytes = sizeof(size_t);
	else
		n_bytes = getc(f);

	for (uint32_t i = 0; i < n_bytes; ++i)
	{
		x += ((size_t) getc(f)) << shift;
		shift += 8;
	}

	return n_bytes + (fixed_size ? 0 : 1);
}

// *******************************************************************************************
// Store descriptions of families at the end of the file
bool CCompressedStockholmFile::StoreFamilyDescriptions(vector<stockholm_family_desc_t> &_v_fam_desc)
{
	size_t footer_size = 0;

	for (auto &fd : _v_fam_desc)
	{
		footer_size += store_uint(fd.n_sequences);
		footer_size += store_uint(fd.n_columns);
		footer_size += store_uint(fd.raw_size);
		footer_size += store_uint(fd.compressed_size);
		footer_size += store_uint(fd.compressed_data_ptr);
		fwrite(fd.ID.c_str(), 1, fd.ID.size() + 1, f);
		footer_size += fd.ID.size() + 1;
		fwrite(fd.AC.c_str(), 1, fd.AC.size() + 1, f);
		footer_size += fd.AC.size() + 1;
	}

	store_uint(footer_size, true);

	return true;
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

	file_pos += sizeof(size_t) + size;

	v_compressed_data.resize(size);
	return fread(v_compressed_data.data(), 1, size, f) == size;
}

// *******************************************************************************************
void CCompressedStockholmFile::preload_family_descriptions(size_t footer_size)
{
	v_fam_desc.clear();

	size_t i = 0;

	while (i < footer_size)
	{
		stockholm_family_desc_t fd;

		i += load_uint(fd.n_sequences);
		i += load_uint(fd.n_columns);
		i += load_uint(fd.raw_size);
		i += load_uint(fd.compressed_size);
		i += load_uint(fd.compressed_data_ptr);

		fd.ID.clear();
		for (int c = 0; (c = getc(f)) > 0; )
			fd.ID.push_back((char)c);
		i += fd.ID.size() + 1;

		fd.AC.clear();
		for (int c = 0; (c = getc(f)) > 0; )
			fd.AC.push_back((char)c);
		i += fd.AC.size() + 1;

		v_fam_desc.push_back(fd);
	}
}

// *******************************************************************************************
// Load descriptions of families from the file footer
bool CCompressedStockholmFile::LoadFamilyDescriptions(vector<stockholm_family_desc_t> &_v_fam_desc)
{
	_v_fam_desc = v_fam_desc;

	return true;
}


// EOF