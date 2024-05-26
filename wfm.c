#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "wfm.h"

struct Wfm *map_wfm(char *path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		warn("Unable to open `%s'", path);
		return NULL;
	}

	struct stat s;
	if (fstat(fd, &s) < 0) {
		warn("Unable to stat `%s'", path);
		close(fd);
		return NULL;
	}

	uint8_t *buf = (uint8_t *) mmap(
		NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (buf == MAP_FAILED) {
		warn("Unable to map `%s'", path);
		close(fd);
		return NULL;
	}

	close(fd);

	struct Wfm *wfm = calloc(1, sizeof(struct Wfm));
	assert(wfm);
	wfm->FastFrames = calloc(1, sizeof(struct WfmFastFrames));
	assert(wfm->FastFrames);

	wfm->Static = (struct WfmStaticFileInfo *) buf;
	wfm->Hdr = (struct WfmHdr *) (buf
		+ sizeof(struct WfmStaticFileInfo));
	wfm->FastFrames->UpdateSpecs = (struct WfmUpdateSpec *) (buf
		+ sizeof(struct WfmStaticFileInfo)
		+ sizeof(struct WfmHdr));
	wfm->FastFrames->CurveSpecs = (struct WfmCurveSpec *) (buf
		+ sizeof(struct WfmStaticFileInfo)
		+ sizeof(struct WfmHdr)
		+ wfm->Static->NFastFrames * sizeof(struct WfmUpdateSpec));
	wfm->CurveBuffer = buf
		+ wfm->Static->CurveBufferOffset;
	wfm->Checksum = *(uint64_t *) (buf
		+ wfm->Static->CurveBufferOffset
		+ (wfm->Static->NFastFrames + 1)
		* wfm->Hdr->CurveSpec.PostchargeStop);
	wfm->Size = s.st_size;

	return wfm;
}

int unmap_wfm(struct Wfm *wfm)
{
	munmap(wfm, wfm->Size);
	return 0;
}
