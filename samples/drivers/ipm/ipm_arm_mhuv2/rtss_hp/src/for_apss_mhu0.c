/* Copyright (C) 2024 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/mhuv2_ipm.h>

#define ITERATIONS 10
const struct device *mhu0_r;
const struct device *mhu0_s;

static volatile bool msg_sent;
static volatile bool msg_received;

static void recv_cb(const struct device *mhuv2_ipmdev, uint32_t *user_data)
{
	printk("MSG received is 0x%x\n", *user_data);
	msg_received = true;
}

static void send_cb(const struct device *mhuv2_ipmdev, uint32_t *user_data)
{
	printk("data sent\n");
	msg_sent = true;
}

static void send(void)
{
	uint32_t set_mhu = 0xFEEDFEED;

	msg_received = false;
	msg_sent = false;
	mhuv2_ipm_send(mhu0_s, 0, &set_mhu);
	while (!msg_sent)
		;
	msg_sent = false;
	set_mhu = 0xC0DEC0DE;
	mhuv2_ipm_send(mhu0_s, 1, &set_mhu);
	while (!msg_sent)
		;
	msg_sent = false;
}

int main(void)
{
	uint32_t recv_data;
	int i = 0;

	msg_received = false;
	msg_sent = false;
	printk("RTSS-HP A32 MHU 0 example on %s\n", CONFIG_BOARD);
	mhu0_r = DEVICE_DT_GET(DT_ALIAS(apssmhu0r));
	mhu0_s = DEVICE_DT_GET(DT_ALIAS(apssmhu0s));
	if (!device_is_ready(mhu0_r) || !device_is_ready(mhu0_s)) {
		printk("MHU devices not ready\n");
		return -1;
	}
	mhuv2_ipm_rb(mhu0_r, recv_cb, &recv_data);
	mhuv2_ipm_rb(mhu0_s, send_cb, NULL);
	while (i < ITERATIONS) {
		while (!msg_received)
			;
		send();
		++i;
	}
	return 0;
}
