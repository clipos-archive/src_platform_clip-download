# SPDX-License-Identifier: LGPL-2.1-or-later
# Copyright © 2007-2018 ANSSI. All Rights Reserved.
sbin_PROGRAMS = downloadmaster start-stop-daemon
downloadmaster_SOURCES = downloadmaster.c download.c download.h
downloadmaster_LDFLAGS = -static 
downloadmaster_CPPFLAGS = -O -Wall -Wextra -Wcast-qual -Wstrict-prototypes -Werror -fno-common -DDOWNLOADSOCKET=\"/mounts/admin_root/var/run/download\"
start_stop_daemon_SOURCES = start-stop-daemon.c
start_stop_daemon_CPPFLAGS = -DHAVE_ERROR_H -DHAVE_TIOCNOTTY -DHAVE_SETSID
