#!/usr/bin/env python3
#
# Copyright (c) 2022 Joseph FAYE <joseph-wagane.faye@insa-rennes.fr>
# SPDX-License-Identifier: BSD-2-Clause
"""
   Script to build bare metal apps.
"""
import json
import os
import shutil
import argparse

_VALID_CORE_NAME = {'firev', 'femtorv'}
def extract_config(config_file, config):
    configuration = {}
    print(config_file)
    assert os.path.exists(config_file)
    with open(config_file, 'r') as f:
        data = json.load(f)
    print(data)
    config_dict = data[config]
    #print(config_dict)
    configuration["name_1"]          = config_dict.get('core_1').get("name")
    configuration["name_2"]          = config_dict.get('core_2').get("name")
    configuration['firm_1']     = config_dict.get("core_1").get("app")
    configuration['firm_2']     = config_dict.get("core_2").get("app")
    return configuration

def main():
    parser = argparse.ArgumentParser(description="LiteX Bare Metal AES App on AMP Architecture.")
    parser.add_argument("--config_file",    default='configs.json', help="config file path.")
    parser.add_argument("--config",         default='config_1',     help='configuration to choose.')
    parser.add_argument("--build_dir",      default='',             help="Base output dir.")
    parser.add_argument('--make_clean',     action="store_true",    help="Make clean command to firmware folders.")
    parser.add_argument('--make_clean_lib', action="store_true",    help="Make clean command to Mbdedtls library folders.")
    parser.add_argument("--with-cxx",       action="store_true",    help="Enable CXX support.")
    args = parser.parse_args()

    cwd = os.getcwd()
    print("MAKE_AND_BUILD_INFO : Program working directory : {}".format(cwd))

    if not args.build_dir:
        args.build_dir = os.path.join(cwd, 'build')
    else:
        args.build_dir = os.path.join(cwd, args.build_dir)

    args.config_file = os.path.join(cwd, args.config_file)
    configuration = extract_config(args.config_file, args.config)
   
    if args.make_clean_lib:
        os.system(f"cp mbedtls_config.h ext_libs/mbedtls/include/mbedtls")
        os.chdir(os.path.join(os.path.join(cwd, 'ext_libs', 'mbedtls')))
        print("MAKE_AND_BUILD_INFO : Move in {}".format(os.getcwd()))

        os.system("CC=riscv64-unknown-elf-gcc CFLAGS='-std=gnu99 -Wall -Os -Wextra -march=rv32im -mabi=ilp32 "
                    "-D__vexriscv__ -MMD' make clean")

        os.system("CC=riscv64-unknown-elf-gcc CFLAGS='-std=gnu99 -Wall -Os -Wextra -march=rv32im -mabi=ilp32 "
                    "-D__vexriscv__ -MMD' make lib")
    os.chdir(cwd)

    # Check argument validity

    assert configuration['name_1'] in _VALID_CORE_NAME
    assert configuration['name_2'] in _VALID_CORE_NAME

    # Set 'archi' variable
    if configuration['name_1'] != configuration['name_2']:
        archi = "Heterogen"
    else:
        archi = "Homogen"

    # Set firmware directories
    if args.config == 'config_1' or args.config == 'config_8':
        firm_dir = os.path.join(cwd, 'firmwares', 'config_1_8')
    elif args.config == 'config_2' or args.config == 'config_6':
        firm_dir = os.path.join(cwd, 'firmwares', 'config_2_6')
    elif args.config == 'config_3' or args.config == 'config_4':
        firm_dir = os.path.join(cwd, 'firmwares', 'config_3_4')
    elif args.config == 'config_5' or args.config == 'config_7':
        firm_dir = os.path.join(cwd, 'firmwares', 'config_5_7')
    elif args.config == 'baseline_1' or args.config == 'baseline_2':
        firm_dir = os.path.join(cwd, 'firmwares', 'baseline_1_2')
    else:
        firm_dir = os.path.join(cwd, 'firmwares', 'dummy')

    firm0 = configuration['firm_1']
    firm1 = configuration['firm_2']

    # Set content for crt0.d file
    os.chdir(os.path.join(firm_dir, firm0))

    with open("crt0.d", 'w', encoding='utf-8') as f:
        f.write("crt0.o: /opt/litex_root/litex/litex/soc/cores/cpu/{}/crt0.S".format(configuration['name_1']))

    os.chdir(os.path.join(firm_dir, firm1))
    with open("crt0.d", 'w', encoding='utf-8') as f:
        f.write("crt0.o: /opt/litex_root/litex/litex/soc/cores/cpu/{}/crt0.S".format(configuration['name_2']))

    # Set build path

    if archi == "Heterogen":
        core_0_build_path = os.path.join(cwd, args.build_dir, "{}_soc".format(configuration['name_1']))
        core_1_build_path = os.path.join(cwd, args.build_dir, "{}_soc".format(configuration['name_2']))

    else:
        core_0_build_path = os.path.join(cwd, args.build_dir, "{}_soc_0".format(configuration['name_1']))
        core_1_build_path = os.path.join(cwd, args.build_dir, "{}_soc_1".format(configuration['name_2']))

    for build_path, firmware in zip([core_0_build_path, core_1_build_path], [firm0, firm1]):
        print("*******************************************************************************************************")
        print(f"Firmware {firmware}")
        # Compile firmware
        build_path = build_path if os.path.isabs(build_path) else os.path.join("..", build_path)
        print("MAKE_AND_BUILD_INFO : Build path {}".format(build_path))
        os.chdir(os.path.join(firm_dir, f"{firmware}"))
        print(os.getcwd())
        if args.make_clean:
            os.system(f"export BUILD_DIR={build_path} && {'export WITH_CXX=1 &&' if args.with_cxx else ''} "
                      f"make clean ")
            os.system(f"export BUILD_DIR={build_path} && {'export WITH_CXX=1 &&' if args.with_cxx else ''} "
                      f"make all ")
        else:
            os.system(f"export BUILD_DIR={build_path} && {'export WITH_CXX=1 &&' if args.with_cxx else ''} "
                      f"make all ")
        os.chdir(cwd)
        print("*******************************************************************************************************")

if __name__ == "__main__":
    main()
