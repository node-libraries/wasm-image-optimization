#include "skia_utils.h"

#include <algorithm>

std::string base64_encode(const uint8_t* data, size_t len) {
    std::string out;
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int val = 0, valb = -6;
    for (size_t i = 0; i < len; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            out.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(chars[((val << (6 + valb)) & 0x3F)]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

std::vector<uint8_t> base64_decode(const std::string& in) {
    std::vector<uint8_t> out;
    static const std::vector<int> T = [] {
        std::vector<int> t(256, -1);
        const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        for (int i = 0; i < 64; i++) t[(unsigned char)chars[i]] = i;
        return t;
    }();

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) continue;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(uint8_t((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

std::string url_decode(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 2 < in.size()) {
                int hex = 0;
                for (int j = 1; j <= 2; ++j) {
                    hex <<= 4;
                    if (in[i + j] >= '0' && in[i + j] <= '9')
                        hex += in[i + j] - '0';
                    else if (in[i + j] >= 'a' && in[i + j] <= 'f')
                        hex += in[i + j] - 'a' + 10;
                    else if (in[i + j] >= 'A' && in[i + j] <= 'F')
                        hex += in[i + j] - 'A' + 10;
                }
                out.push_back((char)hex);
                i += 2;
            }
        } else if (in[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(in[i]);
        }
    }
    return out;
}
