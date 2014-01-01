#include <stdio.h>
//#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "prototype.h"


#define CAPTURE_WAIT_TIME 50
#define SEARCH_WAIT_TIME 20
#define CAPTURE_IMG_WIDTH 1024
#define CAPTURE_IMG_HEIGHT 576

void search_loop(int *loop_count){
    int wait_status;
    pid_t process_id;
    
    time_t timer;
    struct tm *t;
    
    //printf("start...");//デバッグ用ログ
    
    while(1){
        //子プロセス
    	if((process_id = fork())==0){
			search();
            exit(0);
     	}
     	else{
            //親プロセス
            waitpid(process_id,&wait_status,WCONTINUED | WUNTRACED);
            if(WIFEXITED(wait_status))//子プロセスが正常終了したら
                break;
            //else printf("miss...");//デバッグ用ログ
    	}
    }
    
    //make_html();
    
    /*
     //デバッグ用ログ
     printf("loop %d end  ",++*loop_count);
     time(&timer);
     t = localtime(&timer);
     printf("%d/%d/%d//%d:%d\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min);
     */
}

int main(int argc, const char * argv[])
{
    CvCapture *capture = 0;
    IplImage *capture_frame = 0;
    int image_option[] = {CV_IMWRITE_JPEG_QUALITY,100};
    CvFont font;
    //double capture_img_width = 1024, capture_img_height = 576;
    
    pid_t process_id;
    
    time_t timer;
    struct tm *local_time;
    char date_string[62];
    
    int loop_count = 0;

    //フォントの初期化
    cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,1.0f,1.0f,0,2,CV_AA); 
    //コマンドライン引数の番号のカメラの構造体初期化
    if (argc == 1 || (argc == 2 && strlen (argv[1]) == 1 && isdigit (argv[1][0])))
        capture = cvCreateCameraCapture (argc == 2 ? argv[1][0] - '0' : 0);
    //キャプチャサイズの設定．
    cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_WIDTH, CAPTURE_IMG_WIDTH);
    cvSetCaptureProperty (capture, CV_CAP_PROP_FRAME_HEIGHT, CAPTURE_IMG_HEIGHT);
    
    if((process_id = fork())==0){
        //ネットワーク解析プロセス
        //printf("Start search process.\n");//デバッグ用ログ
        while(1){
            search_loop(&loop_count);
            sleep(SEARCH_WAIT_TIME);
        }
    }
    else{
        //printf("Start capture process.\n");//デバッグ用ログ
        while (1) {
            timer = time(NULL);
            local_time = localtime(&timer);
            sprintf(date_string,"%d/%d/%d  %d:%d:%d",local_time->tm_year+1900,local_time->tm_mon+1,local_time->tm_mday,local_time->tm_hour,local_time->tm_min,local_time->tm_sec);
	
            capture_frame = cvQueryFrame (capture);
	
            cvPutText(capture_frame,date_string,cvPoint(10,30),&font,CV_RGB(0,0,0));
            cvSaveImage("image.jpg", capture_frame, image_option);

            sleep(CAPTURE_WAIT_TIME);
        }
        cvReleaseCapture (&capture);
        cvDestroyWindow ("Capture");
    }
    return 0;
}
