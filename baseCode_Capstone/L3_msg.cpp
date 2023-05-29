#include "mbed.h"
#include "L3_msg.h"

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