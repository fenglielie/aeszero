#include "aes0.h"

#include <array>
#include <bitset>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

constexpr int Nr = 10;  // AES-128需要 10 轮加密
constexpr int Nk = 4;   // Nk 表示输入密钥的 word 个数

constexpr int AES_BlockSize = 16;

using Byte = std::bitset<8>;   // 一个字节
using Word = std::bitset<32>;  // 4个字节

using Block = std::bitset<128>;
using BlockByteVec = std::array<Byte, 16>;
using ExpandKeyWordVec = std::array<Word, 4 * (Nr + 1)>;
using KeyWordVec = std::array<Word, 4>;
using FullKeyByteVec = std::array<Byte, 4 * Nk>;

const std::vector<std::vector<Byte>> S_Box = {
    std::vector<Byte>{0x63, 0x7C, 0x77, 0x7B, 0xF2, 0x6B, 0x6F, 0xC5, 0x30,
                      0x01, 0x67, 0x2B, 0xFE, 0xD7, 0xAB, 0x76},
    std::vector<Byte>{0xCA, 0x82, 0xC9, 0x7D, 0xFA, 0x59, 0x47, 0xF0, 0xAD,
                      0xD4, 0xA2, 0xAF, 0x9C, 0xA4, 0x72, 0xC0},
    std::vector<Byte>{0xB7, 0xFD, 0x93, 0x26, 0x36, 0x3F, 0xF7, 0xCC, 0x34,
                      0xA5, 0xE5, 0xF1, 0x71, 0xD8, 0x31, 0x15},
    std::vector<Byte>{0x04, 0xC7, 0x23, 0xC3, 0x18, 0x96, 0x05, 0x9A, 0x07,
                      0x12, 0x80, 0xE2, 0xEB, 0x27, 0xB2, 0x75},
    std::vector<Byte>{0x09, 0x83, 0x2C, 0x1A, 0x1B, 0x6E, 0x5A, 0xA0, 0x52,
                      0x3B, 0xD6, 0xB3, 0x29, 0xE3, 0x2F, 0x84},
    std::vector<Byte>{0x53, 0xD1, 0x00, 0xED, 0x20, 0xFC, 0xB1, 0x5B, 0x6A,
                      0xCB, 0xBE, 0x39, 0x4A, 0x4C, 0x58, 0xCF},
    std::vector<Byte>{0xD0, 0xEF, 0xAA, 0xFB, 0x43, 0x4D, 0x33, 0x85, 0x45,
                      0xF9, 0x02, 0x7F, 0x50, 0x3C, 0x9F, 0xA8},
    std::vector<Byte>{0x51, 0xA3, 0x40, 0x8F, 0x92, 0x9D, 0x38, 0xF5, 0xBC,
                      0xB6, 0xDA, 0x21, 0x10, 0xFF, 0xF3, 0xD2},
    std::vector<Byte>{0xCD, 0x0C, 0x13, 0xEC, 0x5F, 0x97, 0x44, 0x17, 0xC4,
                      0xA7, 0x7E, 0x3D, 0x64, 0x5D, 0x19, 0x73},
    std::vector<Byte>{0x60, 0x81, 0x4F, 0xDC, 0x22, 0x2A, 0x90, 0x88, 0x46,
                      0xEE, 0xB8, 0x14, 0xDE, 0x5E, 0x0B, 0xDB},
    std::vector<Byte>{0xE0, 0x32, 0x3A, 0x0A, 0x49, 0x06, 0x24, 0x5C, 0xC2,
                      0xD3, 0xAC, 0x62, 0x91, 0x95, 0xE4, 0x79},
    std::vector<Byte>{0xE7, 0xC8, 0x37, 0x6D, 0x8D, 0xD5, 0x4E, 0xA9, 0x6C,
                      0x56, 0xF4, 0xEA, 0x65, 0x7A, 0xAE, 0x08},
    std::vector<Byte>{0xBA, 0x78, 0x25, 0x2E, 0x1C, 0xA6, 0xB4, 0xC6, 0xE8,
                      0xDD, 0x74, 0x1F, 0x4B, 0xBD, 0x8B, 0x8A},
    std::vector<Byte>{0x70, 0x3E, 0xB5, 0x66, 0x48, 0x03, 0xF6, 0x0E, 0x61,
                      0x35, 0x57, 0xB9, 0x86, 0xC1, 0x1D, 0x9E},
    std::vector<Byte>{0xE1, 0xF8, 0x98, 0x11, 0x69, 0xD9, 0x8E, 0x94, 0x9B,
                      0x1E, 0x87, 0xE9, 0xCE, 0x55, 0x28, 0xDF},
    std::vector<Byte>{0x8C, 0xA1, 0x89, 0x0D, 0xBF, 0xE6, 0x42, 0x68, 0x41,
                      0x99, 0x2D, 0x0F, 0xB0, 0x54, 0xBB, 0x16}};

const std::vector<std::vector<Byte>> Inv_S_Box = {
    std::vector<Byte>{0x52, 0x09, 0x6A, 0xD5, 0x30, 0x36, 0xA5, 0x38, 0xBF,
                      0x40, 0xA3, 0x9E, 0x81, 0xF3, 0xD7, 0xFB},
    std::vector<Byte>{0x7C, 0xE3, 0x39, 0x82, 0x9B, 0x2F, 0xFF, 0x87, 0x34,
                      0x8E, 0x43, 0x44, 0xC4, 0xDE, 0xE9, 0xCB},
    std::vector<Byte>{0x54, 0x7B, 0x94, 0x32, 0xA6, 0xC2, 0x23, 0x3D, 0xEE,
                      0x4C, 0x95, 0x0B, 0x42, 0xFA, 0xC3, 0x4E},
    std::vector<Byte>{0x08, 0x2E, 0xA1, 0x66, 0x28, 0xD9, 0x24, 0xB2, 0x76,
                      0x5B, 0xA2, 0x49, 0x6D, 0x8B, 0xD1, 0x25},
    std::vector<Byte>{0x72, 0xF8, 0xF6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xD4,
                      0xA4, 0x5C, 0xCC, 0x5D, 0x65, 0xB6, 0x92},
    std::vector<Byte>{0x6C, 0x70, 0x48, 0x50, 0xFD, 0xED, 0xB9, 0xDA, 0x5E,
                      0x15, 0x46, 0x57, 0xA7, 0x8D, 0x9D, 0x84},
    std::vector<Byte>{0x90, 0xD8, 0xAB, 0x00, 0x8C, 0xBC, 0xD3, 0x0A, 0xF7,
                      0xE4, 0x58, 0x05, 0xB8, 0xB3, 0x45, 0x06},
    std::vector<Byte>{0xD0, 0x2C, 0x1E, 0x8F, 0xCA, 0x3F, 0x0F, 0x02, 0xC1,
                      0xAF, 0xBD, 0x03, 0x01, 0x13, 0x8A, 0x6B},
    std::vector<Byte>{0x3A, 0x91, 0x11, 0x41, 0x4F, 0x67, 0xDC, 0xEA, 0x97,
                      0xF2, 0xCF, 0xCE, 0xF0, 0xB4, 0xE6, 0x73},
    std::vector<Byte>{0x96, 0xAC, 0x74, 0x22, 0xE7, 0xAD, 0x35, 0x85, 0xE2,
                      0xF9, 0x37, 0xE8, 0x1C, 0x75, 0xDF, 0x6E},
    std::vector<Byte>{0x47, 0xF1, 0x1A, 0x71, 0x1D, 0x29, 0xC5, 0x89, 0x6F,
                      0xB7, 0x62, 0x0E, 0xAA, 0x18, 0xBE, 0x1B},
    std::vector<Byte>{0xFC, 0x56, 0x3E, 0x4B, 0xC6, 0xD2, 0x79, 0x20, 0x9A,
                      0xDB, 0xC0, 0xFE, 0x78, 0xCD, 0x5A, 0xF4},
    std::vector<Byte>{0x1F, 0xDD, 0xA8, 0x33, 0x88, 0x07, 0xC7, 0x31, 0xB1,
                      0x12, 0x10, 0x59, 0x27, 0x80, 0xEC, 0x5F},
    std::vector<Byte>{0x60, 0x51, 0x7F, 0xA9, 0x19, 0xB5, 0x4A, 0x0D, 0x2D,
                      0xE5, 0x7A, 0x9F, 0x93, 0xC9, 0x9C, 0xEF},
    std::vector<Byte>{0xA0, 0xE0, 0x3B, 0x4D, 0xAE, 0x2A, 0xF5, 0xB0, 0xC8,
                      0xEB, 0xBB, 0x3C, 0x83, 0x53, 0x99, 0x61},
    std::vector<Byte>{0x17, 0x2B, 0x04, 0x7E, 0xBA, 0x77, 0xD6, 0x26, 0xE1,
                      0x69, 0x14, 0x63, 0x55, 0x21, 0x0C, 0x7D}};

const std::vector<Byte> Mul_02 = {
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16,
    0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e,
    0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, 0x40, 0x42, 0x44, 0x46,
    0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76,
    0x78, 0x7a, 0x7c, 0x7e, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e,
    0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e, 0xa0, 0xa2, 0xa4, 0xa6,
    0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe,
    0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6,
    0xd8, 0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee,
    0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe, 0x1b, 0x19, 0x1f, 0x1d,
    0x13, 0x11, 0x17, 0x15, 0x0b, 0x09, 0x0f, 0x0d, 0x03, 0x01, 0x07, 0x05,
    0x3b, 0x39, 0x3f, 0x3d, 0x33, 0x31, 0x37, 0x35, 0x2b, 0x29, 0x2f, 0x2d,
    0x23, 0x21, 0x27, 0x25, 0x5b, 0x59, 0x5f, 0x5d, 0x53, 0x51, 0x57, 0x55,
    0x4b, 0x49, 0x4f, 0x4d, 0x43, 0x41, 0x47, 0x45, 0x7b, 0x79, 0x7f, 0x7d,
    0x73, 0x71, 0x77, 0x75, 0x6b, 0x69, 0x6f, 0x6d, 0x63, 0x61, 0x67, 0x65,
    0x9b, 0x99, 0x9f, 0x9d, 0x93, 0x91, 0x97, 0x95, 0x8b, 0x89, 0x8f, 0x8d,
    0x83, 0x81, 0x87, 0x85, 0xbb, 0xb9, 0xbf, 0xbd, 0xb3, 0xb1, 0xb7, 0xb5,
    0xab, 0xa9, 0xaf, 0xad, 0xa3, 0xa1, 0xa7, 0xa5, 0xdb, 0xd9, 0xdf, 0xdd,
    0xd3, 0xd1, 0xd7, 0xd5, 0xcb, 0xc9, 0xcf, 0xcd, 0xc3, 0xc1, 0xc7, 0xc5,
    0xfb, 0xf9, 0xff, 0xfd, 0xf3, 0xf1, 0xf7, 0xf5, 0xeb, 0xe9, 0xef, 0xed,
    0xe3, 0xe1, 0xe7, 0xe5};

const std::vector<Byte> Mul_03 = {
    0x00, 0x03, 0x06, 0x05, 0x0c, 0x0f, 0x0a, 0x09, 0x18, 0x1b, 0x1e, 0x1d,
    0x14, 0x17, 0x12, 0x11, 0x30, 0x33, 0x36, 0x35, 0x3c, 0x3f, 0x3a, 0x39,
    0x28, 0x2b, 0x2e, 0x2d, 0x24, 0x27, 0x22, 0x21, 0x60, 0x63, 0x66, 0x65,
    0x6c, 0x6f, 0x6a, 0x69, 0x78, 0x7b, 0x7e, 0x7d, 0x74, 0x77, 0x72, 0x71,
    0x50, 0x53, 0x56, 0x55, 0x5c, 0x5f, 0x5a, 0x59, 0x48, 0x4b, 0x4e, 0x4d,
    0x44, 0x47, 0x42, 0x41, 0xc0, 0xc3, 0xc6, 0xc5, 0xcc, 0xcf, 0xca, 0xc9,
    0xd8, 0xdb, 0xde, 0xdd, 0xd4, 0xd7, 0xd2, 0xd1, 0xf0, 0xf3, 0xf6, 0xf5,
    0xfc, 0xff, 0xfa, 0xf9, 0xe8, 0xeb, 0xee, 0xed, 0xe4, 0xe7, 0xe2, 0xe1,
    0xa0, 0xa3, 0xa6, 0xa5, 0xac, 0xaf, 0xaa, 0xa9, 0xb8, 0xbb, 0xbe, 0xbd,
    0xb4, 0xb7, 0xb2, 0xb1, 0x90, 0x93, 0x96, 0x95, 0x9c, 0x9f, 0x9a, 0x99,
    0x88, 0x8b, 0x8e, 0x8d, 0x84, 0x87, 0x82, 0x81, 0x9b, 0x98, 0x9d, 0x9e,
    0x97, 0x94, 0x91, 0x92, 0x83, 0x80, 0x85, 0x86, 0x8f, 0x8c, 0x89, 0x8a,
    0xab, 0xa8, 0xad, 0xae, 0xa7, 0xa4, 0xa1, 0xa2, 0xb3, 0xb0, 0xb5, 0xb6,
    0xbf, 0xbc, 0xb9, 0xba, 0xfb, 0xf8, 0xfd, 0xfe, 0xf7, 0xf4, 0xf1, 0xf2,
    0xe3, 0xe0, 0xe5, 0xe6, 0xef, 0xec, 0xe9, 0xea, 0xcb, 0xc8, 0xcd, 0xce,
    0xc7, 0xc4, 0xc1, 0xc2, 0xd3, 0xd0, 0xd5, 0xd6, 0xdf, 0xdc, 0xd9, 0xda,
    0x5b, 0x58, 0x5d, 0x5e, 0x57, 0x54, 0x51, 0x52, 0x43, 0x40, 0x45, 0x46,
    0x4f, 0x4c, 0x49, 0x4a, 0x6b, 0x68, 0x6d, 0x6e, 0x67, 0x64, 0x61, 0x62,
    0x73, 0x70, 0x75, 0x76, 0x7f, 0x7c, 0x79, 0x7a, 0x3b, 0x38, 0x3d, 0x3e,
    0x37, 0x34, 0x31, 0x32, 0x23, 0x20, 0x25, 0x26, 0x2f, 0x2c, 0x29, 0x2a,
    0x0b, 0x08, 0x0d, 0x0e, 0x07, 0x04, 0x01, 0x02, 0x13, 0x10, 0x15, 0x16,
    0x1f, 0x1c, 0x19, 0x1a};

const std::vector<Byte> Mul_09 = {
    0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f, 0x48, 0x41, 0x5a, 0x53,
    0x6c, 0x65, 0x7e, 0x77, 0x90, 0x99, 0x82, 0x8b, 0xb4, 0xbd, 0xa6, 0xaf,
    0xd8, 0xd1, 0xca, 0xc3, 0xfc, 0xf5, 0xee, 0xe7, 0x3b, 0x32, 0x29, 0x20,
    0x1f, 0x16, 0x0d, 0x04, 0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
    0xab, 0xa2, 0xb9, 0xb0, 0x8f, 0x86, 0x9d, 0x94, 0xe3, 0xea, 0xf1, 0xf8,
    0xc7, 0xce, 0xd5, 0xdc, 0x76, 0x7f, 0x64, 0x6d, 0x52, 0x5b, 0x40, 0x49,
    0x3e, 0x37, 0x2c, 0x25, 0x1a, 0x13, 0x08, 0x01, 0xe6, 0xef, 0xf4, 0xfd,
    0xc2, 0xcb, 0xd0, 0xd9, 0xae, 0xa7, 0xbc, 0xb5, 0x8a, 0x83, 0x98, 0x91,
    0x4d, 0x44, 0x5f, 0x56, 0x69, 0x60, 0x7b, 0x72, 0x05, 0x0c, 0x17, 0x1e,
    0x21, 0x28, 0x33, 0x3a, 0xdd, 0xd4, 0xcf, 0xc6, 0xf9, 0xf0, 0xeb, 0xe2,
    0x95, 0x9c, 0x87, 0x8e, 0xb1, 0xb8, 0xa3, 0xaa, 0xec, 0xe5, 0xfe, 0xf7,
    0xc8, 0xc1, 0xda, 0xd3, 0xa4, 0xad, 0xb6, 0xbf, 0x80, 0x89, 0x92, 0x9b,
    0x7c, 0x75, 0x6e, 0x67, 0x58, 0x51, 0x4a, 0x43, 0x34, 0x3d, 0x26, 0x2f,
    0x10, 0x19, 0x02, 0x0b, 0xd7, 0xde, 0xc5, 0xcc, 0xf3, 0xfa, 0xe1, 0xe8,
    0x9f, 0x96, 0x8d, 0x84, 0xbb, 0xb2, 0xa9, 0xa0, 0x47, 0x4e, 0x55, 0x5c,
    0x63, 0x6a, 0x71, 0x78, 0x0f, 0x06, 0x1d, 0x14, 0x2b, 0x22, 0x39, 0x30,
    0x9a, 0x93, 0x88, 0x81, 0xbe, 0xb7, 0xac, 0xa5, 0xd2, 0xdb, 0xc0, 0xc9,
    0xf6, 0xff, 0xe4, 0xed, 0x0a, 0x03, 0x18, 0x11, 0x2e, 0x27, 0x3c, 0x35,
    0x42, 0x4b, 0x50, 0x59, 0x66, 0x6f, 0x74, 0x7d, 0xa1, 0xa8, 0xb3, 0xba,
    0x85, 0x8c, 0x97, 0x9e, 0xe9, 0xe0, 0xfb, 0xf2, 0xcd, 0xc4, 0xdf, 0xd6,
    0x31, 0x38, 0x23, 0x2a, 0x15, 0x1c, 0x07, 0x0e, 0x79, 0x70, 0x6b, 0x62,
    0x5d, 0x54, 0x4f, 0x46};

const std::vector<Byte> Mul_0b = {
    0x00, 0x0b, 0x16, 0x1d, 0x2c, 0x27, 0x3a, 0x31, 0x58, 0x53, 0x4e, 0x45,
    0x74, 0x7f, 0x62, 0x69, 0xb0, 0xbb, 0xa6, 0xad, 0x9c, 0x97, 0x8a, 0x81,
    0xe8, 0xe3, 0xfe, 0xf5, 0xc4, 0xcf, 0xd2, 0xd9, 0x7b, 0x70, 0x6d, 0x66,
    0x57, 0x5c, 0x41, 0x4a, 0x23, 0x28, 0x35, 0x3e, 0x0f, 0x04, 0x19, 0x12,
    0xcb, 0xc0, 0xdd, 0xd6, 0xe7, 0xec, 0xf1, 0xfa, 0x93, 0x98, 0x85, 0x8e,
    0xbf, 0xb4, 0xa9, 0xa2, 0xf6, 0xfd, 0xe0, 0xeb, 0xda, 0xd1, 0xcc, 0xc7,
    0xae, 0xa5, 0xb8, 0xb3, 0x82, 0x89, 0x94, 0x9f, 0x46, 0x4d, 0x50, 0x5b,
    0x6a, 0x61, 0x7c, 0x77, 0x1e, 0x15, 0x08, 0x03, 0x32, 0x39, 0x24, 0x2f,
    0x8d, 0x86, 0x9b, 0x90, 0xa1, 0xaa, 0xb7, 0xbc, 0xd5, 0xde, 0xc3, 0xc8,
    0xf9, 0xf2, 0xef, 0xe4, 0x3d, 0x36, 0x2b, 0x20, 0x11, 0x1a, 0x07, 0x0c,
    0x65, 0x6e, 0x73, 0x78, 0x49, 0x42, 0x5f, 0x54, 0xf7, 0xfc, 0xe1, 0xea,
    0xdb, 0xd0, 0xcd, 0xc6, 0xaf, 0xa4, 0xb9, 0xb2, 0x83, 0x88, 0x95, 0x9e,
    0x47, 0x4c, 0x51, 0x5a, 0x6b, 0x60, 0x7d, 0x76, 0x1f, 0x14, 0x09, 0x02,
    0x33, 0x38, 0x25, 0x2e, 0x8c, 0x87, 0x9a, 0x91, 0xa0, 0xab, 0xb6, 0xbd,
    0xd4, 0xdf, 0xc2, 0xc9, 0xf8, 0xf3, 0xee, 0xe5, 0x3c, 0x37, 0x2a, 0x21,
    0x10, 0x1b, 0x06, 0x0d, 0x64, 0x6f, 0x72, 0x79, 0x48, 0x43, 0x5e, 0x55,
    0x01, 0x0a, 0x17, 0x1c, 0x2d, 0x26, 0x3b, 0x30, 0x59, 0x52, 0x4f, 0x44,
    0x75, 0x7e, 0x63, 0x68, 0xb1, 0xba, 0xa7, 0xac, 0x9d, 0x96, 0x8b, 0x80,
    0xe9, 0xe2, 0xff, 0xf4, 0xc5, 0xce, 0xd3, 0xd8, 0x7a, 0x71, 0x6c, 0x67,
    0x56, 0x5d, 0x40, 0x4b, 0x22, 0x29, 0x34, 0x3f, 0x0e, 0x05, 0x18, 0x13,
    0xca, 0xc1, 0xdc, 0xd7, 0xe6, 0xed, 0xf0, 0xfb, 0x92, 0x99, 0x84, 0x8f,
    0xbe, 0xb5, 0xa8, 0xa3};

const std::vector<Byte> Mul_0d = {
    0x00, 0x0d, 0x1a, 0x17, 0x34, 0x39, 0x2e, 0x23, 0x68, 0x65, 0x72, 0x7f,
    0x5c, 0x51, 0x46, 0x4b, 0xd0, 0xdd, 0xca, 0xc7, 0xe4, 0xe9, 0xfe, 0xf3,
    0xb8, 0xb5, 0xa2, 0xaf, 0x8c, 0x81, 0x96, 0x9b, 0xbb, 0xb6, 0xa1, 0xac,
    0x8f, 0x82, 0x95, 0x98, 0xd3, 0xde, 0xc9, 0xc4, 0xe7, 0xea, 0xfd, 0xf0,
    0x6b, 0x66, 0x71, 0x7c, 0x5f, 0x52, 0x45, 0x48, 0x03, 0x0e, 0x19, 0x14,
    0x37, 0x3a, 0x2d, 0x20, 0x6d, 0x60, 0x77, 0x7a, 0x59, 0x54, 0x43, 0x4e,
    0x05, 0x08, 0x1f, 0x12, 0x31, 0x3c, 0x2b, 0x26, 0xbd, 0xb0, 0xa7, 0xaa,
    0x89, 0x84, 0x93, 0x9e, 0xd5, 0xd8, 0xcf, 0xc2, 0xe1, 0xec, 0xfb, 0xf6,
    0xd6, 0xdb, 0xcc, 0xc1, 0xe2, 0xef, 0xf8, 0xf5, 0xbe, 0xb3, 0xa4, 0xa9,
    0x8a, 0x87, 0x90, 0x9d, 0x06, 0x0b, 0x1c, 0x11, 0x32, 0x3f, 0x28, 0x25,
    0x6e, 0x63, 0x74, 0x79, 0x5a, 0x57, 0x40, 0x4d, 0xda, 0xd7, 0xc0, 0xcd,
    0xee, 0xe3, 0xf4, 0xf9, 0xb2, 0xbf, 0xa8, 0xa5, 0x86, 0x8b, 0x9c, 0x91,
    0x0a, 0x07, 0x10, 0x1d, 0x3e, 0x33, 0x24, 0x29, 0x62, 0x6f, 0x78, 0x75,
    0x56, 0x5b, 0x4c, 0x41, 0x61, 0x6c, 0x7b, 0x76, 0x55, 0x58, 0x4f, 0x42,
    0x09, 0x04, 0x13, 0x1e, 0x3d, 0x30, 0x27, 0x2a, 0xb1, 0xbc, 0xab, 0xa6,
    0x85, 0x88, 0x9f, 0x92, 0xd9, 0xd4, 0xc3, 0xce, 0xed, 0xe0, 0xf7, 0xfa,
    0xb7, 0xba, 0xad, 0xa0, 0x83, 0x8e, 0x99, 0x94, 0xdf, 0xd2, 0xc5, 0xc8,
    0xeb, 0xe6, 0xf1, 0xfc, 0x67, 0x6a, 0x7d, 0x70, 0x53, 0x5e, 0x49, 0x44,
    0x0f, 0x02, 0x15, 0x18, 0x3b, 0x36, 0x21, 0x2c, 0x0c, 0x01, 0x16, 0x1b,
    0x38, 0x35, 0x22, 0x2f, 0x64, 0x69, 0x7e, 0x73, 0x50, 0x5d, 0x4a, 0x47,
    0xdc, 0xd1, 0xc6, 0xcb, 0xe8, 0xe5, 0xf2, 0xff, 0xb4, 0xb9, 0xae, 0xa3,
    0x80, 0x8d, 0x9a, 0x97};

const std::vector<Byte> Mul_0e = {
    0x00, 0x0e, 0x1c, 0x12, 0x38, 0x36, 0x24, 0x2a, 0x70, 0x7e, 0x6c, 0x62,
    0x48, 0x46, 0x54, 0x5a, 0xe0, 0xee, 0xfc, 0xf2, 0xd8, 0xd6, 0xc4, 0xca,
    0x90, 0x9e, 0x8c, 0x82, 0xa8, 0xa6, 0xb4, 0xba, 0xdb, 0xd5, 0xc7, 0xc9,
    0xe3, 0xed, 0xff, 0xf1, 0xab, 0xa5, 0xb7, 0xb9, 0x93, 0x9d, 0x8f, 0x81,
    0x3b, 0x35, 0x27, 0x29, 0x03, 0x0d, 0x1f, 0x11, 0x4b, 0x45, 0x57, 0x59,
    0x73, 0x7d, 0x6f, 0x61, 0xad, 0xa3, 0xb1, 0xbf, 0x95, 0x9b, 0x89, 0x87,
    0xdd, 0xd3, 0xc1, 0xcf, 0xe5, 0xeb, 0xf9, 0xf7, 0x4d, 0x43, 0x51, 0x5f,
    0x75, 0x7b, 0x69, 0x67, 0x3d, 0x33, 0x21, 0x2f, 0x05, 0x0b, 0x19, 0x17,
    0x76, 0x78, 0x6a, 0x64, 0x4e, 0x40, 0x52, 0x5c, 0x06, 0x08, 0x1a, 0x14,
    0x3e, 0x30, 0x22, 0x2c, 0x96, 0x98, 0x8a, 0x84, 0xae, 0xa0, 0xb2, 0xbc,
    0xe6, 0xe8, 0xfa, 0xf4, 0xde, 0xd0, 0xc2, 0xcc, 0x41, 0x4f, 0x5d, 0x53,
    0x79, 0x77, 0x65, 0x6b, 0x31, 0x3f, 0x2d, 0x23, 0x09, 0x07, 0x15, 0x1b,
    0xa1, 0xaf, 0xbd, 0xb3, 0x99, 0x97, 0x85, 0x8b, 0xd1, 0xdf, 0xcd, 0xc3,
    0xe9, 0xe7, 0xf5, 0xfb, 0x9a, 0x94, 0x86, 0x88, 0xa2, 0xac, 0xbe, 0xb0,
    0xea, 0xe4, 0xf6, 0xf8, 0xd2, 0xdc, 0xce, 0xc0, 0x7a, 0x74, 0x66, 0x68,
    0x42, 0x4c, 0x5e, 0x50, 0x0a, 0x04, 0x16, 0x18, 0x32, 0x3c, 0x2e, 0x20,
    0xec, 0xe2, 0xf0, 0xfe, 0xd4, 0xda, 0xc8, 0xc6, 0x9c, 0x92, 0x80, 0x8e,
    0xa4, 0xaa, 0xb8, 0xb6, 0x0c, 0x02, 0x10, 0x1e, 0x34, 0x3a, 0x28, 0x26,
    0x7c, 0x72, 0x60, 0x6e, 0x44, 0x4a, 0x58, 0x56, 0x37, 0x39, 0x2b, 0x25,
    0x0f, 0x01, 0x13, 0x1d, 0x47, 0x49, 0x5b, 0x55, 0x7f, 0x71, 0x63, 0x6d,
    0xd7, 0xd9, 0xcb, 0xc5, 0xef, 0xe1, 0xf3, 0xfd, 0xa7, 0xa9, 0xbb, 0xb5,
    0x9f, 0x91, 0x83, 0x8d};

// 轮常数，密钥扩展中用到。（AES-128只需要10轮）
const std::vector<Word> Rcon = {0x01000000, 0x02000000, 0x04000000, 0x08000000,
                                0x10000000, 0x20000000, 0x40000000, 0x80000000,
                                0x1b000000, 0x36000000};

std::tuple<int, int> get_byte_index(const Byte &b) {
    int row = static_cast<int>(b[7]) * 8 + static_cast<int>(b[6]) * 4
              + static_cast<int>(b[5]) * 2 + static_cast<int>(b[4]);
    int col = static_cast<int>(b[3]) * 8 + static_cast<int>(b[2]) * 4
              + static_cast<int>(b[1]) * 2 + static_cast<int>(b[0]);

    return {row, col};
}

/**
 *  S盒变换 - 前4位为行号，后4位为列号
 */
void SubBytes(BlockByteVec &mtx) {
    for (int i = 0; i < 16; ++i) {
        auto [row, col] = get_byte_index(mtx[i]);
        mtx[i] = S_Box[row][col];
    }
}

/**
 *  逆S盒变换 - 前4位为行号，后4位为列号
 */
void InvSubBytes(BlockByteVec &mtx) {
    for (int i = 0; i < 16; ++i) {
        auto [row, col] = get_byte_index(mtx[i]);
        mtx[i] = Inv_S_Box[row][col];
    }
}

/**
 *  行变换 - 按字节循环移位
 */
void ShiftRows(BlockByteVec &mtx) {
    // 第二行循环左移一位
    Byte temp = mtx[4];
    for (int i = 0; i < 3; ++i) mtx[i + 4] = mtx[i + 5];
    mtx[7] = temp;
    // 第三行循环左移两位
    for (int i = 0; i < 2; ++i) {
        temp = mtx[i + 8];
        mtx[i + 8] = mtx[i + 10];
        mtx[i + 10] = temp;
    }
    // 第四行循环左移三位
    temp = mtx[15];
    for (int i = 3; i > 0; --i) mtx[i + 12] = mtx[i + 11];
    mtx[12] = temp;
}

/**
 *  逆行变换 - 以字节为单位循环右移
 */
void InvShiftRows(BlockByteVec &mtx) {
    // 第二行循环右移一位
    Byte temp = mtx[7];
    for (int i = 3; i > 0; --i) mtx[i + 4] = mtx[i + 3];
    mtx[4] = temp;
    // 第三行循环右移两位
    for (int i = 0; i < 2; ++i) {
        temp = mtx[i + 8];
        mtx[i + 8] = mtx[i + 10];
        mtx[i + 10] = temp;
    }
    // 第四行循环右移三位
    temp = mtx[12];
    for (int i = 0; i < 3; ++i) mtx[i + 12] = mtx[i + 13];
    mtx[15] = temp;
}

/**
 *  列混淆
 */
void MixColumns(BlockByteVec &mtx) {
    Byte arr[4];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) arr[j] = mtx[i + j * 4];

        mtx[i] = Mul_02[arr[0].to_ulong()] ^ Mul_03[arr[1].to_ulong()] ^ arr[2]
                 ^ arr[3];
        mtx[i + 4] = arr[0] ^ Mul_02[arr[1].to_ulong()]
                     ^ Mul_03[arr[2].to_ulong()] ^ arr[3];
        mtx[i + 8] = arr[0] ^ arr[1] ^ Mul_02[arr[2].to_ulong()]
                     ^ Mul_03[arr[3].to_ulong()];
        mtx[i + 12] = Mul_03[arr[0].to_ulong()] ^ arr[1] ^ arr[2]
                      ^ Mul_02[arr[3].to_ulong()];
    }
}

/**
 *  逆列混淆
 */
void InvMixColumns(BlockByteVec &mtx) {
    Byte arr[4];
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) arr[j] = mtx[i + j * 4];

        mtx[i] = Mul_0e[arr[0].to_ulong()] ^ Mul_0b[arr[1].to_ulong()]
                 ^ Mul_0d[arr[2].to_ulong()] ^ Mul_09[arr[3].to_ulong()];
        mtx[i + 4] = Mul_09[arr[0].to_ulong()] ^ Mul_0e[arr[1].to_ulong()]
                     ^ Mul_0b[arr[2].to_ulong()] ^ Mul_0d[arr[3].to_ulong()];
        mtx[i + 8] = Mul_0d[arr[0].to_ulong()] ^ Mul_09[arr[1].to_ulong()]
                     ^ Mul_0e[arr[2].to_ulong()] ^ Mul_0b[arr[3].to_ulong()];
        mtx[i + 12] = Mul_0b[arr[0].to_ulong()] ^ Mul_0d[arr[1].to_ulong()]
                      ^ Mul_09[arr[2].to_ulong()] ^ Mul_0e[arr[3].to_ulong()];
    }
}

/**
 *  轮密钥加变换 - 将每一列与扩展密钥进行异或
 */
void AddRoundKey(BlockByteVec &mtx, const KeyWordVec &k) {
    for (int i = 0; i < 4; ++i) {
        Word k1 = k[i] >> 24;
        Word k2 = (k[i] << 8) >> 24;
        Word k3 = (k[i] << 16) >> 24;
        Word k4 = (k[i] << 24) >> 24;
        // 异或运算
        mtx[i] = mtx[i] ^ Byte(k1.to_ulong());
        mtx[i + 4] = mtx[i + 4] ^ Byte(k2.to_ulong());
        mtx[i + 8] = mtx[i + 8] ^ Byte(k3.to_ulong());
        mtx[i + 12] = mtx[i + 12] ^ Byte(k4.to_ulong());
    }
}

/******************************下面是密钥扩展部分***********************/
/**
 * 将4个 Byte 转换为一个 Word.
 */
Word ByteToWord(const Byte &k1, const Byte &k2, const Byte &k3,
                const Byte &k4) {
    Word result(0x00000000);
    Word temp;
    temp = k1.to_ulong();  // K1
    temp <<= 24;
    result |= temp;
    temp = k2.to_ulong();  // K2
    temp <<= 16;
    result |= temp;
    temp = k3.to_ulong();  // K3
    temp <<= 8;
    result |= temp;
    temp = k4.to_ulong();  // K4
    result |= temp;
    return result;
}

/**
 *  按字节循环左移一位
 *  即把[a0, a1, a2, a3]变成[a1, a2, a3, a0]
 */
Word RotWord(const Word &rw) {
    Word high = rw << 8;
    Word low = rw >> 24;
    return high | low;
}

/**
 *  对输入word中的每一个字节进行S-盒变换
 */
Word SubWord(const Word &sw) {
    Word temp;
    for (int i = 0; i < 32; i += 8) {
        int row =
            static_cast<int>(sw[i + 7]) * 8 + static_cast<int>(sw[i + 6]) * 4
            + static_cast<int>(sw[i + 5]) * 2 + static_cast<int>(sw[i + 4]);
        int col = static_cast<int>(sw[i + 3]) * 8
                  + static_cast<int>(sw[i + 2]) * 4
                  + static_cast<int>(sw[i + 1]) * 2 + static_cast<int>(sw[i]);

        Byte val = S_Box[row][col];
        for (int j = 0; j < 8; ++j) temp[i + j] = val[j];
    }
    return temp;
}

/**
 * T变换
 */
Word Toper(const Word &sw, const Word &rcon) { return sw ^ rcon; }

/**
 *  密钥扩展函数 - 对128位密钥进行扩展得到 w[4*(Nr+1)]
 */
ExpandKeyWordVec KeyExpansion(const FullKeyByteVec &key) {
    ExpandKeyWordVec w;
    int i = 0;
    // w[]的前4个就是输入的key
    while (i < Nk) {
        w[i] = ByteToWord(key[4 * i], key[4 * i + 1], key[4 * i + 2],
                          key[4 * i + 3]);
        ++i;
    }

    i = Nk;

    while (i < 4 * (Nr + 1)) {
        if (i % Nk == 0) {
            w[i] =
                w[i - Nk] ^ Toper(SubWord(RotWord(w[i - 1])), Rcon[i / Nk - 1]);
        }
        else { w[i] = w[i - Nk] ^ w[i - 1]; }

        ++i;
    }
    return w;
}

/**
 *  将一个char字符数组转化为二进制
 *  存到一个 Byte 数组中
 *  对于不足16个字符时，后面补0（即空格）
 */
BlockByteVec CharToByte(const std::string &str) {
    BlockByteVec out;
    for (int i = 0; i < 16; ++i)
        for (int j = 0; j < 8; ++j) out[i][j] = ((str[i] >> j) & 1);
    return out;
}

ExpandKeyWordVec GetKey(const std::string &key_str) {
    FullKeyByteVec key = CharToByte(key_str);
    return KeyExpansion(key);
}

/**
 *  将连续的128位分成16组，存到一个 Byte 数组中
 */
BlockByteVec DivideByte(const Block &data) {
    BlockByteVec out;
    for (int i = 0; i < 16; ++i) {
        Block temp = (data << 8 * i) >> 120;
        out[i] = temp.to_ulong();
    }
    return out;
}

/**
 *  将16个 Byte 合并成连续的128位
 */
Block MergeByte(const BlockByteVec &in) {
    Block res;
    for (int i = 0; i < 16; ++i) {
        Block temp = in[i].to_ulong();
        temp <<= 8 * (15 - i);
        res |= temp;
    }
    return res;
}

// 填充
BlockByteVec PKCS7Pad(BlockByteVec plain, int bytesRead) {
    int paddingSize = AES_BlockSize - bytesRead;
    for (int i = 0; i < paddingSize; i++) { plain[i] = paddingSize; }

    return plain;
}

// 获取填充长度
int PKCS7Unpad(const BlockByteVec &plain) {
    // 最后一个字节的值是填充长度
    int paddingSize = static_cast<int>(plain[0].to_ulong());
    return AES_BlockSize - paddingSize;
}

// 生成固定长度的随机字符串
std::string GetRandomStr(size_t length) {
    // 定义可见字符范围（不含空格）
    const char visibleCharStart = 33;  // '!'
    const char visibleCharEnd = 126;   // '~'

    // 创建随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(visibleCharStart, visibleCharEnd);

    // 生成随机字符串
    std::string randomString;
    randomString.reserve(length);
    for (size_t i = 0; i < length; i++) {
        randomString += static_cast<char>(dis(gen));
    }

    return randomString;
}

/******************************加密和解密函数**************************/
/**
 *  加密
 */
BlockByteVec BlockEncrypt(BlockByteVec data_block, const ExpandKeyWordVec &w) {
    KeyWordVec key;
    for (int i = 0; i < 4; ++i) key[i] = w[i];
    AddRoundKey(data_block, key);

    for (int round = 1; round < Nr; ++round) {
        SubBytes(data_block);
        ShiftRows(data_block);
        MixColumns(data_block);
        for (int i = 0; i < 4; ++i) key[i] = w[4 * round + i];
        AddRoundKey(data_block, key);
    }

    SubBytes(data_block);
    ShiftRows(data_block);
    for (int i = 0; i < 4; ++i) key[i] = w[4 * Nr + i];
    AddRoundKey(data_block, key);

    return data_block;
}

/**
 *  解密
 */
BlockByteVec BlockDecrypt(BlockByteVec data_block, const ExpandKeyWordVec &w) {
    KeyWordVec key;
    for (int i = 0; i < 4; ++i) key[i] = w[4 * Nr + i];
    AddRoundKey(data_block, key);

    for (int round = Nr - 1; round > 0; --round) {
        InvShiftRows(data_block);
        InvSubBytes(data_block);
        for (int i = 0; i < 4; ++i) key[i] = w[4 * round + i];
        AddRoundKey(data_block, key);
        InvMixColumns(data_block);
    }

    InvShiftRows(data_block);
    InvSubBytes(data_block);
    for (int i = 0; i < 4; ++i) key[i] = w[i];
    AddRoundKey(data_block, key);

    return data_block;
}

// 保证密钥长度为16
std::string AES0::Fixkey(const std::string &key_in) {
    std::string key;

    if (key_in.size() > 16) {
        // 如果输入字符串长度大于16，截断至16位
        key = key_in.substr(0, 16);
    }
    else {
        // 如果长度不足，随机填充可见字符至16位
        key = key_in + GetRandomStr(16 - key_in.size());
    }

    return key;
}

std::string AES0::Mixkey(const std::string &key) {
    if (key.length() != 16) {
        std::cerr << "Mixkey: key length error" << std::endl;
        exit(1);
    }

    const std::vector<size_t> primes = {2,  3,  5,  7,  11, 13, 17, 19,
                                        23, 29, 31, 37, 41, 43, 47, 53};

    std::string mixkey = GetRandomStr(64);  // 先生成随机字符串
    size_t key_index = 0;                   // 在素数指标处替换
    for (size_t index : primes) { mixkey[index] = key[key_index++]; }
    return mixkey;
}

std::string AES0::InvMixkey(const std::string &mixkey) {
    if (mixkey.length() != 64) {
        std::cerr << "InvMixkey: mixkey length error" << std::endl;
        exit(1);
    }

    const std::vector<size_t> primes{2,  3,  5,  7,  11, 13, 17, 19,
                                     23, 29, 31, 37, 41, 43, 47, 53};

    std::string key;
    key.reserve(16);
    for (size_t i = 0; i < 16; i++) { key += mixkey[primes[i]]; }
    return key;
}

void AES0::FileEncrypt(const std::string &in_file, const std::string &out_file,
                       const std::string &key_str) {
    // 处理密钥不足16个字节的情形
    ExpandKeyWordVec w = GetKey(Fixkey(key_str));

    Block data;
    BlockByteVec plain;

    std::ifstream in(in_file, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << in_file << std::endl;
        exit(1);
    }
    std::ofstream out(out_file, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file: " << out_file << std::endl;
        exit(1);
    }

    while (true) {
        in.read((char *)&data, sizeof(data));

        int bytesRead = static_cast<int>(in.gcount());  // 实际读取的字节数
        if (bytesRead < AES_BlockSize) {
            plain = DivideByte(data);
            plain = PKCS7Pad(plain, bytesRead);  // 执行 PKCS7 填充
            plain = BlockEncrypt(plain, w);
            data = MergeByte(plain);
            out.write((char *)&data, sizeof(data));
            break;
        }

        plain = DivideByte(data);
        plain = BlockEncrypt(plain, w);
        data = MergeByte(plain);
        out.write((char *)&data, sizeof(data));

        data.reset();  // 置0
    }
    in.close();
    out.close();
}

void AES0::FileDecrypt(const std::string &in_file, const std::string &out_file,
                       const std::string &key_str) {
    // 处理密钥不足16个字节的情形
    ExpandKeyWordVec w = GetKey(Fixkey(key_str));

    Block data;
    BlockByteVec plain;

    std::ifstream in(in_file, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Failed to open file: " << in_file << std::endl;
        exit(1);
    }
    std::ofstream out(out_file, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Failed to open file: " << out_file << std::endl;
        exit(1);
    }

    while (true) {
        in.read((char *)&data, sizeof(data));
        plain = DivideByte(data);
        plain = BlockDecrypt(plain, w);
        data = MergeByte(plain);

        // 如果这是最后一块数据
        if (in.peek() == EOF) {
            if (in.gcount() > 0 && in.gcount() < AES_BlockSize) {
                // 最后一块不满16个字节，则出现错误，直接丢弃
                break;
            }

            int originalLength = PKCS7Unpad(plain);  // 去除填充
            out.write((char *)&data, originalLength);
            break;
        }
        out.write((char *)&data, sizeof(data));

        data.reset();  // 置0
    }
    in.close();
    out.close();
}
