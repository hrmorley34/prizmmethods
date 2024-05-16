#include "ringing/method.hpp"

const ringing::Method Original5 = ringing::Method{
    .stage = 5,
    .title = {'O', 'r', 'i', 'g', 'i', 'n', 'a', 'l', ' ', 'D', 'o', 'u', 'b', 'l', 'e', 's', '\0'},
    .leadlength = 2,
    .pn = {0b10000, 0b00001},
    .leadcount = 5,
    .huntbells = 0b00001};

const ringing::Method Original6 = ringing::Method{
    .stage = 6,
    .title = {'O', 'r', 'i', 'g', 'i', 'n', 'a', 'l', ' ', 'M', 'i', 'n', 'o', 'r', '\0'},
    .leadlength = 2,
    .pn = {0, 0b100001},
    .leadcount = 6,
    .huntbells = 0b000001};

const ringing::Method PlainBob6 = ringing::Method{
    .stage = 6,
    .title = {'P', 'l', 'a', 'i', 'n', ' ', 'B', 'o', 'b', ' ', 'M', 'i', 'n', 'o', 'r', '\0'},
    .leadlength = 12,
    .pn = {0, 0b100001, 0, 0b100001, 0, 0b100001,
           0, 0b100001, 0, 0b100001, 0, 0b000011},
    .leadcount = 5,
    .huntbells = 0b000001};

const ringing::Method PlainBob12 = ringing::Method{
    .stage = 12,
    .title = {'P', 'l', 'a', 'i', 'n', ' ', 'B', 'o', 'b', ' ', 'M', 'a', 'x', 'i', 'm', 'u', 's', '\0'},
    .leadlength = 24,
    .pn = {0, 0x801, 0, 0x801, 0, 0x801, 0, 0x801, 0, 0x801, 0, 0x801,
           0, 0x801, 0, 0x801, 0, 0x801, 0, 0x801, 0, 0x801, 0, 0x003},
    .leadcount = 11,
    .huntbells = 0x001};

const ringing::Method Grandsire7 = ringing::Method{
    .stage = 7,
    .title = {'G', 'r', 'a', 'n', 'd', 's', 'i', 'r', 'e', ' ', 'T', 'r', 'i', 'p', 'l', 'e', 's', '\0'},
    .leadlength = 14,
    .pn = {0b000'0100, 0b000'0001, 0b100'0000, 0b000'0001, 0b100'0000, 0b000'0001, 0b100'0000,
           0b000'0001, 0b100'0000, 0b000'0001, 0b100'0000, 0b000'0001, 0b100'0000, 0b000'0001},
    .leadcount = 5,
    .huntbells = 0b0000011};

const ringing::Method Stedman5 = ringing::Method{
    .stage = 5,
    .title = {'S', 't', 'e', 'd', 'm', 'a', 'n', ' ', 'D', 'o', 'u', 'b', 'l', 'e', 's', '\0'},
    .leadlength = 12,
    .pn = {0b100, 0b001, 0b10'000, 0b100, 0b001, 0b100,
           0b001, 0b100, 0b10'000, 0b001, 0b100, 0b001},
    .leadcount = 5,
    .huntbells = 0b00000};

const ringing::Method Stedman7 = ringing::Method{
    .stage = 7,
    .title = {'S', 't', 'e', 'd', 'm', 'a', 'n', ' ', 'T', 'r', 'i', 'p', 'l', 'e', 's', '\0'},
    .leadlength = 12,
    .pn = {0b100, 0b001, 0b10'00'000, 0b100, 0b001, 0b100,
           0b001, 0b100, 0b10'00'000, 0b001, 0b100, 0b001},
    .leadcount = 7,
    .huntbells = 0b00000};
