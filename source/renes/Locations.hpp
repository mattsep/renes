#pragma once

#include "renes/Common.hpp"

namespace renes::locations {

constexpr u16 ppu_ctrl = 0x2000;
constexpr u16 ppu_mask = 0x2001;
constexpr u16 ppu_status = 0x2002;
constexpr u16 ppu_scroll = 0x2005;
constexpr u16 ppu_addr = 0x2006;
constexpr u16 ppu_data = 0x2007;

constexpr u16 oam_addr = 0x2003;
constexpr u16 oam_data = 0x2004;
constexpr u16 oam_dma = 0x4014;

constexpr u16 sq1_vol = 0x4000;
constexpr u16 sq1_sweep = 0x4001;
constexpr u16 sq1_lo = 0x4002;
constexpr u16 sq1_hi = 0x4003;

constexpr u16 sq2_vol = 0x4004;
constexpr u16 sq2_sweep = 0x4005;
constexpr u16 sq2_lo = 0x4006;
constexpr u16 sq2_hi = 0x4007;

constexpr u16 tri_linear = 0x4008;
constexpr u16 tri_lo = 0x400A;
constexpr u16 tri_hi = 0x400B;

constexpr u16 noise_vol = 0x400C;
constexpr u16 noise_lo = 0x400E;
constexpr u16 noise_hi = 0x400F;

constexpr u16 dmc_freq = 0x4010;
constexpr u16 dmc_raw = 0x4011;
constexpr u16 dmc_start = 0x4012;
constexpr u16 dmc_len = 0x4013;

constexpr u16 sound_channel = 0x4015;

constexpr u16 joystick1 = 0x4016;
constexpr u16 joystick2 = 0x4017;

constexpr u16 nmi_vector = 0xFFFA;
constexpr u16 reset_vector = 0xFFFC;
constexpr u16 irq_vector = 0xFFFE;

}  // namespace renes::locations