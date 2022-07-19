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
    parser.add_argument("--core_0", default='firev', type=str, help="Core 0 name.", required=True)
    parser.add_argument("--core_1", default='femtorv', type=str, help="Core 1 name", required=True)
    parser.add_argument("--build", action="store_true", help="Build the target platform.")
    parser.add_argument("--build_force", action="store_true", help="Build even if the build directory exists")
    parser.add_argument("--mux", default=False, help="Build the SoC with double output UART or shared UART")
    parser.add_argument("--load", action="store_true", help="Load the code to the board")
    args = parser.parse_args()

    cwd = os.getcwd()

    build_dir = os.path.join(cwd, 'build')

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
            if os.path.isdir('build'):
                print("MAKE_AND_BUILD_INFO : Build already exists and will be deleted.")
                shutil.rmtree(build_dir)
            os.chdir(os.path.join(cwd, soc_path))
            os.system(f"./ridope_soc.py --build --mux {args.mux} --build_dir {build_dir}")
            os.chdir(cwd)
        else:
            if os.path.isdir('build'):
                pass
            else:
                os.chdir(os.path.join(cwd, soc_path))
                os.system(f"./ridope_soc.py --build --mux {args.mux} --build_dir {build_dir}")
                os.chdir(cwd)

if __name__ == "__main__":
    main()