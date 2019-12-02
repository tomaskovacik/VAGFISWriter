//enhanced version of:
//credits: https://github.com/arildlangseid/vw_t4_tcu_temp_to_fis

#ifndef VAGFISWriter_h
#define VAGFISWriter_h

#include <inttypes.h>
#include <Arduino.h>

// based on FIS_emulator

#define FIS_WRITE_PULSEW 4

#define PORT_3LB PORTB
#define DATA  PB3 //MOSI (Arduino Uno 11)
#define CLK   PB5 //SCK (Arduino 13)


// Positions in Message-Array
#define FIS_MSG_COMMAND 0
#define FIS_MSG_LENGTH 1
    static volatile uint8_t sendOutData;
    static uint8_t _FIS_WRITE_ENA;
    static uint8_t _FIS_WRITE_CLK;
    static uint8_t _FIS_WRITE_DATA;
    static char radioData[16];
    static uint8_t radioDataOK=0;

class VAGFISWriter
{
  public:
    VAGFISWriter(uint8_t clkPin, uint8_t dataPin, uint8_t enaPin);
    ~VAGFISWriter();
    void begin();

    uint8_t sendMsg(char msg[]);
    bool sendRadioMsg(char msg[16]);
    //bool sendRadioMsg(String msg);
    void sendString(String line1="", String line2="", bool center=true);
    void sendStringFS(int x, int y, String line);
    void sendMsgFS(uint8_t X,uint8_t Y,uint8_t font,uint8_t size,char msg[]);
    void initScreen(uint8_t mode,uint8_t X,uint8_t Y,uint8_t X1,uint8_t Y1);
    void reset(uint8_t mode = 0x82);
    void initMiddleScreen(uint8_t mode = 0x82);
    void initFullScreen(uint8_t mode = 0x82);
    //void sendRawMsg(char in_msg[]);
    uint8_t sendRawData(char data[]);
    void sendKeepAliveMsg();
    void radioDisplayOff();
    void radioDisplayBlank();
    void GraphicFromArray(uint8_t x,uint8_t y, uint8_t sizex,uint8_t sizey,const uint8_t data[],uint8_t mode);
    void GraphicOut(uint8_t x,uint8_t y, uint16_t size, uint8_t data[],uint8_t mode,uint8_t offset);
    void sendRadioData(uint8_t force = 0);
    static void enableGoesHigh(void);
    static void enableGoesLow(void);

    private:
    void sendByte(uint8_t in_byte);
    void setClockHigh();
    void setClockLow();
    void setDataHigh();
    void setDataLow();
    void startENA();
    void stopENA();
    uint8_t sendSingleByteCommand(uint8_t txByte);
    uint8_t waitEnaHigh();
    uint8_t waitEnaLow();
    uint8_t checkSum( volatile uint8_t in_msg[]);

};


#endif
