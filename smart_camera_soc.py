#!/usr/bin/env python3

# This file is part of LiteX-Boards.
# Copyright (c) 2019 msloniewski <marcin.sloniewski@gmail.com>
# Modified 2021 by mpelcat <mpelcat@insa-rennes.fr>
# Modified 2022 by lesteves <lesteves@insa-rennes.fr>
# SPDX-License-Identifier: BSD-2-Clause


import sys

# caution: path[0] is reserved for script path (or '' in REPL)
sys.path.insert(1, './camera_files')
sys.path.insert(1, './ext_libs/Asymetric-Multi-Processing/Dual_Core')


import os
import argparse

from litex.build import io
from litex.soc.doc import generate_docs, generate_svd
from litex.soc.cores.gpio import GPIOOut, GPIOTristate
from litex.soc.cores.led import LedChaser
from litex_boards.platforms.muselab_icesugar import led_pmod_io_v11

from migen import *
from migen.genlib.cdc import MultiReg
from migen.genlib.resetsync import AsyncResetSynchronizer

from litex_boards.platforms import terasic_de10lite # referencing the platform

from litex.soc.cores.clock import Max10PLL
from litex.soc.integration.soc import SoCRegion
from litex.soc.integration.soc_core import *
from litex.soc.integration.builder import *
from litex.build.generic_platform import *

from litex.soc.interconnect.csr import *
from litex.soc.interconnect import wishbone
import litex.soc.doc as lxsocdoc

from camera_d8m import Camera_D8M
from memlogic import MemLogic
from amp import BaseSoC, extract_config

def main():
    from litex.soc.integration.soc import LiteXSoCArgumentParser
    parser = LiteXSoCArgumentParser(description="LiteX AMP Dual-Core SoC generator on De10Lite")
    target_group = parser.add_argument_group(title="Target options")
    target_group.add_argument("--platform",       default=terasic_de10lite.Platform())
    target_group.add_argument("--toolchain",      default="quartus",           help="FPGA toolchain (vivado, symbiflow or yosys+nextpnr).")
    target_group.add_argument("--sys-clk-freq",   default=50e6,                help="System clock frequency.")
    target_group.add_argument("--bus_data_width", default=16,                  help="Super SoC bus data width.")
    target_group.add_argument('--config_file',    help='Configuration file',   required=True)
    target_group.add_argument('--config',         help='Configuration number', required=True)
    target_group.add_argument("--build",          action="store_true",         help="Build bitstream.")
    target_group.add_argument("--build_dir",      default='',                  help="Base output directory.")
    target_group.add_argument("--load",           action="store_true",         help="Load bitstream.")
    target_group.add_argument("--mux",            default=False,               help="use uart mux.")
    builder_args(parser)
    args = parser.parse_args()

    configuration = extract_config(args.config_file, args.config)

    args.platform.add_source_dir(path="./camera_files/Camera")

    args.platform.add_extension([
                        ("arduino_serial", 0,
                            Subsignal("tx", Pins("AA19"), IOStandard("3.3-V LVTTL")), # Arduino IO11
                            Subsignal("rx", Pins("Y19"), IOStandard("3.3-V LVTTL"))  # Arduino IO12
                        ),
                        ("arduino_serial", 1,
                            Subsignal("tx", Pins("AB20"), IOStandard("3.3-V LVTTL")), # Arduino IO11
                            Subsignal("rx", Pins("F16"), IOStandard("3.3-V LVTTL"))  # Arduino IO12
                        ),
                        ])

    args.platform.add_extension([("d8m", 0,
                        Subsignal("mipi_d", Pins(
                            "W9 V8 W8 V7 W7 W6 V5 W5",
                            "AA15 AA14")),
                        Subsignal("mipi_rst_n",   Pins("AA8")),
                        Subsignal("mipi_clk", Pins("W10")),
                        Subsignal("mipi_hs",    Pins("AA9")),
                        Subsignal("mipi_vs",  Pins("AB10")),
                        Subsignal("mipi_cs_n",   Pins("Y8")),
                        Subsignal("mipi_ref_clk", Pins("AB11")),
                        Subsignal("mipi_scl", Pins("AA5")),
                        Subsignal("mipi_sda", Pins("Y4")),
                        Subsignal("cam_pwdn_n",  Pins("Y7")),
                        Subsignal("cam_scl", Pins("AA7")),
                        Subsignal("cam_sda", Pins("Y6")), 
                        Subsignal("mipi_clk_rsd", Pins("V10")), 
                        Subsignal("mipi_mclk", Pins("AA6")),
                        Subsignal("cam_resv", Pins("AB2")),  
                        IOStandard("3.3-V LVTTL")
                    )])

    soc = BaseSoC(
        platform_name  = 'De10Lite',
        platform       = args.platform,
        toolchain      = args.toolchain,
        sys_clk_freq   = int(float(args.sys_clk_freq)),
        bus_data_width=  int(args.bus_data_width),
        mux            = args.mux,
        build_dir      = args.build_dir,
        shared_ram_size= configuration['shared_ram_size'],
        name_1         = configuration['name_1'],
        name_2         = configuration['name_2'],
        sram_1_size    = configuration['sram_1_size'],
        sram_2_size    = configuration['sram_2_size'],
        ram_1_size     = configuration['ram_1_size'],
        ram_2_size     = configuration['ram_2_size'],
        rom_1_size     = configuration['rom_1_size'],
        rom_2_size     = configuration['rom_2_size'],
        sp_1_size      = configuration['sp_1_size'],
        sp_2_size      = configuration['sp_2_size'],
    )

    soc.crg.clock_domains.cd_d8m = ClockDomain()
    soc.crg.clock_domains.cd_vga = ClockDomain()
    soc.crg.clock_domains.cd_sdram = ClockDomain()
    soc.crg.clock_domains.cd_sdram_ps = ClockDomain()

    soc.crg.pll.create_clkout(soc.crg.cd_sdram, int(float(args.sys_clk_freq))*2)
    soc.crg.pll.create_clkout(soc.crg.cd_sdram_ps, int(float(args.sys_clk_freq))*2, phase=-108)
    soc.crg.pll.create_clkout(soc.crg.cd_vga,  int(float(args.sys_clk_freq))/2)
    soc.crg.pll.create_clkout(soc.crg.cd_d8m,  20e6)

    soc.submodules.camera = Camera_D8M(args.platform)
    soc.add_csr("camera")

    soc.submodules.logicmem = MemLogic(soc)

    # Writing in the scratch-pad mem
    addr = Signal(32)
    write_data = Signal(32)
    logic_counter = Signal(max=3)

    soc.sync.vga += [
        If(soc.camera.read_request==1,
            write_data.eq(Cat(write_data[8:32],soc.camera.r_auto)),
            If(logic_counter == 3,
                soc.logicmem.logic_write_data.eq(write_data),
                soc.logicmem.local_adr.eq(addr),
                addr.eq(addr+1),
                logic_counter.eq(0),
            ).Else(
                logic_counter.eq(logic_counter+1),
            )     
            
        ),

        If(soc.camera.framedone_vga==1,
            addr.eq(0),
            logic_counter.eq(0)
        )
    ]

    args.output_dir = os.path.join(args.build_dir, soc.platform.name) if args.build_dir else ''
    builder = Builder(soc, **builder_argdict(args))
    builder_kwargs = {}
    builder.build(**builder_kwargs, run=args.build)

    if args.load:
        prog = soc.platform.create_programmer()
        prog.load_bitstream(builder.get_bitstream_filename(mode="sram"))

    lxsocdoc.generate_docs(soc, f"{os.path.join(args.build_dir, 'documentation') if args.build_dir else 'build/documentation'}", project_name="Smart Camera SoC", author="Lucas Esteves Rocha")

if __name__ == "__main__":
    main()