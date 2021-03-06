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



//#include "mbed.h"
//#include "TCPSocket.h"
//#include "TCPServer.h"
//#include "stm32l475e_iot01_accelero.h"
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <cmath>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
using namespace std;

#ifndef STATE_NUM
#	define STATE_NUM	3
#endif

#ifndef OBSERV_NUM
#	define OBSERV_NUM	8
#endif

#ifndef MOVEMENT_NUM
#   define MOVEMENT_NUM 5
#endif


//forward decla. of Viterbi
typedef struct{
	char* model_name;
	int state_num;
	int observ_num;
	double initial[STATE_NUM];			//initial prob.
                                //usage: initial[s] = initial prob. of state s
	double transition[STATE_NUM][STATE_NUM];	//transition prob.
                                //usage: transition[n_s][c_s] = prob. from current state c_s to next state n_s
	double observation[OBSERV_NUM][STATE_NUM];	//observation prob.
                                //usage: observation[o_v][s] = prob. of seeing observation value o_v at state s
} HMM;

void HMM_init(HMM *, char *, int, int, double[], double[][STATE_NUM], double[][STATE_NUM]);
static void dumpHMM(FILE *, HMM *);
void viterbi(HMM *, int, int *, int, double *);
//end of forward decla. of Viterbi
/*
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

    //Limit number of network arbitrary to 15 
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
*/
void acc_server(char** argv)
{
    /* 
    TCPServer socket;
    TCPSocket* client;*/
    /*
    TCPSocket socket;
    SocketAddress addr("192.168.1.237",65431);
    nsapi_error_t response;
    */
    int16_t pDataXYZ[3] = {0};
    char recv_buffer[9];
    char acc_json[64];
    int sample_num = 0;
    //queue<double> window;
    const int buff_size = 450;
    const int win_size = 150;
    int iter = 0;
    //int true_iter = 0;
    int buffer[buff_size];

    // Open a socket on the network interface, and create a TCP connection to addr
    /*
    response = socket.open(net);
    if (0 != response){
        printf("Error opening: %d\n", response);
    }
    response = socket.connect(addr);
    
    if (0 != response){
        printf("Error connecting: %d\n", response);
    }
    */

    
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
    //int movement_num = 4;
    HMM movement_arr[MOVEMENT_NUM];
    double pre_pi[STATE_NUM] = {1,9.57E-77,1.82E-41};
    double pre_a[STATE_NUM][STATE_NUM] = {{0.938655874,0.128135097,0.195148175},{0.034714224,0.869569486,3.22172E-25},{0.026629902,0.002295417,0.804851825}};
    double pre_b[OBSERV_NUM][STATE_NUM] = {
        {2.05437E-15,0.012571376,5.05122E-13},
        {2.52998E-22,0.114285236,2.2291E-27},
        {2.27719E-07,0.873138341,2.53256E-25},
        {0.999999772,5.04742E-06,0.002870178},
        {5.84208E-21,3.58201E-37,0.981989022},
        {7.82575E-51,1.4501E-169,0.0151408},
        {0,0,0},
        {0,0,0}};
    double fall_pi[STATE_NUM] = {1,2.02885E-62,1.4382E-69};
    double fall_a[STATE_NUM][STATE_NUM]={{0.847339186,0.094980361,0.117705798},{0.075112028,0.883376935,0.029549676},{0.077548786,0.021642704,0.852744525}};
    double fall_b[OBSERV_NUM][STATE_NUM]={
        {1.16527E-35,3.66807E-31,0.015534493},
        {1.76139E-20,0.000531981,0.166125668},
        {5.75741E-07,6.65886E-05,0.81833829},
        {0.999946355,0.000510303,1.5493E-06},
        {5.3069E-05,0.465112339,3.67054E-12},
        {1.87443E-14,0.273749465,1.1375E-11},
        {1.3259E-10,0.221482264,3.84995E-25},
        {1.53108E-27,0.038547061,1.69949E-30}};
    double run_pi[STATE_NUM] = {5.23E-193,1,8.79E-132};
    double run_a[STATE_NUM][STATE_NUM] = {{0.911397594,0.004520484,0.117142582},{0.016535984,0.928022794,0.105564851},{0.072066422,0.067456722,0.777292568}};
    double run_b[OBSERV_NUM][STATE_NUM] = {
        {0.376659084,2.89177E-36,7.32707E-22},
        {0.623340916,1.77971E-11,0.000132496},
        {4.75775E-11,2.94127E-19,0.579184439},
        {1.64936E-22,5.24812E-05,0.420680016},
        {3.64058E-23,0.329060907,3.04952E-06},
        {2.42714E-31,0.497695775,2.88081E-15},
        {5.99981E-26,0.173190837,6.12921E-17},
        {0,0,0}};
    double walk_pi[STATE_NUM] = {0.008220815,1.15E-36,0.991779185};
    double walk_a[STATE_NUM][STATE_NUM] = {{0.602985679,0.094537721,0.044079307},{0.03443364,0.789671219,0.025672161},{0.362580682,0.11579106,0.930248533}};
    double walk_b[OBSERV_NUM][STATE_NUM] = {{0,0,0},
        {5.87575E-13,0.014759726,8.07681E-24},
        {0.2586356,6.47283E-12,2.29926E-08},
        {0.7413644,2.85666E-05,0.999999977},
        {8.79061E-10,0.944622461,1.31672E-11},
        {1.47098E-21,0.033209383,2.9217E-26},
        {2.06214E-35,0.007379863,3.49125E-42},
        {0,0,0}};
    double still_pi[STATE_NUM] = {1,0,0};
    double still_a[STATE_NUM][STATE_NUM] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    double still_b[OBSERV_NUM][STATE_NUM] = {{0,0,0},
        {0,0,0},
        {0,0,0},
        {1,1,1},
        {0,0,0},
        {0,0,0},
        {0,0,0},
        {0,0,0}};
    char *name_0 = "predict";
    char *name_1 = "fall";
    char *name_2 = "run";
    char *name_3 = "walk";
    char *name_4 = "still";
    HMM_init(&movement_arr[0], name_0, STATE_NUM, OBSERV_NUM, pre_pi, pre_a, pre_b);
    HMM_init(&movement_arr[1], name_1, STATE_NUM, OBSERV_NUM, fall_pi, fall_a, fall_b);
    HMM_init(&movement_arr[2], name_2, STATE_NUM, OBSERV_NUM, run_pi, run_a, run_b);
    HMM_init(&movement_arr[3], name_3, STATE_NUM, OBSERV_NUM, walk_pi, walk_a, walk_b);
    HMM_init(&movement_arr[4], name_4, STATE_NUM, OBSERV_NUM, still_pi, still_a, still_b);
    dumpHMM(stderr, &movement_arr[0]);
    dumpHMM(stderr, &movement_arr[1]);
    dumpHMM(stderr, &movement_arr[2]);
    dumpHMM(stderr, &movement_arr[3]);
    dumpHMM(stderr, &movement_arr[4]);
    //end of declaration of Viterbi variables
    printf("before while loop\n");
    //int k = 0;
    int score[MOVEMENT_NUM] = {0};
    ifstream f;
    ofstream g;
    double abs_acc;
    f.open(argv[1], ios::in);
    g.open(argv[2], ios::out);
    for (int i = 0; i < win_size - 1;++i){
        g << -1 << endl;
    }
    while(f>>abs_acc){
        
        printf("after while loop with iter = %d\n", iter);
        //double abs_acc = double(rand() % 32768) / 32768 * 5 +3;
        //double abs_acc = (k++ % 2 == 1) ? 999 : 999;
        
        if(abs_acc>11){
            buffer[iter] = 7;
        }
        else if(abs_acc>8.7){
            buffer[iter] = 6;
        }
        else if(abs_acc>6.6){
            buffer[iter] = 5;
        }
        else if(abs_acc>4.7){
            buffer[iter] = 4;
        }
        else if(abs_acc>3.4){
            buffer[iter] = 3;
        }
        else if(abs_acc>2.2){
            buffer[iter] = 2;
        }
        else if(abs_acc>1.1){
            buffer[iter] = 1;
        }
        else{
            buffer[iter] = 0;
        }
        if (iter>=win_size-1){
            //run viterbi and send data
            printf("before running viterbi with iter = %d\n",iter);
            double prob[MOVEMENT_NUM] = {0};
            viterbi(movement_arr, win_size, buffer + (iter - win_size + 1), MOVEMENT_NUM, prob);
            printf("finished viterbi\n");
            if(iter==buff_size-1){
                memcpy(buffer, buffer + (iter - win_size + 1), win_size*sizeof(int));
                iter = win_size - 1;
            }
            //end of viterbi
            //select maximum movement
            double max = 0;
            int argmax = 0;
            for (int i = 0; i < MOVEMENT_NUM; ++i){
                if(prob[i]>=max){
                    max = prob[i];
                    argmax = i;
                }
            }
            //end of movement selection
            score[argmax]++;
            printf("%d\n",argmax);
            g << argmax << endl;
            /*
            response = socket.send(&argmax,1);
            if (0 >= response){
                printf("Error seding: %d\n", response);
            }
            */
            
        }

        iter++;
        //true_iter++;
        
        //usleep(1000);
    }
    f.close();
    g.close();
    for (int i = 0; i < MOVEMENT_NUM;++i){
        cout << movement_arr[i].model_name << ": " << score[i] << endl;
    }
    //socket.close();
}


int main(int argc, char** argv)
{
    //cout << 4.94066e-324 * 0.33 << endl;
    /*
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
    printf("RSSI FUCK: %d\n\n", wifi.get_rssi());


    BSP_ACCELERO_Init();    
    */
    acc_server(argv);



}
void HMM_init(HMM *model, char *name, int state_n, int observ_n, double pi[], double a[][STATE_NUM], double b[][STATE_NUM]){
    model->model_name = (char *)malloc( sizeof(char) * (strlen( name)+1));
    strcpy(model->model_name, name);
    model->state_num = state_n;
    model->observ_num = observ_n;
    //printf("sizeof(a) is %d\n", sizeof(a));
    memcpy(model->initial, pi, sizeof(pi) * STATE_NUM);
    memcpy(model->transition, a, sizeof(a) * STATE_NUM * STATE_NUM);
    memcpy(model->observation, b, sizeof(b) * OBSERV_NUM * STATE_NUM);
}

static void dumpHMM( FILE *fp, HMM *hmm )
{
   int i, j;

   //fprintf( fp, "model name: %s\n", hmm->model_name );
   fprintf( fp, "initial: %d\n", hmm->state_num );
   for( i = 0 ; i < hmm->state_num - 1; i++ )
      fprintf( fp, "%.5lf ", hmm->initial[i]);
   fprintf(fp, "%.5lf\n", hmm->initial[ hmm->state_num - 1 ] );

   fprintf( fp, "\ntransition: %d\n", hmm->state_num );
   for( i = 0 ; i < hmm->state_num ; i++ ){
      for( j = 0 ; j < hmm->state_num - 1 ; j++ )
         fprintf( fp, "%.5lf ", hmm->transition[i][j] );
      fprintf(fp,"%.5lf\n", hmm->transition[i][hmm->state_num - 1]);
   }

   fprintf( fp, "\nobservation: %d\n", hmm->observ_num );
   for( i = 0 ; i < hmm->observ_num ; i++ ){
      for( j = 0 ; j < hmm->state_num - 1 ; j++ )
         fprintf( fp, "%.5lf ", hmm->observation[i][j] );
      fprintf(fp,"%.5lf\n", hmm->observation[i][hmm->state_num - 1]);
   }
}

void viterbi(HMM* hmm, int seqlen, int* seq, int model_num, double* p) {
	printf("calling viterbi\n");
	for(int i=0;i<model_num;i++)
	{
        //printf("I\n");
		double viterbi[hmm[i].state_num][seqlen];
        //printf("dafaq\n");
		for(int j=0;j<hmm[i].state_num;j++)
		{
            //printf("am\n");
			viterbi[j][0]=hmm[i].initial[j]*hmm[i].observation[seq[0]][j];
            //printf("am\n");
		}
		for(int k=1;k<seqlen;k++)
		{
            //printf("the\n");
			for(int l=0;l<hmm[i].state_num;l++)
			{
                //printf("bone\n");
				double max=0;
				for(int m=0;m<hmm[i].state_num;m++)
				{
                    //printf("of\n");
					if(viterbi[m][k-1]*hmm[i].transition[m][l]>max)
					{
                        //printf("my\n");
						max=viterbi[m][k-1]*hmm[i].transition[m][l];
					}
				}
				viterbi[l][k]=max*hmm[i].observation[seq[k]][l];
			}
		}
		for(int a=0;a<hmm[i].state_num;a++)
		{
            //printf("sword.\n");
			if(viterbi[a][seqlen-1]>p[i])
			{
				p[i]=viterbi[a][seqlen-1];
			}
		}
	}
	//return p;
    /*
    double sum = p[0] + p[1] + p[2] + p[3];
    printf("%lf, %lf, %lf, %lf.\n", p[0]/sum, p[1]/sum, p[2]/sum, p[3]/sum);
    */
    cout << p[0] << endl
         << p[1] << endl
         << p[2] << endl 
         << p[3] << endl;
}