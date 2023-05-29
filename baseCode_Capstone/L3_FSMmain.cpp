#include "L3_FSMevent.h"
#include "L3_msg.h"
#include "L3_timer.h"
#include "L3_LLinterface.h"
#include "protocol_parameters.h"
#include "mbed.h"

// FSM state -------------------------------------------------
// #define L3STATE_IDLE              0

#define STATE_IDLE 0
#define STATE_CON_WAIT 1
#define STATE_CHAT 2
#define STATE_DIS_WAIT 3

static uint8_t myL3ID = 1;
static uint8_t destL3ID = 0;

// state variables
static uint8_t main_state = STATE_IDLE; // protocol state
static uint8_t prev_state = main_state;

// SDU (input)
static uint8_t originalWord[1030];
static uint8_t wordLen = 0;

static uint8_t sdu[1030];

// ARQ 변수 나중에 바꿀 것 !!!!!!!!!!!!
uint8_t seqNum = 0;  // ARQ sequence number
uint8_t retxCnt = 0; // ARQ retransmission counter
uint8_t arqAck[5];   // ARQ ACK PDU

// serial port interface
static Serial pc(USBTX, USBRX);
static uint8_t myDestId;

// 새로 추가함
uint8_t input_destId = 0;

// application event handler : generating SDU from keyboard input
static void L3service_processInputWord(void)
{
    char c = pc.getc();
    if (main_state == STATE_CHAT &&
        !arqEvent_checkEventFlag(SDU_Rcvd))
    {
        if (c == '\n' || c == '\r')
        {
            originalWord[wordLen++] = '\0';
            L3_event_setEventFlag(SDU_Rcvd);
            debug_if(DBGMSG_L3, "word is ready! ::: %s\n", originalWord);
        }
        else
        {
            originalWord[wordLen++] = c;
            if (wordLen >= L3_MAXDATASIZE - 1)
            {
                originalWord[wordLen++] = '\0';
                L3_event_setEventFlag(SDU_Rcvd);
                pc.printf("\n max reached! word forced to be ready :::: %s\n", originalWord);
            }
        }
    }
    elif (!L3_event_checkEventFlag(ReqCON_Send))
    {
        if (c == '\n' || c == '\r')
        {
            originalWord[wordLen++] = '\0';
            L3_event_setEventFlag(ReqCON_Send);
            debug_if(DBGMSG_L3, "destination word is ready! ::: %s\n", originalWord);
        }
        else
        {
            originalWord[wordLen++] = c;
            if (wordLen >= IDLEMSG_MAXDATASIZE - 1)
            {
                originalWord[wordLen++] = '\0';
                L3_event_setEventFlag(ReqCON_Send);
                pc.printf("\n max reached! destination word forced to be ready :::: %s\n", originalWord);
            }
        }
    }
}

void L3_initFSM(uint8_t destId)
{

    myDestId = destId;
    // initialize service layer
    pc.attach(&L3service_processInputWord, Serial::RxIrq);

    pc.printf("Give a word to send : ");
}

void L3_FSMrun(void)
{
    uint8_t flag_needPrint = 1;
    uint8_t prev_state = 0;

    if (prev_state != main_state)
    {
        // debug_if(DBGMSG_L3, "[L3] State transition from %i to %i\n", prev_state, main_state);
        debug_if(DBGMSG_L3, "L3 STATE !!!!!!");
        prev_state = main_state;
    }

    // FSM should be implemented here! ---->>>>
    switch (main_state)
    {

    // IDLE STATE
    case STATE_IDLE:

        // Retrieving data info.
        uint8_t srcId = L3_LLI_getSrcId();
        uint8_t *dataPtr = L3_LLI_getMsgPtr();
        uint8_t size = L3_LLI_getSize();

        // b
        if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {

            // debug("\n -------------------------------------------------\nRCVD MSG : %s (length:%i)\n -------------------------------------------------\n", dataPtr, size);

            pc.printf("\nCHAT으로 가기 위한 여정의 시작..");
            pc.printf("\n -------------------------------------------------\nRCVD from %i : %s (length:%i, seq:%i)\n -------------------------------------------------\n", srcId, L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
            // pc.printf("Give a word to send : ");

            // pdu
            L3_msg_encodeAck(arqAck, L3_msg_getSeq(dataPtr));
            L3_LLI_sendData(arqAck, L3_MSG_ACKSIZE, srcId);

            // main_state = MAINSTATE_TX;
            main_state = STATE_CON_WAIT;
            flag_needPrint = 1; // flag = 1 일 때 채팅 상태로 넘어감!!

            // L3_event_clearEventFlag(L3_event_dataRcvd);

            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // a: 목적지 설정하는 거
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {

            pc.printf(":: ID for the destination : ");
            pc.scanf("%d", &input_destId);
            myDestId = input_destId;

            // msg header setting
            // PDU라 나중에 수정할 것 !!
            strcpy((char *)sdu, (char *)originalWord);
            debug("[L3] msg length : %i\n", wordLen);
            L3_LLI_dataReqFunc(sdu, wordLen, myDestId);

            pc.printf("[MAIN] sending to %i (seq:%i)\n", destL3ID, (seqNum - 1) % IDLEMSSG_MAX_SEQNUM);

            // main_state = MAINSTATE_TX;
            main_state = STATE_CON_WAIT;
            flag_needPrint = 1;

            wordLen = 0;
            L3_event_clearEventFlag(ReqCON_Send); // 작업 완료

            // debug_if(DBGMSG_L3, "[L3] sending msg....\n");
            wordLen = 0;

            // pc.printf("Give a word to send : ");

            L3_event_clearEventFlag(ReqCON_Send);
        }

        // IDLE 돌아오는거
        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // f
        else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))
        {
            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // k
        else if (L3_event_checkEventFlag(SetDis_Rcvd))
        {
            L3_event_clearEventFlag(SetDis_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDis_Rcvd))
        {
            L3_event_clearEventFlag(CplDis_Rcvd);
        }

        // m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }
        /*
        // k
        else if (flag_needPrint == 1)
        {

            flag_needPrint = 0;
            pc.printf("CplDIS 받음 ");
            pc.printf("\n -------------------------------------------------\nRCVD from %i : %s (length:%i, seq:%i)\n -------------------------------------------------\n", srcId, L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
            pc.printf("Give Word to Send: ");
        }
        */
        break;

    // CON STATE
    case STATE_CON_WAIT:

        // c, SDU 생성 event 안넣었음(위에 있는 함수)
        if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {

            pc.printf("STATE CHANGED CON 2 CHAT ");

            // pdu
            L3_msg_encodeAck(arqAck, L3_msg_getSeq(dataPtr));
            L3_LLI_sendData(arqAck, L3_MSG_ACKSIZE, srcId);

            // main_state = MAINSTATE_TX;
            main_state = STATE_CHAT;
            // flag_needPrint = 1;

            // L3_event_clearEventFlag(L3_event_dataRcvd);

            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            pc.printf("STATE CHANGED CON 2 IDLE ");
            main_state = STATE_IDLE;
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e (timer)
        else if (L3_event_checkEventFlag(CplCON_Rcvd)) // data TX finished
        {
            pc.printf("Timer Starts!! ");
            L3_timer_Chat_Timer();

            main_state = STATE_CHAT;
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // CON 대기로 돌아오는 거
        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            L3_event_clearEventFlag(ReqCON_Send);
        }

        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // f
        else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))
        {
            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // k
        else if (L3_event_checkEventFlag(SetDis_Rcvd))
        {
            L3_event_clearEventFlag(SetDis_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDis_Rcvd))
        {
            L3_event_clearEventFlag(CplDis_Rcvd);
        }

        // m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }
        break;

    // DIS STATE
    case STATE_DIS_WAIT:

        // l
        if (L3_event_checkEventFlag(CplDis_Rcvd))
        {
            pc.printf("STATE CHANGED DIS 2 IDLE & ComDIS");

            main_state = STATE_IDLE;
            L3_event_clearEventFlag(CplDis_Rcvd);
        }

        // k
        else if (L3_event_checkEventFlag(SetDis_Rcvd))
        {
            pc.printf("STATE CHANGED DIS 2 IDLE & SetupDIS");

            // CplDISPDU 보내기

            main_state = STATE_IDLE;
            L3_event_clearEventFlag(SetDis_Rcvd);
        }

        // DIS 대기로 돌아오는 거
        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            L3_event_clearEventFlag(ReqCON_Send);
        }

        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // f
        else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))
        {
            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }
        break;

    // CHAT STATE
    case STATE_CHAT:

        // i
        if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            // L3_timer_expireTimer();
            // PDU 보내기

            main_state = STATE_DIS_WAIT;
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // j
        else if (L3_event_checkEventFlag(ReqDis_Rcvd))
        {

            pc.printf("STATE CHANGE 2 DIC CON ");

            // PDU 수정
            L3_msg_encodeAck(arqAck, L3_msg_getSeq(dataPtr));
            L3_LLI_sendData(arqAck, L3_MSG_ACKSIZE, srcId);

            // main_state = MAINSTATE_TX;
            main_state = STATE_DIS_WAIT;
            // flag_needPrint = 1;

            // L3_event_clearEventFlag(L3_event_dataRcvd);

            L3_event_clearEventFlag(ReqDis_Rcvd);
        }

        // f
        else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))
        {
            pc.printf("SetCON reject ");

            L3_msg_encodeAck(arqAck, L3_msg_getSeq(dataPtr));

            // pdu 생성
            L3_LLI_sendData(arqAck, L3_MSG_ACKSIZE, srcId);

            flag_needPrint = 1;

            // L3_event_clearEventFlag(L3_event_dataRcvd);

            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
        }

        // g
        else if (arqEvent_checkEventFlag(SDU_Rcvd)) // if data needs to be sent (keyboard input)
        {
            // msg header setting
            pduSize = arqMsg_encodeData(arqPdu, originalWord, seqNum, wordLen);
            arqLLI_sendData(arqPdu, pduSize, dest_ID);
            seqNum++;

            pc.printf("[MAIN] sending to %i (seq:%i)\n", dest_ID, (seqNum - 1) % CHATMSSG_MAX_SEQNUM);

            main_state = MAINSTATE_TX;
            flag_needPrint = 1;

            wordLen = 0;
            arqEvent_clearEventFlag(SDU_Rcvd);
        }

        else if (flag_needPrint == 1)
        {
            // flag_needPrint = 0;
            pc.printf("G !!!  ");
            pc.printf("Give Word to Send: ");
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd)) // if data needs to be sent (keyboard input)
        {
            // msg header setting
            strcpy((char *)sdu, (char *)originalWord);
            debug("[L3] msg length : %i\n", wordLen);
            L3_LLI_dataReqFunc(sdu, wordLen, myDestId);

            debug_if(DBGMSG_L3, "[L3] sending msg....\n");
            wordLen = 0;

            pc.printf("Give a word to send : ");

            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // CHAT으로 돌아오는 거
        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            L3_event_clearEventFlag(ReqCON_Send);
        }

        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // k
        else if (L3_event_checkEventFlag(SetDis_Rcvd))
        {
            L3_event_clearEventFlag(SetDis_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDis_Rcvd))
        {
            L3_event_clearEventFlag(CplDis_Rcvd);
        }

        // m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }
        break;

    default:
        break;
    }
}