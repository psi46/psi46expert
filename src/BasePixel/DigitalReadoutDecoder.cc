#include "DigitalReadoutDecoder.h"
#include <iostream>
using namespace std;

#define BITS_PER_WORD	4
#define TBM_HEADER_BITS 0 // FIXME
#define ROC_HEADER_BITS 12
#define COLUMN_ADDRESS_GROUPS 2
#define COLUMN_ADDRESS_GROUP_BITS 3
#define PIXEL_ADDRESS_GROUPS 3
#define PIXEL_ADDRESS_GROUP_BITS 3
#define PULSE_HEIGHT_BITS 8
#define PULSE_HEIGHT_FIELD 9
#define PULSE_HEIGHT_BITS_A 4
#define PULSE_HEIGHT_BITS_B 4

/* Extract a unsigned integer from a bit field which is stored in the array
   of 16 bit integers 'data'. It is possible to specify the offset within
   the bitfield where the integer starts as well as the length of the
   integer, both in bits. The special thing about this function is that the
   data is sparse, this means that not all of the 16 bits of each data
   integer are relevant, but only the BITS_PER_WORD least significant. The
   bit field is left aligned. */
int extract_integer(short data [], int nwords, int bit_offset, int bit_num)
{
	/* Check input values. */
	if (!data || nwords < 0)
		return -1;
	if (bit_offset / BITS_PER_WORD > nwords)
		return -1;
	if ((bit_offset + bit_num) / BITS_PER_WORD > nwords)
		return -1;

	int word_idx;
	int result = 0;
	int bits_copied = 0;

	/* Calculate the offset in 'data' where the integer starts. */
	word_idx = bit_offset / BITS_PER_WORD;

	/* Loop through all of the bits of the integer */
	while (bits_copied < bit_num) {
		/* Calculate the position of the integer within the relevant
		   part of the data word */
		int word_pos = BITS_PER_WORD - bit_offset % BITS_PER_WORD;

		/* Create a mask that will delete all bits that don't belong
		   to the integer. */
		int word_mask;
		if (word_pos < bit_num - bits_copied) {
			word_mask = ((1 << word_pos) - 1);
		} else {
			word_mask = ((1 << word_pos) - 1);
			word_mask &= ~((1 << (word_pos - (bit_num - bits_copied))) - 1);
		}

		/* Shift the found piece of the integer to the correct position
		   in the result. */
		int bit_shift = bit_num - word_pos - bits_copied;
		if (bit_shift < 0)
			result |= (data[word_idx] & word_mask) >> -bit_shift;
		else
			result |= (data[word_idx] & word_mask) << bit_shift;

		/* Increase the number of bits copied and move on to the next word. */
		bits_copied += word_pos;
		bit_offset += word_pos;
		word_idx += 1;
	}
	return result;
}

int find_tbm_header(short data [], int nwords, int bit_offset)
{
	return 0;
}

int decode_tbm_header(short data [], int nwords, int bit_offset, DecodedReadoutTBM * obj)
{
	return 0;
}

int find_roc_header(short data [], int nwords, int bit_offset)
{
	int retval;

	/* Loop through the bit field to find the 12 bit start sequence of the
	   ROC readout. */
	while (bit_offset < BITS_PER_WORD * nwords) {
		retval = extract_integer(data, nwords, bit_offset, ROC_HEADER_BITS);

		/* In 15 of 16 cases the header is 0x7f8, the other time it is 0x7fa. */
		if ((retval | 3) == 0x7fb)
			return bit_offset;
		else if (retval < 0)
			return DRO_ERROR_NO_ROC_HEADER;
		bit_offset++;
	}
	return DRO_ERROR_NO_ROC_HEADER;
}

int decode_roc_header(short data [], int nwords, int bit_offset, DecodedReadoutROC * obj)
{
	return bit_offset + 12;
}

/* Decode groups of 'group_bits' sized integers and interpret them as digits of
   a senary number. This is how the column and row addresses are encoded. The
   address of the row is bit-inverted. */
inline int decode_address(short data [], int nwords, int bit_offset, int groups, int group_bits, int * address, bool invert)
{
	int addr = 0;
	/* Iterate over the senary digits. */
	for (int i = 0; i < groups; i++) {
		/* Promote the last digits. */
		addr = addr * 6;

		/* Read the next digit. */
		int retval;
		retval = extract_integer(data, nwords, bit_offset, group_bits);
		if (retval < 0)
			return DRO_ERROR_NO_MORE_DATA;

		if (invert)
			retval ^= 0x7;

		/* Don't allow digits larger than 5. */
		if (retval > 5)
			return DRO_ERROR_INVALID_ADDRESS;

		/* Add the read digit to the result. */
		addr += retval;

		bit_offset += group_bits;
	}
	*address = addr;

	return bit_offset;
}

/* Decode the hit pattern (24 bits) into column, row, and pulse height */
int decode_hit(short data [], int nwords, int bit_offset, int * col, int * row, int * ph, int flags)
{
	int retval;
	int col_tmp = 0, row_tmp = 0, ph_tmp = 0;

	/* Read the column address (two senary digits encoded in 3 bits each.) */
	retval = decode_address(data, nwords, bit_offset, COLUMN_ADDRESS_GROUPS, COLUMN_ADDRESS_GROUP_BITS, &col_tmp, false);
	if (retval < 0)
		return retval;
	if (col_tmp > 25)
		return DRO_ERROR_INVALID_COLUMN;
	bit_offset = retval;

	/* Read the row address (three senary digits encoded in 3 bits each.) */
	bool invert = !!(flags & DRO_INVERT_ROW_ADDRESS);
	retval = decode_address(data, nwords, bit_offset, PIXEL_ADDRESS_GROUPS, PIXEL_ADDRESS_GROUP_BITS, &row_tmp, invert);
	if (retval < 0)
		return retval;
	if (row_tmp < 2 || row_tmp > 161)
		return DRO_ERROR_INVALID_ROW;
	bit_offset = retval;

	/* Read the pulse height which is an integer of 8 bits embedded in a field of
	   9 bits. One bit in the field is always 1. This is to make sure that the
	   ROC header is unique. */

	/* Read the first part of the pulse height integer. */
	retval = extract_integer(data, nwords, bit_offset, PULSE_HEIGHT_BITS_A);
	if (retval < 0)
		return DRO_ERROR_NO_MORE_DATA;
	ph_tmp = retval << PULSE_HEIGHT_BITS_B;
	bit_offset += PULSE_HEIGHT_BITS_A;
	bit_offset += 1;

	/* Read the second part of the pulse height integer. */
	retval = extract_integer(data, nwords, bit_offset, PULSE_HEIGHT_BITS_B);
	if (retval < 0)
		return DRO_ERROR_NO_MORE_DATA;
	ph_tmp |= retval;
	bit_offset += PULSE_HEIGHT_BITS_B;

	/* Decode the column and row address. */
	*col = 2 * col_tmp + (row_tmp & 1);
	*row = 80 - row_tmp / 2;
	*ph = ph_tmp;

	return bit_offset;
}

/* Decode a single sequence of digital readout, stored in 'data', an array of
   'nwords' short integers. It stores the decoded result into a data structure
   of type DecodedReadoutModule. The flags specify whether or not to expect a
   TBM header and trailer in the readout. */
int decode_digital_readout(DecodedReadoutModule * obj, short data [], int nwords, int nroc, int flags)
{
	int retval;
	int bit_offset = 0;

	/* Search for the first header. This is a TBM header if the flag is set. */
	if (flags & DRO_WITH_TBM) {
		retval = find_tbm_header(data, nwords, bit_offset);
		if (retval < 0)
			return retval;

		/* Decode the header. */
		retval = decode_tbm_header(data, nwords, bit_offset, 0);
		if (retval < 0)
			return retval;
		bit_offset = retval;
	} else {
		retval = find_roc_header(data, nwords, bit_offset);
		if (retval < 0)
			return retval;
		bit_offset = retval;
	}

	/* Iterate over the ROC readouts. These are sequences of a ROC header
	   followed by 6 words of data for each pixel hit. */
	for (int i = 0; i < nroc; i++) {
		retval = decode_roc_header(data, nwords, bit_offset, 0);
		if (retval < 0)
			return retval;
		bit_offset = retval;

		int nhits = 0;
		/* Iterate over the hits for this ROC. */
		while (bit_offset < BITS_PER_WORD * nwords) {
			int x, y, ph;
			retval = decode_hit(data, nwords, bit_offset, &y, &x, &ph, flags);
			if (retval < 0)
				break;

			/* Fill the data structure. */
			obj->roc[i].pixelHit[nhits].rocId = i;
			obj->roc[i].pixelHit[nhits].columnROC = y;
			obj->roc[i].pixelHit[nhits].rowROC = x;
			obj->roc[i].pixelHit[nhits].analogPulseHeight = ph;
			//obj->roc[i].pixelHit[nhits].columnModule;
			//obj->roc[i].pixelHit[nhits].rowModule;
			//obj->roc[i].pixelHit[nhits].ADCword rawADC[6];
			nhits++;

			bit_offset = retval;
		}
		obj->roc[i].numPixelHits = nhits;
	}
	return bit_offset;
}
