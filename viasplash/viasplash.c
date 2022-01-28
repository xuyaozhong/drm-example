#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "lodepng.h"

#include "font.h"
#include "font2img.h"
struct disp_dev {
	struct disp_dev *next;

	uint32_t width;
	uint32_t height;
	uint32_t stride;
	uint32_t size;
	uint32_t handle;
	uint8_t *map;

	drmModeModeInfo mode;
	uint32_t fb;
	uint32_t conn;
	uint32_t crtc;
	drmModeCrtc *saved_crtc;
};

static struct disp_dev *disp_list = NULL;
static uint8_t* bg = NULL;

static int find_encoder_crtc(int fd, drmModeRes *res, drmModeConnector *conn,
			     struct disp_dev *dev);
static int create_dumb_buffer(int fd, struct disp_dev *dev);
static int init_disp_by_connector(int fd, drmModeRes *res, drmModeConnector *conn,
			     struct disp_dev *dev);
static int open_drmfd(int *out, const char *node);
static int prepare_connectors_crtcs(int fd);
static void drawimage(void);
static void draw_progressbar(void);
static void cleanup_drmfd(int fd);

static int open_drmfd(int *out, const char *node)
{
	int fd, ret;
	uint64_t has_dumb;

	fd = open(node, O_RDWR | O_CLOEXEC);
	if (fd < 0) {
		ret = -errno;
		fprintf(stderr, "cannot open '%s': %m\n", node);
		return ret;
	}

	if (drmGetCap(fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 ||
	    !has_dumb) {
		fprintf(stderr, "drm device '%s' does not support dumb buffers\n",
			node);
		close(fd);
		return -EOPNOTSUPP;
	}

	*out = fd;
	return 0;
}

static int prepare_connectors_crtcs(int fd)
{
	drmModeRes *res;
	drmModeConnector *conn;
	unsigned int i;
	struct disp_dev *dev;
	int ret;

	/* retrieve resources */
	res = drmModeGetResources(fd);
	if (!res) {
		fprintf(stderr, "cannot retrieve DRM resources (%d): %m\n",
			errno);
		return -errno;
	}

	/* dlptrate all connectors */
	for (i = 0; i < res->count_connectors; ++i) {
		/* get information for each connector */
		conn = drmModeGetConnector(fd, res->connectors[i]);
		if (!conn) {
			fprintf(stderr, "cannot retrieve DRM connector %u:%u (%d): %m\n",
				i, res->connectors[i], errno);
			continue;
		}

		/* create a device structure */
		dev = malloc(sizeof(*dev));
		memset(dev, 0, sizeof(*dev));
		dev->conn = conn->connector_id;

		/* call helper function to prepare this connector */
		ret = init_disp_by_connector(fd, res, conn, dev);
		if (ret) {
			if (ret != -ENOENT) {
				errno = -ret;
				fprintf(stderr, "cannot setup device for connector %u:%u (%d): %m\n",
					i, res->connectors[i], errno);
			}
			free(dev);
			drmModeFreeConnector(conn);
			continue;
		}

		/* free connector data and link device into global list */
		drmModeFreeConnector(conn);
		dev->next = disp_list;
		disp_list = dev;
	}

	/* free resources again */
	drmModeFreeResources(res);
	return 0;
}

static int init_disp_by_connector(int fd, drmModeRes *res, drmModeConnector *conn,
			     struct disp_dev *dev)
{
	int ret;

	/* check if a monitor is connected */
	if (conn->connection != DRM_MODE_CONNECTED) {
		fprintf(stderr, "ignoring unused connector %u\n",
			conn->connector_id);
		return -ENOENT;
	}

	/* check if there is at least one valid mode */
	if (conn->count_modes == 0) {
		fprintf(stderr, "no valid mode for connector %u\n",
			conn->connector_id);
		return -EFAULT;
	}

	/* copy the mode information into our device structure */
	memcpy(&dev->mode, &conn->modes[0], sizeof(dev->mode));
	dev->width = conn->modes[0].hdisplay;
	dev->height = conn->modes[0].vdisplay;
	fprintf(stderr, "mode for connector %u is %ux%u\n",
		conn->connector_id, dev->width, dev->height);

	/* find a crtc for this connector */
	ret = find_encoder_crtc(fd, res, conn, dev);
	if (ret) {
		fprintf(stderr, "no valid crtc for connector %u\n",
			conn->connector_id);
		return ret;
	}

	/* create a framebuffer for this CRTC */
	ret = create_dumb_buffer(fd, dev);
	if (ret) {
		fprintf(stderr, "cannot create framebuffer for connector %u\n",
			conn->connector_id);
		return ret;
	}

	return 0;
}

static int find_encoder_crtc(int fd, drmModeRes *res, drmModeConnector *conn,
			     struct disp_dev *dev)
{
	drmModeEncoder *enc;
	unsigned int i, j;
	uint32_t crtc;
	struct disp_dev *dlptr;

	/* dlptrate all encoders of this connector */
	for (i = 0; i < conn->count_encoders; ++i) {
		enc = drmModeGetEncoder(fd, conn->encoders[i]);
		if (!enc) {
			fprintf(stderr, "cannot retrieve encoder %u:%u (%d): %m\n",
				i, conn->encoders[i], errno);
			continue;
		}

		/* dlptrate all global CRTCs */
		for (j = 0; j < res->count_crtcs; ++j) {
			/* check whether this CRTC works with the encoder */
			if (!(enc->possible_crtcs & (1 << j)))
				continue;

			/* check that no other device already uses this CRTC */
			crtc = res->crtcs[j];
			for (dlptr = disp_list; dlptr; dlptr = dlptr->next) {
				if (dlptr->crtc == crtc) {
					crtc = -1;
					break;
				}
			}

			/* we have found a CRTC, so save it and return */
			if (crtc >= 0) {
				drmModeFreeEncoder(enc);
				dev->crtc = crtc;
				return 0;
			}
		}

		drmModeFreeEncoder(enc);
	}

	fprintf(stderr, "cannot find suitable CRTC for connector %u\n",
		conn->connector_id);
	return -ENOENT;
}

static int create_dumb_buffer(int fd, struct disp_dev *dev)
{
	struct drm_mode_create_dumb creq;
	struct drm_mode_destroy_dumb dreq;
	struct drm_mode_map_dumb mreq;
	int ret;

	/* create dumb buffer */
	memset(&creq, 0, sizeof(creq));
	creq.width = dev->width;
	creq.height = dev->height;
	creq.bpp = 32;
	ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
	if (ret < 0) {
		fprintf(stderr, "cannot create dumb buffer (%d): %m\n",
			errno);
		return -errno;
	}
	dev->stride = creq.pitch;
	dev->size = creq.size;
	dev->handle = creq.handle;

	/* create framebuffer object for the dumb-buffer */
	ret = drmModeAddFB(fd, dev->width, dev->height, 24, 32, dev->stride,
			   dev->handle, &dev->fb);
	if (ret) {
		fprintf(stderr, "cannot create framebuffer (%d): %m\n",
			errno);
		ret = -errno;
		goto err_destroy;
	}

	/* prepare buffer for memory mapping */
	memset(&mreq, 0, sizeof(mreq));
	mreq.handle = dev->handle;
	ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
	if (ret) {
		fprintf(stderr, "cannot map dumb buffer (%d): %m\n",
			errno);
		ret = -errno;
		goto err_fb;
	}

	/* perform actual memory mapping */
	dev->map = mmap(0, dev->size, PROT_READ | PROT_WRITE, MAP_SHARED,
		        fd, mreq.offset);
	if (dev->map == MAP_FAILED) {
		fprintf(stderr, "cannot mmap dumb buffer (%d): %m\n",
			errno);
		ret = -errno;
		goto err_fb;
	}

	memset(dev->map, 0x80, dev->size);

	return 0;

err_fb:
	drmModeRmFB(fd, dev->fb);
err_destroy:
	memset(&dreq, 0, sizeof(dreq));
	dreq.handle = dev->handle;
	drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
	return ret;
}


char* decodepng(const char* filename) {
  unsigned error;
  unsigned char* image = 0;
  unsigned width, height;

  error = lodepng_decode32_file(&image, &width, &height, filename);
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
  printf("%s, width = %d, height = %d \n",filename, width ,height);

  return image;
}


int main(int argc, char **argv)
{
	int ret, fd;
	const char *card;
	struct disp_dev *dlptr;

	const char* filename = argc > 1 ? argv[1] : "bg.png";

	bg = decodepng(filename);

	/* check which DRM device to open */
	card = "/dev/dri/card0";

	/* open the DRM device */
	ret = open_drmfd(&fd, card);
	if (ret)
		goto out_return;

	/* prepare all connectors and CRTCs */
	ret = prepare_connectors_crtcs(fd);
	if (ret)
		goto out_close;

	/* perform actual modesetting on each found connector+CRTC */
	for (dlptr = disp_list; dlptr; dlptr = dlptr->next) {
		dlptr->saved_crtc = drmModeGetCrtc(fd, dlptr->crtc);
		ret = drmModeSetCrtc(fd, dlptr->crtc, dlptr->fb, 0, 0,
				     &dlptr->conn, 1, &dlptr->mode);
		if (ret)
			fprintf(stderr, "cannot set CRTC for connector %u (%d): %m\n",
				dlptr->conn, errno);
	}

	draw_progressbar();
//	drawimage();

	cleanup_drmfd(fd);

	ret = 0;

out_close:
	close(fd);
	free(bg);
out_return:
	if (ret) {
		errno = -ret;
		fprintf(stderr, "modeset failed with error %d: %m\n", errno);
	} else {
		fprintf(stderr, "exiting\n");
	}
	return ret;
}


static void draw_progressbar(void)
{
        uint8_t r, g, b;
	uint32_t progress;
        unsigned int i, j, k, off;
        struct disp_dev *dlptr;

        progress = 0;

	r = 0xcc;
	g = 0x44;
	b = 0x88;

        for (i = 0; i < 50000; ++i) {

		if(progress == 0)
		{	
        		for (dlptr = disp_list; dlptr; dlptr = dlptr->next) {
				printf("dlptr->width =%d \n",dlptr->width);
				printf("dlptr->height =%d \n",dlptr->height);
				printf("dlptr->stride =%d \n",dlptr->stride);
				memcpy(dlptr->map,bg, 1920*1080*4);

				/*render char */
				char c;
				int cx = 100; int cy =100;
				for(c = ' '; c < 127 ; c++ )
				{
					rendercharonscreen32(dlptr->map,1920,1080,c, cx,cy, 0xff);
					cx += 100;
					if(cx > 1800)
					{
						cx = 100;
						cy += 100;
					}
				}
				c++;
        		}
		}
	
                for (dlptr = disp_list; dlptr; dlptr = dlptr->next) {
                        for (j = 900; j < dlptr->height - 100; ++j) {
                                //for (k = 0; k < dlptr->width; ++k) {
                                for (k = 0; k < progress; ++k) {
                                        off = dlptr->stride * j + k * 4;
                                        *(uint32_t*)&dlptr->map[off] = (r << 16) | (g << 8) | b;
                                }
                        }
			progress++ ;
			progress = progress % dlptr->width;
                }

                //usleep(100000);
                usleep(10000);
        }
}



static void cleanup_drmfd(int fd)
{
	struct disp_dev *dlptr;
	struct drm_mode_destroy_dumb dreq;

	while (disp_list) {
		/* remove from global list */
		dlptr = disp_list;
		disp_list = dlptr->next;

		/* restore saved CRTC configuration */
		drmModeSetCrtc(fd,
			       dlptr->saved_crtc->crtc_id,
			       dlptr->saved_crtc->buffer_id,
			       dlptr->saved_crtc->x,
			       dlptr->saved_crtc->y,
			       &dlptr->conn,
			       1,
			       &dlptr->saved_crtc->mode);
		drmModeFreeCrtc(dlptr->saved_crtc);

		/* unmap buffer */
		munmap(dlptr->map, dlptr->size);

		/* delete framebuffer */
		drmModeRmFB(fd, dlptr->fb);

		/* delete dumb buffer */
		memset(&dreq, 0, sizeof(dreq));
		dreq.handle = dlptr->handle;
		drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);

		/* free allocated memory */
		free(dlptr);
	}
}
