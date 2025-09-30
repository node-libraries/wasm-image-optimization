#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

std::vector<uint8_t> rgbaToThumbHash(int w, int h, const uint8_t *rgba)
{
  const double PI = 3.141592653589793;
  const int size = w * h;

  // --- Average color ---
  double avg_r = 0, avg_g = 0, avg_b = 0, avg_a = 0;
  for (int i = 0; i < size; ++i)
  {
    double alpha = rgba[i * 4 + 3] / 255.0;
    double f = alpha / 255.0;
    avg_r += f * rgba[i * 4 + 0];
    avg_g += f * rgba[i * 4 + 1];
    avg_b += f * rgba[i * 4 + 2];
    avg_a += alpha;
  }
  if (avg_a > 0)
  {
    avg_r /= avg_a;
    avg_g /= avg_a;
    avg_b /= avg_a;
  }

  bool hasAlpha = avg_a < size;
  int l_limit = hasAlpha ? 5 : 7;
  int lx = std::max(1, (int)std::round((double)l_limit * w / std::max(w, h)));
  int ly = std::max(1, (int)std::round((double)l_limit * h / std::max(w, h)));

  std::vector<float> l(size), p(size), q(size), a(size);

  // --- Convert to LPQA ---
  for (int i = 0; i < size; ++i)
  {
    double alpha = rgba[i * 4 + 3] / 255.0;
    double r = avg_r * (1 - alpha) + (alpha / 255.0) * rgba[i * 4 + 0];
    double g = avg_g * (1 - alpha) + (alpha / 255.0) * rgba[i * 4 + 1];
    double b = avg_b * (1 - alpha) + (alpha / 255.0) * rgba[i * 4 + 2];
    l[i] = (r + g + b) / 3.0f;
    p[i] = ((r + g) / 2.0f) - b;
    q[i] = r - g;
    a[i] = alpha;
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
