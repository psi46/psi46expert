#ifndef __DIGITAL_READOUT_DECODER_H__
#define __DIGITAL_READOUT_DECODER_H__

#include "DecodedReadout.h"

/* Digital readout flags */
#define DRO_WITH_TBM 1

/* Digital readout error modes */
#define DRO_ERROR_NO_TBM_HEADER -1
#define DRO_ERROR_INVALID_TBM_HEADER -2
#define DRO_ERROR_NO_ROC_HEADER -3
#define DRO_ERROR_INVALID_ROC_HEADER -4
#define DRO_ERROR_INVALID_ADDRESS -5
#define DRO_ERROR_INVALID_COLUMN -6
#define DRO_ERROR_INVALID_ROW -7
#define DRO_ERROR_NO_MORE_DATA -8

int decode_digital_readout(DecodedReadoutModule * obj, short data [], int nwords, int nroc, int flags);

#endif
