#!/usr/bin/env python3
"""Native rules/storage/input and production LVGL application tests; no hardware."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
ENV = os.environ.copy()
SDK = Path('/Library/Developer/CommandLineTools/SDKs/MacOSX15.4.sdk')
if sys.platform == 'darwin' and SDK.exists():
    ENV.setdefault('SDKROOT', str(SDK))
BUILD = ROOT/'tests/build'
BUILD.mkdir(exist_ok=True)
INCLUDES = ['main', 'main/core', 'main/input', 'main/storage', 'main/game_manager', 'main/display', 'main/ui', 'tests/core_stubs']
FLAGS = ['-std=c++17', '-Wall', '-Wextra', '-Werror', '-g', '-fsanitize=address,undefined']
def run(args):
    subprocess.run(args, cwd=ROOT, env=ENV, check=True)
def compile_test(name, sources, extra=()):
    target=BUILD/name
    run(['c++', *FLAGS, *('-I'+str(ROOT/p) for p in INCLUDES), *extra, *(str(ROOT/p) for p in sources), '-o', str(target)])
    run([str(target)])

run([sys.executable, str(ROOT/'tests/backup_reader_test.py')])

# Compile the actual ESP-IDF FatFs core against a RAM disk with short names.
IDF = Path(ENV.get('IDF_PATH', Path.home()/'esp/esp-idf'))
FATFS = IDF/'components/fatfs/src'
fat_object = BUILD/'fatfs.o'
fat_includes = ['-I'+str(ROOT/'tests/fatfs_stubs'), '-I'+str(FATFS)]
run(['cc', '-std=c11', '-g', '-fsanitize=address,undefined', *fat_includes,
     '-c', str(FATFS/'ff.c'), '-o', str(fat_object)])
compile_test('fatfs', ['tests/fatfs_test.cpp', str(fat_object)], fat_includes)

compile_test('input', ['tests/input_test.cpp','main/input/input_state.cpp'])
compile_test('sd_mount', ['tests/sd_mount_test.cpp','main/storage/sd_card.cpp'], ['-I'+str(ROOT/'tests/sd_stubs')])
for game, test in [('block_drop','block'),('snake','snake'),('retro_racer','racer'),('space_invaders','invaders'),('space_impact','impact'),('breakout','breakout'),('frogger','frogger'),('pacman','pacman'),('bomberman','bomberman'),('sokoban','sokoban'),('asteroids','asteroids')]:
    compile_test(test, [f'tests/{test}_test.cpp',f'main/games/{game}/model.cpp'], ['-I'+str(ROOT/f'main/games/{game}')])
compile_test('storage', ['tests/storage_test.cpp','main/storage/journal.cpp','main/storage/records.cpp','main/storage/settings.cpp'])

file_sources = ['tests/file_storage_test.cpp', 'main/storage/file_backend.cpp',
                'main/storage/journal.cpp', 'main/storage/records.cpp', 'main/storage/settings.cpp',
                'main/game_manager/game_manager.cpp']
file_sources += [str(p.relative_to(ROOT)) for p in (ROOT/'main/games').glob('*/*.cpp')]
compile_test('file_storage', file_sources)

config=BUILD/'lv_conf.h'
config.write_text('''#pragma once
#define LV_CONF_H
#define LV_COLOR_DEPTH 16
#define LV_MEM_SIZE (8 * 1024 * 1024)
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_UNSCII_16 1
#define LV_USE_OS LV_OS_NONE
#define LV_USE_LOG 0
#define LV_USE_THORVG_INTERNAL 0
#define LV_BUILD_EXAMPLES 0
#define LV_BUILD_DEMOS 0
''')
library=BUILD/'lvgl'
cmake=['cmake','-S',str(ROOT/'managed_components/lvgl__lvgl'),'-B',str(library),'-DLV_BUILD_CONF_PATH='+str(config),'-DCONFIG_LV_BUILD_DEMOS=OFF','-DCONFIG_LV_BUILD_EXAMPLES=OFF','-DCONFIG_LV_USE_THORVG_INTERNAL=OFF']
if sys.platform=='darwin' and ENV.get('SDKROOT'):
    cmake.append('-DCMAKE_OSX_SYSROOT='+ENV['SDKROOT'])
for command in [cmake,['cmake','--build',str(library),'-j','8']]:
    output=subprocess.run(command,cwd=ROOT,env=ENV,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
    (BUILD/'lvgl-build.log').write_text(output.stdout)
    if output.returncode:
        print(output.stdout)
        raise SystemExit(output.returncode)
sources=['tests/application_test.cpp','main/core/application.cpp','main/ui/menu.cpp',
         'main/display/lvgl_canvas.cpp','main/input/input_state.cpp','main/storage/settings.cpp',
         'main/storage/journal.cpp','main/storage/records.cpp','main/game_manager/game_manager.cpp']
sources += [str(p.relative_to(ROOT)) for p in (ROOT/'main/games').glob('*/*.cpp')]
target=BUILD/'application'
stub_first=['-I'+str(ROOT/'tests/core_stubs')]
run(['c++',*FLAGS,*stub_first,*('-I'+str(ROOT/p) for p in INCLUDES),'-I'+str(ROOT/'managed_components/lvgl__lvgl'),f'-DLV_CONF_PATH="{config}"',
     *(str(ROOT/p) for p in sources),str(library/'lib/liblvgl.a'),'-o',str(target)])
run([str(target),str(BUILD/'retro-racer.ppm')])
print('All native tests passed. Physical Tab5 validation is separate.')
