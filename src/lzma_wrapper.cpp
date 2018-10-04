// *******************************************************************************************
// This file is a part of CoMSA software distributed under GNU GPL 3 licence.
// The homepage of the CoMSA project is http://sun.aei.polsl.pl/REFRESH/CoMSA
//
// Author : Sebastian Deorowicz
// Version: 1.2
// Date   : 2018-10-04
// *******************************************************************************************

// This code is based on the example from XZ library

#include "lzma_wrapper.h"
#include <algorithm>
#include <iostream>

// *******************************************************************************************
// Do processing
void CLZMAWrapper::operator()()
{
	if (forward_mode)
		forward();
	else
		reverse();
}

// *******************************************************************************************
// Run LZMA compression
void CLZMAWrapper::forward()
{
	if (v_text->empty())
		return;

	lzma_stream strm = LZMA_STREAM_INIT;

	bool success = init_encoder(&strm, compression_mode);
	if (success)
		success = compress(&strm, *v_text, *v_text_compressed);

	lzma_end(&strm);
}

// *******************************************************************************************
// Run LZMA decompression
void CLZMAWrapper::reverse()
{
	lzma_stream strm = LZMA_STREAM_INIT;

	bool success;

	success = init_decoder(&strm);
	if(success)
		success = decompress(&strm, *v_text_compressed, *v_text);

	lzma_end(&strm);
}

// *******************************************************************************************
// Initialize LZMA coder
bool CLZMAWrapper::init_encoder(lzma_stream *strm, uint32_t preset)
{
	lzma_ret ret = lzma_easy_encoder(strm, preset, LZMA_CHECK_CRC64);

	if (ret == LZMA_OK)
		return true;

	cerr << "Some bug in LZMA stage\n";

	return false;
}

// *******************************************************************************************
// Do actual compression
bool CLZMAWrapper::compress(lzma_stream *strm, vector<uint8_t> &v_in, vector<uint8_t> &v_out)
{
	lzma_action action = LZMA_RUN;

	uint8_t inbuf[BUFSIZ];
	uint8_t outbuf[BUFSIZ];

	strm->next_in = NULL;
	strm->avail_in = 0;
	strm->next_out = outbuf;
	strm->avail_out = sizeof(outbuf);

	size_t in_size = v_in.size();
	size_t in_pos = 0;

	while (true) {
		if (strm->avail_in == 0 && in_pos < in_size) {
			strm->next_in = inbuf;
			size_t to_read = std::min((size_t) sizeof(inbuf), in_size - in_pos);
			copy_n(v_in.begin() + in_pos, to_read, inbuf);
			in_pos += to_read;
			strm->avail_in = to_read;

			if(in_pos == in_size)
				action = LZMA_FINISH;
		}

		lzma_ret ret = lzma_code(strm, action);

		if (strm->avail_out == 0 || ret == LZMA_STREAM_END) {
			size_t write_size = sizeof(outbuf) - strm->avail_out;

			for (size_t i = 0; i < write_size; ++i)
				v_out.push_back(outbuf[i]);

			// Reset next_out and avail_out.
			strm->next_out = outbuf;
			strm->avail_out = sizeof(outbuf);
		}

		if (ret != LZMA_OK) {
			if (ret == LZMA_STREAM_END)
				return true;
	
			cerr << "Some bug in LZMA compression\n";

			return false;
		}
	}
}

// *******************************************************************************************
// Initialize LZMA decoder
bool CLZMAWrapper::init_decoder(lzma_stream *strm)
{
	lzma_ret ret = lzma_stream_decoder(strm, UINT64_MAX, LZMA_CONCATENATED);

	if (ret == LZMA_OK)
		return true;

	cerr << "Some bug in LZMA compression\n";

	return false;
}

// *******************************************************************************************
// Do actual decompression
bool CLZMAWrapper::decompress(lzma_stream *strm, vector<uint8_t> &v_in, vector<uint8_t> &v_out)
{
	lzma_action action = LZMA_RUN;

	uint8_t inbuf[BUFSIZ];
	uint8_t outbuf[BUFSIZ];

	strm->next_in = NULL;
	strm->avail_in = 0;
	strm->next_out = outbuf;
	strm->avail_out = sizeof(outbuf);

	size_t in_pos = 0;
	size_t in_size = v_in.size();

	while (true) {
		if (strm->avail_in == 0 && in_pos < in_size) {
			strm->next_in = inbuf;

			size_t to_read = std::min((size_t) sizeof(inbuf), in_size - in_pos);
			copy_n(v_in.begin() + in_pos, to_read, inbuf);
			in_pos += to_read;
			strm->avail_in = to_read;
			
			if(in_pos == in_size)
				action = LZMA_FINISH;
		}

		lzma_ret ret = lzma_code(strm, action);

		if (strm->avail_out == 0 || ret == LZMA_STREAM_END) {
			size_t write_size = sizeof(outbuf) - strm->avail_out;

			for (size_t i = 0; i < write_size; ++i)
				v_out.push_back(outbuf[i]);

			strm->next_out = outbuf;
			strm->avail_out = sizeof(outbuf);
		}

		if (ret != LZMA_OK) {
			if (ret == LZMA_STREAM_END)
				return true;

			return false;
		}
	}

	return true;
}

// EOF
