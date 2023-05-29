/*typedef enum L3_event
{
    L3_event_msgRcvd = 2,
    L3_event_dataToSend = 4,
    L3_event_dataSendCnf = 5,
    L3_event_recfgSrcIdCnf = 6,

    L3_event_dataConDone = 7,
    L3_event_dataDisDone = 8,

    // state IDLE
    ReqCON_Rcvd = 15,

    // state CON
    SetCON_Accept_Rcvd = 9,
    SetCON_Reject_Rcvd = 10,
    CplCON_Rcvd = 11,

    // state DIS
    SetDis_Rcvd = 12,
    CplDis_Rcvd = 13,

    //all state
    OTHERPDU = 14

} L3_event_e;

void L3_event_setEventFlag(L3_event_e event);
void L3_event_clearEventFlag(L3_event_e event);
void L3_event_clearAllEventFlag(void);
int L3_event_checkEventFlag(L3_event_e event);
*/

typedef enum L3_event
{
    /*
    L3_event_msgRcvd = 2,
    L3_event_dataToSend = 4,
    L3_event_dataSendCnf = 5,
    L3_event_recfgSrcIdCnf = 6,
    */

    // IDLE
    //연결을 위한 SDU 받음 event a
    ReqCON_Rcvd = 2, // event b 
    ReqCON_Send = 4, // 어떤 event인지 모르겠음

    L3_event_dataSendCnf = 5,
    L3_event_recfgSrcIdCnf = 6,

    L3_event_dataConDone = 7,
    L3_event_dataDisDone = 8,

    // state CON
    SetCON_Accept_Rcvd = 9, // event c
    SetCON_Reject_Rcvd = 10,// event d
    CplCON_Rcvd = 11,   // event e

    // 연결상대가 아닌 다른 ID에서 ReqCON을 받음 event f
    // 채팅 SDU를 받음 -> 채팅용 // event g
    // 채팅 상대에게 SDU를 받음 -> 채팅용  // event h

    // state Chat
    Chat_Timer_Expire = 14, // event i
    ReqDIS_Rcvd = 34, // event j
   
    // state DIS 
    SetDIS_Rcvd = 12, //event k 
    CplDIS_Rcvd = 13 // event l
     ReqCON_Other_Rcvd = 16 // event m

} L3_event_e;

void L3_event_setEventFlag(L3_event_e event);
void L3_event_clearEventFlag(L3_event_e event);
void L3_event_clearAllEventFlag(void);
int L3_event_checkEventFlag(L3_event_e event);

/*typedef enum L3_event
{
    L3_event_msgRcvd = 2,
    L3_event_dataToSend = 4,
    L3_event_dataSendCnf = 5,
    L3_event_recfgSrcIdCnf = 6,

    L3_event_dataConDone = 7,
    L3_event_dataDisDone = 8,

    // state IDLE
    ReqCON_Rcvd = 15,

    // state CON
    SetCON_Accept_Rcvd = 9,
    SetCON_Reject_Rcvd = 10,
    CplCON_Rcvd = 11,

    // state DIS
    SetDis_Rcvd = 12,
    CplDis_Rcvd = 13,

    //all state
    OTHERPDU = 14

} L3_event_e;

void L3_event_setEventFlag(L3_event_e event);
void L3_event_clearEventFlag(L3_event_e event);
void L3_event_clearAllEventFlag(void);
int L3_event_checkEventFlag(L3_event_e event);
*/