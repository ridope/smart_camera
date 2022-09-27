#!/usr/bin/env python3

# This file is part of Ridope project.
# Modified 2022 by lesteves <lesteves@insa-rennes.fr>
# SPDX-License-Identifier: BSD-2-Clause

from doctest import master
from migen import *

from litex.soc.cores import uart
from litex.soc.interconnect import wishbone
from litex.soc.interconnect import stream

class MemLogic(Module):

    def __init__(self, amp_soc):

        data_width = amp_soc.bus.data_width

        # A Float image has a size of -> 28x28 = 784, since the bus is addressed as 32bit, no need to multiply by four the size.
        # The char image will start at the base address 784 (0XC40).
        addr_base = 784
        
        self.logic_write_data = Signal(data_width)
        self.local_adr = Signal(30)

        logic_write_enable_signal = Signal()

        pre_write_data = Signal(data_width)
        pre_local_adr = Signal(30) 
       
        mem_if = wishbone.Interface()      

        amp_soc.bus.add_master(master=mem_if)

        # FSM.
        self.submodules.fsm = fsm = FSM(reset_state="IDLE")
        
        fsm.act("IDLE",
            NextValue(mem_if.cyc, 0),
            NextValue(mem_if.stb, 0),
            NextValue(mem_if.we, 0),
            NextValue(mem_if.adr, addr_base + self.local_adr),
            NextValue(mem_if.dat_w, self.logic_write_data),
            NextValue(mem_if.sel, 15),

            If(logic_write_enable_signal == 1,
                NextState("BUS-REQUEST")
            )
        )

        fsm.act("BUS-REQUEST",
            NextValue(mem_if.cyc, 1),
            NextValue(mem_if.stb, 1),
            NextValue(mem_if.we, 1),
            If(mem_if.ack == 1,
                NextState("IDLE")
            )
        )

        # Write Logic
        self.sync += [
            If((pre_write_data != self.logic_write_data) | (pre_local_adr != self.local_adr),
                pre_write_data.eq(self.logic_write_data),
                pre_local_adr.eq(self.local_adr),
                logic_write_enable_signal.eq(1),
            ).Else(
                logic_write_enable_signal.eq(0)
            )
        ]

