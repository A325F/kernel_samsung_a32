/*
 *
 * (C) COPYRIGHT ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 *//* SPDX-License-Identifier: GPL-2.0 */
/*
 *
 * (C) COPYRIGHT 2015-2018, 2020 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you can access it online at
 * http://www.gnu.org/licenses/gpl-2.0.html.
 *
 */

/*
 * Vinstr, used to provide an ioctl for userspace access to periodic hardware
 * counters.
 */

#ifndef _KBASE_VINSTR_H_
#define _KBASE_VINSTR_H_

struct kbase_vinstr_context;
struct kbase_hwcnt_virtualizer;
struct kbase_ioctl_hwcnt_reader_setup;

//MTK COUNTER
typedef enum {
	VINSTR_GPU_FREQ,
	VINSTR_GPU_VOLT,
	VINSTR_GPU_LOADING,
	VINSTR_GPU_ACTIVE,
	VINSTR_EXEC_INSTR_FMA,
	VINSTR_EXEC_INSTR_CVT,
	VINSTR_EXEC_INSTR_SFU,
	VINSTR_EXEC_INSTR_MSG,
	VINSTR_EXEC_CORE_ACTIVE,
	VINSTR_FRAG_ACTIVE,
	VINSTR_TILER_ACTIVE,
	VINSTR_VARY_SLOT_32,
	VINSTR_VARY_SLOT_16,
	VINSTR_TEX_FILT_NUM_OPERATIONS,
	VINSTR_LS_MEM_READ_FULL,
	VINSTR_LS_MEM_WRITE_FULL,
	VINSTR_LS_MEM_READ_SHORT,
	VINSTR_LS_MEM_WRITE_SHORT,
	VINSTR_L2_EXT_WRITE_BEATS,
	VINSTR_L2_EXT_READ_BEATS,
	VINSTR_L2_EXT_RRESP_0_127,
	VINSTR_L2_EXT_RRESP_128_191,
	VINSTR_L2_EXT_RRESP_192_255,
	VINSTR_L2_EXT_RRESP_256_319,
	VINSTR_L2_EXT_RRESP_320_383,
	VINSTR_L2_ANY_LOOKUP,
	VINSTR_JS0_ACTIVE,
	VINSTR_JS1_ACTIVE,
	VINSTR_STALL0,
	VINSTR_STALL1,
	VINSTR_STALL2,
	VINSTR_STALL3,
	VINSTR_PERF_COUNTER_LAST
} mtk_vinstr_perf_counter;

typedef enum {
	pm_non,
	pm_ltr,
	pm_swpm,
	pm_met
} mtk_pm_tool_used;

/**
 * kbase_vinstr_init() - Initialise a vinstr context.
 * @hvirt:    Non-NULL pointer to the hardware counter virtualizer.
 * @out_vctx: Non-NULL pointer to where the pointer to the created vinstr
 *            context will be stored on success.
 *
 * On creation, the suspend count of the context will be 0.
 *
 * Return: 0 on success, else error code.
 */
int kbase_vinstr_init(
	struct kbase_hwcnt_virtualizer *hvirt,
	struct kbase_vinstr_context **out_vctx);

/**
 * kbase_vinstr_term() - Terminate a vinstr context.
 * @vctx: Pointer to the vinstr context to be terminated.
 */
void kbase_vinstr_term(struct kbase_vinstr_context *vctx);

/**
 * kbase_vinstr_suspend() - Increment the suspend count of the context.
 * @vctx: Non-NULL pointer to the vinstr context to be suspended.
 *
 * After this function call returns, it is guaranteed that all timers and
 * workers in vinstr will be cancelled, and will not be re-triggered until
 * after the context has been resumed. In effect, this means no new counter
 * dumps will occur for any existing or subsequently added periodic clients.
 */
void kbase_vinstr_suspend(struct kbase_vinstr_context *vctx);

/**
 * kbase_vinstr_resume() - Decrement the suspend count of the context.
 * @vctx: Non-NULL pointer to the vinstr context to be resumed.
 *
 * If a call to this function decrements the suspend count from 1 to 0, then
 * normal operation of vinstr will be resumed (i.e. counter dumps will once
 * again be automatically triggered for all periodic clients).
 *
 * It is only valid to call this function one time for each prior returned call
 * to kbase_vinstr_suspend.
 */
void kbase_vinstr_resume(struct kbase_vinstr_context *vctx);

/**
 * kbase_vinstr_hwcnt_reader_setup() - Set up a new hardware counter reader
 *                                     client.
 * @vinstr_ctx: Non-NULL pointer to the vinstr context.
 * @setup:      Non-NULL pointer to the hwcnt reader configuration.
 *
 * Return: file descriptor on success, else a (negative) error code.
 */
int kbase_vinstr_hwcnt_reader_setup(
	struct kbase_vinstr_context *vinstr_ctx,
	struct kbase_ioctl_hwcnt_reader_setup *setup);


int MTK_kbase_vinstr_hwcnt_reader_setup(
	struct kbase_vinstr_context *vctx,
	struct kbase_ioctl_hwcnt_reader_setup *setup);
void MTK_update_mtk_pm(int flag);
int MTK_get_mtk_pm(void);
void MTK_kbasep_vinstr_hwcnt_set_interval(unsigned int interval);
void MTK_kbasep_vinstr_hwcnt_release(void);
#if IS_ENABLED(CONFIG_MTK_SWPM)
void MTK_reset_urate(void);
void MTK_update_gpu_swpm(void);
#endif
void MTK_update_gpu_LTR(void);

#endif /* _KBASE_VINSTR_H_ */
