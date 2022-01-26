/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <libdrm/drm.h>

#include "framebuffer.h"

static void usage(void)
{
    printf ("\ndrm-framebuffer [OPTIONS...]\n\n"
            "Pipe data to a framebuffer\n\n"
            "  -d dri device /dev/dri/cardN\n"
            "  -l list connectors\n"
            "  -c connector to use (HDMI-A-1, LVDS-1)\n"
            "  -r get resolution dri device and connector needs to be set\n"
            "  -h show this message\n\n");
}


int list_resources(const char *dri_device)
{
    int fd;
    drmModeResPtr res;


    fd = open(dri_device, O_RDWR);
    if (fd < 0) {
        printf("Could not open dri device %s\n", dri_device);
        return -EINVAL;
    }

    res = drmModeGetResources(fd);
    if (!res) {
        printf("Could not get drm resources\n");
        return -EINVAL;
    }

    printf("connectors:");
    for (int i = 0; i < res->count_connectors; i++) {
        drmModeConnectorPtr connector = 0;
        drmModeEncoderPtr encoder = 0;

        printf("\nNumber: %d ", res->connectors[i]);
        connector = drmModeGetConnectorCurrent(fd, res->connectors[i]);
        if (!connector)
            continue;

        printf("Name: %s-%u ", connector_type_name(connector->connector_type), connector->connector_type_id);

        printf("Encoder: %d ", connector->encoder_id);

        encoder = drmModeGetEncoder(fd, connector->encoder_id);
        if (!encoder)
            continue;

        printf("Crtc: %d", encoder->crtc_id);

        drmModeFreeEncoder(encoder);
        drmModeFreeConnector(connector);
    }

    printf("\nFramebuffers: ");
    for (int i = 0; i < res->count_fbs; i++) {
        printf("%d ", res->fbs[i]);
    }

    printf("\nCRTCs: ");
    for (int i = 0; i < res->count_crtcs; i++) {
        printf("%d ", res->crtcs[i]);
    }

    printf("\nencoders: ");
    for (int i = 0; i < res->count_encoders; i++) {
        printf("%d ", res->encoders[i]);
    }
    printf("\n");

    drmModeFreeResources(res);

    return 0;

}

int get_resolution(const char *dri_device, const char *connector_name)
{
    int err = 0;
    int fd;
    drmModeResPtr res;

    fd = open(dri_device, O_RDWR);
    if (fd < 0) {
        printf("Could not open dri device %s\n", dri_device);
        return -EINVAL;
    }

    res = drmModeGetResources(fd);
    if (!res) {
        printf("Could not get drm resources\n");
        return -EINVAL;
    }

    /* Search the connector provided as argument */
    drmModeConnectorPtr connector = 0;
    for (int i = 0; i < res->count_connectors; i++) {
        char name[32];

        connector = drmModeGetConnectorCurrent(fd, res->connectors[i]);
        if (!connector)
            continue;

        snprintf(name, sizeof(name), "%s-%u", connector_type_name(connector->connector_type),
                connector->connector_type_id);

        if (strncmp(name, connector_name, sizeof(name)) == 0)
                break;

        drmModeFreeConnector(connector);
    }

    if (!connector) {
        printf("Could not find matching connector %s\n", connector_name);
        return -EINVAL;
    }

    /* Get the preferred resolution */
    drmModeModeInfoPtr resolution = 0;
    for (int i = 0; i < connector->count_modes; i++) {
            resolution = &connector->modes[i];
            if (resolution->type & DRM_MODE_TYPE_PREFERRED)
                    break;
    }

    if (!resolution) {
        printf("Could not find preferred resolution\n");
        err = -EINVAL;
        goto error;
    }

    printf("%ux%u\n", resolution->hdisplay, resolution->vdisplay);

error:
    drmModeFreeConnector(connector);
    drmModeFreeResources(res);
    close(fd);
    return err;
}


void fill_framebuffer_from_stdin(struct framebuffer *fb)
{
    size_t total_read = 0;

    while (total_read < fb->dumb_framebuffer.size)
        total_read += read(STDIN_FILENO, &fb->data[total_read], fb->dumb_framebuffer.size - total_read);

    /* Make sure we synchronize the display with the buffer. This also works if page flips are enabled */
    drmSetMaster(fb->fd);
    drmModeSetCrtc(fb->fd, fb->crtc->crtc_id, 0, 0, 0, NULL, 0, NULL);
    drmModeSetCrtc(fb->fd, fb->crtc->crtc_id, fb->buffer_id, 0, 0, &fb->connector->connector_id, 1, fb->resolution);
    drmDropMaster(fb->fd);

    sigset_t wait_set;
    sigemptyset(&wait_set);
    sigaddset(&wait_set, SIGTERM);
    sigaddset(&wait_set, SIGINT);

    int sig;
    sigprocmask(SIG_BLOCK, &wait_set, NULL );
    sigwait(&wait_set, &sig);
}

int main(int argc, char** argv)
{
    char *dri_device = 0;
    char *connector = 0;
    int c;
    int list = 0;
    int resolution = 0;

    opterr = 0;
    while ((c = getopt (argc, argv, "d:c:lrh")) != -1) {
        switch (c)
        {
        case 'd':
            dri_device = optarg;
            break;
        case 'c':
            connector = optarg;
            break;
        case 'l':
            list = 1;
            break;
        case 'r':
            resolution = 1;
            break;
        case 'h':
            usage();
            return 1;
        default:
            break;
        }
    }

    if (dri_device == 0) {
        printf("Please set a device\n");
        usage();
        return 3;
    }

    if (list) {
        return list_resources(dri_device);
    }

    if (connector == 0) {
        printf("Please set a connector\n");
        usage();
        return 4;
    }

    if (resolution) {
        return get_resolution(dri_device, connector);
    }

    struct framebuffer fb;
    if (get_framebuffer(dri_device, connector, &fb) == 0) {
        fill_framebuffer_from_stdin(&fb);
    }
    release_framebuffer(&fb);

    return 0;
}

