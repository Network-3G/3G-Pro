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
uint8_t retxCnt = 0; // ARQ retransmission counter
uint8_t pdu[5];   // ACK PDU

// serial port interface
static Serial pc(USBTX, USBRX);
static uint8_t myDestId;

// 새로 추가함
uint8_t input_destId = 0;
uint8_t cond_IDinput; // 0 - no ID, 1 - ID input

static uint8_t input_thisId ; //나의 id
//destination sdu
static uint8_t destinationWord[10];
static uint8_t destinationLen = 0;

static uint8_t destinationsdu[10];
static uint8_t check = 0;

char destinationString[12];

// application event handler : generating SDU from keyboard input
static void L3service_processInputWord(void)
{
    char c = pc.getc();

    if (main_state == STATE_IDLE&&
        cond_IDinput == 0)
    {
       // 입력받고 변경하기 char 를 intf로, condition =1로
       pc.printf(":: ID for the destination : ");
        if (c == '\n' || c == '\r')
            {
                destinationWord[destinationLen++] = '\0';
                sprintf(destinationString, "%s", destinationWord);
                myDestId = atoi(destinationString);
               // myDestId=atoi(destinationWord);//a to i Tmrl
                //L3_event_setEventFlag(SDU_Rcvd); //하나 만들기
                debug_if(DBGMSG_L3, "destination is ready! ::: %s\n", myDestId);
            }
            else
            {
                destinationWord[wordLen++] = c;
                
            }
       cond_IDinput=1;
    }
    else if (main_state == STATE_CHAT &&
        !L3_event_checkEventFlag(SDU_Rcvd))
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
    else if (!L3_event_checkEventFlag(ReqCON_Send))
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

//void로 하기
void L3_initFSM(uint8_t input_thisId)
{

    input_thisId = input_thisId;
    // initialize service layer
    pc.attach(&L3service_processInputWord, Serial::RxIrq);

    pc.printf("Give a word to send : ");
}

void L3_FSMrun(void)
{
    uint8_t flag_needPrint = 1;
    uint8_t prev_state = 0;
    uint8_t conID = 0;

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
            
        // b
        if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            
            conID = L3_LLI_getSrcId();

            // debug("\n -------------------------------------------------\nRCVD MSG : %s (length:%i)\n -------------------------------------------------\n", dataPtr, size);

            pc.printf("\nCHAT으로 가기 위한 여정의 시작..");
            pc.printf("\n -------------------------------------------------\nRCVD from %i : %s (length:%i, seq:%i)\n -------------------------------------------------\n", srcId, L3_msg_getWord(dataPtr), size, L3_msg_getSeq(dataPtr));
            // pc.printf("Give a word to send : ");

            // pdu
            //L3_msg_encodeAck(arqAck, L3_msg_getSeq(dataPtr));

            srcId=myDestId;  //받은 ID가 내 목적지가 됨


            //처음 받자마자 accept setCONPDU를 보내야 함

            Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_ACCEPT);   //srcID로 한 이유는 처음 받은 애한테는 무조건 받아야 해서..
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

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

            // msg header setting
            // PDU라 나중에 수정할 것 !!
            // strcpy((char *)sdu, (char *)originalWord);
            // debug("[L3] msg length : %i\n", wordLen);
            Msg_encodeCONPDU(pdu, MSG_RSC_Req, MSG_ACP_ACCEPT);
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId); //reqcon을 만들고 보내야 하는데 이건 뭐지?

            pc.printf("[MAIN] sending to %i \n", myDestId);

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
            pc.printf("c");
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            pc.printf("d");
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            pc.printf("e");
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("g");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            pc.printf("h");
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("i");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            pc.printf("j");
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // k
        else if (L3_event_checkEventFlag(SetDis_Rcvd))
        {
            pc.printf("k");
            L3_event_clearEventFlag(SetDis_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDis_Rcvd))
        {
            pc.printf("l");
            L3_event_clearEventFlag(CplDis_Rcvd);
        }

        // m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            pc.printf("m");
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
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID =! conID)
            {
                check=1;
            }
            else {
            pc.printf("STATE CHANGED CON 2 CHAT ");

            // pdu
            //L3_msg_encodeAck(arqAck, L3_msg_getSeq(dataPtr));
            
            //cplCON을 보내야 함
            Msg_encodeCONPDU(pdu, MSG_RSC_Cpl, MSG_ACP_ACCEPT);
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

            // main_state = MAINSTATE_TX;
            main_state = STATE_CHAT;
            // flag_needPrint = 1;

            // L3_event_clearEventFlag(L3_event_dataRcvd);

            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
            }
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
             uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID =! conID)
            {
                check=1;
            }
            else{
           
            pc.printf("STATE CHANGED CON 2 IDLE ");
            cond_IDinput = 0;
            main_state = STATE_IDLE;
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
            }
        }

        // e (timer)
        else if (L3_event_checkEventFlag(CplCON_Rcvd)) // data TX finished
        {
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID =! conID)
            {
                check=1;
            }
            else{
            pc.printf("Timer Starts!! ");
            L3_timer_Chat_Timer();

            main_state = STATE_CHAT;
            L3_event_clearEventFlag(CplCON_Rcvd);
            }
        }

        // CON 대기로 돌아오는 거
        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            pc.printf("a");
            L3_event_clearEventFlag(ReqCON_Send);
        }

        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            pc.printf("b");
            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // f
        else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))
        {
            pc.printf("f");
            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("g");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            pc.printf("h");
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("i");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            pc.printf("j");
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // k
        else if (L3_event_checkEventFlag(SetDis_Rcvd))
        {
            pc.printf("k");
            L3_event_clearEventFlag(SetDis_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDis_Rcvd))
        {
            pc.printf("l");
            L3_event_clearEventFlag(CplDis_Rcvd);
        }

        // m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            pc.printf("m");
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }
        break;

    // DIS STATE
    case STATE_DIS_WAIT:

        // l
        if (L3_event_checkEventFlag(CplDis_Rcvd))
        {
        {   
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID == conID)
            {
            pc.printf("STATE CHANGED DIS 2 IDLE & ComDIS");
            cond_IDinput = 0;   
            main_state = STATE_IDLE;
            L3_event_clearEventFlag(CplDis_Rcvd);
            }
            else
            {
                check ==2
            }

        }
        }

        // k
        else if (L3_event_checkEventFlag(SetDis_Rcvd))
        {
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID == conID)
            {
            pc.printf("STATE CHANGED DIS 2 IDLE & SetupDIS");

            // CplDISPDU 보내기
            Msg_encodeDISPDU(pdu, MSG_RSC_Cpl);
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

            cond_IDinput = 0; 
            main_state = STATE_IDLE;
            L3_event_clearEventFlag(SetDis_Rcvd);  
            }

            else
            {
                check ==2
            }

        // DIS 대기로 돌아오는 거
        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            pc.printf("a");
            L3_event_clearEventFlag(ReqCON_Send);
        }

        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            pc.printf("b");
            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            pc.printf("c");
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            pc.printf("d");
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            pc.printf("e");
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // f
        else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))
        {
            pc.printf("f");
            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("g");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // h
        else if (L3_event_checkEventFlag(Chat_Rcvd))
        {
            pc.printf("h");
            L3_event_clearEventFlag(Chat_Rcvd);
        }

        // i
        else if (L3_event_checkEventFlag(SDU_Rcvd))
        {
            pc.printf("i");
            L3_event_clearEventFlag(SDU_Rcvd);
        }

        // j
        else if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {
            pc.printf("j");
            L3_event_clearEventFlag(Chat_Timer_Expire);
        }

        // m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            pc.printf("m");
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }
        break;

    // CHAT STATE
    case STATE_CHAT:

        // i
        if (L3_event_checkEventFlag(Chat_Timer_Expire))
        {    
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID == conID)
            {

            Msg_encodeDISPDU(pdu, MSG_RSC_Req);
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);
            
            main_state = STATE_DIS_WAIT;
            L3_event_clearEventFlag(Chat_Timer_Expire);

            }
            else
            {
                check =3;
            }
        }

        // j
        else if (L3_event_checkEventFlag(ReqDis_Rcvd))
        {
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID == conID)
            {
            pc.printf("STATE CHANGE 2 DIC CON ");

            // set
            Msg_encodeDISPDU(pdu, MSG_RSC_Set);
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

            // main_state = MAINSTATE_TX;
            main_state = STATE_DIS_WAIT;
            // flag_needPrint = 1;

            // L3_event_clearEventFlag(L3_event_dataRcvd);

            L3_event_clearEventFlag(ReqDis_Rcvd);
            }

            else
            {
                check = 3;
            }
        }
            
        // f
  
        else if (L3_event_checkEventFlag(ReqCON_Other_Rcvd))    //다른 ID로부터 req를 받으면
        {
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID == conID)
            {
            pc.printf("SetCON reject ");

            uint8_t srcId = L3_LLI_getSrcId();

            // setCON reject pdu 생성
            Msg_encodeCONPDU(pdu, MSG_RSC_Set, MSG_ACP_REJECT);
            L3_LLI_dataReqFunc(pdu, L3_PDU_SIZE, myDestId);

            flag_needPrint = 1;

            // L3_event_clearEventFlag(L3_event_dataRcvd);

            L3_event_clearEventFlag(ReqCON_Other_Rcvd);
            }

            else
            {
                check = 3;
            }
        }

        // g
        else if (L3_event_checkEventFlag(SDU_Rcvd)) // if data needs to be sent (keyboard input)
        {
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID == conID)
            {
            // msg header setting
            pduSize = Msg_encodeCHAT(sdu, originalWord, wordLen);
            //Msg_encodeCHAT
            L3_LLI_dataReqFunc(sdu, pduSize, myDestId);

            pc.printf("[MAIN] sending to %i \n", myDestId);

            
            flag_needPrint = 1;

            wordLen = 0;
            L3_event_clearEventFlag(SDU_Rcvd);
            }
            else
            {
                check = 3;
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
            uint8_t verseID = L3_LLI_getSrcId(); //accept의 id
            if(verseID == conID)
            {
            uint8_t* dataPtr = L3_LLI_getMsgPtr();
            uint8_t size = L3_LLI_getSize();

            debug("\n -------------------------------------------------\nRCVD MSG : %s (length:%i)\n -------------------------------------------------\n", 
                            dataPtr, size);
                

            pc.printf("Give a word to send : ");

            L3_event_clearEventFlag(Chat_Rcvd);
            }
            else
            {
                check = 3; 
            }
        }
        // CHAT으로 돌아오는 거
        // a
        else if (L3_event_checkEventFlag(ReqCON_Send))
        {
            pc.printf("a");
            L3_event_clearEventFlag(ReqCON_Send);
        }

        // b
        else if (L3_event_checkEventFlag(ReqCON_Rcvd))
        {
            pc.printf("b");
            L3_event_clearEventFlag(ReqCON_Rcvd);
        }

        // c
        else if (L3_event_checkEventFlag(SetCON_Accept_Rcvd))
        {
            pc.printf("c");
            L3_event_clearEventFlag(SetCON_Accept_Rcvd);
        }

        // d
        else if (L3_event_checkEventFlag(SetCON_Reject_Rcvd))
        {
            pc.printf("d");
            L3_event_clearEventFlag(SetCON_Reject_Rcvd);
        }

        // e
        else if (L3_event_checkEventFlag(CplCON_Rcvd))
        {
            pc.printf("e");
            L3_event_clearEventFlag(CplCON_Rcvd);
        }

        // k
        else if (L3_event_checkEventFlag(SetDis_Rcvd))
        {
            pc.printf("k");
            L3_event_clearEventFlag(SetDis_Rcvd);
        }

        // l
        else if (L3_event_checkEventFlag(CplDis_Rcvd))
        {
            pc.printf("l");
            L3_event_clearEventFlag(CplDis_Rcvd);
        }

        // m
        else if (L3_event_checkEventFlag(Other_PDU_Rcvd))
        {
            pc.printf("m");
            L3_event_clearEventFlag(Other_PDU_Rcvd);
        }
        break;

    default:
        break;
    }
}
