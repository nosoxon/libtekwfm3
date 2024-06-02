#include <err.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "capture.h"
#include "wfm.h"

/* Given the paths to a set of waveforms from the same session,
 * intialize a Capture structure and return a pointer to it.
 *
 * Does not ensure waveforms are of same record / temporal length
 */
struct Capture *make_capture(int nch, char **wfm_paths, double debounce)
{
	struct Capture *cap = calloc(1, sizeof(struct Capture));
	if (!cap)
		goto error_cleanup;
	cap->nch = nch;

	cap->wfms = calloc(nch, sizeof(struct Wfm *));
	if (!cap->wfms)
		goto error_cleanup;

	for (size_t i = 0; i < nch; ++i) {
		cap->wfms[i] = map_wfm(wfm_paths[i]);
		if (!cap->wfms[i])
			goto error_cleanup;
	}

	struct Wfm *ref_wfm = cap->wfms[0];
	cap->nframes = ref_wfm->Static->NFastFrames + 1;

	cap->record_bytes = ref_wfm->Hdr->CurveSpec.CurveBufferEnd;
	cap->record_len = cap->record_bytes / 2;

	cap->precharge_len = ref_wfm->Hdr->CurveSpec.DataStart / 2;
	cap->postcharge_len = cap->record_len
		- ref_wfm->Hdr->CurveSpec.PostchargeStart / 2;

	cap->data_len = cap->record_len
		- cap->precharge_len - cap->postcharge_len;

	cap->t_scale = ref_wfm->Hdr->ImpDim1.Scale;
	cap->t_offset = ref_wfm->Hdr->ImpDim1.Offset;

	cap->times = calloc(cap->data_len, sizeof(double));
	if (!cap->times)
		goto error_cleanup;
	for (size_t t = 0; t < cap->data_len; ++t)
		cap->times[t] = (double) t * cap->t_scale + cap->t_offset;

	cap->v_scale = calloc(nch, sizeof(double));
	cap->v_offset = calloc(nch, sizeof(double));
	if (!cap->v_scale || !cap->v_offset)
		goto error_cleanup;

	for (size_t i = 0; i < nch; ++i) {
		cap->v_scale[i]  = cap->wfms[i]->Hdr->ExpDim1.Scale;
		cap->v_offset[i] = cap->wfms[i]->Hdr->ExpDim1.Offset;
	}

	cap->all_channels = calloc(nch * cap->nframes, sizeof(struct Channel));
	cap->all_data = calloc(nch * cap->nframes * cap->record_len,
	                    sizeof(double));
	if (!cap->all_channels || !cap->all_data)
		goto error_cleanup;

	cap->frames = calloc(cap->nframes, sizeof(struct Frame));
	if (!cap->frames)
		goto error_cleanup;

	for (size_t f = 0; f < cap->nframes; ++f) {
		cap->frames[f].ch = cap->all_channels + nch * f;
		for (size_t n = 0; n < nch; ++n)
			if (calc_stats(cap, f, n, debounce))
				goto error_cleanup;
	}

	return cap;

error_cleanup:
	warn("Failed to create capture");
	destroy_capture(cap);

	return NULL;
}

/* Destroy a given Capture, even if only partially intialized. Depends
 * on any array of pointers or structs being zero-initialized.
 */
int destroy_capture(struct Capture *cap)
{
	if (!cap)
		return 0;

	if (cap->wfms) {
		for (size_t i = 0; i < cap->nch; ++i)
			if (cap->wfms[i])
				unmap_wfm(cap->wfms[i]);
		free(cap->wfms);
	}

	free(cap->times);
	free(cap->v_scale);
	free(cap->v_offset);

	for (size_t i = 0; i < cap->nch * cap->nframes; ++i) {
		free(cap->all_channels[i].rising_edges);
		free(cap->all_channels[i].falling_edges);
		free(cap->all_channels[i].rising_times);
		free(cap->all_channels[i].falling_times);
	}

	free(cap->all_channels);
	free(cap->all_data);
	free(cap->frames);
	free(cap);
	return 0;
}

static int compare_int16(const void* a, const void* b)
{
    if(*(int16_t *) a == *(int16_t *) b)
	return 0;
    return *(int16_t *) a < *(int16_t *) b ? -1 : 1;
}

static int realloc_list(uint32_t **list, size_t cap)
{
	uint32_t *new_list = realloc(*list, cap * sizeof(uint32_t));
	if (cap && !new_list)
		return 1;

	*list = new_list;
	return 0;
}

/* Calculate the statistics for a specific channel of a given frame.
 * Currently lots of opportunities for optimization!
 *   cap: pointer to partially-intialized Capture
 *   f:        frame index
 *   n:        channel index
 *   debounce: poor man's hysteresis. seconds to skip after an edge is
 *             detected before looking for another edge
 */
static int calc_stats(struct Capture *cap, size_t f, size_t n, double debounce)
{
	struct Channel *ch = cap->frames[f].ch + n;
	int16_t *data = (int16_t *) cap->wfms[n]->CurveBuffer
	              + f * cap->record_len + cap->precharge_len;
	double v_scale = cap->v_scale[n];
	double v_offset = cap->v_offset[n];

	/* Step 1: calculate minimum and maximum. These values (after
	 * conversion) match the corresponding values determined by
	 * TekScope.
	 */
	int16_t min = INT16_MAX, max = INT16_MIN;
	for (size_t t = 0; t < cap->data_len; ++t) {
		if (data[t] < min)
			min = data[t];
		else if (data[t] > max)
			max = data[t];
	}

	/* Step 2: calculate preliminary midpoint. This will need some
	 * TODO because it doesn't account for asymmetrical voltage
	 * spikes, which could majorly impact the base or top
	 * calculations. Perhaps some sort of smoothing (EWMA?) is in
	 * order.
	 */
	int16_t midpoint = min + (max - min) / 2;

	/* Step 3: find base and top ("modes" below and above midpoint
	 * according to TekScope) of waveform data (downsampled by 3
	 * bits). This routine arrives at results similar to, but not
	 * identical to, Tektronix's algorithm. I suspect they may
	 * convert to real (floating point) units and bin those values
	 * to determine base and top.
	 */
	int16_t sorted_data[cap->data_len];
	memcpy(sorted_data, data, 2 * cap->data_len);
	qsort(sorted_data, cap->data_len, 2, compare_int16);

	/* TODO revisit these defaults */
	int16_t mode_base = INT16_MIN, mode_top = INT16_MAX;
	int16_t current_val = INT16_MIN;

	size_t t_mode = 0, count = 0, max_count = 0;
	while (current_val < midpoint && t_mode < cap->data_len) { /* base */
		sorted_data[t_mode] >>= 3;
		if (sorted_data[t_mode] == current_val &&
		    ++count > max_count) {
			max_count = count;
			mode_base = current_val << 3;
		} else
			count = 0;
		current_val = sorted_data[t_mode++];
	}

	count = 0, max_count = 0;
	while (t_mode < cap->data_len) { /* top */
		sorted_data[t_mode] >>= 3;
		if (sorted_data[t_mode] == current_val &&
		    ++count > max_count) {
			max_count = count;
			mode_top = current_val << 3;
		} else
			count = 0;
		current_val = sorted_data[t_mode++];
	}

	/* Step 4: determine final midpoint (between mode and base) */
	midpoint = mode_base + (mode_top - mode_base) / 2;

	/* Step 5: Detect rising and falling edges triggering on
	 * midpoint. Currently uses a time-based "debounce", but needs
	 * TODO: real hysteresis */
	size_t raw_debounce = (size_t) (debounce / cap->t_scale);
	size_t cap_rising = 2;
	size_t cap_falling = 2;
	if (realloc_list(&ch->rising_edges, cap_rising) ||
	    realloc_list(&ch->falling_edges, cap_falling))
		return 1;
	for (size_t t = 1; t < cap->data_len; ++t) {
		if (data[t] >= midpoint && data[t - 1] < midpoint) {
			ch->rising_edges[ch->nrising++] = t;
			if (ch->nrising == cap_rising) {
				cap_rising *= 2;
				if (realloc_list(&ch->rising_edges,
				                 cap_rising))
					return 1;
			}
		} else if (data[t] <= midpoint && data[t - 1] > midpoint) {
			ch->falling_edges[ch->nfalling++] = t;
			if (ch->nfalling == cap_falling) {
				cap_falling *= 2;
				if (realloc_list(&ch->falling_edges,
				                 cap_falling))
					return 1;
			}
		} else
			continue;
		t += raw_debounce;
	}

	if (realloc_list(&ch->rising_edges, ch->nrising) ||
	    realloc_list(&ch->falling_edges, ch->nfalling))
		return 1;

	ch->rising_times = calloc(ch->nrising, sizeof(double));
	ch->falling_times = calloc(ch->nfalling, sizeof(double));
	if ((ch->nrising && !ch->rising_times) ||
	    (ch->nfalling && !ch->falling_times))
		return 1;

	/* Convert raw rising and falling times to real units (seconds). */
	for (size_t i = 0; i < ch->nrising; ++i)
		ch->rising_times[i] = (double) ch->rising_edges[i]
		                    * cap->t_scale + cap->t_offset;
	for (size_t i = 0; i < ch->nfalling; ++i)
		ch->falling_times[i] = (double) ch->falling_edges[i]
		                     * cap->t_scale + cap->t_offset;

	/* Step 6: Detect frequency for rising and falling edges separately. */
	ch->rising_frequency = 0;
	if (ch->nrising > 1) {
		ch->rising_period = 0.0;
		for (size_t i = 1; i < ch->nrising; ++i)
			ch->rising_period += ch->rising_edges[i]
			                   - ch->rising_edges[i - 1];
		ch->rising_period /= (double) (ch->nrising - 1);
		ch->rising_period *= cap->t_scale;
		ch->rising_frequency = 1.0 / ch->rising_period;
	}

	ch->falling_frequency = 0;
	if (ch->nfalling > 1) {
		ch->falling_period = 0.0;
		for (size_t i = 1; i < ch->nfalling; ++i)
			ch->falling_period += ch->falling_edges[i]
			                    - ch->falling_edges[i - 1];
		ch->falling_period /= (double) (ch->nfalling - 1);
		ch->falling_period *= cap->t_scale;
		ch->falling_frequency = 1.0 / ch->falling_period;
	}

	/* Step 7: Convert raw sample values to real units (Volts). */
	ch->min = min * v_scale + v_offset;
	ch->max = max * v_scale + v_offset;
	ch->base = mode_base * v_scale + v_offset;
	ch->top = mode_top * v_scale + v_offset;
	ch->midpoint = midpoint * v_scale + v_offset;

	ch->raw_data = data;
	ch->data = cap->all_data + (cap->nch * f + n) * cap->record_len
	                      + cap->precharge_len;
	for (size_t t = -cap->precharge_len; t < cap->data_len
	                                       + cap->postcharge_len; ++t)
		ch->data[t] = (double) t * cap->t_scale + cap->t_offset;

	return 0;
}
