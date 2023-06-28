#include "crr.h"

namespace {

constexpr uint64_t get_pow2(int32_t bits) {
    return UINT64_C(1) << bits;
}

constexpr uint64_t get_mask(int32_t bits) {
    return get_pow2(bits) - UINT64_C(1);
}

constexpr uint64_t mask(uint64_t value, int32_t bits) {
    return value & get_mask(bits);
}

constexpr uint64_t mod_inverse(uint64_t value) {
    uint64_t x = ((value << 1 ^ value) & 4) << 1 ^ value;
    x += x - value * x * x;
    x += x - value * x * x;
    x += x - value * x * x;
    x += x - value * x * x;
    return x;
}

int32_t number_of_trailing_zeroes(uint64_t value) {
    if (value == 0) {
        return 64;
    }

    int32_t bits = 0;

    while ((value & 1) == 0) {
        bits += 1;
        value >>= 1;
    }

    return bits;
}

constexpr uint64_t MASK_48 = get_mask(48);
constexpr uint64_t M1 = 25214903917;
constexpr uint64_t A1 = 11;
constexpr uint64_t M2 = 205749139540585;
constexpr uint64_t A2 = 277363943098;
constexpr uint64_t M3 = 233752471717045;
constexpr uint64_t A3 = 11718085204285;
constexpr uint64_t M4 = 55986898099985;
constexpr uint64_t A4 = 49720483695876;

uint64_t get_population_seed_112(uint64_t structure_seed, uint64_t x, uint64_t z) {
    uint64_t xored_structure_seed = structure_seed ^ M1;
    uint32_t c1 = (((xored_structure_seed * M1 + A1) & MASK_48)) >> 16;
    uint32_t c2 = (((xored_structure_seed * M2 + A2) & MASK_48)) >> 16;
    uint32_t c3 = (((xored_structure_seed * M3 + A3) & MASK_48)) >> 16;
    uint32_t c4 = (((xored_structure_seed * M4 + A4) & MASK_48)) >> 16;
    uint64_t l1 = (static_cast<uint64_t>(c1) << 32) + static_cast<uint64_t>(static_cast<int32_t>(c2));
    uint64_t l2 = (static_cast<uint64_t>(c3) << 32) + static_cast<uint64_t>(static_cast<int32_t>(c4));

    uint64_t a = static_cast<uint64_t>(static_cast<int64_t>(l1) / 2 * 2 + 1);
    uint64_t b = static_cast<uint64_t>(static_cast<int64_t>(l2) / 2 * 2 + 1);
    return (x * a + z * b ^ structure_seed) & MASK_48;
}

void get_offsets_112(uint64_t *offsets, int32_t *offsets_len, uint64_t x, uint64_t z) {
    for (uint64_t i = 0; i < 3; i++) {
        for (uint64_t j = 0; j < 3; j++) {
            uint64_t offset = x * i + z * j;

            bool duplicate = false;
            for (int32_t k = 0; k < *offsets_len; k++) {
                if (offsets[k] == offset) {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate) {
                offsets[(*offsets_len)++] = offset;
            }
        }
    }
}

void reverse_population_seed_112_fallback_recurse(uint64_t offset, uint64_t structure_seed, int32_t structure_seed_bits, uint64_t population_seed, uint64_t x, uint64_t z, std::vector<uint64_t> &out) {
    if ((((x * (((structure_seed ^ M1) * M2 + A2) >> 16) + z * (((structure_seed ^ M1) * M4 + A4) >> 16) + offset) ^ structure_seed ^ population_seed) & get_mask(structure_seed_bits - 16)) != 0) {
        return;
    }

    if (structure_seed_bits == 48) {
        if (get_population_seed_112(structure_seed, x, z) == population_seed) {
            out.push_back(structure_seed);
        }

        return;
    }

    reverse_population_seed_112_fallback_recurse(offset, structure_seed, structure_seed_bits + 1, population_seed, x, z, out);
    reverse_population_seed_112_fallback_recurse(offset, structure_seed + get_pow2(structure_seed_bits), structure_seed_bits + 1, population_seed, x, z, out);
}

void reverse_population_seed_112_fallback(uint64_t population_seed, uint64_t x, uint64_t z, std::vector<uint64_t> &out) {
    uint64_t offsets[9];
    int32_t offsets_len = 0;
    get_offsets_112(offsets, &offsets_len, x, z);

    for (int32_t k = 0; k < offsets_len; k++) {
        uint64_t offset = offsets[k];
        for (uint64_t structure_seed_low = 0; structure_seed_low < get_pow2(16); structure_seed_low += 1) {
            reverse_population_seed_112_fallback_recurse(offset, structure_seed_low, 16, population_seed, x, z, out);
        }
    }
}

uint64_t get_population_seed_113(uint64_t structure_seed, uint64_t x, uint64_t z) {
    uint64_t xored_structure_seed = structure_seed ^ M1;
    uint32_t c1 = (((xored_structure_seed * M1 + A1) & MASK_48)) >> 16;
    uint32_t c2 = (((xored_structure_seed * M2 + A2) & MASK_48)) >> 16;
    uint32_t c3 = (((xored_structure_seed * M3 + A3) & MASK_48)) >> 16;
    uint32_t c4 = (((xored_structure_seed * M4 + A4) & MASK_48)) >> 16;
    uint64_t l1 = (static_cast<uint64_t>(c1) << 32) + static_cast<uint64_t>(static_cast<int32_t>(c2));
    uint64_t l2 = (static_cast<uint64_t>(c3) << 32) + static_cast<uint64_t>(static_cast<int32_t>(c4));

    uint64_t a = l1 | 1;
    uint64_t b = l2 | 1;
    return (x * a + z * b ^ structure_seed) & MASK_48;
}

void get_offsets_113(uint64_t *offsets, int32_t *offsets_len, uint64_t x, uint64_t z) {
    for (uint64_t i = 0; i < 2; i++) {
        for (uint64_t j = 0; j < 2; j++) {
            uint64_t offset = x * i + z * j;

            bool duplicate = false;
            for (int32_t k = 0; k < *offsets_len; k++) {
                if (offsets[k] == offset) {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate) {
                offsets[(*offsets_len)++] = offset;
            }
        }
    }
}

void reverse_population_seed_113_fallback_recurse(uint64_t offset, uint64_t structure_seed, int32_t structure_seed_bits, uint64_t population_seed, uint64_t x, uint64_t z, std::vector<uint64_t> &out) {
    if ((((x * (((structure_seed ^ M1) * M2 + A2) >> 16) + z * (((structure_seed ^ M1) * M4 + A4) >> 16) + offset) ^ structure_seed ^ population_seed) & get_mask(structure_seed_bits - 16)) != 0) {
        return;
    }

    if (structure_seed_bits == 48) {
        if (get_population_seed_113(structure_seed, x, z) == population_seed) {
            out.push_back(structure_seed);
        }

        return;
    }

    reverse_population_seed_113_fallback_recurse(offset, structure_seed, structure_seed_bits + 1, population_seed, x, z, out);
    reverse_population_seed_113_fallback_recurse(offset, structure_seed + get_pow2(structure_seed_bits), structure_seed_bits + 1, population_seed, x, z, out);
}

void reverse_population_seed_113_fallback(uint64_t population_seed, uint64_t x, uint64_t z, std::vector<uint64_t> &out) {
    uint64_t offsets[9];
    int32_t offsets_len = 0;
    get_offsets_113(offsets, &offsets_len, x, z);

    for (int32_t k = 0; k < offsets_len; k++) {
        uint64_t offset = offsets[k];
        for (uint64_t structure_seed_low = 0; structure_seed_low < get_pow2(16); structure_seed_low += 1) {
            reverse_population_seed_113_fallback_recurse(offset, structure_seed_low, 16, population_seed, x, z, out);
        }
    }
}

}

void reverse_population_seed_112(uint64_t population_seed, uint64_t x, uint64_t z, std::vector<uint64_t> &out) {
    if (x == 0 && z == 0) {
        out.push_back(population_seed);
        return;
    }

    uint64_t constant_mult = x * M2 + z * M4;
    int32_t constant_mult_zeros = number_of_trailing_zeroes(constant_mult);
    if (constant_mult_zeros >= 16) {
        reverse_population_seed_112_fallback(population_seed, x, z, out);
        return;
    }
    uint64_t constant_mult_mod_inv = mod_inverse(constant_mult >> constant_mult_zeros);
    int32_t bits_per_iter = 16 - constant_mult_zeros;

    int32_t x_zeros = number_of_trailing_zeroes(x);
    int32_t z_zeros = number_of_trailing_zeroes(z);
    int32_t xz_zeros = number_of_trailing_zeroes(x | z);

    uint64_t offsets[9];
    int32_t offsets_len = 0;
    get_offsets_112(offsets, &offsets_len, x, z);

    uint64_t xored_structure_seed_low = (population_seed ^ M1) & get_mask(xz_zeros + 1) ^ ((x_zeros != z_zeros) << xz_zeros);
    for (; xored_structure_seed_low < get_pow2(16); xored_structure_seed_low += get_pow2(xz_zeros + 1)) {
        uint64_t addend_const_no_offset = x * ((xored_structure_seed_low * M2 + A2) >> 16) + z * ((xored_structure_seed_low * M4 + A4) >> 16);
        
        for (int32_t k = 0; k < offsets_len; k++) {
            uint64_t offset = offsets[k];

            uint64_t addend_const = addend_const_no_offset + offset;
            uint64_t result_const = population_seed ^ M1;

            bool invalid = false;
            uint64_t xored_structure_seed = xored_structure_seed_low;
            int32_t xored_structure_seed_bits = 16;
            while (xored_structure_seed_bits < 48) {
                int32_t bits_left = 48 - xored_structure_seed_bits;
                int32_t bits_this_iter = bits_left - constant_mult_zeros;
                if (bits_this_iter > 16 - constant_mult_zeros) bits_this_iter = 16 - constant_mult_zeros;

                uint64_t addend = addend_const + (xored_structure_seed >> 16) * constant_mult;
                uint64_t result = result_const ^ xored_structure_seed;
                uint64_t mult_result = (result - addend) >> (xored_structure_seed_bits - 16);
                if (bits_this_iter <= 0) {
                    if ((mult_result & get_mask(bits_left)) != 0) {
                        invalid = true;
                        break;
                    }

                    break;
                }
                
                if ((mult_result & get_mask(constant_mult_zeros)) != 0) {
                    invalid = true;
                    break;
                }
                mult_result >>= constant_mult_zeros;

                xored_structure_seed += ((mult_result * constant_mult_mod_inv) & get_mask(bits_this_iter)) << xored_structure_seed_bits;
                xored_structure_seed_bits += bits_this_iter;
            }

            if (!invalid && xored_structure_seed_bits == 48 - constant_mult_zeros) {
                for (uint64_t structure_seed = mask(xored_structure_seed ^ M1, xored_structure_seed_bits); structure_seed < get_pow2(48); structure_seed += get_pow2(xored_structure_seed_bits)) {
                    if (get_population_seed_112(structure_seed, x, z) == population_seed) {
                        out.push_back(structure_seed);
                    }
                }
            }
        }
    }
}

void reverse_population_seed_113(uint64_t population_seed, uint64_t x, uint64_t z, std::vector<uint64_t> &out) {
    if (x == 0 && z == 0) {
        out.push_back(population_seed);
        return;
    }

    uint64_t constant_mult = x * M2 + z * M4;
    int32_t constant_mult_zeros = number_of_trailing_zeroes(constant_mult);
    if (constant_mult_zeros >= 16) {
        reverse_population_seed_113_fallback(population_seed, x, z, out);
        return;
    }
    uint64_t constant_mult_mod_inv = mod_inverse(constant_mult >> constant_mult_zeros);
    int32_t bits_per_iter = 16 - constant_mult_zeros;

    int32_t x_zeros = number_of_trailing_zeroes(x);
    int32_t z_zeros = number_of_trailing_zeroes(z);
    int32_t xz_zeros = number_of_trailing_zeroes(x | z);

    uint64_t offsets[4];
    int32_t offsets_len = 0;
    get_offsets_113(offsets, &offsets_len, x, z);

    uint64_t xored_structure_seed_low = (population_seed ^ M1) & get_mask(xz_zeros + 1) ^ ((x_zeros != z_zeros) << xz_zeros);
    for (; xored_structure_seed_low < get_pow2(16); xored_structure_seed_low += get_pow2(xz_zeros + 1)) {
        uint64_t addend_const_no_offset = x * ((xored_structure_seed_low * M2 + A2) >> 16) + z * ((xored_structure_seed_low * M4 + A4) >> 16);
        
        for (int32_t k = 0; k < offsets_len; k++) {
            uint64_t offset = offsets[k];

            uint64_t addend_const = addend_const_no_offset + offset;
            uint64_t result_const = population_seed ^ M1;

            bool invalid = false;
            uint64_t xored_structure_seed = xored_structure_seed_low;
            int32_t xored_structure_seed_bits = 16;
            while (xored_structure_seed_bits < 48) {
                int32_t bits_left = 48 - xored_structure_seed_bits;
                int32_t bits_this_iter = bits_left - constant_mult_zeros;
                if (bits_this_iter > 16 - constant_mult_zeros) bits_this_iter = 16 - constant_mult_zeros;

                uint64_t addend = addend_const + (xored_structure_seed >> 16) * constant_mult;
                uint64_t result = result_const ^ xored_structure_seed;
                uint64_t mult_result = (result - addend) >> (xored_structure_seed_bits - 16);
                if (bits_this_iter <= 0) {
                    if ((mult_result & get_mask(bits_left)) != 0) {
                        invalid = true;
                        break;
                    }

                    break;
                }
                
                if ((mult_result & get_mask(constant_mult_zeros)) != 0) {
                    invalid = true;
                    break;
                }
                mult_result >>= constant_mult_zeros;

                xored_structure_seed += ((mult_result * constant_mult_mod_inv) & get_mask(bits_this_iter)) << xored_structure_seed_bits;
                xored_structure_seed_bits += bits_this_iter;
            }

            if (!invalid && xored_structure_seed_bits == 48 - constant_mult_zeros) {
                for (uint64_t structure_seed = mask(xored_structure_seed ^ M1, xored_structure_seed_bits); structure_seed < get_pow2(48); structure_seed += get_pow2(xored_structure_seed_bits)) {
                    if (get_population_seed_113(structure_seed, x, z) == population_seed) {
                        out.push_back(structure_seed);
                    }
                }
            }
        }
    }
}