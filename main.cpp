/* WiFi Example
 * Copyright (c) 2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include "mbed.h"
#include "TCPSocket.h"
#include "TCPServer.h"
#include "stm32l475e_iot01_accelero.h"
#include <stdlib.h>
#include <queue>
#include <cmath>

//forward decla. of Viterbi
typedef struct{
	char model_name;
	int state_num = 3;
	int observ_num = 8;
	double initial[3];			//initial prob.
                                //usage: initial[s] = initial prob. of state s
	double transition[3][3];	//transition prob.
                                //usage: transition[n_s][c_s] = prob. from current state c_s to next state n_s
	double observation[8][3];	//observation prob.
                                //usage: observation[o_v][s] = prob. of seeing observation value o_v at state s
} HMM;

void viterbi(HMM*, int, int*, int, double*);
//end of forward decla. of Viterbi

#define WIFI_IDW0XX1    2

#if (defined(TARGET_DISCO_L475VG_IOT01A) || defined(TARGET_DISCO_F413ZH))
#include "ISM43362Interface.h"
ISM43362Interface wifi(false);

#else // External WiFi modules

#if MBED_CONF_APP_WIFI_SHIELD == WIFI_IDW0XX1
#include "SpwfSAInterface.h"
SpwfSAInterface wifi(MBED_CONF_APP_WIFI_TX, MBED_CONF_APP_WIFI_RX);
#endif // MBED_CONF_APP_WIFI_SHIELD == WIFI_IDW0XX1

#endif

#define SCALE_MULTIPLIER    0.004


const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

int scan_demo(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;

    printf("Scan:\n");

    int count = wifi->scan(NULL,0);
    printf("%d networks available.\n", count);

    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;

    ap = new WiFiAccessPoint[count];
    count = wifi->scan(ap, count);
    for (int i = 0; i < count; i++)
    {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }

    delete[] ap;
    return count;
}

void acc_server(NetworkInterface *net)
{
    /* 
    TCPServer socket;
    TCPSocket* client;*/
    TCPSocket socket;
    SocketAddress addr("192.168.1.237",65431);
    nsapi_error_t response;

    int16_t pDataXYZ[3] = {0};
    char recv_buffer[9];
    char acc_json[64];
    int sample_num = 0;
    queue<double> window;

    

    // Open a socket on the network interface, and create a TCP connection to addr
    response = socket.open(net);
    if (0 != response){
        printf("Error opening: %d\n", response);
    }
    response = socket.connect(addr);
    
    if (0 != response){
        printf("Error connecting: %d\n", response);
    }


    
    /*while (sample_num < 1000){
        ++sample_num;
        BSP_ACCELERO_AccGetXYZ(pDataXYZ);
        float x = pDataXYZ[0]*SCALE_MULTIPLIER, y = pDataXYZ[1]*SCALE_MULTIPLIER, z = pDataXYZ[2]*SCALE_MULTIPLIER;
        int len = sprintf(acc_json,"%f,%f,%f,",(float)((int)(x*10000))/10000,
                                        (float)((int)(y*10000))/10000, (float)((int)(z*10000))/10000);

            
        response = socket.send(acc_json,len);
        if (0 >= response){
            printf("Error seding: %d\n", response);
        }
        wait(0.01);
    }*/

    //declaration of Viterbi variables
    int movement_num = 4;
    HMM movement_arr[movement_num];

    //end of declaration of Viterbi variables
    while(1){
        BSP_ACCELERO_AccGetXYZ(pDataXYZ);
        float x = pDataXYZ[0]*SCALE_MULTIPLIER, y = pDataXYZ[1]*SCALE_MULTIPLIER, z = pDataXYZ[2]*SCALE_MULTIPLIER;
        double sqr_sum = x*x + y*y + z*z;
        double abs_acc = sqrt(sqr_sum);
        if(abs_acc>11){
            window.push(7);
        }
        else if(abs_acc>8.7){
            window.push(6);
        }
        else if(abs_acc>6.6){
            window.push(5);
        }
        else if(abs_acc>4.7){
            window.push(4);
        }
        else if(abs_acc>3.4){
            window.push(3);
        }
        else if(abs_acc>2.2){
            window.push(2);
        }
        else if(abs_acc>1.1){
            window.push(1);
        }
        else{
            window.push(0);
        }
        if (window.size()==150){
            //masturbate
            window.pop();
        }
    }
    socket.close();
}


int main()
{

    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    //wifi.set_network("192.168.130.105","255.255.255.0","192.168.130.254");
    int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error\n");
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", wifi.get_mac_address());
    printf("IP: %s\n", wifi.get_ip_address());
    printf("Netmask: %s\n", wifi.get_netmask());
    printf("Gateway: %s\n", wifi.get_gateway());
    printf("RSSI: %d\n\n", wifi.get_rssi());


    BSP_ACCELERO_Init();    

    acc_server(&wifi);



}
void viterbi(HMM* hmm, int seqlen, int* seq, int model_num, double* p) {
	

	for(int i=0;i<model_num;i++)
	{
		double viterbi[hmm[model_num].state_num][seqlen];
		for(int j=0;j<hmm[model_num].state_num;j++)
		{
			viterbi[j][0]=hmm[i].initial[j]*hmm[i].observation[seq[0]][j];
		}
		for(int k=1;k<seqlen;k++)
		{
			for(int l=0;l<hmm[model_num].state_num;l++)
			{
				double max=0;
				for(int m=0;m<hmm[model_num].state_num;m++)
				{
					if(viterbi[m][k-1]*hmm[i].transition[m][l]>max)
					{
						max=viterbi[m][k-1]*hmm[i].transition[m][l];
					}
				}
				viterbi[l][k]=max*hmm[i].observation[seq[k]][l];
			}
		}
		for(int a=0;a<hmm[model_num].state_num;a++)
		{
			if(viterbi[a][seqlen-1]>p[i])
			{
				p[i]=viterbi[a][seqlen-1];
			}
		}
	}
	//return p;
	
}
