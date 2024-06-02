#pragma once
#include <stddef.h>
#include <stdint.h>

#include "wfm.h"

struct Capture *make_capture(int nch, char **wfm_paths,
                             double debounce);
int destroy_capture(struct Capture *cap);
static int calc_stats(struct Capture *cap, size_t f, size_t n,
                      double debounce);

struct Channel {
	double min;
	double max;
	double base;
	double top;
	double midpoint;
	double rising_frequency;
	double rising_period;
	double falling_frequency;
	double falling_period;

	uint32_t nrising;
	uint32_t nfalling;

	int16_t *raw_data;
	double *data;

	uint32_t *rising_edges;
	uint32_t *falling_edges;

	double *rising_times;
	double *falling_times;
};

struct Frame {
	struct Channel *ch;
};

struct Capture {
	uint32_t nch;
	uint32_t nframes;

	uint32_t record_bytes;
	/* in pts */
	uint32_t record_len;
	uint32_t data_len;
	uint32_t precharge_len;
	uint32_t postcharge_len;

	double t_scale;
	double t_offset;

	double *v_scale;
	double *v_offset;

	double *times;
	struct Frame *frames;
	struct Wfm **wfms;

	struct Channel *all_channels;
	double *all_data;
};
