// *******************************************************************************************
// This file is a part of MSAC software distributed under GNU GPL 3 licence.
// The homepage of the MSAC project is http://sun.aei.polsl.pl/msac
//
// Author: Sebastian Deorowicz
// Version: 1.0
// Date   : 2017-12-27
// *******************************************************************************************

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <cstring>

#include "msa.h"
#include "fasta_file.h"
#include "stockholm.h"

using namespace std;

#define NORM(x,mi,ma)	((x) < (mi) ? (mi) : (x) > (ma) ? (ma) : (x))

enum class task_mode_t {FASTA_compress, FASTA_decompress, Stockholm_compress, Stockholm_decompress};

task_mode_t mode;
string in_name;
string out_name;
int wrap_width = 0;
bool fast_variant = false;

CMSACompress *msac;

CFastaFile fasta;

vector<string> v_sequences;
vector<string> v_names;
vector<uint8_t> v_compressed_data;

bool FASTA_compress();
bool FASTA_decompress();

bool Stockholm_compress();
bool Stockholm_decompress();

bool parse_params(int argc, char **argv);
void usage();

// *******************************************************************************************
// Show usage info
void usage()
{
	cout << "Usage: msac <mode> [options] <in_file> <out_file>\n";
	cout << "Parameters:\n";
	cout << "   mode       - Fc (Fasta compress), Fd (Fasta decompress), Sc (Stockholm compress), Sd (Stockholm decompress)\n";
	cout << "   in_file    - name of input file\n";
	cout << "   out_file   - name of output file\n";
	cout << "Options:\n";
	cout << "   -w <width> - wrap sequences in FASTA file to given length (only for Fd mode); default: 0 (no wrapping)\n";
	cout << "   -f         - turn on fast variant (MTF in place of WFC)\n";
}

// *******************************************************************************************
// Parse input params
bool parse_params(int argc, char **argv)
{
	if (argc < 4)
	{
		usage();

		return false;
	}

	if (strcmp(argv[1], "Fc") == 0)
		mode = task_mode_t::FASTA_compress;
	else if (strcmp(argv[1], "Fd") == 0)
		mode = task_mode_t::FASTA_decompress;
	else if (strcmp(argv[1], "Sc") == 0)
		mode = task_mode_t::Stockholm_compress;
	else if (strcmp(argv[1], "Sd") == 0)
		mode = task_mode_t::Stockholm_decompress;
	else
	{
		cerr << "Invalid mode: " << argv[1] << endl;
		return false;
	}

	int arg_no = 2;

	while (arg_no + 2 < argc)
	{
		if (strcmp(argv[arg_no], "-w") == 0)
		{
			wrap_width = NORM(atoi(argv[arg_no + 1]), 0, 100000000);
			arg_no += 2;
		}
		else if (strcmp(argv[arg_no], "-f") == 0)
		{
			fast_variant = true;
			arg_no++;
		}
	}

	if (arg_no + 2 > argc)
	{
		usage();

		return false;
	}

	in_name = string(argv[arg_no++]);
	out_name = string(argv[arg_no]);

	return true;
}

// *******************************************************************************************
// FASTA compression
bool FASTA_compress()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	if (!fasta.ReadFile(in_name))
	{
		cerr << "Cannot read file: " << in_name << endl;
		return false;
	}

	fasta.GetSequences(v_names, v_sequences);

	size_t comp_text_size;
	size_t comp_seq_size;

	msac->Compress(v_names, v_sequences, v_compressed_data, comp_text_size, comp_seq_size, fast_variant);

	if (!CCompressedFastaFile::Save(out_name, v_compressed_data))
		return false;

	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end_time - start_time;

	cout << "Sequences compressed to: " << comp_seq_size << " bytes               \n";
	cout << "Names compressed to    : " << comp_text_size << " bytes               \n";
	cout << "Total size             : " << comp_text_size + comp_seq_size << " bytes               \n";
	cout << "Compression time       : " << diff.count() << " s\n";
	
	return true;
}

// *******************************************************************************************
// FASTA decompression
bool FASTA_decompress()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	if (!CCompressedFastaFile::Load(in_name, v_compressed_data))
		return false;

	msac->Decompress(v_compressed_data, v_names, v_sequences);

	fasta.PutSequences(v_names, v_sequences, wrap_width);

	if (!fasta.SaveFile(out_name))
	{
		cerr << "Cannot sava data in " << out_name << " file\n";
		return false;
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end_time - start_time;

	cout << "Decompression time : " << diff.count() << " s\n";
	
	return true;
}

// *******************************************************************************************
// Stockholm compression
bool Stockholm_compress()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	CStockholmFile sf;
	CCompressedStockholmFile csf;

	if (!sf.OpenForReading(in_name))
		return false;

	if (!csf.OpenForWriting(out_name))
		return false;

	size_t total_comp_text_size = 0;
	size_t total_comp_seq_size = 0;
	uint32_t dataset_no = 0;

	while (!sf.Eof())
	{
		vector<vector<uint8_t>> v_meta;
		vector<uint32_t> v_offsets;
		vector<string> v_names;
		vector<string> v_sequences;
		vector<uint8_t> v_compressed_data;

		size_t comp_text_size;
		size_t comp_seq_size;

		if(!sf.GetSequences(v_meta, v_offsets, v_names, v_sequences))
			continue;

		if (!msac->Compress(v_meta, v_offsets, v_names, v_sequences, v_compressed_data, comp_text_size, comp_seq_size, fast_variant))
		{
			cerr << "Fatal error during compression\n";
			return false;
		}

		if (!csf.Store(v_compressed_data))
		{
			cerr << "Fatal error during saving compressed data\n";
			return false;
		}

		total_comp_text_size += comp_text_size;
		total_comp_seq_size += comp_seq_size;

		if(total_comp_text_size + total_comp_seq_size < 20000)
			cout << "Dataset no. " << dataset_no++
			<< "   (" << sf.GetPos() << " -> " << total_comp_text_size + total_comp_seq_size << ")  compression ratio: "
			<< (double)sf.GetPos() / (total_comp_text_size + total_comp_seq_size) << "        \r";
		else if (total_comp_text_size + total_comp_seq_size < 20000000)
			cout << "Dataset no. " << dataset_no++
			<< "   (" << sf.GetPos() / 1000 << "K -> " << (total_comp_text_size + total_comp_seq_size) / 1000 << "K)  compression ratio: "
			<< (double)sf.GetPos() / (total_comp_text_size + total_comp_seq_size) << "            \r";
		else
			cout << "Dataset no. " << dataset_no++
			<< "   (" << sf.GetPos() / 1000000 << "M -> " << (total_comp_text_size + total_comp_seq_size) / 1000000 << "M)  compression ratio: "
			<< (double)sf.GetPos() / (total_comp_text_size + total_comp_seq_size) << "        \r";
	}

	sf.Close();
	csf.Close();

	cout << endl;

	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end_time - start_time;

	cout << "Compression time : " << diff.count() << " s\n";

	return true;
}

// *******************************************************************************************
// Stockholm decompression
bool Stockholm_decompress()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	CStockholmFile sf;
	CCompressedStockholmFile csf;

	if (!csf.OpenForReading(in_name))
		return false;

	if (!sf.OpenForWriting(out_name))
		return false;

	uint32_t dataset_no = 0;

	while (!csf.Eof())
	{
		vector<vector<uint8_t>> v_meta;
		vector<uint32_t> v_offsets;
		vector<string> v_names;
		vector<string> v_sequences;
		vector<uint8_t> v_compressed_data;

		if (!csf.Load(v_compressed_data))
			break;

		if (!msac->Decompress(v_compressed_data, v_meta, v_offsets, v_names, v_sequences))
		{
			cerr << "Fatal error during decompression\n";
			return false;
		}

		if (!sf.PutSequences(v_meta, v_offsets, v_names, v_sequences))
		{
			cerr << "Fatal error during saving compressed data\n";
			return false;
		}

		cout << "Dataset no. " << dataset_no++ << "\r";
	}

	cout << endl;

	sf.Close();
	csf.Close();

	cout << endl;

	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end_time - start_time;

	cout << "Decompression time : " << diff.count() << " s\n";

	return true;
}

// *******************************************************************************************
// Main function
int main(int argc, char **argv)
{
	if (!parse_params(argc, argv))
		return 0;

	msac = new CMSACompress();

	string a;

	if (mode == task_mode_t::FASTA_compress)
		FASTA_compress();
	else if (mode == task_mode_t::FASTA_decompress)
		FASTA_decompress();
	else if (mode == task_mode_t::Stockholm_compress)
		Stockholm_compress();
	else if (mode == task_mode_t::Stockholm_decompress)
		Stockholm_decompress();

	delete msac;

	return 0;
}

// EOF
