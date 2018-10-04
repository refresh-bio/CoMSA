// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

#include "msa.h"
#include <iostream>

// *******************************************************************************************
//
CMSACompress::CMSACompress()
{
	Transpose_fwd_mode = stage_mode_t::forward;
	Transpose_rev_mode = stage_mode_t::reverse;

	PBWT_fwd_mode = stage_mode_t::forward;
	PBWT_rev_mode = stage_mode_t::reverse;

	SS_fwd_mode = stage_mode_t::forward;
	SS_rev_mode = stage_mode_t::reverse;

	RLE0_fwd_mode = stage_mode_t::forward;
	RLE0_rev_mode = stage_mode_t::reverse;
}

// *******************************************************************************************
//
CMSACompress::~CMSACompress()
{
}

#ifdef EXPERIMENTAL_MODE
// *******************************************************************************************
void CMSACompress::SetCopyModes(bool Transpose_copy_mode, bool PBWT_copy_mode, bool SS_copy_mode, bool RLE0_copy_mode)
{
	if (Transpose_copy_mode)
	{
		Transpose_fwd_mode = stage_mode_t::copy_forward;
		Transpose_rev_mode = stage_mode_t::copy_reverse;
	}

	if (PBWT_copy_mode)
	{
		PBWT_fwd_mode = stage_mode_t::copy_forward;
		PBWT_rev_mode = stage_mode_t::copy_reverse;
	}

	if (SS_copy_mode)
	{
		SS_fwd_mode = stage_mode_t::copy_forward;
		SS_rev_mode = stage_mode_t::copy_reverse;
	}

	if (RLE0_copy_mode)
	{
		RLE0_fwd_mode = stage_mode_t::copy_forward;
		RLE0_rev_mode = stage_mode_t::copy_reverse;
	}
}
#endif

// *******************************************************************************************
// Compression of Stockholm files. 
// Parameters:
//    * v_meta		 - metadata
//    * v_offsets    - offsets between metadata included in the sequence part of block
//    * v_names		 - ids of sequences
//    * v_sequences  - protein sequences
//    * fast_variant - turn on MTF (much faster) instead of WFC 
bool CMSACompress::Compress(vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences, vector<uint8_t> &v_compressed_data,
	size_t &comp_text_size, size_t &comp_seq_size, bool _fast_variant)
{
	v_text.clear();
	append_text(v_meta);
	append_text(v_names);
	append_text(v_offsets);

	fast_variant = _fast_variant;

	return compress(v_text, v_sequences, LZMA_mode_Stockholm, v_compressed_data, comp_text_size, comp_seq_size);
}

// *******************************************************************************************
// Compression of FASTA files. 
// Parameters:
//    * v_names		 - ids of sequences
//    * v_sequences  - protein sequences
bool CMSACompress::Compress(vector<string> &v_names, vector<string> &v_sequences, vector<uint8_t> &v_compressed_data,
	size_t &comp_text_size, size_t &comp_seq_size, bool _fast_variant)
{
	v_text.clear();
	append_text(v_names);

	fast_variant = _fast_variant;

	return compress(v_text, v_sequences, LZMA_mode_FASTA, v_compressed_data, comp_text_size, comp_seq_size);
}

// *******************************************************************************************
// Decompression of FASTA files
bool CMSACompress::Decompress(vector<uint8_t> &v_compressed_data, vector<string> &v_names, vector<string> &v_sequences)
{
	v_text.clear();

	decompress(v_text, v_sequences, v_compressed_data);
	
	v_text_pos = 0;

	load_text(v_names);

	return true;
}

// *******************************************************************************************
// Decompression of Stockholm files
bool CMSACompress::Decompress(vector<uint8_t> &v_compressed_data, vector<vector<uint8_t>> &v_meta, vector<uint32_t> &v_offsets, vector<string> &v_names, vector<string> &v_sequences)
{
	v_text.clear();
	decompress(v_text, v_sequences, v_compressed_data);

	v_text_pos = 0;
		
	load_text(v_meta);
	load_text(v_names);
	load_text(v_offsets);

	return true;
}

// *******************************************************************************************
// Append text to the metadata string 
void CMSACompress::append_text(vector<string> &vs)
{
	// Calculate total size of text
	uint64_t len = 0;

	for (auto &x : vs)
		len += x.size() + 1;

	store_uint(v_text, len);

	v_text.reserve(v_text.size() + len);

	for (auto &x : vs)
	{
		for (auto c : x)
			v_text.push_back((uint8_t)c);
		v_text.push_back('\n');
	}
}

// *******************************************************************************************
// Append text to the metadata string 
void CMSACompress::append_text(vector<vector<uint8_t>> &vs)
{
	// Calculate total size of text
	uint64_t len = 0;

	for (auto &x : vs)
		len += x.size() + 1;

	store_uint(v_text, len);

	v_text.reserve(v_text.size() + len);

	for (auto &x : vs)
	{
		for (auto c : x)
			v_text.push_back((uint8_t)c);
		v_text.push_back('\n');
	}
}

// *******************************************************************************************
// Append text to the metadata string 
void CMSACompress::append_text(vector<uint32_t> &vu)
{
	store_uint(v_text, vu.size());

	v_text.reserve(v_text.size() + vu.size() * 5);

	for (auto x : vu)
		store_uint(v_text, x);
}

// *******************************************************************************************
// Load text from metadata string
void CMSACompress::load_text(vector<string> &vs)
{
	size_t len;

	vs.clear();

	len = load_uint(v_text, v_text_pos);

	vs.reserve(vs.size() + len);

	vs.push_back("");

	for (size_t i = 0; i < len; ++i)
	{
		auto c = v_text[v_text_pos++];

		if (c == '\n')
			vs.push_back("");
		else
			vs.back().push_back(c);
	}

	if (vs.back().empty())
		vs.pop_back();
}

// *******************************************************************************************
// Load text from metadata string
void CMSACompress::load_text(vector<vector<uint8_t>> &vs)
{
	size_t len;

	vs.clear();

	len = load_uint(v_text, v_text_pos);

	vs.reserve(vs.size() + len);

	vs.push_back(vector<uint8_t>());

	for (size_t i = 0; i < len; ++i)
	{
		auto c = v_text[v_text_pos++];

		if (c == '\n')
			vs.push_back(vector<uint8_t>());
		else
			vs.back().push_back(c);
	}

	if (vs.back().empty())
		vs.pop_back();
} 

// *******************************************************************************************
// Load text from metadata string
void CMSACompress::load_text(vector<uint32_t> &vu)
{
	size_t len;

	vu.clear();

	len = load_uint(v_text, v_text_pos);

	vu.reserve(len);

	for (size_t i = 0; i < len; ++i)
		vu.push_back((uint32_t) load_uint(v_text, v_text_pos));
}

// *******************************************************************************************
// Actual compression 
bool CMSACompress::compress(vector<uint8_t> &v_text, vector<string> &v_sequences, uint32_t LZMA_mode, vector<uint8_t> &v_compressed_data,
	size_t &comp_text_size, size_t &comp_seq_size)
{
	vector<uint8_t> v_seq_compressed;

	// Classify file - to set context lengths
	size_t file_size = 0;
	ctx_length_t ctx_length;

	if(!v_sequences.empty())
		file_size = v_sequences.size() * v_sequences.front().size();

	if (file_size < 10000)
		ctx_length = ctx_length_t::tiny;		// 2, 1, 1
	else if (file_size < 200000)
		ctx_length = ctx_length_t::small;		// 3, 2, 1
	else if (file_size < 5000000)
		ctx_length = ctx_length_t::medium;		// 4, 2, 2
	else if (file_size < 20000000)
		ctx_length = ctx_length_t::large;		// 5, 2, 2
	else
		ctx_length = ctx_length_t::huge;		// 5, 3, 2

	// Queues
#ifdef _DEBUG
	int n_thr_ss = 1;
#else
	int n_thr_ss = fast_variant ? 2 : 4;	
#endif

	// Text data - sequence names and (optional) metadata
	v_text_compressed.clear();
	CLZMAWrapper *lzma = new CLZMAWrapper(&v_text, &v_text_compressed, true, LZMA_mode);
	thread *thr_lzma = new thread(std::ref(*lzma));

	if (!v_sequences.empty())
	{
		// Sequence data
		CRegisteringPriorityQueue<vector<string> *> *q_pre_transpose = new CRegisteringPriorityQueue<vector<string> *>(1);
		CRegisteringPriorityQueue<string> *q_post_transpose = new CRegisteringPriorityQueue<string>(1);
		CRegisteringPriorityQueue<string> *q_post_PBWT = new CRegisteringPriorityQueue<string>(1);
		CRegisteringPriorityQueue<string> *q_post_SS = new CRegisteringPriorityQueue<string>(n_thr_ss);
		CRegisteringPriorityQueue<string> *q_post_RLE = new CRegisteringPriorityQueue<string>(1);
		CVectorIOStream *v_post_entropy = new CVectorIOStream(v_seq_compressed);

		// Transpose
		CTranspose *transpose = new CTranspose(q_pre_transpose, q_post_transpose, 0, 0, Transpose_fwd_mode);
		thread *thr_transpose = new thread(std::ref(*transpose));

		// PBWT
		CPBWT *pbwt = new CPBWT(q_post_transpose, q_post_PBWT, PBWT_fwd_mode);
		thread *thr_pbwt = new thread(std::ref(*pbwt));

		vector<CSecondStage *> v_ss(n_thr_ss);
		vector<thread *> v_thr_ss(n_thr_ss);

		if (fast_variant)
		{
			// MTF
			for (int i = 0; i < n_thr_ss; ++i)
			{
				v_ss[i] = new CMTF(q_post_PBWT, q_post_SS, SS_fwd_mode);
				v_thr_ss[i] = new thread(std::ref(*(v_ss[i])));
			}
		}
		else
		{
			// WFC
			for (int i = 0; i < n_thr_ss; ++i)
			{
				v_ss[i] = new CWFC(q_post_PBWT, q_post_SS, SS_fwd_mode);
				v_thr_ss[i] = new thread(std::ref(*(v_ss[i])));
			}
		}

		// RLE-0
		CRLE *rle = new CRLE(q_post_SS, q_post_RLE, RLE0_fwd_mode);
		thread *thr_rle = new thread(std::ref(*rle));

		// Entropy
		CEntropy *entropy = new CEntropy(q_post_RLE, v_post_entropy, pre_entropy_sequences_size, 0, true, ctx_length);
		thread *thr_entropy = new thread(std::ref(*entropy));

		// Push input sequences into the first queue
		q_pre_transpose->Push(0, &v_sequences);

		q_pre_transpose->MarkCompleted();

		thr_transpose->join();
		thr_pbwt->join();
		for (auto &x : v_thr_ss)
			x->join();
		thr_rle->join();
		thr_entropy->join();

		delete transpose;
		delete thr_transpose;

		delete pbwt;
		delete thr_pbwt;

		for (int i = 0; i < n_thr_ss; ++i)
		{
			delete v_ss[i];
			delete v_thr_ss[i];
		}

		delete rle;
		delete thr_rle;

		delete entropy;
		delete thr_entropy;

		delete q_pre_transpose;
		delete q_post_transpose;
		delete q_post_PBWT;
		delete q_post_SS;
		delete q_post_RLE;
		delete v_post_entropy;
	}
	else
		pre_entropy_sequences_size = 0;

	thr_lzma->join();

	delete lzma;
	delete thr_lzma;

	store_data_in_stream(ctx_length, fast_variant, v_seq_compressed, v_text_compressed, pre_entropy_sequences_size, 
		(uint32_t) v_sequences.size(), pre_entropy_sequences_size ? (uint32_t) v_sequences.front().size(): 0, v_compressed_data);

	comp_text_size = v_text_compressed.size();
	comp_seq_size = v_seq_compressed.size();

	return true;
}

// *******************************************************************************************
// Store some extra values in the compressed stream
void CMSACompress::store_data_in_stream(ctx_length_t ctx_length, bool fast_variant, vector<uint8_t> &v_seq_compressed, vector<uint8_t> &v_text_compressed, size_t pre_entropy_sequences_size,
	uint32_t n_sequences, uint32_t n_columns, vector<uint8_t> &v_compressed_data)
{
	v_compressed_data.clear();

	v_compressed_data.reserve(1 + 4 * sizeof(size_t) + v_seq_compressed.size() + v_text_compressed.size());

	v_compressed_data.push_back((uint8_t)ctx_length + (fast_variant ? 64 : 0));
	store_uint(v_compressed_data, (size_t)n_sequences);
	store_uint(v_compressed_data, (size_t)n_columns);
	store_uint(v_compressed_data, v_text_compressed.size());
	store_uint(v_compressed_data, v_seq_compressed.size());
	store_uint(v_compressed_data, pre_entropy_sequences_size);

	v_compressed_data.insert(v_compressed_data.end(), v_text_compressed.begin(), v_text_compressed.end());
	v_compressed_data.insert(v_compressed_data.end(), v_seq_compressed.begin(), v_seq_compressed.end());
}

// *******************************************************************************************
// Load some extra values from the compressed stream
void CMSACompress::load_data_from_stream(ctx_length_t &ctx_length, bool &fast_variant, vector<uint8_t> &v_seq_compressed, vector<uint8_t> &v_text_compressed, size_t &pre_entropy_sequences_size, 
	uint32_t &n_sequences, uint32_t &n_columns, vector<uint8_t> &v_compressed_data)
{
	size_t vu_pos = 0;

	uint8_t t = v_compressed_data[vu_pos++];

	if (t & 64)
	{
		fast_variant = true;
		t -= 64;
	}
	else
		fast_variant = false;
	ctx_length = (ctx_length_t)t;

	n_sequences = (uint32_t) load_uint(v_compressed_data, vu_pos);
	n_columns = (uint32_t) load_uint(v_compressed_data, vu_pos);
	size_t v_text_compressed_size = load_uint(v_compressed_data, vu_pos);
	size_t v_seq_compressed_size = load_uint(v_compressed_data, vu_pos);
	pre_entropy_sequences_size = load_uint(v_compressed_data, vu_pos);

	v_text_compressed.resize(v_text_compressed_size);
	v_seq_compressed.resize(v_seq_compressed_size);

	copy_n(v_compressed_data.data() + vu_pos, v_text_compressed_size, v_text_compressed.data());
	vu_pos += v_text_compressed_size;
	copy_n(v_compressed_data.data() + vu_pos, v_seq_compressed_size, v_seq_compressed.data());
}

// *******************************************************************************************
// Store single unsigned integer in the compressed stream
void CMSACompress::store_uint(vector<uint8_t> &vu, size_t x)
{
	uint8_t n_bytes = 0;

	// Find no. of bytes necessary to store for x
	for (size_t t = x; t; ++n_bytes)
		t >>= 8;

	vu.reserve(vu.size() + n_bytes + 1);

	vu.push_back((uint8_t)n_bytes);

	for (uint32_t i = 0; i < n_bytes; ++i)
	{
		vu.push_back(x & 0xff);
		x >>= 8;
	}
}

// *******************************************************************************************
// Load single unsigned integer from the compressed stream
size_t CMSACompress::load_uint(vector<uint8_t> &vu, size_t &vu_pos)
{
	uint32_t shift = 0;
	size_t x = 0;

	uint32_t n_bytes = vu[vu_pos++];

	for (uint32_t i = 0; i < n_bytes; ++i)
	{
		x += ((size_t)vu[vu_pos++]) << shift;
		shift += 8;
	}

	return x;
}

// *******************************************************************************************
// Actual decompression
bool CMSACompress::decompress(vector<uint8_t> &v_text, vector<string> &v_sequences, vector<uint8_t> &v_compressed_data)
{
	vector<uint8_t> v_text_compressed;
	vector<uint8_t> v_seq_compressed;

	size_t pre_entropy_sequences_size;
	uint32_t n_sequences;
	uint32_t n_columns;
	ctx_length_t ctx_length;

	load_data_from_stream(ctx_length, fast_variant, v_seq_compressed, v_text_compressed, pre_entropy_sequences_size, n_sequences, n_columns, v_compressed_data);

#ifdef _DEBUG
	int n_thr_ss = 1;
#else
	int n_thr_ss = fast_variant ? 2 : 4;
#endif

	// Names and meta
	CLZMAWrapper *lzma = new CLZMAWrapper(&v_text, &v_text_compressed, false, 0);
	thread *thr_lzma = new thread(std::ref(*lzma));

	if (pre_entropy_sequences_size)
	{
		CVectorIOStream *v_pre_entropy = new CVectorIOStream(v_seq_compressed);
		CRegisteringPriorityQueue<string> *q_post_entropy = new CRegisteringPriorityQueue<string>(1);
		CRegisteringPriorityQueue<string> *q_post_RLE = new CRegisteringPriorityQueue<string>(1);
		CRegisteringPriorityQueue<string> *q_post_SS = new CRegisteringPriorityQueue<string>(n_thr_ss);
		CRegisteringPriorityQueue<string> *q_post_PBWT = new CRegisteringPriorityQueue<string>(1);
		CRegisteringPriorityQueue<vector<string> *> *q_post_transpose = new CRegisteringPriorityQueue<vector<string> *>(1);

		// Entropy
		CEntropy *entropy = new CEntropy(q_post_entropy, v_pre_entropy, pre_entropy_sequences_size, Transpose_fwd_mode == stage_mode_t::forward ? n_sequences : n_columns, false, ctx_length);
		thread *thr_entropy = new thread(std::ref(*entropy));

		// RLE-0
		CRLE *rle = new CRLE(q_post_entropy, q_post_RLE, RLE0_rev_mode);
		thread *thr_rle = new thread(std::ref(*rle));

		vector<CSecondStage *> v_ss(n_thr_ss);
		vector<thread *> v_thr_ss(n_thr_ss);

		if (fast_variant)
		{
			// MTF
			for (int i = 0; i < n_thr_ss; ++i)
			{
				v_ss[i] = new CMTF(q_post_RLE, q_post_SS, SS_rev_mode);
				v_thr_ss[i] = new thread(std::ref(*(v_ss[i])));
			}
		}
		else
		{
			// WFC
			for (int i = 0; i < n_thr_ss; ++i)
			{
				v_ss[i] = new CWFC(q_post_RLE, q_post_SS, SS_rev_mode);
				v_thr_ss[i] = new thread(std::ref(*(v_ss[i])));
			}
		}

		// PBWT
		CPBWT *pbwt = new CPBWT(q_post_SS, q_post_PBWT, PBWT_rev_mode);
		thread *thr_pbwt = new thread(std::ref(*pbwt));

		// Transpose
		CTranspose *transpose = new CTranspose(q_post_transpose, q_post_PBWT, n_sequences, n_columns, Transpose_rev_mode);
		thread *thr_transpose = new thread(std::ref(*transpose));

		thr_entropy->join();
		thr_rle->join();
		for (auto &x : v_thr_ss)
			x->join();

		thr_pbwt->join();
		thr_transpose->join();

		vector<string> *matrix = nullptr;
		uint64_t priority;
		q_post_transpose->Pop(priority, matrix);
		v_sequences.resize(matrix->size());

		for (size_t i = 0; i < matrix->size(); ++i)
			v_sequences[i] = move((*matrix)[i]);

		delete entropy;
		delete thr_entropy;

		delete rle;
		delete thr_rle;

		for (int i = 0; i < n_thr_ss; ++i)
		{
			delete v_ss[i];
			delete v_thr_ss[i];
		}

		delete pbwt;
		delete thr_pbwt;

		delete transpose;
		delete thr_transpose;

		delete v_pre_entropy;
		delete q_post_entropy;
		delete q_post_RLE;
		delete q_post_SS;
		delete q_post_PBWT;
		delete q_post_transpose;
	}
	else
		v_sequences.clear();

	thr_lzma->join();
	delete lzma;
	delete thr_lzma;

	return true;
}

// EOF
