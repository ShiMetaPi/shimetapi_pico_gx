#ifndef SPI_HAL_H
#define SPI_HAL_H

#include <stdint.h>

/*
 * spi_hal - SPI 屏传输层（硬件 SPI2 = /dev/spidev2.0，DC/RES 用 GPIO）。
 *
 * 引脚映射（查 7206V11A PIN_OUT 表3，用户指定）：
 *   SCK  = GPIO4_7  pad 0x100C0028  func4=SPI2_SCLK  -> spidev2.0
 *   MOSI = GPIO5_0  pad 0x100C002C  func4=SPI2_SDO   -> spidev2.0
 *   CS   = GPIO5_1  pad 0x100C0030  func0=GPIO5_1    -> GPIO(本驱动手动 framing)
 *   DC   = GPIO4_5  pad 0x100C0020  func5=GPIO4_5    -> GPIO(默认JTAG_TDO!)
 *   RES  = GPIO4_4  pad 0x100C001C  func5=GPIO4_4    -> GPIO(默认JTAG_TDI!)
 *
 * DC=0 写命令、DC=1 写数据；每笔事务 CS 手动拉低(选中)，结束拉高(释放)。
 */

/* 初始化：pad 复用(5脚) + 打开 /dev/spidev2.0 + DC/RES 作 GPIO 输出 + 复位屏。成功 0。 */
int  spi_hal_init(void);

/* 释放资源。 */
void spi_hal_deinit(void);

/* RES 脉冲复位。 */
void spi_hal_reset(void);

/* 写单条命令（DC=0）。 */
void spi_hal_write_cmd(uint8_t cmd);

/* 写一组命令（DC=0，一次 SPI 事务）。 */
void spi_hal_write_cmds(const uint8_t *cmds, int n);

/* 写一批显存数据（DC=1，一次 SPI 事务）。 */
void spi_hal_write_data(const uint8_t *data, int len);

#endif /* SPI_HAL_H */
