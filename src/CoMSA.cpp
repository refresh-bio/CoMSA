// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
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

enum class task_mode_t {FASTA_compress, FASTA_decompress, 
	Stockholm_compress, Stockholm_decompress, Stockholm_extract, Stockholm_list};

task_mode_t mode;
vector<string> v_in_names;
string out_name;
int wrap_width = 0;
bool fast_variant = false;
string extract_ID;
string extract_AC;
bool extract_sequences_only = false;

#ifdef EXPERIMENTAL_MODE
bool Transpose_copy_mode = false;
bool PBWT_copy_mode = false;
bool SS_copy_mode = false;
bool RLE0_copy_mode = false;
#endif

CMSACompress *msac;

CFastaFile fasta;

vector<string> v_sequences;
vector<string> v_names;
vector<uint8_t> v_compressed_data;

bool FASTA_compress();
bool FASTA_decompress();

bool Stockholm_compress();
bool Stockholm_decompress();
bool Stockholm_extract();
bool Stockholm_list();

bool parse_params(int argc, char **argv);
void usage();

// *******************************************************************************************
// Show usage info
void usage()
{
	cout << "Usage: CoMSA <mode> [options] <in_file> <out_file>\n";
	cout << "       CoMSA <mode> [options] @<in_file_list> <out_file>\n";
	cout << "Parameters:\n";
	cout << "   mode         - working mode, possible values:\n";
	cout << "      Fc - compress FASTA file\n";
	cout << "      Fd - decompress FASTA file\n";
	cout << "      Sc - compress Stockholm file\n";
	cout << "      Sd - decompress Stockholm file\n";
	cout << "      Se - extract a single family from compressed Stockholm file\n";
	cout << "      Sl - list families in compressed Stockholm file\n";
	cout << "   in_file      - name of input file\n";
	cout << "   in_file_list - name of file containing Stockholm file names to compress (only for Sc mode)\n";
	cout << "   out_file     - name of output file\n";
	cout << "Options:\n";
	cout << "   -w <width>   - wrap sequences in FASTA file to given length (only for Fd mode); default: 0 (no wrapping)\n";
	cout << "   -f           - turn on fast variant (MTF in place of WFC)\n";
	cout << "   -eID <id>    - extract family of given id (only for 'Se' mode)\n";
	cout << "   -eAC <ac>    - extract family of given accession number (only for 'Se' mode)\n";
	cout << "   -es          - extract sequences only (without gaps)\n";
#ifdef EXPERIMENTAL_MODE
	cout << "   -Tcm         - turn on no-transposition mode\n";
	cout << "   -Pcm         - turn on PBWT copy mode\n";
	cout << "   -Scm         - turn on MTF/WFC copy mode\n";
	cout << "   -Rcm         - turn on RLE0 copy mode\n";
#endif
	cout << "Sample executions:\n";
	cout << "   CoMSA Fc PF00005.fasta PF00005.msac\n";
	cout << "   CoMSA Fd PF00005.msac PF00005.fasta\n";
	cout << "   CoMSA Sc pfam.stockholm pfam.smsac\n";
	cout << "   CoMSA Sc @files.txt pfam.smsac\n";
	cout << "   CoMSA Sl pfam.smsac\n";
	cout << "   CoMSA Se -eAC PF00005.26 pfam.stockholm pfam.smsac\n";
}

// *******************************************************************************************
// Parse input params
bool parse_params(int argc, char **argv)
{
	if (argc < 3)
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
	else if (strcmp(argv[1], "Se") == 0)
		mode = task_mode_t::Stockholm_extract;
	else if (strcmp(argv[1], "Sl") == 0)
		mode = task_mode_t::Stockholm_list;
	else
	{
		cerr << "Invalid mode: " << argv[1] << endl;
		return false;
	}

	int arg_no = 2;

	while (arg_no + (mode == task_mode_t::Stockholm_list ? 1 : 2) < argc)
	{
		if (strcmp(argv[arg_no], "-w") == 0 && arg_no + 2 < argc)
		{
			wrap_width = NORM(atoi(argv[arg_no + 1]), 0, 100000000);
			arg_no += 2;
		}
		else if (strcmp(argv[arg_no], "-f") == 0 && arg_no + 1 < argc)
		{
			fast_variant = true;
			arg_no++;
		}
		else if (strcmp(argv[arg_no], "-es") == 0 && arg_no + 1 < argc)
		{
			extract_sequences_only = true;
			arg_no++;
		}
		else if (strcmp(argv[arg_no], "-eID") == 0 && mode == task_mode_t::Stockholm_extract && arg_no + 2 < argc)
		{
			extract_ID = string(argv[arg_no + 1]);
			arg_no += 2;
		}
		else if (strcmp(argv[arg_no], "-eAC") == 0 && mode == task_mode_t::Stockholm_extract && arg_no + 2 < argc)
		{
			extract_AC = string(argv[arg_no + 1]);
			arg_no += 2;
		}
#ifdef EXPERIMENTAL_MODE
		else if (strcmp(argv[arg_no], "-Tcm") == 0)
		{
			Transpose_copy_mode = true;
			++arg_no;
		}
		else if (strcmp(argv[arg_no], "-Pcm") == 0)
		{
			PBWT_copy_mode = true;
			++arg_no;
		}
		else if (strcmp(argv[arg_no], "-Scm") == 0)
		{
			SS_copy_mode = true;
			++arg_no;
		}
		else if (strcmp(argv[arg_no], "-Rcm") == 0)
		{
			RLE0_copy_mode = true;
			++arg_no;
		}
#endif
		else
		{
			cout << "Invalid option: " << string(argv[arg_no]) << endl;
			return false;
		}
	}

	if (arg_no + 1 > argc)
	{
		usage();

		return false;
	}

	if (argv[arg_no][0] == '@' && mode == task_mode_t::Stockholm_compress)
	{
		FILE *f = fopen(argv[arg_no]+1, "rb");
		if (!f)
			return false;

		char s[1024];
		while (!feof(f))
		{
			if (fscanf(f, "%s", s) == 1)
			{
				v_in_names.push_back(string(s));
				cout << s << endl;
			}
		}

		fclose(f); 

		++arg_no;
	}
	else
		v_in_names.push_back(string(argv[arg_no++]));

	if(mode != task_mode_t::Stockholm_list && arg_no + 1 > argc)
	{
		usage();

		return false;
	}

	if (mode != task_mode_t::Stockholm_list)
		out_name = string(argv[arg_no]);

	return true;
}

// *******************************************************************************************
// FASTA compression
bool FASTA_compress()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	if (!fasta.ReadFile(v_in_names.front()))
	{
		cerr << "Cannot read file: " << v_in_names.front() << endl;
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

	if (!CCompressedFastaFile::Load(v_in_names.front(), v_compressed_data))
		return false;

	msac->Decompress(v_compressed_data, v_names, v_sequences);

	fasta.PutSequences(v_names, v_sequences, wrap_width, extract_sequences_only);

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

	vector<stockholm_family_desc_t> v_fam_desc;

	if (!csf.OpenForWriting(out_name))
		return false;

	size_t total_comp_text_size = 0;
	size_t total_comp_seq_size = 0;
	uint32_t dataset_no = 0;

	for (auto &sto_name : v_in_names)
	{
		if (!sf.OpenForReading(sto_name))
		{
			cout << "Cannot open: " << sto_name << endl;
			return false;
		}

		while (!sf.Eof())
		{
			vector<vector<uint8_t>> v_meta;
			vector<uint32_t> v_offsets;
			vector<string> v_names;
			vector<string> v_sequences;
			vector<uint8_t> v_compressed_data;

			size_t comp_text_size;
			size_t comp_seq_size;
			string ID, AC;

			size_t f_pos1 = sf.GetPos();

			if (!sf.GetSequences(v_meta, v_offsets, v_names, v_sequences, ID, AC))
				continue;

			size_t f_pos2 = sf.GetPos();

			if (!msac->Compress(v_meta, v_offsets, v_names, v_sequences, v_compressed_data, comp_text_size, comp_seq_size, fast_variant))
			{
				cerr << "Fatal error during compression\n";
				return false;
			}

			v_fam_desc.push_back(stockholm_family_desc_t(
				v_sequences.size(),
				v_sequences.empty() ? 0 : v_sequences.front().size(),
				f_pos2 - f_pos1,
				comp_text_size + comp_seq_size,
				csf.GetPos(),
				ID,
				AC
			));

			if (!csf.Store(v_compressed_data))
			{
				cerr << "Fatal error during saving compressed data\n";
				return false;
			}

			total_comp_text_size += comp_text_size;
			total_comp_seq_size += comp_seq_size;

			if (total_comp_text_size + total_comp_seq_size < 20000)
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
	}

	// Store family descriptions
	csf.StoreFamilyDescriptions(v_fam_desc);

	csf.Close();

	cout << endl;

	auto end_time = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> diff = end_time - start_time;

	cout << "Total size of metadata : " << total_comp_text_size << "B\n";
	cout << "Total size of alignment: " << total_comp_seq_size << "B\n";
	cout << "Compression time       : " << diff.count() << " s\n";

	return true;
}

// *******************************************************************************************
// Stockholm decompression
bool Stockholm_decompress()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	CStockholmFile sf;
	CCompressedStockholmFile csf;

	if (!csf.OpenForReading(v_in_names.front()))
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

		if (!sf.PutSequences(v_meta, v_offsets, v_names, v_sequences, wrap_width, extract_sequences_only))
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
bool Stockholm_extract()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	CStockholmFile sf;
	CCompressedStockholmFile csf;

	if (!csf.OpenForReading(v_in_names.front()))
		return false;

	vector<stockholm_family_desc_t> v_fam_desc;
	if (!csf.LoadFamilyDescriptions(v_fam_desc))
		return false;

	if (!sf.OpenForWriting(out_name))
		return false;

	uint32_t dataset_no = 0;

	for(auto &fd : v_fam_desc)
	{
		if (!extract_ID.empty() && fd.ID != extract_ID)
			continue;
		if (!extract_AC.empty() && fd.AC != extract_AC)
			continue;

		csf.SetPos(fd.compressed_data_ptr);

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

		if (!sf.PutSequences(v_meta, v_offsets, v_names, v_sequences, wrap_width, extract_sequences_only))
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

	cout << "Extraction time : " << diff.count() << " s\n";

	return true;
}

// *******************************************************************************************
bool Stockholm_list()
{
	auto start_time = std::chrono::high_resolution_clock::now();

	CCompressedStockholmFile csf;

	if (!csf.OpenForReading(v_in_names.front()))
		return false;

	vector<stockholm_family_desc_t> v_fam_desc;
	if (csf.LoadFamilyDescriptions(v_fam_desc))
	{
		cout << "ID\t AC\t no. sequences\t no. columns\t uncompressed size\t compressed size\n";
		for (auto &fd : v_fam_desc)
			cout << fd.ID << "\t " << fd.AC << "\t " << fd.n_sequences << "\t " << fd.n_columns << "\t " <<
				fd.raw_size << "\t " << fd.compressed_size << endl;
	}

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

#ifdef EXPERIMENTAL_MODE
	msac->SetCopyModes(Transpose_copy_mode, PBWT_copy_mode, SS_copy_mode, RLE0_copy_mode);
#endif

	if (mode == task_mode_t::FASTA_compress)
		FASTA_compress();
	else if (mode == task_mode_t::FASTA_decompress)
		FASTA_decompress();
	else if (mode == task_mode_t::Stockholm_compress)
		Stockholm_compress();
	else if (mode == task_mode_t::Stockholm_decompress)
		Stockholm_decompress();
	else if (mode == task_mode_t::Stockholm_extract)
		Stockholm_extract();
	else if (mode == task_mode_t::Stockholm_list)
		Stockholm_list();

	delete msac;

	return 0;
}

// EOF
