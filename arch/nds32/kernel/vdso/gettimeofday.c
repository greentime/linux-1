/*
 * Copyright 2015 Mentor Graphics Corporation.
 * Copyright (C) 2005-2017 Andes Technology Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/compiler.h>
#include <linux/hrtimer.h>
#include <linux/time.h>
#include <asm/io.h>
#include <asm/barrier.h>
#include <asm/bug.h>
#include <asm/page.h>
#include <asm/unistd.h>
#include <asm/vdso_datapage.h>
#include <asm/asm-offsets.h>

#define X(x) #x
#define Y(x) X(x)

extern struct vdso_data *__get_datapage(void);
extern struct vdso_data *__get_timerpage(void);

static notrace unsigned int __vdso_read_begin(const struct vdso_data *vdata)
{
	u32 seq;
repeat:
	seq = ACCESS_ONCE(vdata->seq_count);
	if (seq & 1) {
		cpu_relax();
		goto repeat;
	}
	return seq;
}

static notrace unsigned int vdso_read_begin(const struct vdso_data *vdata)
{
	unsigned int seq;

	seq = __vdso_read_begin(vdata);

	smp_rmb();		/* Pairs with smp_wmb in vdso_write_end */
	return seq;
}

static notrace int vdso_read_retry(const struct vdso_data *vdata, u32 start)
{
	smp_rmb();		/* Pairs with smp_wmb in vdso_write_begin */
	return vdata->seq_count != start;
}

static notrace long clock_gettime_fallback(clockid_t _clkid,
					   struct timespec *_ts)
{
	register struct timespec *ts asm("$r1") = _ts;
	register clockid_t clkid asm("$r0") = _clkid;
	register long ret asm("$r0");

	asm volatile ("	syscall " Y(__NR_clock_gettime) "\n":"=r"(ret)
		      :"r"(clkid), "r"(ts)
		      :"memory");

	return ret;
}

static notrace int do_realtime_coarse(struct timespec *ts,
				      struct vdso_data *vdata)
{
	u32 seq;

	do {
		seq = vdso_read_begin(vdata);

		ts->tv_sec = vdata->xtime_coarse_sec;
		ts->tv_nsec = vdata->xtime_coarse_nsec;

	} while (vdso_read_retry(vdata, seq));
	return 0;
}

static notrace int do_monotonic_coarse(struct timespec *ts,
				       struct vdso_data *vdata)
{
	struct timespec tomono;
	u32 seq;

	do {
		seq = vdso_read_begin(vdata);

		ts->tv_sec = vdata->xtime_coarse_sec;
		ts->tv_nsec = vdata->xtime_coarse_nsec;

		tomono.tv_sec = vdata->wtm_clock_sec;
		tomono.tv_nsec = vdata->wtm_clock_nsec;

	} while (vdso_read_retry(vdata, seq));

	ts->tv_sec += tomono.tv_sec;
	timespec_add_ns(ts, tomono.tv_nsec);
	return 0;
}

static inline u64 vgetsns(struct vdso_data *vdso)
{
	u32 cycle_now;
	u32 cycle_delta;
	u32 *timer_cycle_base;

	timer_cycle_base =
	    (u32 *) ((char *)__get_timerpage() + vdso->cycle_count_offset);
	cycle_now = readl_relaxed(timer_cycle_base);
	if (true == vdso->cycle_count_down)
		cycle_now = ~(*timer_cycle_base);
	cycle_delta = cycle_now - (u32) vdso->cs_cycle_last;
	return ((u64) cycle_delta & vdso->cs_mask) * vdso->cs_mult;
}

static notrace int do_realtime(struct timespec *ts, struct vdso_data *vdata)
{
	unsigned count;
	u64 ns;
	do {
		count = vdso_read_begin(vdata);
		ts->tv_sec = vdata->xtime_clock_sec;
		ns = vdata->xtime_clock_nsec;
		ns += vgetsns(vdata);
		ns >>= vdata->cs_shift;
	} while (vdso_read_retry(vdata, count));

	ts->tv_sec += __iter_div_u64_rem(ns, NSEC_PER_SEC, &ns);
	ts->tv_nsec = ns;

	return 0;
}

static notrace int do_monotonic(struct timespec *ts, struct vdso_data *vdata)
{
	struct timespec tomono;
	u64 nsecs;
	u32 seq;

	do {
		seq = vdso_read_begin(vdata);

		ts->tv_sec = vdata->xtime_clock_sec;
		nsecs = vdata->xtime_clock_nsec;
		nsecs += vgetsns(vdata);
		nsecs >>= vdata->cs_shift;

		tomono.tv_sec = vdata->wtm_clock_sec;
		tomono.tv_nsec = vdata->wtm_clock_nsec;

	} while (vdso_read_retry(vdata, seq));

	ts->tv_sec += tomono.tv_sec;
	ts->tv_nsec = 0;
	timespec_add_ns(ts, nsecs + tomono.tv_nsec);
	return 0;
}

notrace int __vdso_clock_gettime(clockid_t clkid, struct timespec *ts)
{
	struct vdso_data *vdata;
	int ret = -1;

	vdata = __get_datapage();

	switch (clkid) {
	case CLOCK_REALTIME_COARSE:
		ret = do_realtime_coarse(ts, vdata);
		break;
	case CLOCK_MONOTONIC_COARSE:
		ret = do_monotonic_coarse(ts, vdata);
		break;
	case CLOCK_REALTIME:
		ret = do_realtime(ts, vdata);
		break;
	case CLOCK_MONOTONIC:
		ret = do_monotonic(ts, vdata);
		break;
	default:
		break;
	}

	if (ret)
		ret = clock_gettime_fallback(clkid, ts);

	return ret;
}

static notrace long clock_getres_fallback(clockid_t _clk_id,
					  struct timespec *_res)
{
	register clockid_t clk_id asm("$r0") = _clk_id;
	register struct timespec *res asm("$r1") = _res;
	register long ret asm("$r0");

	asm volatile ("	syscall " Y(__NR_clock_getres) " \n":"=r"(ret)
		      :"r"(clk_id), "r"(res)
		      :"memory");

	return ret;
}

notrace int __vdso_clock_getres(clockid_t clk_id, struct timespec *res)
{
	if (res == NULL)
		return -EFAULT;
	switch (clk_id) {
	case CLOCK_REALTIME:
	case CLOCK_MONOTONIC:
	case CLOCK_MONOTONIC_RAW:
		res->tv_sec = 0;
		res->tv_nsec = CLOCK_REALTIME_RES;
		break;
	case CLOCK_REALTIME_COARSE:
	case CLOCK_MONOTONIC_COARSE:
		res->tv_sec = 0;
		res->tv_nsec = CLOCK_COARSE_RES;
		break;
	default:
		clock_getres_fallback(clk_id, res);
		break;
	}
	return 0;
}

notrace int __vdso_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct timespec ts;
	struct vdso_data *vdata;
	int ret;

	vdata = __get_datapage();

	ret = do_realtime(&ts, vdata);

	if (tv) {
		tv->tv_sec = ts.tv_sec;
		tv->tv_usec = ts.tv_nsec / 1000;
	}
	if (tz) {
		tz->tz_minuteswest = vdata->tz_minuteswest;
		tz->tz_dsttime = vdata->tz_dsttime;
	}

	return ret;
}
