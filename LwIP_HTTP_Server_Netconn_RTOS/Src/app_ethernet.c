/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Netconn_RTOS/Src/app_ethernet.c 
  * @author  MCD Application Team
  * @brief   Ethernet specefic module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip/opt.h"
#include "main.h"
#include "lwip/dhcp.h"
#include "app_ethernet.h"
#include "ethernetif.h"
#include "lcd_log.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/apps/mqtt_opts.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#ifdef USE_DHCP
#define MAX_DHCP_TRIES  4
__IO uint8_t DHCP_state = DHCP_OFF;
#endif

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Notify the User about the network interface config status
  * @param  netif: the network interface
  * @retval None
  */
void User_notification(struct netif *netif) 
{
  if (netif_is_up(netif))
  {
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_START;
#else
    uint8_t iptxt[20];
    sprintf((char *)iptxt, "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
    LCD_UsrLog ("Static IP address: %s\n", iptxt);
#endif /* USE_DHCP */
  }
  else
  {  
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_LINK_DOWN;
#endif  /* USE_DHCP */
    LCD_UsrLog ("The network cable is not connected \n");
  } 
}

#ifdef USE_DHCP
/**
  * @brief  DHCP Process
  * @param  argument: network interface
  * @retval None
  */
void DHCP_thread(void const * argument)
{
  struct netif *netif = (struct netif *) argument;
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
  struct dhcp *dhcp;
  uint8_t iptxt[20];
  
  for (;;)
  {
    switch (DHCP_state)
    {
    case DHCP_START:
      {
        ip_addr_set_zero_ip4(&netif->ip_addr);
        ip_addr_set_zero_ip4(&netif->netmask);
        ip_addr_set_zero_ip4(&netif->gw);       
        dhcp_start(netif);
        DHCP_state = DHCP_WAIT_ADDRESS;
        LCD_UsrLog ("  State: Looking for DHCP server ...\n");
      }
      break;
      
    case DHCP_WAIT_ADDRESS:
      {                
        if (dhcp_supplied_address(netif)) 
        {
          DHCP_state = DHCP_ADDRESS_ASSIGNED;	
         
          sprintf((char *)iptxt, "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));   
          LCD_UsrLog ("IP address assigned by a DHCP server: %s\n", iptxt);

		#ifdef USE_MQTT
			/* Start DHCPClient */
			osThreadDef(MQTT, MQTT_thread, osPriorityLow, 0, configMINIMAL_STACK_SIZE * 2);
			osThreadCreate (osThread(MQTT), NULL);
		#endif
        }
        else
        {
          dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);
    
          /* DHCP timeout */
          if (dhcp->tries > MAX_DHCP_TRIES)
          {
            DHCP_state = DHCP_TIMEOUT;
            
            /* Stop DHCP */
            dhcp_stop(netif);
            
            /* Static address used */
            IP_ADDR4(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
            IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
            IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
            netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));
            
            sprintf((char *)iptxt, "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
            LCD_UsrLog ("DHCP Timeout !! \n");
            LCD_UsrLog ("Static IP address: %s\n", iptxt);  
          }
        }
      }
      break;
  case DHCP_LINK_DOWN:
    {
      /* Stop DHCP */
      dhcp_stop(netif);
      DHCP_state = DHCP_OFF; 
    }
    break;
    default: break;
    }
    /* wait 250 ms */
    osDelay(250);
  }
}
#endif  /* USE_DHCP */



#ifdef USE_MQTT
/**
  * @brief  MQTT Process
  * @retval None
  */
void MQTT_thread(void const * argument)
{
	// get IP address of the host
	ip_addr_t server_ip;
	IP4_ADDR(&server_ip, 192,168,1,122);

	/* Set client information */
	struct mqtt_connect_client_info_t client_info;
	memset(&client_info, 0, sizeof(client_info));
	client_info.client_id = "lwip_test";
	client_info.client_user = NULL;
	client_info.client_pass = NULL;
	client_info.keep_alive = 0;
	client_info.will_topic = NULL;
	client_info.will_msg = NULL;
	client_info.will_retain = 0;
	client_info.will_qos = 0;

	/* Allocate memory for MQTT client */
	mqtt_client_t client;
	memset(&client, 0, sizeof(mqtt_client_t));

	/* Connect to the server */
	err_t err;
	err = mqtt_client_connect(&client, &server_ip.addr, 1883, mqtt_connection_cb, 0, &client_info);
	if(err != ERR_OK)
	{
		LCD_UsrLog("mqtt_client_connect return: %d\n", err);
	}
	for(;;){
		osDelay(250);
	}
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
  err_t err;
  if(status == MQTT_CONNECT_ACCEPTED) {
	  LCD_UsrLog("mqtt_connection_cb: Successfully connected\n");

	err = mqtt_subscribe(client, "NB-CB_temperature", 0, mqtt_sub_request_cb, arg);
	LCD_UsrLog((char *)"subscribe topic NB-CB_temperature: %d\n", err);
	err = mqtt_subscribe(client, "NB-CB_humidity", 0, mqtt_sub_request_cb, arg);
	LCD_UsrLog((char *)"subscribe topic NB-CB_humidity: %d\n", err);
	mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);

    if(err != ERR_OK) {
    	LCD_UsrLog("mqtt_subscribe return: %d\n", err);
    }
  } else {
	  LCD_UsrLog("mqtt_connection_cb: Disconnected, reason: %d\n", status);
	  MQTT_thread(NULL);
  }
}

static void mqtt_sub_request_cb(void *arg, err_t result)
{
	LCD_UsrLog("Subscribe result: %d\n", result);
}

enum {
	TOPIC_TEMPERATURE,
	TOPIC_HUMIDITY
};

static int inpub_id;
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{

  /* Decode topic string into a user defined reference */
  if(strcmp(topic, "JGD-OPT_temperature") == 0) {
    inpub_id = TOPIC_TEMPERATURE;
  }
    /* Decode topic string into a user defined reference */
  if(strcmp(topic, "JGD-OPT_humidity") == 0) {
    inpub_id = TOPIC_HUMIDITY;
  }
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{

  if(flags & MQTT_DATA_FLAG_LAST) {

	  if(inpub_id == TOPIC_HUMIDITY) {
		  char str[50];
		  for(int i = 0; i < len; i++) {
			  str[i] = (char) data[i];
		  }
		  LCD_UsrLog((char *)"Humidity %s\n", str);
	  }

	  if(inpub_id == TOPIC_TEMPERATURE) {
		  char str[50];
		  for(int i = 0; i < len; i++) {
			  str[i] = (char) data[i];
		  }
		  LCD_UsrLog((char *)"Temperature %s\n", str);
	  }

  }
}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
