#!/usr/bin/env python3
#
# Copyright (c) 2022 Joseph FAYE <joseph-wagane.faye@insa-rennes.fr>
# SPDX-License-Identifier: BSD-2-Clause
"""
   Script to build bare metal apps.
"""
import os
import shutil
import argparse

_VALID_CORE_NAME = {'firev', 'femtorv'}

def main():
    parser = argparse.ArgumentParser(description="LiteX Bare Metal AES App on AMP Architecture.")
    parser.add_argument("--core_0", type=str, help="Core 0 name.", required=True)
    parser.add_argument("--core_1",  type=str, help="Core 1 name", required=True)
    parser.add_argument("--build_dir", type=str, help="Build the target platform.", required=True)
    parser.add_argument('--make_clean', action="store_true", help="Make clean command to firmware folders.")
    parser.add_argument('--make_clean_lib', action="store_true", help="Make clean command to Mbdedtls library folders.")
    parser.add_argument("--with-cxx",   action="store_true", help="Enable CXX support.")
    args = parser.parse_args()

    cwd = os.getcwd()
    print("MAKE_AND_BUILD_INFO : Program working directory : {}".format(cwd))
   
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

    assert args.core_0 in _VALID_CORE_NAME
    assert args.core_1 in _VALID_CORE_NAME

    # Set 'archi' variable
    if args.core_0 != args.core_1:
        archi = "Heterogen"
    else:
        archi = "Homogen"

    # Set firmware directories

    firm0 = "core_0_firmware"
    os.makedirs(firm0, exist_ok=True)
    firm1 = "core_1_firmware"
    os.makedirs(firm1, exist_ok=True)

    # Set content for crt0.d file
    os.chdir(os.path.join(cwd, firm0))

    with open("crt0.d", 'w', encoding='utf-8') as f:
        f.write("crt0.o: /opt/litex_root/litex/litex/soc/cores/cpu/{}/crt0.S".format(args.core_0))

    os.chdir(os.path.join(cwd, firm1))
    with open("crt0.d", 'w', encoding='utf-8') as f:
        f.write("crt0.o: /opt/litex_root/litex/litex/soc/cores/cpu/{}/crt0.S".format(args.core_1))

    # Set build path

    if archi == "Heterogen":
        core_0_build_path = os.path.join(cwd, args.build_dir, "{}_soc".format(args.core_0))
        core_1_build_path = os.path.join(cwd, args.build_dir, "{}_soc".format(args.core_1))

    else:
        core_0_build_path = os.path.join(cwd, args.build_dir, "{}_soc_0".format(args.core_0))
        core_1_build_path = os.path.join(cwd, args.build_dir, "{}_soc_1".format(args.core_1))

    for build_path, i in zip([core_0_build_path, core_1_build_path], range(2)):
        print("*******************************************************************************************************")
        print("Firmware {}".format(i))
        # Compile firmware
        build_path = build_path if os.path.isabs(build_path) else os.path.join("..", build_path)
        print("MAKE_AND_BUILD_INFO : Build path {}".format(build_path))
        os.chdir(os.path.join(cwd, f"core_{i}_firmware"))
        os.system(f"export BUILD_DIR={build_path} && {'export WITH_CXX=1 &&' if args.with_cxx else ''} "
                  f" {'make clean' if args.make_clean else 'make all'}")
        os.chdir(cwd)
        print("*******************************************************************************************************")


if __name__ == "__main__":
    main()
