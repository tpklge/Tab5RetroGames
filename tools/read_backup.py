#!/usr/bin/env python3
"""Inspect/extract a Tab5 complementary SD backup; validate journal CRCs."""
import argparse
from pathlib import Path
import re
import struct
import zlib

def entries(data):
    if len(data) < 16 or struct.unpack_from('<II', data) != (0x35504241, 1):
        raise ValueError('Unknown backup header or version')
    offset = 8
    found = set()
    result = []
    while True:
        if offset + 8 > len(data):
            raise ValueError('Truncated archive')
        key_size, size = struct.unpack_from('<II', data, offset)
        offset += 8
        if key_size == size == 0:
            if offset != len(data):
                raise ValueError('Trailing archive data')
            return result
        if not 1 <= key_size <= 15 or not 24 <= size <= 65560 or offset + key_size + size > len(data):
            raise ValueError('Invalid record lengths')
        key = data[offset:offset + key_size].decode('ascii')
        offset += key_size
        record = data[offset:offset + size]
        offset += size
        if not re.fullmatch(r'(config|save\d+|rank\d+)[ab]', key) or key in found:
            raise ValueError('Invalid or duplicate storage key')
        found.add(key)
        magic, version, kind, generation, length, crc = struct.unpack_from('<6I', record)
        actual = zlib.crc32(record[24:], zlib.crc32(record[:20]))
        valid = magic == 0x35504341 and version == 1 and kind in (1, 2, 3) and length == size - 24 and crc == actual
        result.append((key, record, generation, valid))

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('backup', type=Path)
    parser.add_argument('--extract', type=Path)
    args = parser.parse_args()
    data = entries(args.backup.read_bytes())
    if args.extract:
        args.extract.mkdir(parents=True, exist_ok=True)
    for key, record, generation, valid in data:
        print(f'{key}: {len(record)} bytes, generation {generation}, CRC {"OK" if valid else "INVALID"}')
        if args.extract and valid:
            (args.extract / (key + '.bin')).write_bytes(record)
    return 0 if all(row[3] for row in data) else 1

if __name__ == '__main__':
    raise SystemExit(main())
