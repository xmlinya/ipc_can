#
#  Copyright (c) 2012-2015 Texas Instruments Incorporated - http://www.ti.com
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  *  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#  *  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  *  Neither the name of Texas Instruments Incorporated nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
#  ======== products.mak ========
#

# look for other products.mak file to override local settings
ifneq (,$(wildcard $(EXBASE)/../products.mak))
include $(EXBASE)/../products.mak
else
ifneq (,$(wildcard $(EXBASE)/../../products.mak))
include $(EXBASE)/../../products.mak
# Define IPC_INSTALL_DIR since not defined in IPC top-level products.mak
IPC_INSTALL_DIR = $(word 1,$(subst /examples, examples,$(CURDIR)))
else
ifneq (,$(wildcard $(EXBASE)/../../../products.mak))
# Define IPC_INSTALL_DIR since not defined in IPC top-level products.mak
IPC_INSTALL_DIR = $(word 1,$(subst /examples, examples,$(CURDIR)))
include $(EXBASE)/../../../products.mak
endif
endif
endif

# By default, the necessary build variables are found/assigned via
# ../products.mak or ../../products.mak, included above.  If you want to
# override these variables, or are building this example without
# ../products.mak or ../../products.mak, uncomment and assign the variables
# below.

TOOLCHAIN_LONGNAME     = arm-linux-gnueabihf
TOOLCHAIN_INSTALL_DIR  = $(SDK_PATH_NATIVE)
TOOLCHAIN_PREFIX       = $(TOOLCHAIN_INSTALL_DIR)/usr/bin/$(TOOLCHAIN_LONGNAME)-

#XDC_INSTALL_DIR               =$(EXBASE)/ti-xdctools-tree
XDC_INSTALL_DIR               =$(SDK_PATH_TARGET)/usr/share/ti/ti-xdctools-tree
BIOS_INSTALL_DIR              =$(SDK_PATH_TARGET)/usr/share/ti/ti-sysbios-tree
IPC_INSTALL_DIR               =$(SDK_PATH_TARGET)/usr/share/ti/ti-ipc-tree

#please make it as soft link
PDK_INSTALL_DIR               =$(EXBASE)/pdk_dra7xx_1_0_10


gnu.targets.arm.A15F          =$(SDK_PATH_NATIVE)/usr/share/ti/gcc-arm-none-eabi
ti.targets.elf.C66            =$(SDK_PATH_NATIVE)/usr/share/ti/cgt-c6x
ti.targets.elf.C66_big_endian =$(SDK_PATH_NATIVE)/usr/share/ti/cgt-c6x
ti.targets.arm.elf.M4         =$(SDK_PATH_NATIVE)/usr/share/ti/ti-cgt-arm


# Use this goal to print your product variables.
.show:
	@echo "HOST TOOLCHAIN_PREFIX = $(TOOLCHAIN_PREFIX)"
	@echo "BIOS_INSTALL_DIR      = $(BIOS_INSTALL_DIR)"
	@echo "IPC_INSTALL_DIR       = $(IPC_INSTALL_DIR)"
	@echo "XDC_INSTALL_DIR       = $(XDC_INSTALL_DIR)"
	@echo "ti.targets.arm.elf.M4 = $(ti.targets.arm.elf.M4)"
	@echo "ti.targets.elf.C66    = $(ti.targets.elf.C66)"
