#define _BSD_SOURCE 1
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/ioctl.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<net/if.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<pcap.h>
#include<sys/time.h>
#include<time.h>

//イーサネットアドレス（MACアドレス）は6バイト
#define ETHER_ADDR_LEN	6
#define SIZE_ETHERNET 14

//イーサネットヘッダ
struct sniff_ethernet {
	u_char ether_dhost[ETHER_ADDR_LEN];//送信先ホストアドレス
	u_char ether_shost[ETHER_ADDR_LEN];//送信元ホストアドレス
	u_short ether_type;
};

//IPヘッダ
struct sniff_ip {
	u_char ip_vhl;		//バージョン（上位4ビット）、ヘッダ長（下位4ビット）
	u_char ip_tos;		//サービスタイプ
	u_short ip_len;		//パケット長
	u_short ip_id;		//識別子
	u_short ip_off;		//フラグメントオフセット
    #define IP_RF 0x8000		//未使用フラグ（必ず0が立つ）
    #define IP_DF 0x4000		//分割禁止フラグ
    #define IP_MF 0x2000		//more fragments フラグ
    #define IP_OFFMASK 0x1fff	//フラグメントビットマスク
	u_char ip_ttl;		//生存時間（TTL）
	u_char ip_p;		//プロトコル
	u_short ip_sum;		//チェックサム
	struct in_addr ip_src,ip_dst; //送信元、送信先IPアドレス
};


//監視でバイスのセット
void set_device(char **device,char *errer_buf){
	*device = pcap_lookupdev(errer_buf);
	if(*device == NULL){
		fprintf(stderr,"none device:%s\n",errer_buf);
		exit(1);
	}
}

//監視デバイスのオープン
void open_device(char *device,char *errer_buf,pcap_t **handle){
	*handle = pcap_open_live(device,BUFSIZ,1,6000,errer_buf);
	if(*handle == NULL){
		fprintf(stderr,"デバイス[%s]が開けませんでした:%s\n",device,errer_buf);
		exit(1);
	}
}

//net情報の取得
void set_network(char *device,bpf_u_int32 *network,bpf_u_int32 *mask,char *errer_buf){
	if(pcap_lookupnet(device,network,mask,errer_buf) == -1){
		fprintf(stderr,"デバイス[%s]を開けませんでした\n",device);
		*network=0;
		*mask=0;
	}
}

//自分のIP取得
void get_my_ip(char **device,char *my_adr,char *filter_adr,char *broadcast_adr){
    int fd;
    struct ifreq ifr;
    
    fd = socket(AF_INET,SOCK_DGRAM,0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name,*device,IFNAMSIZ-1);
    ioctl(fd,SIOCGIFADDR,&ifr);
    close(fd);
    sprintf(my_adr,"%s",inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr));
    sprintf(my_adr,"%s",strtok(my_adr,"s"));
    
    strcpy(filter_adr,my_adr);
    filter_adr[(int)(strrchr(filter_adr,(int)'.')-filter_adr)]='\0';
    sprintf(broadcast_adr,"%s.255",filter_adr);
}

//対応表にないアドレスをメモ
void note_adr(char *adr){
	FILE *note;
	time_t timer;
	struct tm *local_time;
	char note_mac[256],out_date[64];
	int flag=0;
    
	note = fopen("note.txt","r");
    //既にメモされているか確認
    
	while(fscanf(note,"%s",note_mac)!=EOF)
		if(strcmp(adr,note_mac)==0){
			flag=1;
			break;
		}
    
	if(flag==0){
		fclose(note);
		note = fopen("note.txt","a");
		time(&timer);
		local_time = localtime(&timer);
		sprintf(out_date,"%d/%d/%d//%d:%d",local_time->tm_year+1900,local_time->tm_mon+1,local_time->tm_mday,local_time->tm_hour,local_time->tm_min);
		fprintf(note,"%s %s\n",adr,out_date);
		fclose(note);
	}
    
}

//ユーザーの照合
void user_search(){
    FILE *get_mac_adr,*mac_adr_list,*user_list;
    int flag=2;
    char get_mac_bff[24],mac_list_bff[24],bff[24];
    
    get_mac_adr = fopen("GetMacAdr.txt","r");//取得macアドレス情報
    mac_adr_list = fopen("MacAdrList.txt","r");//macアドレスの対応表
    user_list = fopen("UserList.txt","w");//ネットワークにいるユーザーりすと
    
    fprintf(user_list,"DateServer\n");//サーバーをユーザーリストに追加
    
    while(fscanf(get_mac_adr,"%s",get_mac_bff) != EOF){
        sprintf(bff,"%s",get_mac_bff);//なんか消えるのでバッファとって
        while (fscanf(mac_adr_list,"%s",mac_list_bff) != EOF) {
            sprintf(get_mac_bff,"%s",bff);
            
            if(strcmp(get_mac_bff,mac_list_bff)==0){//ネットワーク上にいたらユーザーリストに書く
                fscanf(mac_adr_list,"%s",mac_list_bff);
                fprintf(user_list,"%s\n",mac_list_bff);
                flag+=1;
                break;
            }
        }
        
        if(flag%2 == 0){//macアドレスの対応表にないアドレスだったら
            fprintf(user_list,"unkownPC%d\n",flag/2);
            sprintf(get_mac_bff,"%s",bff);
            note_adr(get_mac_bff);
            flag+=2;
        }else
            flag--;
        
        rewind(mac_adr_list);
    }
    
    fclose(get_mac_adr);
    fclose(mac_adr_list);
    fclose(user_list);
}


void search(){
	pcap_t *handle;			//パケットキャプチャディスクリプタ
	char *device;			//監視でバイス名
	char errer_buf[PCAP_ERRBUF_SIZE];	//エラー理由用配列
	struct bpf_program fp;		//コンパイル済みフィルタ用構造体
	bpf_u_int32 mask;		//監視デバイスのネットマスク
	bpf_u_int32 net;		//監視デバイスのIP
    struct pcap_pkthdr header;
    u_char *packet;
    
    struct sniff_ethernet *ethernet;
    struct sniff_ip *ip;
    u_int size_ip;
    int packet_count,mac_token_count;
    char my_adr[24],mac_list_bff[512],adr_src[24],
            mac_string[24]="",filter_adr[24],broadcast_adr[24];
    FILE *get_mac_adr;
    
	set_device(&device,errer_buf);
	set_network(device,&net,&mask,errer_buf);
	open_device(device,errer_buf,&handle);
    
    get_my_ip(&device,my_adr,filter_adr,broadcast_adr);

	get_mac_adr = fopen("GetMacAdr.txt","w");
    
    for(packet_count=0;packet_count<5;){
        packet = (u_char *)pcap_next(handle,&header);//パケットを取る
        ethernet = (struct sniff_ethernet*)packet;
        ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);

        sprintf(adr_src,"%s",inet_ntoa(ip->ip_src));//送信元アドレスの取得
    
        if(strstr(adr_src,my_adr)==NULL &&
           strstr(adr_src,filter_adr) &&
           strstr(adr_src,broadcast_adr)==NULL){
            
            packet_count++;//ループカウント

            for(mac_token_count=0;mac_token_count<6;mac_token_count++){
                if(mac_token_count==5){
                    sprintf(mac_string,"%s%x",mac_string,ethernet->ether_shost[mac_token_count]);
                }
                else{
                    sprintf(mac_string,"%s%x:",mac_string,ethernet->ether_shost[mac_token_count]);
                }
            }

            if(strstr(mac_list_bff,mac_string)==NULL){
                strcat(mac_list_bff,mac_string);
                fprintf(get_mac_adr,"%s\n",mac_string);
            }
            
            sprintf(mac_string,"");
        }
    }

    pcap_close(handle);
    fclose(get_mac_adr);
    user_search();
}

