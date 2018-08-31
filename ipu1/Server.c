/*
 * Copyright (c) 2013-2014, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== Server.c ========
 *
 */

/* this define must precede inclusion of any xdc header file */
#define Registry_CURDESC Test__Desc
#define MODULE_NAME "Server"

/* xdctools header files */
#include <xdc/std.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Registry.h>

#include <stdio.h>

/* package header files */
#include <ti/ipc/MessageQ.h>
#include <ti/ipc/MultiProc.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

//csl
#include <ti/csl/csl_gpio.h>
#if defined (SOC_DRA72x) 
#include <ti/board/board.h>
#endif
#include <ti/csl/soc.h>
#include <ti/csl/hw_types.h>

/* local header files */
#include "../shared/AppCommon.h"
#include <ti/osal/RegisterIntr.h>


/* module header file */
#include "Server.h"

/* module structure */
typedef struct {
    UInt16              hostProcId;         // host processor id
    MessageQ_Handle     slaveQue;           // created locally
} Server_Module;

/* private data */
Registry_Desc               Registry_CURDESC;
static Server_Module        Module;

#define VIRT_PHY_OFFSET (0x20000000)

/*
 * ======== Clock configuration and Pin Muxing for DCAN1 ========
 */
void DCAN1Prcm_clck_ram()
{
    uint32_t status;
	uint32_t i = 0;
#if defined (SOC_TDA2XX) || defined (SOC_TDA2PX) || defined (SOC_TDA2EX) || defined (SOC_DRA72x) || defined (SOC_DRA75x)
    /*CM_WKUPAON_DCAN1_CLKCTRL*/
    HW_WR_FIELD32(SOC_WKUPAON_CM_BASE + CM_WKUPAON_DCAN1_CLKCTRL + VIRT_PHY_OFFSET,
                CM_WKUPAON_DCAN1_CLKCTRL_MODULEMODE,
                CM_WKUPAON_DCAN1_CLKCTRL_MODULEMODE_ENABLE);

	Log_print0(Diags_INFO,"==========a wait enter 1");
    while ((HW_RD_FIELD32(SOC_WKUPAON_CM_BASE + CM_WKUPAON_DCAN1_CLKCTRL + VIRT_PHY_OFFSET,
                CM_WKUPAON_DCAN1_CLKCTRL_IDLEST)) ==
                CM_WKUPAON_DCAN1_CLKCTRL_IDLEST_DISABLE)
            {
                ;
            }
    Log_print0(Diags_INFO,"==========a wait leave 1");

    /*pin mux for dcan1*/
    HW_WR_REG32(SOC_CORE_PAD_IO_REGISTERS_BASE+CTRL_CORE_PAD_DCAN1_TX + VIRT_PHY_OFFSET,0x000E0000);
    HW_WR_REG32(SOC_CORE_PAD_IO_REGISTERS_BASE+CTRL_CORE_PAD_WAKEUP0 + VIRT_PHY_OFFSET,0x00010001);

    /* Clear the start bit so that pulse is generated when run second time */
    HW_WR_FIELD32((uint32_t) SOC_CTRL_MODULE_CORE_CORE_REGISTERS_BASE +
                CTRL_CORE_CONTROL_IO_2 + VIRT_PHY_OFFSET,
                CTRL_CORE_CONTROL_IO_2_DCAN1_RAMINIT_START,
                0U);

    HW_WR_FIELD32((uint32_t) SOC_CTRL_MODULE_CORE_CORE_REGISTERS_BASE +
                CTRL_CORE_CONTROL_IO_2 + VIRT_PHY_OFFSET,
                CTRL_CORE_CONTROL_IO_2_DCAN1_RAMINIT_START,
                CTRL_CORE_CONTROL_IO_2_DCAN1_RAMINIT_START_SET);
    status =((uint32_t) 0x1 << CTRL_CORE_CONTROL_IO_2_DCAN1_RAMINIT_DONE_SHIFT) &
                CTRL_CORE_CONTROL_IO_2_DCAN1_RAMINIT_DONE_MASK;


Log_print0(Diags_INFO,"==========wait enter 2");
    while (status != ((status & HW_RD_REG32(SOC_CTRL_MODULE_CORE_CORE_REGISTERS_BASE +
                CTRL_CORE_CONTROL_IO_2 + VIRT_PHY_OFFSET))))
            {
                if(++i > 250) {
					Log_print0(Diags_INFO,"=======timeout! dcan1 clk ram setting failure !");
					return;
				}
            }
Log_print0(Diags_INFO,"==========wait leave 2");
    /* Clear the done bit */
    HW_WR_FIELD32((uint32_t) SOC_CTRL_MODULE_CORE_CORE_REGISTERS_BASE +
                CTRL_CORE_CONTROL_IO_2 + VIRT_PHY_OFFSET,
                CTRL_CORE_CONTROL_IO_2_DCAN1_RAMINIT_DONE,
                1U);
#endif

}



/*
 *  ======== Server_init ========
 */
Void Server_init(Void)
{
    Registry_Result result;

    /* register with xdc.runtime to get a diags mask */
    result = Registry_addModule(&Registry_CURDESC, MODULE_NAME);
    Assert_isTrue(result == Registry_SUCCESS, (Assert_Id)NULL);

    /* initialize module object state */
    Module.hostProcId = MultiProc_getId("HOST");
}


/*
 *  ======== Server_create ========
 */
Int Server_create()
{
    Int                 status = 0;
    MessageQ_Params     msgqParams;
    char                msgqName[32];

    /* enable some log events */
    Diags_setMask(MODULE_NAME"+EXF");

    /* create local message queue (inbound messages) */
    MessageQ_Params_init(&msgqParams);
    sprintf(msgqName, App_SlaveMsgQueName, MultiProc_getName(MultiProc_self()));
    Module.slaveQue = MessageQ_create(msgqName, &msgqParams);

    if (Module.slaveQue == NULL) {
        status = -1;
        goto leave;
    }

    Log_print0(Diags_INFO,"Server_create: server is ready");

leave:
    Log_print1(Diags_EXIT, "<-- Server_create: %d", (IArg)status);
    return (status);
}




/*
 *  ======== Server_exec ========
 */
Int Server_exec()
{
    Int                 status;
    Bool                running = TRUE;
    App_Msg *           msg;
    MessageQ_QueueId    queId;

	Log_print0(Diags_INFO,"==============DCAN1Prcm_clck_ram enter\n");
	 DCAN1Prcm_clck_ram();
	Log_print0(Diags_INFO,"==============DCAN1Prcm_clck_ram leave\n");

    while (running) {

        /* wait for inbound message */
        status = MessageQ_get(Module.slaveQue, (MessageQ_Msg *)&msg,
            MessageQ_FOREVER);

        if (status < 0) {
            goto leave;
        }

        if (msg->cmd == App_CMD_SHUTDOWN) {
            running = FALSE;
        }
		
		//GPIOPinWrite(gpio_base_address, gpio_pin, GPIO_PIN_HIGH);
        /* process the message */
        Log_print1(Diags_INFO, "Server_exec: processed cmd=0x%x", msg->cmd);

        /* send message back */
        queId = MessageQ_getReplyQueue(msg); /* type-cast not needed */
        MessageQ_put(queId, (MessageQ_Msg)msg);
    } /* while (running) */

leave:
    Log_print1(Diags_EXIT, "<-- Server_exec: %d", (IArg)status);
    return(status);
}

/*
 *  ======== Server_delete ========
 */

Int Server_delete()
{
    Int         status;

    Log_print0(Diags_ENTRY, "--> Server_delete:");

    /* delete the video message queue */
    status = MessageQ_delete(&Module.slaveQue);

    if (status < 0) {
        goto leave;
    }

leave:
    if (status < 0) {
        Log_error1("Server_finish: error=0x%x", (IArg)status);
    }

    /* disable log events */
    Log_print1(Diags_EXIT, "<-- Server_delete: %d", (IArg)status);
    Diags_setMask(MODULE_NAME"-EXF");

    return(status);
}

/*
 *  ======== Server_exit ========
 */

Void Server_exit(Void)
{
    /*
     * Note that there isn't a Registry_removeModule() yet:
     *     https://bugs.eclipse.org/bugs/show_bug.cgi?id=315448
     *
     * ... but this is where we'd call it.
     */
}
