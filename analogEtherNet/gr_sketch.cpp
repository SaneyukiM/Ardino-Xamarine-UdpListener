#include <rxduino.h>
#include <ethernet.h>
#include <string>
#include <vector>

#define INTERVAL 120
#define BUFFER_SIZE 80

BOOL onReceiveUDP(unsigned char dest_ip[4], unsigned char src_ip[4],
    unsigned short dest_port,unsigned  short src_port, unsigned char *data, short datalen);

unsigned char ledpat[6][4] = {
    { 20, 50, 100, 200 },
    {  50,100,200,  20 },
    { 100,200, 20,  50 },
    { 200, 20, 50, 100 },
    { 100,200, 20,  50 },
    {  50,100,200,  20 },
};

bool has_ether_init = false;
bool has_dhcp_get = false;

byte mac[] = {0x00,0x16,0x3e,0x4e,0x8a,0x6c};
//byte dstip[] = {192,168,11,255};
byte dstip[] = {192,168,11,7};
const unsigned short broadcastport = 11001;
const unsigned short socketport = 55000;
TEthernet Ethernet;

int getFlag;
unsigned char memo_ip[4];
std::string udpRecvMsg;

bool EtherInit()
{
    Ethernet.begin(mac);
    Ethernet.registUdpHandler(onReceiveUDP);
    has_ether_init = true;
    return Ethernet.isLinkup();
}

void setup()
{
	pinMode(PIN_LED0,OUTPUT);
	pinMode(PIN_LED1,OUTPUT);
	pinMode(PIN_LED2,OUTPUT);
	pinMode(PIN_LED3,OUTPUT);
	
	pinMode(PIN_P40,INPUT);
	analogReference(RAW12BIT);
	
    udpRecvMsg = "First";
    getFlag = 0;
	memo_ip[0] = 0;
	memo_ip[1] = 0;
	memo_ip[2] = 0;
	memo_ip[3] = 0;
}

void waveIndicator()    //正常時
{
	digitalWrite(PIN_LED0, 1);
	delay(INTERVAL);
	digitalWrite(PIN_LED1, 1);
	delay(INTERVAL);
	digitalWrite(PIN_LED2, 1);
	delay(INTERVAL);
	digitalWrite(PIN_LED3, 1);
	delay(INTERVAL);
	digitalWrite(PIN_LED0, 0);
	delay(INTERVAL);
	digitalWrite(PIN_LED1, 0);
	delay(INTERVAL);
	digitalWrite(PIN_LED2, 0);
	delay(INTERVAL);
	digitalWrite(PIN_LED3, 0);
	delay(INTERVAL);
}

void blinkIndicator()    //異常時
{
    for(int i=0;i<5;i++)
    {
    	digitalWrite(PIN_LED0, 1);
    	digitalWrite(PIN_LED1, 1);
    	digitalWrite(PIN_LED2, 1);
    	digitalWrite(PIN_LED3, 1);
    	delay(INTERVAL);
    	digitalWrite(PIN_LED0, 0);
    	digitalWrite(PIN_LED1, 0);
    	digitalWrite(PIN_LED2, 0);
    	digitalWrite(PIN_LED3, 0);
    	delay(INTERVAL);
    }

    /*
    int i;
    for (i = 0; i < 6; i++){
        analogWrite(PIN_LED0, ledpat[i][0]);
        analogWrite(PIN_LED1, ledpat[i][1]);
        analogWrite(PIN_LED2, ledpat[i][2]);
        analogWrite(PIN_LED3, ledpat[i][3]);
        delay(100);
    }
    */
}

int findNull(unsigned char* str , int size )
{
	for (int i = 0; i < size; ++i)
	{
		if (str[i] == NULL) return i;
	}
	return size;
}

void loop()
{
    if(!has_ether_init) 
    {
        EtherInit();
    }

    if(Ethernet.isLinkup())
    {
        if(!has_dhcp_get)
        {
            has_dhcp_get = Ethernet.dhcp();
        }
        if(has_dhcp_get)
        {
        	unsigned char buff[BUFFER_SIZE];
        	double volt = analogRead(PIN_P40) * 5.0 / 1024;
            //double volt = 44.55;
	        int n = volt - fmod(volt , 1.0);
	        int m = fmod(volt, 1.0) * 1000;
        	//sprintf((char*)buff, "(%9du)    %3d.%3dV  %s \n",  micros(), n,m , udpRecvMsg.c_str());
        	sprintf((char*)buff, "(%9du)    %3d.%3dV  R:%d dev:%d.%d host:%d.%d \n",  micros(), n,m , getFlag, memo_ip[0] , memo_ip[2] , memo_ip[1] , memo_ip[3] );
        	
        	udpRecvMsg = "";
        	Ethernet.sendUDP(dstip,broadcastport,buff  ,findNull( buff, BUFFER_SIZE ));
        }
        waveIndicator();

    }
    else
    {
        blinkIndicator();
        has_dhcp_get = false;
    }

}

BOOL onReceiveUDP(
    unsigned char dest_ip[4],
    unsigned char src_ip[4],
    unsigned short dest_port,
    unsigned short src_port,
    unsigned char *data, // UDPパケットのデータ部
    short datalen          // UDPパケットのデータの長さ(ヘッダは含まない)
)
{
    if (src_port == 22136) return false;
    
    unsigned char buff[BUFFER_SIZE];
    memo_ip[0] = dest_ip[2];
	memo_ip[1] = src_ip[2];
    memo_ip[2] = dest_ip[3];
	memo_ip[3] = src_ip[3];

    getFlag++;

    return false; // この関数で処理したことにしない
}

