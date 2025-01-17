/********************************************************************************
 *    ___             _   _     _			
 *   / _ \ _ __   ___| |_| |__ (_)_ __ __  __
 *  | | | | '_ \ / _ \ __| '_ \| | '_ \\ \/ /
 *  | |_| | | | |  __/ |_| | | | | | | |>  < 
 *   \___/|_| |_|\___|\__|_| |_|_|_| |_/_/\_\
 *
 ********************************************************************************
 *
 * Copyright (c) 2019 Onethinx BV <info@onethinx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ********************************************************************************/

// Core revisions:
// 0x000000AA	Wrong devaddr in onethinx_core.h and wrong byte amount reserved
// 0x000000AB	Implemented M0+ reset: failed debugging
// 0x000000AC	Implemented breakCurrentFunction
// 0x000000AD	Added configurations pointer validity check

// Known Issues:
// Confirmed downlink message doesn't get confimation from device
// ADR not working yet

#ifndef ONETHINXCORE01_H
#define ONETHINXCORE01_H	
 
#include <stdint.h>
#include <stdbool.h>

typedef enum {
	coreFunction_Init					= 0x01,
	coreFunction_LW_join				= 0x20,
	coreFunction_LW_send				= 0x30,
	coreFunction_LW_getRXdata			= 0x40,
} coreFunctions_e;

typedef struct __attribute__((packed, aligned(4)))
{
	uint8_t							clientId;
	uint8_t							userCode;
	uint16_t						intrMask;
	void *							coreArgumentsPtr;
} ipc_msg_t;

typedef struct __attribute__((packed, aligned(4)))
{
	ipc_msg_t	 forCM0;
	ipc_msg_t	 fromCM0;
} ipc_msgs_t;

 typedef enum  {
	ABP_10x_key		 			= 0x01,
	OTAA_10x_key	 			= 0x02,
	OTAA_11x_key	 			= 0x03,
	PreStored_key				= 0xF0,
	UserStored_key				= 0xF1,
} keyType_e;

typedef struct __attribute__ ((__packed__)) {
	keyType_e					KeyType;
	uint8_t						KeyIndex;
} StoredKeys_t;

typedef struct __attribute__ ((__packed__))  {
	uint8_t						DevEui[8];
	uint8_t						AppEui[8];
	uint8_t						AppKey[16];
} OTAA_10x_t;
	
typedef struct __attribute__ ((__packed__))  {
	uint8_t						DevEui[8];
	uint8_t						JoinEui[8];
	uint8_t						AppKey[16];
	uint8_t						NwkKey[16];
} OTAA_11x_t;

typedef struct __attribute__ ((__packed__)) {
	uint8_t						DevEui[8];
	uint32_t			   		DevAddr;
	uint8_t						NwkSkey[16];
	uint8_t						AppSkey[16];
} ABP_10x_t;
		
typedef struct  __attribute__ ((__packed__))  {
	keyType_e					KeyType 		: 8;
	uint8_t						reserved		: 7;
	bool						PublicNetwork	: 1;
	union {
		uint8_t					totalbytes[64];
		StoredKeys_t			StoredKeys;
		OTAA_10x_t				OTAA_10x;
		OTAA_11x_t				OTAA_11x;
		ABP_10x_t				ABP_10x;
	};
} LoRaWAN_keys_t;

typedef enum {
    DR_0						= 0x00,
    DR_1						= 0x01,
    DR_2						= 0x02,
    DR_3						= 0x03,
    DR_4						= 0x04,
    DR_5						= 0x05,
    DR_6						= 0x06,
    DR_ADR						= 0xF0,
} Radio_DataRate_e;
#define	MAX_DR	6

typedef enum {
	PWR_MAX						= 0x00,
	PWR_ATT_2dB					= 0x01,
	PWR_ATT_4dB					= 0x02,
	PWR_ATT_6dB					= 0x03,
	PWR_ATT_8dB					= 0x04,
	PWR_ATT_10dB				= 0x05,
	PWR_ATT_12dB				= 0x06,
	PWR_ATT_14dB				= 0x07,
	PWR_ADR						= 0xF0,
} Radio_TXpower_e;
#define	MAX_TXpower		7

typedef enum __attribute__ ((__packed__)) {
	param_OK					= 0x00,
	param_Invalid				= 0x01,
	param_OutOfRange			= 0x02,
	param_UndefinedError		= 0xFA,
} paramErrors_e;

typedef enum __attribute__ ((__packed__)) {
	radio_OK					= 0x00,
	radio_BusyError				= 0x01,		//!< Radio Busy Error
	radio_BuckStartError		= 0x02,		//!< Buck converter failed to start
	radio_XoscStartError		= 0x03,		//!< XOSC failed to start
	radio_Rc13mCalibError		= 0x04,		//!< RC 13MHz oscillator calibration failed
	radio_Rc64kCalibError		= 0x05,		//!< RC 64kHz oscillator calibration failed
	radio_PllCalibError			= 0x06,		//!< PLL calibration failed
	radio_PllLockError			= 0x07,		//!< PLL lock failed
	radio_ImgCalibError			= 0x08,		//!< Image calibration failed
	radio_AdcCalibError			= 0x09,		//!< ADC calibration failed
	radio_PaRampError			= 0x0A,		//!< PA ramp failed
	radio_UndefinedError		= 0xFA
} radioErrors_e;

typedef enum __attribute__ ((__packed__)) {
	mac_OK						= 0x00,
	mac_BusyError				= 0x01,
	mac_NotJoinedError			= 0x02,
	mac_ChannelsOccupiedError	= 0x03,
	mac_UnrecognizedKeyType		= 0x04,
    mac_EmptyPayload_Error 		= 0x05,
	mac_RXheader_Error 			= 0x06,
	mac_RX_MIC_Error 			= 0x07,
	mac_RX_InvalidDevAddr 		= 0x08,
	mac_InvalidPacket_Error 	= 0x09,
	mac_RXtimeout_Error 		= 0x0A,
	mac_UndefinedError			= 0xFA,
} macErrors_e;

typedef enum __attribute__ ((__packed__)) {
	system_OK					= 0x00,
	system_BusyError			= 0x01,
	system_NotStarted			= 0x02,
	system_UndefinedError	   	= 0xFA
} systemErrors_e ;

typedef union {
	struct
	{
		paramErrors_e 			errorStatus 	: 8;  //!< Parameter Errors
		uint8_t									: 8;  //!< reserved
	};
	uint8_t reserved[16];
} parameterStatus_t;


typedef union {
	struct
	{
		radioErrors_e 			errorStatus 	: 8;  //!< Radio Errors
		uint8_t							 		: 7;  //!< reserved
		bool 					isBusy			: 1;  //!< Radio is busy?
	};
	uint8_t reserved[16];
} radioStatus_t;

typedef union {
	struct
	{
		macErrors_e 			errorStatus  	: 8;  //!< MAC Errors
		uint8_t		 			bytesToRead  	: 8;  //!< Total bytes in Receive buffer
		uint8_t							 		: 3;  //!< reserved
		bool 					isConfigured	: 1;  //!< System is configured?
		bool 					messageReceived	: 1;  //!< The MAC received a message
		bool 					isJoined 		: 1;  //!< Chip mode
		bool 					isBusy			: 1;  //!< Chip mode
		bool 					isPublicNetwork	: 1;  //!< True for LoRaWAN Public Network
		uint32_t				devAddr;
	};
	uint8_t reserved[16];
}  macStatus_t;

typedef union {
	struct
	{
		uint32_t				version;
		systemErrors_e 			errorStatus  			: 8;  //!< System Errors
		uint8_t							 				: 5;  //!< reserved
		bool 					breakCurrentFunction	: 1;  //!< Break current execution
		bool 					isStarted  				: 1;  //!< System is started?
		bool 					isBusy  				: 1;  //!< System is busy?
	};
	uint8_t reserved[16];
}  systemStatus_t;

typedef struct {
	parameterStatus_t		parameters;
	radioStatus_t			radio;
	macStatus_t				mac;
	systemStatus_t			system;
} coreStatus_t;

typedef union {
	struct  __attribute__ ((__packed__)) {
		struct  __attribute__ ((__packed__)) {
			bool					Confirmed			: 1;
			uint8_t										: 7;
			Radio_DataRate_e		DataRate			: 8;
			Radio_TXpower_e			Power				: 8;
			uint8_t					FPort				: 8;
		} TX;
		struct  __attribute__ ((__packed__)) {
			LoRaWAN_keys_t *		KeysPtr;
			Radio_DataRate_e		DataRate			: 8;
			Radio_TXpower_e			Power				: 8;
			uint8_t					MAXTries			: 8;
		} Join;
	};
	uint8_t reserved[32];
} coreConfiguration_t;

typedef struct {
	coreFunctions_e				function;
	coreConfiguration_t *		configurationPtr;
	coreStatus_t				status;
	uint8_t * 					coreTXbufPtr;
	uint8_t	 					coreTXbufLength;
	uint8_t * 					coreRXbufPtr;
	uint8_t	 					coreRXbufLength;
} coreArguments_t;

typedef union {
	uint32_t					errorValue;
	struct  __attribute__ ((__packed__)) {
		paramErrors_e			paramErrors			: 8;
		radioErrors_e			radioErrors			: 8;
		macErrors_e				macErrors			: 8;
		systemErrors_e			systemErrors		: 8;

	};
} errorStatus_t;

#define errorStatus_NoError					0

extern coreArguments_t * 		coreArgumentsPtr;
extern coreConfiguration_t * 	coreConfigurationPtr;
extern coreStatus_t *			coreStatusPtr;
	
void			LoRaWAN_Reset(void);
coreStatus_t 	LoRaWAN_Init(coreConfiguration_t * coreConfigurationPtr);
coreStatus_t	LoRaWAN_Join(bool waitTillFinished);
coreStatus_t 	LoRaWAN_GetRXdata(uint8_t * RXdata, uint8_t length);
coreStatus_t 	LoRaWAN_Send(uint8_t* buffer, uint8_t length, bool waitTillFinished);
coreStatus_t 	LoRaWAN_GetStatus();
errorStatus_t 	LoRaWAN_GetError();

#endif /* ONETHINXCORE01_H */
