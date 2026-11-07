"""Backup reader accepts legacy keys and the FAT 8.3 Asteroids key."""
import importlib.util
from pathlib import Path
import struct
import zlib
spec = importlib.util.spec_from_file_location('backup_reader', Path(__file__).resolve().parents[1] / 'tools/read_backup.py')
reader = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reader)
def archive(key):
    payload = b'example'
    head = struct.pack('<5I', 0x35504341, 1, 2, 7, len(payload))
    record = head + struct.pack('<I', zlib.crc32(payload, zlib.crc32(head))) + payload
    encoded = key.encode('ascii')
    return struct.pack('<4I', 0x35504241, 1, len(encoded), len(record)) + encoded + record + bytes(8)
for key in ('configa', 'save9b', 'save10a', 'rank933a', 'rankA00a', 'rankA33b'):
    row, = reader.entries(archive(key))
    assert row[0] == key and row[2:] == (7, True)
for key in ('rankA44a', '../configa', 'rankaaa'):
    try:
        reader.entries(archive(key))
    except ValueError:
        pass
    else:
        raise AssertionError(key)
print('PASS backup reader: legacy and Asteroids keys, CRC and invalid names')
