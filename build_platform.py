#!/usr/bin/env python3
#
# Copyright (c) 2022 Joseph FAYE <joseph-wagane.faye@insa-rennes.fr>
# SPDX-License-Identifier: BSD-2-Clause
"""
   Script to build Anisotropic platform
"""
import os
import shutil
import argparse

def main():
    parser = argparse.ArgumentParser(description="LiteX Bare Metal AES App on AMP Architecture.")
    parser.add_argument("--core_0", default='firev', type=str, help="Core 0 name.")
    parser.add_argument("--core_1", default='femtorv', type=str, help="Core 1 name")
    parser.add_argument("--build", action="store_true", help="Build the target platform.")
    parser.add_argument("--build_force", action="store_true", help="Build even if the build directory exists.")
    parser.add_argument("--mux", default=False, help="Build the SoC with double output UART or shared UART.")
    parser.add_argument("--shared_ram_size", default=0x100, help="Shared RAM size value.")
    parser.add_argument("--sram_size", default=0x1000, help="SRAM size value, for the two cores.")
    parser.add_argument("--main_ram_size", default=0x4000, help="Main RAM size value, for the two cores.")
    parser.add_argument("--build_dir", default='', help="Base output dir.")
    parser.add_argument("--load", action="store_true", help="Load the code to the board.")
    args = parser.parse_args()

    cwd = os.getcwd()

    if not args.build_dir:
        args.build_dir = os.path.join(cwd, 'build')

    assert type(args.sram_size) == type(args.shared_ram_size)

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

        if args.build_force:
            if os.path.isdir(args.build_dir):
                print("MAKE_AND_BUILD_INFO : Build already exists and will be deleted.")
                shutil.rmtree(args.build_dir)
            os.chdir(os.path.join(cwd, soc_path))
            os.system(f"./ridope_soc.py --build --mux {args.mux} --build_dir {args.build_dir} "
                      f"--sram_size {args.sram_size} --main_ram_size {args.main_ram_size} "
                      f"--shared_ram_size {args.shared_ram_size}")
            os.chdir(cwd)
        else:
            if os.path.isdir(args.build_dir):
                pass
            else:
                os.chdir(os.path.join(cwd, soc_path))
                os.system(
                    f"./ridope_soc.py --build --mux {args.mux} --build_dir {args.build_dir} --sram_size {args.sram_size}  "
                    f"--main_ram_size {args.main_ram_size} "
                    f"--shared_ram_size {args.shared_ram_size}")
                os.chdir(cwd)

if __name__ == "__main__":
    main()