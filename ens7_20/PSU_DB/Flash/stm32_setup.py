from com.arm.debug.flashprogrammer.IFlashClient import MessageLevel

from flashprogrammer.device import ensureDeviceOpen
from flashprogrammer.execution import ensureDeviceStopped
from flashprogrammer.device_memory import writeToTarget, readFromTarget, intToBytes, intFromBytes

import time

RCC_BASE = 0x40021000 # base address for clock control block
RCC_CR = RCC_BASE + 0  # Clock control register
RCC_CFGR = RCC_BASE + 4  # Clock configuration register
RCC_CIR = RCC_BASE + 8  # Clock interrupt register

SYST_CSR = 0xE000E010 # SysTick Control and Status Register

def setup(client, services):
    # get a connection to the core
    conn = services.getConnection()
    dev = conn.getDeviceInterfaces().get("Cortex-M3")
    ensureDeviceOpen(dev)
    ensureDeviceStopped(dev)

    # The Keil flash algorithms for the STM32 devices change the 
    # wait state for the flash to 0 cycles - this requires SYSCLK to be <24Mhz
    # set the clock mode to use the internal 8MHz clock (HSI)

    # Enable HSI     
    writeToTarget(dev, RCC_CR, intToBytes(0x81))

    # use HSI, set prescalers for AHB,APB to 1
    rccCfg = intFromBytes(readFromTarget(dev, RCC_CFGR, 4))
    #  bits 1:0 = 0 for HSI, 13:11 = 0 for PPRE2=1, 10:8 = 0 for PPRE1 = 1, 7:4 = 0 for HPRE = 1
    rccCfg = rccCfg & ~(0x3FF3)
    writeToTarget(dev, RCC_CFGR, intToBytes(rccCfg))

    # clear and disable all clock interrupts
    writeToTarget(dev, RCC_CIR, intToBytes(0x00FF0000))

    # disable system timer
    writeToTarget(dev, SYST_CSR, intToBytes(0))