/********************************************************************************
DEFINITIONS
********************************************************************************/
#undef DEBUG                                     // sortie console pour debug
/*******************************************************************************/
#define BUSYPIN 4                                 // n° de la pin Busy
#define RTSPIN 6                                  // n° de la pin RTS (output)
#define CTSPIN 5                                  // n° de la pin CTS (INPUT)
#define LEDVERTE 13                               // LED verte pour test
/*******************************************************************************/
#define HOUR_ADJUST_CHECK 50UL*60UL*1000UL        // interval check pour la maj de l'heure de internet (50 minutes)
#define HOUR_ADJUST_CHECK_THIN 1UL*60UL*1000UL    // interval check pour la maj de l'heure de internet (10 minutes)
#define WAITFORLININO  55                         // temps d'attente de démarrage de linino (mini 50s)
/*******************************************************************************/
#define DS1338_NVRAM_REG_SAMPLING              0  // Adresse offset Nvram du DS1338 pour la periode d'échantillonage
#define DS1338_NVRAM_REG_UART_RTS_TELEINFO     1  // RTS qui dit qu'un message teleinfo est reçu
#define DS1338_NVRAM_REG_UART_REPEAT           2  // demande de renvoie du message
/*******************************************************************************/
#define DATE_STRING_SIZE 28                       // taille max du tableau qui contient la date
/*******************************************************************************/
#define DATE_ISO8601 1
#define DATE_CUSTOM 2
#define UNIX_TIME 3
#define DOW 4
/*******************************************************************************/
#define EPOCH_INCREMENT 1800                      // increment de la date en mode test (en secondes)
#define EPOCH_TIME 1428732560                     // date de depart du mode test
/*******************************************************************************/

/********************************************************************************
FUNCTION DECLARATION IN TELEINFO
********************************************************************************/
void GeneralInit(void);
String ProcExec(String Comm, String Par);
String LininoGetTimeStamp();
void I2C_RequestToSend(void);
void I2C_ClearToSend(void);
void Blink_Led(unsigned char count);
int run_python_script_config(char *str);
void PrintRtcDate(void);
char *getdate(char format);
void PrintLininoDate(void);
void WaitForLinino(void);
void Is_Uart_Data(void);
void SyncLininoClock(void);
char  *epoch_to_iso8601(unsigned long millisAtEpoch);
unsigned long timeInEpoch(void);
uint8_t Is_DST_Time(void);
void rtc_to_linino_date_update(void);
void Srv_UpdateTeleinfoDb(String);

/********************************************************************************
FUNCTION DECLARATION IN SERVICE
********************************************************************************/
void Srv_Out_Event(void);
void Srv_read_uart_data(void);
void Srv_AdjustDateEveryDay(void);
uint8_t Srv_PingGoogle(void);
uint8_t isConnectedToInternet(void);
void Srv_Ntp_To_Rtc_Update(void);


