#include "mbed.h"
#include "L3_msg.h"

//------------¼ÒÈñ ÄÚµå
int Msg_checkIfReqCON(uint8_t* msg) // event c
{
    return ((msg[MSG_OFFSET_TYPE] == MSG_TYPE_CON) && (msg[MSG_OFFSET_RSC] == MSG_RSC_Req) && (msg[MSG_OFFSET_Acp] == MSG_ACP_ACCEPT));
}

int Msg_checkIfSetCON_Accept_Rcvd(uint8_t* msg) // event c
{
    return ((msg[MSG_OFFSET_TYPE] == MSG_TYPE_CON) && (msg[MSG_OFFSET_RSC] == MSG_RSC_Set) && (msg[MSG_OFFSET_Acp] == MSG_ACP_ACCEPT));
}

int Msg_checkIfSetCON_Reject_Rcvd(uint8_t* msg) // event d
{
    return ((msg[MSG_OFFSET_TYPE] == MSG_TYPE_CON) && (msg[MSG_OFFSET_RSC] == MSG_RSC_Set) && (msg[MSG_OFFSET_Acp] == MSG_ACP_REJECT));
}

int Msg_checkIfCplCON_Rcvd(uint8_t* msg) // event e
{
    return ((msg[MSG_OFFSET_TYPE] == MSG_TYPE_CON) && (msg[MSG_OFFSET_RSC] == MSG_RSC_Cpl) && (msg[MSG_OFFSET_Acp] == MSG_ACP_ACCEPT));
}

int Msg_checkIfSetDIS_Rcvd(uint8_t* msg) // event k
{
    return ((msg[MSG_OFFSET_TYPE] == MSG_TYPE_DIS) && (msg[MSG_OFFSET_RSC] == MSG_RSC_Set) && (msg[MSG_OFFSET_Acp] == MSG_ACP_ACCEPT));
}
int Msg_checkIfCplDIS_Rcvd(uint8_t* msg) // event l
{
    return ((msg[MSG_OFFSET_TYPE] == MSG_TYPE_DIS) && (msg[MSG_OFFSET_RSC] == MSG_RSC_Cpl) && (msg[MSG_OFFSET_Acp] == MSG_ACP_ACCEPT));
}

// ³ª = input_thisId

uint8_t Msg_encodeCONPDU(uint8_t* msg_CONPDU, int rsc, int acp, int srcID, int destID)
{
    msg_CONPDU[MSG_OFFSET_TYPE] = MSG_TYPE_CON;
    msg_CONPDU[MSG_OFFSET_RSC] = rsc;
    msg_CONPDU[MSG_OFFSET_Acp] = acp;
    msg_CONPDU[MSG_OFFSET_srcID] = srcID;
    msg_CONPDU[MSG_OFFSET_destID] = destID;

}

uint8_t Msg_encodeDISPDU(uint8_t* msg_DISPDU, int rsc, int srcID, int destID)
msg_DISPDU[MSG_OFFSET_TYPE] = MSG_TYPE_DIS;
msg_DISPDU[MSG_OFFSET_RSC] = rsc;
msg_DISPDU[MSG_OFFSET_Acp] = MSG_ACP_ACCEPT;
msg_DISPDU[MSG_OFFSET_srcID] = srcID;
msg_DISPDU[MSG_OFFSET_destID] = destID;

}

uint8_t Msg_encodeCHAT(uint8_t* msg_data, uint8_t* data, int srcID, int destID, int len)
{
    msg_data[MSG_OFFSET_TYPE] = MSG_TYPE_CHAT;
    msg_data[MSG_OFFSET_RSC] = rsc;
    msg_data[MSG_OFFSET_Acp] = acp;
    msg_data[MSG_OFFSET_srcID] = srcID;
    msg_data[MSG_OFFSET_destID] = destID;

    memcpy(&msg_data[MSG_OFFSET_CHAT], data, len * sizeof(uint8_t));

    return len + MSG_OFFSET_CHAT;
}


uint8_t* arqMsg_getWord(uint8_t* msg)
{
    return &msg[ARQMSG_OFFSET_DATA];
}

/*
uint8_t L3_msg_getSeq(uint8_t *msg)
{
    return msg[L3_MSG_OFFSET_SEQ];
}

uint8_t *L3_msg_getWord(uint8_t *msg)
{
    return &msg[L3_MSG_OFFSET_DATA];
}

uint8_t L3_msg_encodeAck(uint8_t *msg_ack, uint8_t seq)
{
    msg_ack[L3_MSG_OFFSET_TYPE] = L3_MSG_TYPE_ACK;
    msg_ack[L3_MSG_OFFSET_SEQ] = seq;
    msg_ack[L3_MSG_OFFSET_DATA] = 1;

    return L3_MSG_ACKSIZE;
}
*/