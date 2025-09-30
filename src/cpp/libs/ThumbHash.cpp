#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <wasm_simd128.h>

std::vector<uint8_t> rgbaToThumbHash(int w, int h, const uint8_t *rgba)
{
  const double PI = 3.141592653589793;
  const int size = w * h;

  // --- Average color ---
  v128_t avg_rgba_f = wasm_f32x4_splat(0.0f);
  for (int i = 0; i < size; ++i)
  {
    float alpha = rgba[i * 4 + 3] / 255.0f;
    float f = alpha / 255.0f;
    v128_t pixel = wasm_f32x4_make(
        f * rgba[i * 4 + 0],
        f * rgba[i * 4 + 1],
        f * rgba[i * 4 + 2],
        alpha);
    avg_rgba_f = wasm_f32x4_add(avg_rgba_f, pixel);
  }

  float avg_buf[4];
  wasm_v128_store(avg_buf, avg_rgba_f);
  double avg_a = avg_buf[3];
  double avg_r = 0, avg_g = 0, avg_b = 0;
  if (avg_a > 0)
  {
    avg_r = avg_buf[0] / avg_a;
    avg_g = avg_buf[1] / avg_a;
    avg_b = avg_buf[2] / avg_a;
  }

  bool hasAlpha = avg_a < size;
  int l_limit = hasAlpha ? 5 : 7;
  int lx = std::max(1, (int)std::round((double)l_limit * w / std::max(w, h)));
  int ly = std::max(1, (int)std::round((double)l_limit * h / std::max(w, h)));

  std::vector<float> l(size), p(size), q(size), a(size);

  // --- Convert to LPQA ---
  const v128_t avg_rgb_f = wasm_f32x4_make(avg_r, avg_g, avg_b, 0.0f);
  const v128_t inv_255 = wasm_f32x4_splat(1.0f / 255.0f);
  const v128_t one = wasm_f32x4_splat(1.0f);
  const v128_t half = wasm_f32x4_splat(0.5f);
  const v128_t third = wasm_f32x4_splat(1.0f / 3.0f);

  for (int i = 0; i < size; i += 4)
  {
    int count = std::min(4, size - i);
    v128_t r_u8 = wasm_v128_load8_splat(&rgba[(i) * 4 + 0]);
    v128_t g_u8 = wasm_v128_load8_splat(&rgba[(i) * 4 + 1]);
    v128_t b_u8 = wasm_v128_load8_splat(&rgba[(i) * 4 + 2]);
    v128_t a_u8 = wasm_v128_load8_splat(&rgba[(i) * 4 + 3]);

    v128_t r_u32 = wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(r_u8));
    v128_t g_u32 = wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(g_u8));
    v128_t b_u32 = wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(b_u8));
    v128_t a_u32 = wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(a_u8));

    v128_t r_f = wasm_f32x4_convert_i32x4(r_u32);
    v128_t g_f = wasm_f32x4_convert_i32x4(g_u32);
    v128_t b_f = wasm_f32x4_convert_i32x4(b_u32);
    v128_t alpha_f = wasm_f32x4_mul(wasm_f32x4_convert_i32x4(a_u32), inv_255);

    v128_t r_final = wasm_f32x4_add(wasm_f32x4_mul(wasm_f32x4_splat(avg_r), wasm_f32x4_sub(one, alpha_f)), wasm_f32x4_mul(wasm_f32x4_mul(alpha_f, inv_255), r_f));
    v128_t g_final = wasm_f32x4_add(wasm_f32x4_mul(wasm_f32x4_splat(avg_g), wasm_f32x4_sub(one, alpha_f)), wasm_f32x4_mul(wasm_f32x4_mul(alpha_f, inv_255), g_f));
    v128_t b_final = wasm_f32x4_add(wasm_f32x4_mul(wasm_f32x4_splat(avg_b), wasm_f32x4_sub(one, alpha_f)), wasm_f32x4_mul(wasm_f32x4_mul(alpha_f, inv_255), b_f));

    v128_t l_f = wasm_f32x4_mul(wasm_f32x4_add(r_final, wasm_f32x4_add(g_final, b_final)), third);
    v128_t p_f = wasm_f32x4_sub(wasm_f32x4_mul(wasm_f32x4_add(r_final, g_final), half), b_final);
    v128_t q_f = wasm_f32x4_sub(r_final, g_final);

    wasm_v128_store32_lane(&l[i], l_f, 0);
    wasm_v128_store32_lane(&p[i], p_f, 0);
    wasm_v128_store32_lane(&q[i], q_f, 0);
    wasm_v128_store32_lane(&a[i], alpha_f, 0);
    if (count > 1)
    {
      wasm_v128_store32_lane(&l[i + 1], l_f, 1);
      wasm_v128_store32_lane(&p[i + 1], p_f, 1);
      wasm_v128_store32_lane(&q[i + 1], q_f, 1);
      wasm_v128_store32_lane(&a[i + 1], alpha_f, 1);
    }
    if (count > 2)
    {
      wasm_v128_store32_lane(&l[i + 2], l_f, 2);
      wasm_v128_store32_lane(&p[i + 2], p_f, 2);
      wasm_v128_store32_lane(&q[i + 2], q_f, 2);
      wasm_v128_store32_lane(&a[i + 2], alpha_f, 2);
    }
    if (count > 3)
    {
      wasm_v128_store32_lane(&l[i + 3], l_f, 3);
      wasm_v128_store32_lane(&p[i + 3], p_f, 3);
      wasm_v128_store32_lane(&q[i + 3], q_f, 3);
      wasm_v128_store32_lane(&a[i + 3], alpha_f, 3);
    }
  }

  // --- DCT helper ---
  auto encodeChannel = [&](const std::vector<float> &channel, int nx, int ny)
  {
    std::vector<float> transformed(h * nx);
    std::vector<float> ac;
    double dc = 0, scale = 0;

    // DCT rows
    for (int y = 0; y < h; ++y)
    {
      for (int cx = 0; cx < nx; ++cx)
      {
        double sum = 0;
        for (int x = 0; x < w; ++x)
        {
          sum += channel[x + y * w] * std::cos(PI / w * cx * (x + 0.5));
        }
        transformed[cx + y * nx] = sum;
      }
    }

    // DCT columns
    for (int cy = 0; cy < ny; ++cy)
    {
      for (int cx = 0; cx < nx; ++cx)
      {
        if (cx * ny >= nx * (ny - cy))
          continue;
        double sum = 0;
        for (int y = 0; y < h; ++y)
        {
          sum += transformed[cx + y * nx] * std::cos(PI / h * cy * (y + 0.5));
        }
        sum /= (w * h);
        if (cx == 0 && cy == 0)
        {
          dc = sum;
        }
        else
        {
          ac.push_back(sum);
          scale = std::max(scale, std::abs(sum));
        }
      }
    }

    if (scale > 0)
    {
      for (auto &v : ac)
      {
        v = 0.5 + 0.5 * v / scale;
      }
    }
    return std::make_tuple(dc, ac, scale);
  };

  auto [l_dc, l_ac, l_scale] = encodeChannel(l, std::max(3, lx), std::max(3, ly));
  auto [p_dc, p_ac, p_scale] = encodeChannel(p, 3, 3);
  auto [q_dc, q_ac, q_scale] = encodeChannel(q, 3, 3);
  auto [a_dc, a_ac, a_scale] = hasAlpha ? encodeChannel(a, 5, 5) : std::make_tuple(0.0, std::vector<float>{}, 0.0);

  // --- Header ---
  bool isLandscape = w > h;
  int header24 =
      static_cast<unsigned int>(std::round(63 * l_dc)) |
      (static_cast<unsigned int>(std::round(31.5 + 31.5 * p_dc)) << 6) |
      (static_cast<unsigned int>(std::round(31.5 + 31.5 * q_dc)) << 12) |
      (static_cast<unsigned int>(std::round(31 * l_scale)) << 18) |
      (hasAlpha << 23);
  int header16 =
      (isLandscape ? ly : lx) |
      (static_cast<unsigned int>(std::round(63 * p_scale)) << 3) |
      (static_cast<unsigned int>(std::round(63 * q_scale)) << 9) |
      (isLandscape << 15);

  std::vector<uint8_t> hash = {
      (uint8_t)(header24 & 0xFF),
      (uint8_t)((header24 >> 8) & 0xFF),
      (uint8_t)((header24 >> 16) & 0xFF),
      (uint8_t)(header16 & 0xFF),
      (uint8_t)((header16 >> 8) & 0xFF)};

  if (hasAlpha)
  {
    hash.push_back((uint8_t)(static_cast<unsigned int>(std::round(15 * a_dc)) | (static_cast<unsigned int>((static_cast<unsigned int>(std::round(15 * a_scale))) << 4))));
  }

  // --- AC coefficients ---
  int ac_start = hasAlpha ? 6 : 5;
  int ac_index = 0;
  const std::vector<std::vector<float>> ac_channels = hasAlpha ? std::vector{l_ac, p_ac, q_ac, a_ac} : std::vector{l_ac, p_ac, q_ac};

  for (const auto &ac : ac_channels)
  {
    for (float f : ac)
    {
      int byteIndex = ac_start + (ac_index >> 1);
      int shift = (ac_index & 1) << 2;
      if (byteIndex >= hash.size())
        hash.push_back(0);
      hash[byteIndex] |= (uint8_t)(static_cast<unsigned int>(std::round(15 * f)) << shift);
      ++ac_index;
    }
  }

  return hash;
}
