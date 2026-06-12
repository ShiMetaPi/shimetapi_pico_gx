/* 
 * yaffs2utils: Utilities to make/extract a YAFFS2/YAFFS1 image.
 * Copyright (C) 2010-2011 Luen-Yung Lin <penguin.lin@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <sys/ioctl.h>

#include "progress_bar.h"

/*----------------------------------------------------------------------------*/

static unsigned yaffs2_progress_columns = 0;

/*----------------------------------------------------------------------------*/

static void
progress_winch_updater (int sig)
{
	int retval;
	struct winsize wsize;

	if (sig != SIGWINCH)
		return;

	retval = ioctl(1, TIOCGWINSZ, &wsize);
	yaffs2_progress_columns = retval < 0 ? 80 : wsize.ws_col;
}

/*----------------------------------------------------------------------------*/

void
progress_bar (unsigned current, unsigned max)
{
	int maxdigits = ceil(log10(max));
	int used = maxdigits * 2 + 10;
	int hashes = (current * (yaffs2_progress_columns - used)) / max;
	int spaces = yaffs2_progress_columns - used - hashes;
	int print_times = 0;

	if (yaffs2_progress_columns - used < 0)
		return;

	printf("\r[");

	print_times = 0;
	while (hashes-- > 0) {
		print_times++;

		putchar('=');

		if (print_times > 256)
			break;
	}

	print_times = 0;
	while (spaces-- > 0) {
		print_times++;

		putchar(' ');

		if (print_times > 256)
			break;
	}

	printf("] %*u/%*u", maxdigits, current, maxdigits, max);
	printf(" %3u%%", current * 100 / max);
	fflush(stdout);
}

int
progress_init (void)
{
	progress_winch_updater(SIGWINCH);
	signal(SIGWINCH, progress_winch_updater);

	return 0;
}
