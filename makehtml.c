#include<stdio.h>
#include<stdlib.h>
#include<string.h>
void make_html(){
	char html_head[] = "<!DOCTYPE html>\n<html lang='jp'>\n<head>\n<meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>\n<!--<meta charset='UTF-8'>-->\n</head>\n<body>\n";
	char html_end[]="</body>\n</html>\n";
	FILE *user_html,*user_list,*user_twitter_list;
	char user_name[128],twitter_name[128],bff[128];
    int flag = 1;
	
    user_html = fopen("user.html","w");
	user_list = fopen("UserList.txt","r");
	user_twitter_list = fopen("UserTwitterList.txt","r");
    
    fprintf(user_html,"%s",html_head);
    
	while(fscanf(user_list,"%s",user_name)!=EOF){
        sprintf(bff,"%s",user_name);//なんか消えるんでバッファに
		while(fscanf(user_twitter_list,"%s",twitter_name)!=EOF){
            sprintf(user_name,"%s",bff);
            if(strcmp(user_name,twitter_name)==0){
                fscanf(user_twitter_list,"%s",twitter_name);
                if(strcmp("none",twitter_name)!=0)
                    fprintf(user_html,"<div id='line' style=' color: #fdffff;margin-top;2px;margin-bottom: 2px; font-size:20px;'>\n%s<a href='https://twitter.com/intent/tweet?screen_name=%s' class='twitter-mention-button' data-lang='ja' data-size='' data-related='%s' data-hashtags='DensanLive '>Tweet to @%s</a>\n<script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0],p=/^http:/.test(d.location)?'http':'https';if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src=p+'://platform.twitter.com/widgets.js';fjs.parentNode.insertBefore(js,fjs);}}(document, 'script', 'twitter-wjs');</script>\n</div>\n",user_name,twitter_name,twitter_name,twitter_name);
                else fprintf(user_html,"<div id='line' style='background:  #009b84; color: #fdffff;margin-top;2px;margin-bottom: 2px;'>\n%s\n</div>\n",user_name);
                flag++;
                break;
            }
        }
        if(flag%2 == 1){
            fprintf(user_html,"<div id='line' style='font-size:20px; color: #fdffff;margin-top;2px;margin-bottom: 2px;'>\n%s\n</div>\n",user_name);
        }else flag--;
        rewind(user_twitter_list);
        }

	fprintf(user_html,"%s",html_end);
	fclose(user_html);
    fclose(user_list);
    fclose(user_twitter_list);
}
