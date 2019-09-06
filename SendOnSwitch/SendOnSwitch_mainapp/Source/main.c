/* ==========================================================
 *    ___             _   _     _
 *   / _ \ _ __   ___| |_| |__ (_)_ __ __  __
 *  | | | | '_ \ / _ \ __| '_ \| | '_ \\ \/ /
 *  | |_| | | | |  __/ |_| | | | | | | |>  <
 *   \___/|_| |_|\___|\__|_| |_|_|_| |_/_/\_\
 *
 * Copyright Onethinx, 2018
 * All Rights Reserved
 *
 * UNPUBLISHED, LICENSED SOFTWARE.
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Onethinx BV
 *
 * ==========================================================
*/
#include "OnethinxCore01.h"
#include "DemoKit01.h"
#include "LoRaWAN_keys.h"
#include "cy_pdl.h"
#include "cycfg.h"
#include "stdio.h"

/* 
 Include this file if using CapSense: #include "cycfg_capsense.h"
 Include this file if using SMIF/QSPI: #include "cycfg_qspi_memslot.h"
 Include this file if using USB device: #include "cycfg_usbdev.h"
*/

const char * paramErrors_txt[] = {
    "param_OK",
    "param_Invalid",
    "param_OutOfRange",
};

const char * radioErrors_txt[] = {
	"radio_OK",
	"radio_BusyError",
	"radio_BuckStartError",
	"radio_XoscStartError",
	"radio_Rc13mCalibError",
	"radio_Rc64kCalibError",
	"radio_PllCalibError",
	"radio_PllLockError",
	"radio_ImgCalibError",
	"radio_AdcCalibError",
	"radio_PaRampError",
};

const char * macErrors_txt[] = {
	"mac_OK",
	"mac_BusyError",
	"mac_NotJoinedError",
	"mac_ChannelsOccupiedError",
	"mac_UnrecognizedKeyType",
	"mac_EmptyPayload_Error",
	"mac_RXheader_Error",
	"mac_RX_MIC_Error",
	"mac_RX_InvalidDevAddr",
	"mac_InvalidPacket_Error",
	"mac_RXtimeout_Error",
	"mac_CRC_Error",
};

const char * systemErrors_txt[] = {
	"system_OK",
	"system_BusyError",
	"system_NotStarted",
	"system_IPCError",
	"system_FlashWriteError",
	"system_VersionMatchError",
};


/* Go to ../OnethinxCore/LoRaWAN_keys.h and fill in the fields of the TTN_OTAAkeys structure */
coreConfiguration_t coreConfig = {
	.Join.KeysPtr =		&TTN_OTAAkeys,
	.Join.DataRate =		DR_AUTO,
	.Join.Power =		PWR_ATT_14dB,
	.Join.MAXTries =		100,
	.Join.SubBand_1st =	US_SUB_BAND_2,
	.Join.SubBand_2nd =	US_SUB_BAND_NONE,
	.TX.Confirmed =		false,
	.TX.DataRate =		DR_0,
	.TX.Power = 			PWR_ATT_14dB,
	.TX.FPort =			1,
};

cy_stc_sysint_t PIN_BUTTON_SYSINT_CFG = {
		.intrSrc	   = ioss_interrupts_gpio_0_IRQn,
		.intrPriority  = 0,
};

void PinButtonInterruptHandler( void );

/* OnethinxCore uses the following structures and variables, which should be defined globally */
coreStatus_t status, status2;
errorStatus_t errorStatus;
char RXbuffer[64];
char TXbuffer[64];
char buffer[64];
uint32_t handler_cnt = 0;
bool handler_flag = false;
cy_stc_scb_uart_context_t uartContext;

sleepConfig_t sleepConfigP = {
	.sleepMode = modeSleepDebugOn,
	.sleepCores = coresM4,
	.wakeUpPin = wakeUpPinHigh(true),
	.wakeUpTime = wakeUpTimeOff
};

sleepConfig_t sleepConfigD = {
	.sleepMode = modeSleepDebugOn,
	.sleepCores = coresM4,
	.wakeUpPin = wakeUpPinOff,
	.wakeUpTime = wakeUpDelay(0,0,0,30)
};

/* Populate configuration structure */
const cy_stc_scb_uart_config_t uartConfig =
{
    .uartMode                   = CY_SCB_UART_STANDARD,
    .enableMutliProcessorMode   = false,
    .smartCardRetryOnNack       = false,
    .irdaInvertRx               = false,
    .irdaEnableLowPowerReceiver = false,
    .oversample                 = 12UL,
    .enableMsbFirst             = false,
    .dataWidth                  = 8UL,
    .parity                     = CY_SCB_UART_PARITY_NONE,
    .stopBits                   = CY_SCB_UART_STOP_BITS_1,
    .enableInputFilter          = false,
    .breakWidth                 = 11UL,
    .dropOnFrameError           = false,
    .dropOnParityError          = false,
    .receiverAddress            = 0UL,
    .receiverAddressMask        = 0UL,
    .acceptAddrInFifo           = false,
    .enableCts                  = false,
    .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .rtsRxFifoLevel             = 0UL,
    .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
    .rxFifoTriggerLevel  = 0UL,
    .rxFifoIntEnableMask = 0UL,
    .txFifoTriggerLevel  = 0UL,
    .txFifoIntEnableMask = 0UL,
};

void printErrorStatus() {
	/* check for errors */
	errorStatus = LoRaWAN_GetError();
	if( errorStatus.errorValue != errorStatus_NoError ){
		/* errors encountered, list the 4 categories. See OnethinxCore01.h for the enumeration */
		// sprintf( buffer, "\r\nParameter error: 0x%lx", (errorStatus.errorValue >> 0) & 0xFF );
		sprintf( buffer, "%s", paramErrors_txt[(errorStatus.errorValue >> 0) & 0xFF] );
		Cy_SCB_UART_PutString(UART_HW, buffer );
		// sprintf( buffer, "\r\nRadio Error:     0x%lx", (errorStatus.errorValue >> 8) & 0xFF );
		sprintf( buffer, ",%s", radioErrors_txt[(errorStatus.errorValue >> 8) & 0xFF] );
		Cy_SCB_UART_PutString(UART_HW, buffer );
		// sprintf( buffer, "\r\nMAC Error:       0x%lx", (errorStatus.errorValue >> 16) & 0xFF );
		sprintf( buffer, ",%s", macErrors_txt[(errorStatus.errorValue >> 16) & 0xFF] );
		Cy_SCB_UART_PutString(UART_HW, buffer );
		// sprintf( buffer, "\r\nSystem Error:    0x%lx", (errorStatus.errorValue >> 24) & 0xFF );
		sprintf( buffer, ",%s", systemErrors_txt[(errorStatus.errorValue >> 24) & 0xFF] );
		Cy_SCB_UART_PutString(UART_HW, buffer );
	} else {
		Cy_SCB_UART_PutString(UART_HW, "OK" );
	}
};

/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
*   Configures an interrupt on PIN_BUTTON (pin 0.4).
*   The Interrupt service routine sets a flag and increases a counter.
*   The interrupt itself will wake the module which will the execute the for-loop (once).
* Details:
*   Remember to update LoRaWANKeys.h.
*
*******************************************************************************/
int main(void)
{

	uint8_t j=0;

	/* initialize hardware configuration */
	init_cycfg_all();

	/* a delay is necessary to prevent spurious joins in the debug procedure */
	CyDelay(1000);

    /* set up the interrupt handler */
    Cy_SysInt_Init(&PIN_BUTTON_SYSINT_CFG, &PinButtonInterruptHandler );
    NVIC_ClearPendingIRQ(PIN_BUTTON_SYSINT_CFG.intrSrc);
    NVIC_EnableIRQ(PIN_BUTTON_SYSINT_CFG.intrSrc);

	/* enable global interrupts. */
	__enable_irq();

	LED_B_SET(LED_ON);

	/* welcome string */
	Cy_SCB_UART_Init( UART_HW, &UART_config, &uartContext);
	Cy_SCB_UART_Enable( UART_HW );
	Cy_SCB_UART_PutString(UART_HW, "\r\n------ Send On Switch example -------\r\n" );

	printErrorStatus();

	Cy_SCB_UART_PutString(UART_HW, "\r\nLoRaWAN_Init() " );
	/* initialize radio with parameters in coreConfig */
	status = LoRaWAN_Init(&coreConfig);
	printErrorStatus();

	Cy_SCB_UART_PutString(UART_HW, "\r\nLoRaWAN_Join() " );
	/* send join using parameters in coreConfig, blocks until either success or MAXtries */
	status = LoRaWAN_Join(true);
	printErrorStatus();

	/* check for successful join */
	if (!status.mac.isJoined){
		Cy_SCB_UART_PutString(UART_HW, "\r\nJOIN FAILED" );
		while(1) {
			LED_B_INV;
			CyDelay(100);
		}
	} else {
		LED_B_SET(LED_OFF);
		/*delay before first message will be sent */
		CyDelay(1000);
	}

	/* main loop ends with a sleep command. This ensures that it is executed only once (after an interrupt) */
	for(;;)
	{
		LED_B_SET(LED_ON);

		if( handler_flag ){
			// status = LoRaWAN_GetStatus();
			// printErrorStatus();

			/* prepare a message */
			j=0;
			TXbuffer[j++] = handler_cnt >> 8  & 0xFF;
			TXbuffer[j++] = handler_cnt       & 0xFF;

			/* send with LoRaWAN */
			Cy_SCB_UART_PutString(UART_HW, "\r\nLoRaWAN_Send() " );
			status = LoRaWAN_Send((uint8_t *) TXbuffer, j, true);
			printErrorStatus();


			/* check for downlink messages */
			if (status.mac.messageReceived) {
				LoRaWAN_GetRXdata((uint8_t *) RXbuffer, status.mac.bytesToRead);
				LoRaWAN_Send((uint8_t *) RXbuffer, status.mac.bytesToRead, true);
				Cy_SCB_UART_PutString(UART_HW, "\r\nReceived a message: " );
				Cy_SCB_UART_PutString(UART_HW, RXbuffer );
			}

			/* clear the handler_flag */
			handler_flag = false;

		} //end if( handler_flag )

		/* put in sleep and wait for next interrupt */
		sprintf( buffer, "\r\nLoRaWAN_Sleep(Pin) "); Cy_SCB_UART_PutString(UART_HW, buffer );
		while (!Cy_SCB_IsTxComplete(UART_HW)) {
		};

		LED_B_SET( LED_OFF );
		status2 = LoRaWAN_Sleep(&sleepConfigP);
		handler_cnt++;
		handler_flag = true;
		sprintf( buffer, "Wake(#%d) ", (int) handler_cnt ); Cy_SCB_UART_PutString(UART_HW, buffer );
		printErrorStatus();

		/*
	    if( CY_SYSPM_SUCCESS != Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT) ) {
			Cy_SCB_UART_PutString(UART_HW, "\r\nfailed to go to sleep" );
		}
*/
	} //end for
} //end main

/*******************************************************************************
* Summary:
*   handles an interrupt on PIN_BUTTON (pin 0.4).
*   see design.modus for details on pin configuration.
*   note that a single button press may invoke this function more than once.
*   the function that
*******************************************************************************/
void PinButtonInterruptHandler( void ){
	/* First thing to do is to clear the interrupt, otherwise it will retrigger */
	Cy_GPIO_ClearInterrupt( PIN_BUTTON_PORT, PIN_BUTTON_PIN );
	// handler_cnt++;
	// handler_flag = true;
}

/* [] END OF FILE */
