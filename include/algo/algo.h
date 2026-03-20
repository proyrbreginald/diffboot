/**
 * @file algo.h
 * @author reginald.yang (proyrb@yeah.net)
 * @version 0.1
 * @date 2026-03-19
 * @copyright Copyright (c) 2026
 * @brief 提供软件算法。
 */

#ifndef _ALGO_H_
#define _ALGO_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief 计算crc16校验值。
 * @param data 指向待校验数据的指针。
 * @param len 数据长度。
 * @return uint16_t 计算出的crc校验值。
 */
uint16_t algo_crc16(const uint8_t *data, size_t len);

#endif