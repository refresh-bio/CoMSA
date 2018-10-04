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

#include "transpose.h"
#include "queue.h"
#include "pbwt.h"
#include "mtf.h"
#include "wfc.h"
#include "rle.h"
#include "entropy.h"

#include "lzma_wrapper.h"

using namespace std;

const uint32_t LZMA_mode_FASTA = 9 | LZMA_PRESET_EXTREME;
const uint32_t LZMA_mode_Stockholm = 9;

// *******************************************************************************************
//
// *******************************************************************************************
class CMSACompress
{
	vector<uint8_t> v_text;
	vector<uint8_t> v_text_compressed;
	size_t v_text_pos;

	// Just for debug purposes
	stage_mode_t Transpose_fwd_mode, Transpose_rev_mode;
	stage_mode_t PBWT_fwd_mode, PBWT_rev_mode;
	stage_mode_t SS_fwd_mode, SS_rev_mode;
	stage_mode_t RLE0_fwd_mode, RLE0_rev_mode;

	size_t pre_entropy_sequences_size;
	bool fast_variant;

	bool compress(vector<uint8_t> &v_text, vector<string> &v_sequences, uint32_t LZMA_mode, vector<uint8_t> &v_compressed_data,
		size_t &comp_text_size, size_t &comp_seq_size);
	bool decompress(vector<uint8_t> &v_text, vector<string> &v_sequences, vector<uint8_t> &v_compressed_data);

	void append_text(vector<string> &vs);
	void append_text(vector<vector<uint8_t>> &vs);
	void append_text(vector<uint32_t> &vu);

	void load_text(vector<string> &vs);
	void load_text(vector<vector<uint8_t>> &vs);
	void load_text(vector<uint32_t> &vu);

	void store_uint(vector<uint8_t> &vu, size_t x);
	size_t load_uint(vector<uint8_t> &vu, size_t &vu_pos);

	void store_data_in_stream(ctx_length_t ctx_length, bool fast_variant, vector<uint8_t> &v_seq_compressed, vector<uint8_t> &v_text_compressed, size_t pre_entropy_sequences_size,
		uint32_t n_sequences, uint32_t n_columns, vector<uint8_t> &v_compressed_data);
	void load_data_from_stream(ctx_length_t &ctx_length, bool &fast_variant, vector<uint8_t> &v_seq_compressed, vector<uint8_t> &v_text_compressed, size_t &pre_entropy_sequences_size,
		uint32_t &n_sequences, uint32_t &n_columns, vector<uint8_t> &v_compressed_data);

public:
	CMSACompress();
	~CMSACompress();

#ifdef EXPERIMENTAL_MODE
	void SetCopyModes(bool Transpose_copy_mode, bool PBWT_copy_mode, bool SS_copy_mode, bool RLE0_copy_mode);
#endif

	bool Compress(vector<string> &v_names, vector<string> &v_sequences, vector<uint8_t> &compressed_data, 
		size_t &comp_text_size, size_t &comp_seq_size, bool _fast_variant);
	bool Compress(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences, vector<uint8_t> &v_compressed_data,
		size_t &comp_text_size, size_t &comp_seq_size, bool _fast_variant);

	bool Decompress(vector<uint8_t> &v_compressed_data, vector<string> &v_names, vector<string> &v_sequences);
	bool Decompress(vector<uint8_t> &v_compressed_data, vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences);
};

// EOF
