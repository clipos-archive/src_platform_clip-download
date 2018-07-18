// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2007-2018 ANSSI. All Rights Reserved.

/**
 * downloadmaster.c
 *
 * @brief downloadmaster starts a daemon listening to the socket /var/run/download.
 * @see usbclt
 *
 **/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "download.h"

int main(void)
{
	if (start_download_daemon()) {
		fprintf(stderr, "Error starting DOWNLOAD_DAEMON\n");
		return 1;
	}
	return 0;
}
