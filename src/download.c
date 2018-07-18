// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2007-2018 ANSSI. All Rights Reserved.

/**
 * download.c
 *
 * @brief download listen to the socket /var/run/download and executes clip_download on connection.
 * @see usbadmin
 *
 **/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>
#include <syslog.h>
#include <arpa/inet.h>

#include <clip.h>

#include "download.h"

#define DOWNLOAD_SCRIPT "/usr/bin/clip_download"
#define LOCK_SCRIPT "/usr/bin/clip_update_lock"
#define UNLOCK_SCRIPT "/usr/bin/clip_update_unlock"
#define CLIP_DOWNLOAD_COMMAND 'A'
#define CLIP_LOCK_COMMAND 'D'
#define CLIP_UNLOCK_COMMAND 'G'
#define CLIP_ARG "clip" 
#define RMH_DOWNLOAD_COMMAND 'B'
#define RMH_LOCK_COMMAND 'E'
#define RMH_UNLOCK_COMMAND 'H'
#define RMH_ARG "rm_h" 
#define RMB_DOWNLOAD_COMMAND 'C'
#define RMB_LOCK_COMMAND 'F'
#define RMB_UNLOCK_COMMAND 'I'
#define RMB_ARG "rm_b" 

#define NOLOCK_ARG	"-trylock"

#define ERROR(fmt, args...) \
	syslog(LOG_DAEMON|LOG_ERR, fmt, ##args)

#define INFO(fmt, args...) \
	syslog(LOG_DAEMON|LOG_INFO, fmt, ##args)

#define PERROR(msg) \
	syslog(LOG_DAEMON|LOG_ERR, msg ": %s", strerror(errno))

static int launch_script(char *script, char *jail, char *more)
{
	INFO("Lancement de %s pour la cage %s", script, jail);
	char *argv[] = { script, "-j", jail, NULL , NULL};
	char *envp[] = { NULL };

	if (more)
		argv[3] = more;
	
	return -execve(argv[0], argv, envp);
}

int start_download_daemon(void)
{
	if (clip_daemonize()) {
		PERROR("clip_fork");
		return 1;
	}

	openlog("DOWNLOAD", LOG_PID, LOG_DAEMON);
	
	int s, s_com, status;
	pid_t f, wret;
	socklen_t len;
	struct sockaddr_un sau;
	char command= 0;

	/* We will write to a socket that may be closed on client-side, and
	   we don't want to die. */
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		PERROR("signal");
		return 1;
	}

        INFO("Start listening to %s ...",DOWNLOADSOCKET);
	
	s = clip_sock_listen(DOWNLOADSOCKET, &sau, 0);

	if (s < 0) {
		return 1;
	}

	for (;;) {
		len = sizeof(struct sockaddr_un);
		s_com = accept(s, (struct sockaddr *)&sau, &len);
		if (s_com < 0) {
			PERROR("accept");
			close(s);
			return 1;
		}

		INFO("Connection accepted");

		/* Get the command */
		if ( read(s_com, &command, 1) != 1)
		{
			PERROR("read command");
			close(s);
			return 1;
		}

		INFO("Command %c",command);

		f = fork();
		if (f < 0) {
			PERROR("fork");
			close(s_com);
			continue;
		} else if (f > 0) {
			char c = 'N';
			/* Father */
			wret = waitpid(f, &status, 0);
			if (wret < 0) {
				PERROR("waitpid");
				if (write(s_com, "N", 1) != 1)
					PERROR("write N");
				close(s_com);
				continue;
			}
			switch (WEXITSTATUS(status))
			{
				case 0:
					c = 'Y';
					break;
				case 8:
					c = 'L';
					break;
				case 11:
					c = 's';
					break;
				case 12:
					c = 'r';
					break;
				case 21:
					c = 'u';
					break;
				case 22:
					c = 'd';
					break;
				case 23:
					c = 'm';
					break;
				default :
					c = 'N';
			}
			if (write(s_com, &c, 1) != 1)
				PERROR("write");
			close(s_com);
			continue;
		} else {
			/* Child */
			close(s);

			switch (command)
			{
				case CLIP_DOWNLOAD_COMMAND:
					exit(launch_script(DOWNLOAD_SCRIPT, CLIP_ARG, NOLOCK_ARG));
					break;
				case RMH_DOWNLOAD_COMMAND:
					exit(launch_script(DOWNLOAD_SCRIPT, RMH_ARG, NOLOCK_ARG));
					break;
				case RMB_DOWNLOAD_COMMAND:
					exit(launch_script(DOWNLOAD_SCRIPT, RMB_ARG, NOLOCK_ARG));
					break;
				case CLIP_LOCK_COMMAND:
					exit(launch_script(LOCK_SCRIPT, CLIP_ARG, NULL));
					break;
				case RMH_LOCK_COMMAND:
					exit(launch_script(LOCK_SCRIPT, RMH_ARG, NULL));
					break;
				case RMB_LOCK_COMMAND:
					exit(launch_script(LOCK_SCRIPT, RMB_ARG, NULL));
					break;
				case CLIP_UNLOCK_COMMAND:
					exit(launch_script(UNLOCK_SCRIPT, CLIP_ARG, NULL));
					break;
				case RMH_UNLOCK_COMMAND:
					exit(launch_script(UNLOCK_SCRIPT, RMH_ARG, NULL));
					break;
				case RMB_UNLOCK_COMMAND:
					exit(launch_script(UNLOCK_SCRIPT, RMB_ARG, NULL));
					break;
				default:
					exit(-1);
			}
		}
	}

	INFO("Stop listening...");

	return 0;
}
