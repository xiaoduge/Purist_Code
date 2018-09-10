#include <string.h>
#include "common.h"

#include "UartCmd.h"

#include "config.h"

#include "task.h"

#include "sapp.h"

#include "sbl_app.h"

#include "at.h"

#include "osal_snv.h"



// This is the max byte count per OTA message.
uint8 rpcBuf[SAPP_BUF_SIZE], sbFcs, sbIdx;
uint8 *const sbBuf = rpcBuf+1;
uint8 sbLen;

static rpcSte_t rpcSte;

uint8 sappFlags;
Interface_t sappItfType = Interface_RS232;
uint8 sappItfPort       = 0;
uint8 sappTgtType       = RPC_UART_SOF4TA;

/**************************************************************************************************
 * @fn          fillElecID
 *
 * @brief       fillElecId.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static uint8 fillElecID(uint8 *dat)
{
    uint8 len = 0;

    dat[len] = (HAL_ELEC_ID_SIZE << 1);
    len += 1;
    GetDeviceElecId(&dat[len]);
    len += (HAL_ELEC_ID_SIZE << 1);

    return len;
}

/**************************************************************************************************
 * @fn          fillCanID
 *
 * @brief       fill can address.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static uint8 fillCanID(uint8 *dat)
{
    uint8 len = 2;
    dat[0] = (0 >> 8) & 0XFF;
    dat[1] = (0 >> 0) & 0XFF;
    return len;
}



/**************************************************************************************************
 * @fn          SHZNAPP_SerialAppProc
 *
 * @brief       Process the SB command and received buffer.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
 
uint8 SHZNAPP_SerialAppProc(void)
{
  //uint16 t16 = BUILD_UINT16(sbBuf[SAPP_REQ_ADDR_LSB], sbBuf[SAPP_REQ_ADDR_MSB]);
  uint8 len = 1, rsp = SAPP_SUCCESS;

  switch (sbBuf[RPC_POS_CMD1])
  {
  case SAPP_CMD_RESET:
      {
          sappFlags = (1 << SAPP_CMD_RESET);
      }
      break;
  case SAPP_CMD_SYS_INIT:
      {
          osal_snv_reset();
          sappFlags = (1 << SAPP_CMD_SYS_INIT);
      }
      break;
  case SAPP_CMD_AT:
      rsp = appAtCmd(&sbBuf[RPC_POS_DAT0],&sbBuf[SAPP_RSP_STATUS+1],SAPP_PAYLOAD_LEN(sbLen),&len);
      break;
  case SAPP_CMD_DATA:
      {
	  	   // for app to handle
		     extern uint8 Disp_DataHandler(uint8 *pCmd,uint8 *pRsp,uint8 CmdLen,uint8 *pucRspLen);
		     rsp = Disp_DataHandler(&sbBuf[RPC_POS_DAT0],&sbBuf[SAPP_RSP_STATUS+1],SAPP_PAYLOAD_LEN(sbLen),&len);
		     if (SAPP_IGNORED == rsp)
		     {
		        return FALSE;
		     }
      }
      break;
  case SAPP_CMD_CFG:
      rsp = Config_Sapp_Entry(&sbBuf[RPC_POS_DAT0],&sbBuf[SAPP_RSP_STATUS+1],SAPP_PAYLOAD_LEN(sbLen),&len);
      break;
  case SAPP_CMD_INNER:
  	  {
		     uint8 Disp_InnerHandler(uint8 *pCmd,uint8 *pRsp,uint8 CmdLen,uint8 *pucRspLen);
		     rsp = Disp_InnerHandler(&sbBuf[RPC_POS_DAT0],&sbBuf[SAPP_RSP_STATUS+1],SAPP_PAYLOAD_LEN(sbLen),&len);
		     if (SAPP_IGNORED == rsp)
		     {
		        return FALSE;
		     }
  	  }
  	  break;
  default:
	  memcpy(&sbBuf[SAPP_RSP_STATUS+1],&sbBuf[RPC_POS_LEN],(sbLen));
	  len += SAPP_PAYLOAD_LEN(sbLen);
      rsp = SAPP_FAILURE;
      break;
  }

  sbBuf[RPC_POS_LEN] = len; // len for data area (NOT INCLUDE CMD0&CMD1&LEN itself)
  sbBuf[RPC_POS_CMD1] |= SAPP_RSP_MASK;
  sbBuf[RPC_POS_DAT0] = rsp; // state

  return TRUE;
}

/**************************************************************************************************
 * @fn          SHZNAPP_SerialBootProc
 *
 * @brief       Process the SB command and received buffer.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
 
uint8 SHZNAPP_SerialBootProc(void)
{
  uint8 len = 1, rsp = SAPP_SUCCESS;

  switch (sbBuf[RPC_POS_CMD1])
  {
#ifdef IAP		
  case SBL_TGT_BOOTLOAD:
      {
          // erase image & boot
          sappFlags = (1 << SAPP_CMD_RESET);

          appForceBoot();
      }
      break;
#endif	  
  case SBL_QUERY_ID_CMD:
      {
          len += fillElecID(&sbBuf[RPC_POS_DAT0 + len]);
          len += fillCanID(&sbBuf[RPC_POS_DAT0 + len]);
      }
      break;
  case SBL_QUERY_VERSION_CMD:
      {
        int verlen = strlen(Config_GetVersion());
        sbBuf[RPC_POS_DAT0 + len] = verlen;
        len += 1;
        memcpy((char *)&sbBuf[RPC_POS_DAT0 + len],Config_GetVersion(),verlen);
        len += (verlen);
      }
      break;
  case SBL_HANDSHAKE_CMD:
      break;
  case SBL_SET_ADDR_CMD:
      {
      }
      break;
  default:
      rsp = SAPP_FAILURE;
      break;
  }

  sbBuf[RPC_POS_LEN]   = len; // len for data area (NOT INCLUDE CMD0&CMD1&LEN itself)
  sbBuf[RPC_POS_CMD1] |= SAPP_RSP_MASK;
  sbBuf[RPC_POS_DAT0]  = rsp; // state

  return TRUE;
}


/**************************************************************************************************
 * @fn          SHZNAPP_SerialUnknowProc
 *
 * @brief       Process the SB command and received buffer.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
 
uint8 SHZNAPP_SerialUnknowProc(void)
{
  uint8 len = 1, rsp = SAPP_FAILURE;

  sbBuf[RPC_POS_LEN] = len; // len for data area (NOT INCLUDE CMD0&CMD1&LEN itself)
  sbBuf[RPC_POS_CMD1] |= SAPP_RSP_MASK;
  sbBuf[RPC_POS_DAT0] = rsp; // state

  return TRUE;
}


/**************************************************************************************************
 * @fn          SHZNAPP_SerialResp
 *
 * @brief       Make the SB response.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if the downloaded code has been enabled; FALSE otherwise.
 */
uint8 SHZNAPP_SerialResp(uint8 ucPort)
{
  uint8 fcs = 0, len = sbBuf[RPC_POS_LEN] + RPC_FRAME_HDR_SZ;
  uint8 rtrn = FALSE;
	uint8 idx;
	
  for ( idx = RPC_POS_LEN; idx < len; idx++)
  {
    fcs ^= sbBuf[idx];
  }
  sbBuf[len] = fcs;


  //rpcBuf[0] = RPC_UART_SOF;
  rpcBuf[0] = sappTgtType; // 

  sappTgtType = RPC_UART_SOF4TA; // restore to default
  
  (void)HalUARTWrite(ucPort, rpcBuf, len + RPC_UART_FRAME_OVHD);

  return rtrn;
}


/**************************************************************************************************
 * @fn          SHZNAPP_SerialParse
 *
 * @brief       Serial Boot poll & parse according to the RPC protocol.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if the downloaded code has been enabled; FALSE otherwise.
 */
 // ylf: h->c |SOF|LEN|CMD0|CMD1|DATAS|FCS|
 // FCS = FUN(|SB_LEN|SB_CMD1|SB_CMD2|DATAS(=SB_LEN)|)
 //      c->h 
uint8 SHZNAPP_SerialParse(uint8 ucPort)
{
  uint8 ch;

  while (HalUARTRead(ucPort, &ch, 1))
  {
    switch (rpcSte)
    {
    case rpcSteSOF:
      if (RPC_UART_SOF == ch
	  	  ||RPC_UART_SOF4TA == ch)
      {
        rpcSte = rpcSteLen;
      }
      break;

    case rpcSteLen:
      if (ch > SAPP_MAX_SIZE)
      {
        rpcSte = rpcSteSOF;
        break;
      }
      else
      {
        rpcSte = rpcSteData;
        sbFcs = sbIdx = 0;
        sbLen = ch + 3;  // Combine the parsing of Len, Cmd0 & Cmd1 with the data.
        // no break;
      }

    case rpcSteData:
      sbFcs ^= ch;
      sbBuf[sbIdx] = ch;

      if (++sbIdx == sbLen)
      {
        rpcSte = rpcSteFcs;
      }
      break;

    case rpcSteFcs:
      rpcSte = rpcSteSOF;

      if ((sbFcs == ch) 
        /*&& ((sbBuf[RPC_POS_CMD0] & RPC_SUBSYSTEM_MASK) == RPC_SYS_APP)*/)
      {
      
         Message *msg;
         
        //SHZNAPP_SerialAppProc();
        
        //(void)SHZNAPP_SerialResp();  // Send the SB response setup in the sbBuf passed to sblProc().

        // copy
        msg = MessageAlloc(PID_SAPP,0);
        //  UART_PutChar('T');
        
        if (msg)
        {
            msg->msgHead.MsgSeq = ucPort;
            MessageSend(msg);
        }
  
      }
      break;

    default:
     break;
    }
  }

  return FALSE;
}

/**************************************************************************************************
 * @fn          SHZNAPP_CanParse
 *
 * @brief       Serial Boot poll & parse according to the RPC protocol.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      TRUE if the downloaded code has been enabled; FALSE otherwise.
 */
 // ylf: h->c |SOF|LEN|CMD0|CMD1|DATAS|FCS|
 // FCS = FUN(|SB_LEN|SB_CMD1|SB_CMD2|DATAS(=SB_LEN)|)
 //      c->h 
uint8 SHZNAPP_CanParse(uint8 *data,uint16 len)
{
  uint8 ch;

  while (len--)
  {
    ch = *data++;
    switch (rpcSte)
    {
    case rpcSteSOF:
      if (RPC_UART_SOF == ch)
      {
        rpcSte = rpcSteLen;
      }
      break;

    case rpcSteLen:
      if (ch > SAPP_MAX_SIZE)
      {
        rpcSte = rpcSteSOF;
        break;
      }
      else
      {
        rpcSte = rpcSteData;
        sbFcs = sbIdx = 0;
        sbLen = ch + 3;  // Combine the parsing of Len, Cmd0 & Cmd1 with the data.
        // no break;
      }

    case rpcSteData:
      sbFcs ^= ch;
      sbBuf[sbIdx] = ch;

      if (++sbIdx == sbLen)
      {
        rpcSte = rpcSteFcs;
      }
      break;

    case rpcSteFcs:
      rpcSte = rpcSteSOF;

      if (sbFcs == ch) 
      {
         // process
         return TRUE;
      }
      break;

    default:
     break;
    }
  }

  return FALSE;
}

void SHZNAPP_SetTgtType(uint8 ucType)
{
	sappTgtType = ucType;
}


void SHZNAPP_SerialInit(void)
{
	rpcSte = rpcSteSOF;
	sappFlags = 0;
	sappItfType = Interface_RS232;
	sappItfPort		= 0;
	sappTgtType		= RPC_UART_SOF4TA;
}
