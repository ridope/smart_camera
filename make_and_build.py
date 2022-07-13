#!/usr/bin/env python3
#
# Copyright (c) 2022 Joseph FAYE <joseph-wagane.faye@insa-rennes.fr>
# SPDX-License-Identifier: BSD-2-Clause
"""Par contre, change moi ce nom de fichier.
   Script to build bare metal apps, and construct the platform to load with the firmwares for each core.
   It helps build the platform; build the firmwares app and load the code to the board
"""
import os
import argparse

_VALID_CORE_NAME = {'firev', 'femtorv'}
def main():
    parser = argparse.ArgumentParser(description="LiteX Bare Metal AES App on AMP architecture.")
    parser.add_argument("--core_0", default='firev', type=str, help="Core 0 name.", required=True)
    parser.add_argument("--core_1", default='femtorv', type=str, help="Core 1 name", required=True)
    parser.add_argument('--make_clean', action="store_true", help="Make clean command to firmware folders.")
    parser.add_argument("--build", action="store_true", help="Build the target platform.")
    parser.add_argument("--mux", default=False, help="Build the SoC with double output UART or shared UART")
    parser.add_argument("--load", action="store_true", help="Load the code to the board")
    parser.add_argument("--litex_term, action", help="Launch litex_term")
    parser.add_argument("--with-cxx",   action="store_true", help="Enable CXX support.")
    args = parser.parse_args()

    cwd = os.getcwd()

    # Build Platform

    if args.build:
        if args.core_0 != args.core_1:
            archi = "Heterogen"
            soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop/Dual_Core/{archi}/Fire_Femto"
        else:
            archi = "Homogen"
            if args.core0 == 'firev':
                soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop/Dual_Core/{archi}/Fire_Fire"
            else:
                soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop/Dual_Core/{archi}/Femto_Femto"
        os.chdir(os.path.join(cwd, soc_path))
        os.system(f"./ridope_soc.py --build --mux {args.mux}")
        os.chdir(cwd)

    # Load Platform

    if args.load:
        if args.core_0 != args.core_1:
            archi = "Heterogen"
            soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop//Dual_Core/{archi}/Fire_Femto"
        else:
            archi = "Homogen"
            if args.core0 == 'firev':
                soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop/Dual_Core/{archi}/Fire_Fire"
            else:
                soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop//Dual_Core/{archi}/Femto_Femto"
        os.chdir(os.path.join(cwd, soc_path))
        os.system(f"./ridope_soc.py --load")
        os.chdir(cwd)


    # Check argument validity
    assert args.core_0 in _VALID_CORE_NAME
    assert args.core_1 in _VALID_CORE_NAME

    # Set firmware directories

    firm0 = "core_0_firmware"
    os.makedirs(firm0, exist_ok=True)
    firm1 = "core_0_firmware"
    os.makedirs(firm1, exist_ok=True)


    # Set content for crt0.d file
    os.chdir(os.path.join(cwd, firm0))
    with open("crt0.d", 'w', encoding='utf-8') as f:
        f.write("crt0.o: /opt/litex_root/litex/litex/soc/cores/cpu/{}/crt0.S".format(args.core_0))

    os.chdir(os.path.join(cwd, firm1))
    with open("crt0.d", 'w', encoding='utf-8') as f:
        f.write("crt0.o: /opt/litex_root/litex/litex/soc/cores/cpu/{}/crt0.S".format(args.core_1))

    if not args.build:
        if args.core_0 != args.core_1:
            archi = "Heterogen"
            soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop//Dual_Core/{archi}/Fire_Femto"
        else:
            archi = "Homogen"
            if args.core0 == 'firev':
                soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop/Dual_Core/{archi}/Fire_Fire"
            else:
                soc_path = f"ext_libs/Asymetric-Multi-Processing/Anisotrop//Dual_Core/{archi}/Femto_Femto"

    # Set build path
    if archi == "Heterogen":
        core_0_build_path = os.path.join(cwd, soc_path, "build", "{}_soc".format(args.core_0))
        core_1_build_path = os.path.join(cwd, soc_path, "build", "{}_soc".format(args.core_1))

    else:
        core_0_build_path = os.path.join(cwd, soc_path, "build", "{}_soc_0".format(args.core_0))
        core_1_build_path = os.path.join(cwd, soc_path, "build", "{}_soc_1".format(args.core_1))

    for build_path, i in zip([core_1_build_path, core_0_build_path], range(2)):
        os.system(f"cp mbedtls_config.h ext_lib/include/mbedtls")

        os.system("CC=riscv64-unknown-elf-gcc CFLAGS='-std=gnu99 -Wall -Wextra -march=rv32im -mabi=ilp32 "
                  "-D__vexriscv__ -MMD' make lib")
        # Compile firmware
        if args.make_clean:
            os.system(f"cd core_{i}_firmware && make clean")
        build_path = build_path if os.path.isabs(build_path) else os.path.join("..", build_path)
        os.system(f"export BUILD_DIR={build_path} && {'export WITH_CXX=1 &&' if args.with_cxx else ''} "
                  f"cd core_{i}_firmware && make")

    # Copy demo.bin
    # os.system("cp demo/demo.bin ./")

if __name__ == "__main__":
    main()
