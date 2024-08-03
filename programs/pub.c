#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

#define CLIENTID    "sensor"
#define PAYLOAD     "USEFULL_DATA"
#define QOS         1
#define TIMEOUT     10000L
#define DELAY       5

struct sensor
{
    uint16_t    year;
    uint8_t    month;
    uint16_t     day;
    uint8_t     hour;
    uint8_t   minute;
    int8_t      temp;
};

void addrecord(struct sensor* info, int number, uint16_t year, uint8_t month, uint16_t day, uint8_t hour, uint8_t minute, int8_t temp)
{
    info[number].year   = year;
    info[number].month  = month;
    info[number].day    = day;
    info[number].hour   = hour;
    info[number].minute = minute;
    info[number].temp   = temp;
}

int main()
{
    int max = 20;
    int rc;
    int Y,M,D,H,MIN,T;
    int r;
    int count       = 0;
    int i           = 0;
    char str_filename[max];
    char str_address [max];
    char str_user    [max];
    char str_password[max];
    char str_topic   [max];
    const char *address;
    const char *username;
    const char *password;
    const char *topic;
    
    

    printf("Input login:");			        //login
	scanf("%s", str_user);
	
	printf("Input password:");	            //password
	scanf("%s", str_password);

	printf("Input host + port:");			//host
	scanf("%s", str_address);
	
	printf("input topic:");					//topic
	scanf("%s", str_topic);
	
	printf("Input filename:");			    //filename
	scanf("%s", str_filename);

    username = str_user;
    address  = str_address;
    password = str_password;
    topic    = str_topic;
    
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
   
    MQTTClient_create(&client, address, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession      = 1;
    conn_opts.username          = username;
    conn_opts.password          = password;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    struct sensor*info = malloc(365*24*60*sizeof(struct sensor));
    FILE *file;
    file = fopen(str_filename,"r");

   
    for (;(r=fscanf(file,"%d;%d;%d;%d;%d;%d",&Y,&M,&D,&H,&MIN,&T))>0;count++)
    {
        if (r<6)
        {
            char s[20], c;
            r = fscanf (file, "%[^\n]%c",s,&c);
            printf("Wrong format in line %s\n",s);
        }
        else
        {
            printf("%d %d %d %d %d %d\n",Y,M,D,H,MIN,T);
            addrecord(info,count,Y,M,D,H,MIN,T);
        }
	}
    fclose(file);

    while(1)
    {
        clock_t begin = clock();       

        char str[255];
        sprintf(str,"%d",info[i++].temp);
        printf("%s,%d\n",str,i);
        
        if(i>=count)
            i=0;
        
        
        pubmsg.payload = str;
        pubmsg.payloadlen = strlen(str);
        pubmsg.qos = QOS;
        pubmsg.retained = 0;
        MQTTClient_publishMessage(client, topic, &pubmsg, &token);
        rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
        
        while ((double)(clock() - begin)/CLOCKS_PER_SEC<DELAY)
        {}
    }
    
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}


