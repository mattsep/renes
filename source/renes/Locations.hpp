#pragma once

#include "renes/Common.hpp"

namespace renes::locations {

constexpr addr_t ppu_ctrl = 0x2000;
constexpr addr_t ppu_mask = 0x2001;
constexpr addr_t ppu_status = 0x2002;
constexpr addr_t ppu_scroll = 0x2005;
constexpr addr_t ppu_addr = 0x2006;
constexpr addr_t ppu_data = 0x2007;

constexpr addr_t oam_addr = 0x2003;
constexpr addr_t oam_data = 0x2004;
constexpr addr_t oam_dma = 0x4014;

constexpr addr_t sq1_vol = 0x4000;
constexpr addr_t sq1_sweep = 0x4001;
constexpr addr_t sq1_lo = 0x4002;
constexpr addr_t sq1_hi = 0x4003;

constexpr addr_t sq2_vol = 0x4004;
constexpr addr_t sq2_sweep = 0x4005;
constexpr addr_t sq2_lo = 0x4006;
constexpr addr_t sq2_hi = 0x4007;

constexpr addr_t tri_linear = 0x4008;
constexpr addr_t tri_lo = 0x400A;
constexpr addr_t tri_hi = 0x400B;

constexpr addr_t noise_vol = 0x400C;
constexpr addr_t noise_lo = 0x400E;
constexpr addr_t noise_hi = 0x400F;

constexpr addr_t dmc_freq = 0x4010;
constexpr addr_t dmc_raw = 0x4011;
constexpr addr_t dmc_start = 0x4012;
constexpr addr_t dmc_len = 0x4013;

constexpr addr_t sound_channel = 0x4015;

constexpr addr_t joystick1 = 0x4016;
constexpr addr_t joystick2 = 0x4017;

constexpr addr_t nmi_vector = 0xFFFA;
constexpr addr_t reset_vector = 0xFFFC;
constexpr addr_t irq_vector = 0xFFFE;

}  // namespace renes::locations