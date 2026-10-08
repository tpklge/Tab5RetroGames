#!/usr/bin/env python3
"""Package already-built ESP32-P4 artifacts; never accesses a serial port."""
import hashlib
import json
from pathlib import Path
import shutil
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / 'build'
OUT = ROOT / 'newversion'
OUT.mkdir(exist_ok=True)
description = json.loads((BUILD / 'project_description.json').read_text())
layout = json.loads((BUILD / 'flasher_args.json').read_text())
app = BUILD / description['app_bin']
image = app.read_bytes()
if description['target'] != 'esp32p4' or image[0] != 0xe9:
    raise SystemExit('Expected an ESP32-P4 application image.')
if struct.unpack_from('<H', image, 12)[0] != 18:
    raise SystemExit('ESP image chip ID must be ESP32-P4 (18).')
limit = 1600 * 1024
if len(image) > limit:
    raise SystemExit(f'App exceeds conservative Launcher slot: {len(image)} > {limit}.')
info = subprocess.run([sys.executable, '-m', 'esptool', '--chip', 'esp32p4',
                       'image_info', '--version', '2', str(app)],
                      text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=True)
(OUT / 'image-info.txt').write_text(info.stdout)
ota = OUT / 'tab5_retro_arcade-ota.bin'
shutil.copy2(app, ota)
parts = sorted((int(offset, 0), BUILD / filename)
               for offset, filename in layout['flash_files'].items())
merged = bytearray(b'\xff' * max(offset + file.stat().st_size for offset, file in parts))
end = 0
for offset, file in parts:
    if offset < end:
        raise SystemExit('Overlapping flash components.')
    data = file.read_bytes()
    merged[offset:offset + len(data)] = data
    end = offset + len(data)
usb = OUT / 'tab5_retro_arcade-usb-full.bin'
usb.write_bytes(merged)
manifest = {
    'project': description['project_name'], 'version': description['project_version'],
    'target': description['target'], 'idf': description['git_revision'],
    'ota': {'file': ota.name, 'bytes': len(image), 'format': 'application-only',
            'conservative_slot_limit_bytes': limit,
            'destination_offset': 'selected by installed Launcher partition table'},
    'usb': {'file': usb.name, 'bytes': len(merged), 'write_offset': '0x0',
            'format': 'merged bootloader + standalone partition table + application',
            'components': layout['flash_files']},
    'hardware_validation': 'pending; no flash/serial access performed',
    'files': {}
}
for file in (ota, usb):
    manifest['files'][file.name] = hashlib.sha256(file.read_bytes()).hexdigest()
(OUT / 'manifest.json').write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + '\n')
(OUT / 'SHA256SUMS.txt').write_text(''.join(f'{sha}  {name}\n' for name, sha in manifest['files'].items()))
print(f'OTA: {ota} ({len(image)} bytes)')
print(f'USB: {usb} ({len(merged)} bytes, offset 0x0; separate layout)')
