/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include "wifi_client_hal.h"
#include <ctype.h> 

#ifndef BOOL
#define BOOL  unsigned char
#endif
#ifndef TRUE
#define TRUE     1
#endif
#ifndef FALSE
#define FALSE    0
#endif
#define SIZE 64
#define TIME_WAIT 15


void* mallocAlloc(int typeData,int size);

typedef INT (*radioFunc)(INT radioIndex, CHAR *output_string);
void radioFunction(char *str,int value,BOOL dispAllValues );
void getallSSID(int radioIndex);
void getTrafficStats(int radioIndex);
void usage();
void wpsPushFunc(int waitTime);
INT test_wifi_connect_callback(INT ssidIndex, CHAR *AP_SSID, wifiStatusCode_t *error);
INT test_wifi_disconnect_callback(INT ssidIndex, CHAR *AP_SSID, wifiStatusCode_t *error);
int testWifiConnect(INT ssidIndex, CHAR *AP_SSID,CHAR *AP_security_KeyPassphrase);
void ssidDisconnect(char *,int );
pthread_mutex_t connMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t connCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t disconMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t disconCond = PTHREAD_COND_INITIALIZER;
int wpsWaitTime;
int disconnectWaitTime;
int wifiIntialized;
void test_wifi_init(void);
int check;
int connectFlag;
int disconnectFlag;
pthread_t connectThread=NULL;

static struct RadioStruct
{
    char* inputStr;
    radioFunc radioDetails;
} radioStruct[] =
{
    {"radioEnable", wifi_getRadioEnable},
    {"radioStatus", wifi_getRadioStatus},
    {"MaxBitRate", wifi_getRadioMaxBitRate},
    {"IfName", wifi_getRadioIfName},
    {"SuppFreqBands", wifi_getRadioSupportedFrequencyBands},
    {"OpFreqBands", wifi_getRadioOperatingFrequencyBand},
    {"SupportedStandards", wifi_getRadioSupportedStandards},
    {"possChannels", wifi_getRadioPossibleChannels},
    {"OpChannelBwidth", wifi_getRadioOperatingChannelBandwidth},
    {"ExtChannel", wifi_getRadioExtChannel},
    {"transmitPwrSupp", wifi_getRadioTransmitPowerSupported},
    {"basicDataTransRates", wifi_getRadioBasicDataTransmitRates},
    {"IEEE80211hSupported", wifi_getRadioIEEE80211hSupported},
    {"IEEE80211hEnabled", wifi_getRadioIEEE80211hEnabled},
    {"autoChannelSupported", wifi_getRadioAutoChannelSupported},
    {"autoChannelEnable", wifi_getRadioAutoChannelEnable},
    {"getRadioGuardInterval", wifi_getRadioGuardInterval}
};


int main(int argc, char * argv[])
{
	char value;
	int timeoutWps;
	int num;
	char ssid[72];
	char keyPassphrase[72];
	int timeout;
	wifi_connectEndpoint_callback_register(test_wifi_connect_callback);    
	wifi_disconnectEndpoint_callback_register(test_wifi_disconnect_callback);    
	usage();
	while(1)
	{
    		printf("-------------------------------------------------------------------------------------------\n");
    		printf("Enter your choice\n");	
    		printf("-------------------------------------------------------------------------------------------\n");
    		scanf(" %c",&value);
//		printf("\n value is  %c \n",value);
		if(value == 'a')
		{
			usage();
    			printf("-------------------------------------------------------------------------------------------\n");
    			printf("Enter your choice\n");	
    			printf("-------------------------------------------------------------------------------------------\n");
    			scanf(" %c",&value);
			
		}
		else if (value == 'q')
		{
    			printf("Quitting Test App press y to continue n to cancel it \n");
    			scanf(" %c",&value);
			if (value == 'y')
			{	
				exit(0);
			}
			else
			{
				printf("continuing test app\n");
				continue;
			}
		}	
		if(!isdigit(value))
		{ 
			printf("entered charcter %c is not numeric or the option number is wrong\n",value);
			continue;
			
		}
		num=value - '0';
		if (num >7)
		{
			printf("entered number %d not supported \n",num);
			continue;
		}
		switch(num)
		{
			case 1:
				test_wifi_init();
				printf("wifi intialization done \n",num);
				break;
			case 2:
    				printf("-------------------------------------------------------------------------------------------\n");
				printf("please specify the timeout for wps\n");
    				printf("-------------------------------------------------------------------------------------------\n");
				scanf(" %d",&timeoutWps);
				wpsPushFunc(timeoutWps);
				break;
			case 4:
				getallSSID(1);
				break;
                        case 5:
                                printf("-------------------------------------------------------------------------------------------\n");
                                printf("please specify the SSID\n");
                                printf("-------------------------------------------------------------------------------------------\n");
				scanf(" %s",ssid);
                                printf("-------------------------------------------------------------------------------------------\n");
                                printf("please specify the passphrase for ssid %s \n",ssid);
                                printf("-------------------------------------------------------------------------------------------\n");
				scanf(" %s",keyPassphrase);
                                testWifiConnect(1,ssid,keyPassphrase);
                                break;
                        case 6:
                                printf("-------------------------------------------------------------------------------------------\n");
                                printf("please specify the SSID\n");
                                printf("-------------------------------------------------------------------------------------------\n");
                                scanf(" %s",ssid);
                                printf("-------------------------------------------------------------------------------------------\n");
                                printf("please specify the timeout for disconnect\n");
                                printf("-------------------------------------------------------------------------------------------\n");
				scanf(" %d",&timeout);
                                ssidDisconnect(ssid,timeout);
                                break;

			case 7:
				exit(0);
			default:
				printf("options not supported \n ");
				continue;
			
		}
    		printf("-------------------------------------------------------------------------------------------\n");
		printf(" press a for list of options \n ");
    		printf("-------------------------------------------------------------------------------------------\n");
	}
	
}



#if 0
int main(int argc, char * argv[])
{
    int c;
    int longOptPtr=0;
    const char    *short_opt = "r:w::a:s:t:h::";
    char commands[SIZE];
    struct option   long_opt[] =
    {
        {"radio",       required_argument,0, 'r'},
        {"wps",        required_argument,0, 'w'},
        {"ap",          required_argument,0, 'a'},
        {"ssid",        required_argument,0, 's'},
        {"traffic",     required_argument,0, 't'},
        {"help",        optional_argument,0, 'h'}
    };
    printf("*  argc = %d argv = %s \n",argc,argv[0]);

    while((c = getopt_long(argc, argv, short_opt, long_opt, &longOptPtr)) != -1)
    {
        printf("Come here %d \n",longOptPtr);
        switch(c)
        {
        case -1:       /* no more arguments */
        case 0:        /* long options toggles */
            break;

        case 'r':
            if(argc == 3)
            {
                printf("bdchbcjshb");
                radioFunction(argv[1],atoi(argv[2]),TRUE);
            }
            else if(argc == 4)
            {
                radioFunction(argv[3],atoi(argv[2]),FALSE);
            }
            else
            {
                printf("\noptions are not right  %s  %d \n", optarg, argc);
                printf("./wifi -r <radio index> get all radio parameters based on radio index \n");
                printf("./wifi -r <radio index> <parameter>   to get individual radio parameter \n");
                printf("./wifi --radio <radio index> get all radio parameters based on radio index \n");
                printf("./wifi --radio <radio index> <parameter>  to get individual radio parameter \n");
            }
            // snprintf(commands,SIZE,"%s",optarg);
            break;

        case 'w':
		if(argc==3)
			wpsPushFunc(atoi(argv[2]));
		else
			wpsPushFunc(TIME_WAIT);
//         printf("you entered \"%s\"\n", optarg);
            /*	 printf("short option - %c  long option - %s\n",c, long_opt[longOptPtr].name);
            	 snprintf(commands,SIZE,"%s",optarg);*/
            break;

        case 'a':
            /*	 printf("short option - %c  long option - %s\n",c, long_opt[longOptPtr].name);
            	 snprintf(commands,SIZE,"%s",optarg);*/
            break;

        case 's':
            if(argc == 3)
            {
                getallSSID(atoi(argv[2]));
            }
            else
            {
                printf("\noptions are not right  %s  %d \n", optarg, argc);
                printf("./wifi -s <radio index> get all ssid information based on radio index\n");
                printf("./wifi --ssid <radio index> get all ssid information based on radio index\n");
            }
            break;

        case 't':
            if(argc == 3)
            {
                getTrafficStats(atoi(argv[2]));
            }
            else
            {
                printf("\noptions are not right  %s  %d \n", optarg, argc);
                printf("./wifi -t <radio index> get traffic data between radio and connected SSID\n");
                printf("./wifi --traffic <radio index> get traffic data between radio and connected SSID\n");
            }
            break;


        case 'h':
            usage();
            printf("\n");
            return(0);

        case ':':
        case '?':
            fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
            return(-2);

        default:
            fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
            fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
            return(-2);
        };
    };

    return(0);
}
#endif
void radioFunction(char *str,int value,BOOL dispAllValues)
{
    int i = 0;
    char output_string[64];
    printf(" inside radio function");
    int len = sizeof(radioStruct)/sizeof( struct RadioStruct );
    for (; i < len; i++ )
    {
        if(dispAllValues)
        {
            if(i == 0)
                printf("\n paramters \t----------\t value \n");

            radioStruct[i].radioDetails(value,output_string);
            printf("%s \t %s \n",radioStruct[i].inputStr,output_string);
        }
        else
        {
            if (strcmp(radioStruct[i].inputStr,str) == 0)
            {
                radioStruct[i].radioDetails(value,output_string);
                printf("\nradio index %d output string %s\n",value,output_string);
            }
        }
    }
}


void usage()
{

    printf("\n------------------------------------ Usage ----------------------------------------------\n");
    printf(" 1 ----- wifi init			------- \n");	
    printf(" 2 ----- press wps button 		------- \n");	
    printf(" 3 ----- get STA radio details 	------- \n");	
    printf(" 4 ----- SSID details 		------- \n");	
    printf(" 5 ----- connect to ssid 		------- \n");	
    printf(" 6 ----- disconnect from ssid	------- \n");	
    printf(" q ----- Quit	 		------- \n");	
    printf("-------------------------------------------------------------------------------------------\n");
}
#if 0
void usage()
{
    printf("\n------------------------------------ Usage ----------------------------------------------\n");
    printf("./wifi -r <radio index> get all radio parameters based on radio index \n");
    printf("./wifi --radio <radio index> get all radio parameters based on radio index \n");
    printf("./wifi -r <radio index> <parameter> to get individual radio parameter\n");
    printf("./wifi --radio <radio index> <parameter>  to get individual radio parameter\n");
    printf("./wifi -s <radio index> get all ssid information based on radio index\n");
    printf("./wifi --ssid <radio index> get all ssid information based on radio index\n");
    printf("./wifi -t <radio index> get traffic data between radio and connected SSID\n");
    printf("./wifi --traffic <radio index> get traffic data between radio and connected SSID\n");
    printf("\n-------------------------------------------- ----------------------------------------------\n");
}
#endif
void getallSSID(int radioIndex)
{
    wifi_neighbor_ap_t *neighborAPlist;
    UINT outputSize;
    UINT size;
    wifi_getNeighboringWiFiDiagnosticResult(1,&neighborAPlist,&outputSize);
    printf("\nradio no %d \n ",radioIndex);
    for(size=0; size < outputSize; size++)
    {
        printf("\n -----------------------------------------------------------------------------------------------------------------------------------------------\n");
        printf("  ap_SSID is %s \t",neighborAPlist[size].ap_SSID);
        printf("  ap_BSSID is %s \t",neighborAPlist[size].ap_BSSID);
        printf("  ap_Mode is %s \t",neighborAPlist[size].ap_Mode);
        printf("  ap_Channel is %d \t ",neighborAPlist[size].ap_Channel);
        printf("  ap_SignalStrength is %+d \t ",neighborAPlist[size].ap_SignalStrength);
        printf("\nap_SecurityModeEnabled is %s \t ",neighborAPlist[size].ap_SecurityModeEnabled);
        printf("  ap_EncryptionMode is %s \t ",neighborAPlist[size].ap_EncryptionMode);
        printf("  ap_OperatingFrequencyBand is %s \t ",neighborAPlist[size].ap_OperatingFrequencyBand);
        printf("\nap_SupportedStandards is %s \t ",neighborAPlist[size].ap_SupportedStandards);
        printf("  ap_OperatingStandards is %s \t ",neighborAPlist[size].ap_OperatingStandards);
        printf("\nap_OperatingChannelBandwidth is %s \t ",neighborAPlist[size].ap_OperatingChannelBandwidth);
        printf("  ap_BeaconPeriod is %d \t ",neighborAPlist[size].ap_BeaconPeriod);
        printf("  ap_Noise is %+d \t ",neighborAPlist[size].ap_Noise);
        printf("  ap_BasicDataTransferRates is %s \t ",neighborAPlist[size].ap_BasicDataTransferRates);
        printf("\nap_SupportedDataTransferRates is %s \t ",neighborAPlist[size].ap_SupportedDataTransferRates);
        printf("  ap_DTIMPeriod is %d \t ",neighborAPlist[size].ap_DTIMPeriod);
    }
    printf("\n -----------------------------------------------------------------------------------------------------------------------------------------------\n");
    free(neighborAPlist);

}

void getTrafficStats(int radioIndex)
{
    wifi_radioTrafficStats_t *trafficStats;
    trafficStats = (wifi_radioTrafficStats_t *)malloc(sizeof(wifi_radioTrafficStats_t));
    if(trafficStats == NULL)
    {
        printf("Malloc Memory allocation failure\n");
        return;
    }
    printf("malloc allocated = %d ", malloc_usable_size(trafficStats));
    wifi_getRadioTrafficStats(radioIndex, trafficStats);
    printf("\n -----------------------------------------------------------------------------------------------------------------------------------------------\n");
    printf(" BytesSent = %lu ",trafficStats->radio_BytesSent);
    printf(" BytesReceived = %lu ",trafficStats->radio_BytesReceived);
    printf(" PacketsSent = %lu ",trafficStats->radio_PacketsSent);
    printf(" PacketsReceived = %lu ",trafficStats->radio_PacketsReceived);
    printf(" ErrorsSent = %lu ",trafficStats->radio_ErrorsSent);
    printf(" ErrorsReceived = %lu ",trafficStats->radio_ErrorsReceived);
    printf(" DiscardPacketsSent = %lu ",trafficStats->radio_DiscardPacketsSent);
    printf(" \nDiscardPacketsReceived = %lu ",trafficStats->radio_DiscardPacketsReceived);
    printf(" PLCPErrorCount = %lu ",trafficStats->radio_PLCPErrorCount);
    printf(" FCSErrorCount = %lu ",trafficStats->radio_FCSErrorCount);
    printf(" InvalidMACCount = %lu ",trafficStats->radio_InvalidMACCount);
    printf(" PacketsOtherReceived = %lu ",trafficStats->radio_PacketsOtherReceived);
    printf(" NoiseFloor = %d ",trafficStats->radio_NoiseFloor);
    printf("\n ChannelUtilization = %lu ",trafficStats->radio_ChannelUtilization);
    printf(" ActivityFactor = %d ",trafficStats->radio_ActivityFactor);
    printf(" CarrierSenseThreshold_Exceeded = %d ", trafficStats->radio_CarrierSenseThreshold_Exceeded);
    printf(" RetransmissionMetirc = %d ",trafficStats->radio_RetransmissionMetirc);
    printf(" MaximumNoiseFloorOnChannel = %d ",trafficStats->radio_MaximumNoiseFloorOnChannel);
    printf("\n MinimumNoiseFloorOnChannel = %d ",trafficStats->radio_MinimumNoiseFloorOnChannel);
    printf(" MedianNoiseFloorOnChannel = %d ",trafficStats->radio_MedianNoiseFloorOnChannel);
    printf(" StatisticsStartTime = %lu ",trafficStats->radio_StatisticsStartTime);
    printf("\n -----------------------------------------------------------------------------------------------------------------------------------------------\n");
    free(trafficStats);
}


void *connThreadFunc(void* arg)
{
    struct timespec waitTime;
    struct timeval now;
    int ret;

    ret =  gettimeofday(&now, NULL);
    
    waitTime.tv_sec  = now.tv_sec;
    waitTime.tv_nsec = now.tv_usec * 1000;
    waitTime.tv_sec += wpsWaitTime;
    
    pthread_mutex_lock(&connMutex);
    ret=0;
    while (!connectFlag && ret != ETIMEDOUT)
    {
    	ret = pthread_cond_timedwait(&connCond, &connMutex, &waitTime);
    }
    if (ret == ETIMEDOUT) 
    {
	check=1;
        printf("timed out connecting to AP \n");
    }
    connectFlag=0;
    ret = pthread_mutex_unlock(&connMutex);
    sleep(10);
    pthread_exit(NULL);
}

void *disconnectThreadFunc(void* arg)
{
    struct timespec waitTime;
    struct timeval now;
    int ret;

    ret =  gettimeofday(&now, NULL);
    
    waitTime.tv_sec  = now.tv_sec;
    waitTime.tv_nsec = now.tv_usec * 1000;
    waitTime.tv_sec += disconnectWaitTime;
    
    pthread_mutex_lock(&disconMutex);
    ret=0;
    while (!disconnectFlag && ret != ETIMEDOUT)
    {
    	ret = pthread_cond_timedwait(&disconCond, &disconMutex, &waitTime);
    }
    if (ret == ETIMEDOUT) 
    {
	check=1;
        printf("timed out connecting to AP \n");
    }
    disconnectFlag=0;
    ret = pthread_mutex_unlock(&disconMutex);

    pthread_exit(NULL);
}

INT test_wifi_disconnect_callback(INT ssidIndex, CHAR *AP_SSID, wifiStatusCode_t *error)
{
    pthread_mutex_lock(&disconMutex);
    disconnectFlag=1;
    pthread_cond_broadcast(&disconCond);
    pthread_mutex_unlock(&disconMutex);
    if(*error == WIFI_HAL_SUCCESS)
    {
        printf("disconnected from %s \n ", AP_SSID);
        return RETURN_OK;
     }
     printf("disonnection error %d to ssid %s  \n ",*error,AP_SSID);
     return RETURN_ERR;
//    pthread_exit(NULL);
}


INT test_wifi_connect_callback(INT ssidIndex, CHAR *AP_SSID, wifiStatusCode_t *error)
{
    pthread_mutex_lock(&connMutex);
    connectFlag=1;
    pthread_cond_broadcast(&connCond);
    pthread_mutex_unlock(&connMutex);
    if(*error == WIFI_HAL_SUCCESS)
    {
   	printf("Connected to %s \n ", AP_SSID);
  	return RETURN_OK;
     }
     printf("Connection error %d to %s  \n ",*error,AP_SSID);
     return RETURN_ERR;
//    pthread_exit(NULL);
}

void wpsPushFunc(int waitTime)
{
    void *ret;
    
    wpsWaitTime=waitTime;
    if(connectThread == NULL)
	pthread_create(&connectThread, NULL,connThreadFunc, NULL);
    else
	printf("connecting to AP is going on please try after sometime \n");
//    pthread_join(wpsThread,&ret);
    if(wifi_setCliWpsButtonPush(1) == RETURN_OK)
    {
        printf("\n WPS push button press success \n ");
    }
    else
    {
        printf("\n WPS button press failed \n ");
        return;
    }
//    pthread_join(wpsThread,&ret);
//    pthread_cond_destroy(&connCond);
//    pthread_mutex_destroy(&connMutex);

}
void ssidDisconnect(char* ssid,int waitTime)
{
	
    pthread_t disconnectThread;
    void *ret;
    disconnectWaitTime=waitTime;
    pthread_create(&disconnectThread, NULL,disconnectThreadFunc, NULL);
	//    pthread_join(wpsThread,&ret);
    wifi_disconnectEndpoint(1, ssid);
}

void test_wifi_init()
{
    if(wifi_init() == RETURN_OK)
    {
    	wifiIntialized=1;
        printf("\n WiFi intialize success  \n ");
    }
    else
    {
        printf("\n WiFi Intialization failure \n ");
    }
}

int testWifiConnect(INT ssidIndex, CHAR *AP_SSID,CHAR *AP_security_KeyPassphrase)
{
	int ret;
	wpsWaitTime=3;
    	if(connectThread == NULL)
	{
		pthread_create(&connectThread, NULL,connThreadFunc, NULL);
	}
    	else
	{
		printf("testWifiConnect - connecting to AP is going on please try after sometime \n");
		return;
	}
        ret=wifi_connectEndpoint(ssidIndex,AP_SSID,WIFI_SECURITY_NONE,NULL,NULL,AP_security_KeyPassphrase, 1, NULL, NULL, NULL, NULL);
	if(ret)
	{
		printf("Error in connecting to ssid %s  with passphrase %s \n",AP_SSID,AP_security_KeyPassphrase);
	}
	else
	{
		printf("connecting to ssid %s  with passphrase %s \n",AP_SSID,AP_security_KeyPassphrase);
	}


}

