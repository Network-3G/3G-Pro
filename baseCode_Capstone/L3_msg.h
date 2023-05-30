#include "mbed.h"

#include "mbed.h"

/*#define MSG_MO   3
#define MSG_MI   4
*/

#define MSG_TYPE_CHAT   0
#define MSG_TYPE_CON    1		//type header
#define MSG_TYPE_DIS    2

#define MSG_RSC_Req		0		//RSC header
#define MSG_RSC_Set		1
#define MSG_RSC_Cpl		2

#define MSG_ACP_ACCEPT  1		//Accept header
#define MSG_ACP_REJECT  0

#define MSG_OFFSET_TYPE  0		//PDU
#define MSG_OFFSET_RSC   1
#define MSG_OFFSET_Acp   2
#define MSG_OFFSET_srcID	 3
#define MSG_OFFSET_destID	 4
#define MSG_OFFSET_CHAT	 5 




#define ARQMSG_ACKSIZE      3
#define L3_MSG_ACKSIZE 3


#define ARQMSG_MAXDATASIZE  26
#define ARQMSSG_MAX_SEQNUM  1024


int Msg_checkIfReqCON(uint8_t* msg) // event c
int Msg_checkIfSetCON_Accept_Rcvd(uint8_t* msg) // event c
int Msg_checkIfSetCON_Reject_Rcvd(uint8_t* msg) // event d
int Msg_checkIfCplCON_Rcvd(uint8_t* msg) // event e
int Msg_checkIfSetDIS_Rcvd(uint8_t* msg) // event k
int Msg_checkIfCplDIS_Rcvd(uint8_t* msg) // event l


uint8_t Msg_encodeCONPDU(uint8_t* msg_CONPDU, int rsc, int acp, int srcID, int destID)
uint8_t Msg_encodeDISPDU(uint8_t* msg_DISPDU, int rsc, int srcID, int destID)
uint8_t Msg_encodeCHAT(uint8_t* msg_data, uint8_t* data, int rsc, int srcID, int destID, int len)


int arqMsg_checkIfData(uint8_t* msg);
int arqMsg_checkIfAck(uint8_t* msg);
uint8_t arqMsg_encodeAck(uint8_t* msg_ack, uint8_t seq);
uint8_t arqMsg_encodeData(uint8_t* msg_data, uint8_t* data, int seq, int len);
//uint8_t arqMsg_getSeq(uint8_t* msg);
uint8_t* arqMsg_getWord(uint8_t* msg);

//state IDLE
int Msg_checkIfReqCON(uint8_t* msg) // event b

//state CON
int Msg_checkIfSetCON_Accept_Rcvd(uint8_t* msg) // event c
int Msg_checkIfSetCON_Reject_Rcvd(uint8_t* msg) // event d
int Msg_checkIfCplCON_Rcvd(uint8_t* msg) // event e

//state DIS
int Msg_checkIfSetDIS_Rcvd(uint8_t* msg) // event k
int Msg_checkIfCplDIS_Rcvd(uint8_t* msg) // event l


// 이거 추가함
#define CHATMSG_MAXDATASIZE 26
#define CHATMSSG_MAX_SEQNUM 1024

#define IDLEMSG_MAXDATASIZE 26
#define IDLEMSSG_MAX_SEQNUM 128

#define L3_MSG_OFFSET_TYPE 0

/*
#define L3_MSG_TYPE_ACK 0
#define L3_MSG_TYPE_DATA 1
#define L3_MSG_TYPE_DATA_CONT 2

#define L3_MSG_OFFSET_TYPE 0
#define L3_MSG_OFFSET_SEQ 1
#define L3_MSG_OFFSET_DATA 2

#

#define L3_MSG_MAXDATASIZE 26


int L3_msg_checkIfData(uint8_t *msg);
int L3_msg_checkIfAck(uint8_t *msg);
int L3_msg_checkIfEndData(uint8_t *msg);
uint8_t L3_msg_encodeAck(uint8_t *msg_ack, uint8_t seq);
uint8_t L3_msg_encodeData(uint8_t *msg_data, uint8_t *data, int seq, int len, uint8_t);
uint8_t L3_msg_getSeq(uint8_t *msg);
uint8_t *L3_msg_getWord(uint8_t *msg);
*/