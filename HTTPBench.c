#include "HTTPBench.h"

void exp1_session_error()
{
  pthread_mutex_lock(&g_mutex);
  g_error_count++;
  pthread_mutex_unlock(&g_mutex);
}

void* exp1_eval_thread(void* param)
{
  int sock;
  sdata** data;
  int fileid;
  char command[1024];
  char buf[2048];
  int total;
  int ret;
  int i;
  char str[256];

  int** pp;
  int** qq;

  size_t pid;

  data = (sdata**) param;
  fileid = data[i]->pfileid;
  
  pp = (int **)malloc(sizeof(int *) * data[0]->ps_num);
  for(i = 0;i < data[0]->ps_num;i++){
    pp[i] = (int *)malloc(sizeof(int) * 2);
  }

  qq = (int **)malloc(sizeof(int *) * data[0]->ps_num);
  for(i = 0;i < data[0]->ps_num;i++){
    qq[i] = (int *)malloc(sizeof(int) * 2);
  }

  for(i=0; i<data[0]->ps_num; i++){
    
    if(pipe(pp[i]) == -1)
    {
      /* printf("pipe error!");*/
      perror("pipe");
      exit(0);
    }
    if(pipe(qq[i]) == -1)
    {
      /*printf("pipe error!");*/
      perror("pipe");
      exit(0);
    }

    pid = fork();
    if(pid == 0){
  
      close(pp[i][0]);
      close(qq[i][1]);

      data[i]->start = gettimeofday_sec();

      sock = exp1_tcp_connect(g_hostname, data[i]->port);

      data[i]->tcp_end = gettimeofday_sec();
      if(sock < 0){
	      printf("session error!(sock open failed)\n");

        write(pp[i][1],"error",sizeof("error"));
        read(qq[i][0],str,sizeof(str));

	      exp1_session_error();
	      exit(0);
      }

      sprintf(command, "GET /%03d.jpg HTTP/1.0\r\n\r\n", fileid);
      ret = send(sock, command, strlen(command), 0);
      data[i]->http_end = gettimeofday_sec();
      if(ret < 0){
	      printf("session error!(get request send failed)\n");
        
        write(pp[i][1],"error",sizeof("error"));
        read(qq[i][0],str,sizeof(str));
        
	      exp1_session_error();
	      exit(0);
      }

      total = 0;
      ret = recv(sock, buf, 2048, 0);
      while(ret > 0){
        total += ret;
        ret = recv(sock, buf, 2048, 0);
      }
      data[i]->end = gettimeofday_sec();
      close(sock);
      /*printf("%f,%f,%f\n",data[i]->tcp_end,data[i]->http_end,data[i]->end);*/


      sprintf(str,"%f",data[i]->start);
      write(pp[i][1],str,sizeof(str));
      read(qq[i][0],str,sizeof(str));

      sprintf(str,"%f",data[i]->tcp_end);
      write(pp[i][1],str,sizeof(str));
      read(qq[i][0],str,sizeof(str));

      sprintf(str,"%f",data[i]->http_end);
      write(pp[i][1],str,sizeof(str));
      read(qq[i][0],str,sizeof(str));

      sprintf(str,"%f",data[i]->end);
      write(pp[i][1],str,sizeof(str));
      read(qq[i][0],str,sizeof(str));

      close(pp[i][0]);
      close(qq[i][1]);
      
      exit(0);
    }
    else if(pid == -1){
      exit(0);
    }
    else {
      close(pp[i][1]);
      close(qq[i][0]);
    }
  }

  for(i=0;i<data[0]->ps_num;i++){
    read(pp[i][0],str,sizeof(str));
    if(strcmp(str,"error") == 0)
    {
      data[i]->is_error = 1;
      wait(0);
      continue;
    }
    data[i]->start = atof(str);
    write(qq[i][1],str,sizeof(str));

    read(pp[i][0],str,sizeof(str));
    data[i]->tcp_end = atof(str);
    write(qq[i][1],str,sizeof(str));

    read(pp[i][0],str,sizeof(str));
    data[i]->http_end = atof(str);
    write(qq[i][1],str,sizeof(str));

    read(pp[i][0],str,sizeof(str));
    data[i]->end = atof(str);
    write(qq[i][1],str,sizeof(str));

    wait(0);
    /*data[i]->end = exp1_gettimeofday_sec();*/
  }
  
  for(i = 0;i < data[0]->ps_num;i++){
    free(qq[i]);
    free(pp[i]);
  }
  free(qq);
  free(pp);
}

int main(int argc, char** argv)
{
  int num,ps_num, tstart;
  int i,j,opt;
  int errorcount = 0;
  int aopt = 0;
  int bopt = 0;
  double tcp_average = 0;
  double http_average = 0;
  double start_time;
  double total_max = 0;

  pthread_t *th;
  sdata ***data;
  
  if(argc != 5){
    printf("usage: %s [ip address] [# of thread] [# of process] [port]\n", argv[0]);
    exit(-1);
  }
  strcpy(g_hostname, argv[1]);

  g_error_count = 0;
  pthread_mutex_init(&g_mutex, NULL);
 
  num = atoi(argv[2]);  
  ps_num = atoi(argv[3]);
  th = malloc(sizeof(pthread_t) * num);
  
  data = (sdata ***)malloc(sizeof(sdata **) * num);
  for(i = 0;i < num;i++){
    data[i] = (sdata **)malloc(sizeof(sdata *) * ps_num);
    for(j = 0; j < ps_num; j++){
      data[i][j] = (sdata *)malloc(sizeof(sdata));
    }
  }
  srand(0);
  for(i = 0; i < num; i++){
    for(j = 0; j < ps_num; j++){
      int fileid = rand() % 100;
      data[i][j]->pfileid = fileid;
      data[i][j]->port = atoi(argv[4]);
      /*data[i][j]->start = gettimeofday_sec();*/
      data[i][j]->ps_num = ps_num;
      data[i][j]->is_error = 0;
    }
    start_time = gettimeofday_sec();
    pthread_create(&th[i], NULL, exp1_eval_thread, data[i]);
  }

  for(i = 0; i < num; i++) {
    pthread_join(th[i], NULL);
  }

  for(i = 0;i < num;i++) {
    for(j = 0; j < ps_num; j++) {
      if(data[i][j]->is_error == 1) {
        errorcount++;
        continue;
      }
      /*calc sum(tcp&http)*/
      tcp_average += (data[i][j]->tcp_end - data[i][j]->start);
      http_average += (data[i][j]->http_end - data[i][j]->start);
      if(data[i][j]->end >= total_max){
	total_max = data[i][j]->end;
      }
    }
  }

  for(i = 0;i < num;i++){
    for(j = 0; j < ps_num; j++){
      free(data[i][j]);
    }
  }
  for(i = 0;i < num;i++){
    free(data[i]);
  }
  free(data);
  free(th);

  /*calc average*/
  /*printf("%d\n",(num*ps_num)-errorcount);*/
  tcp_average /= (num*ps_num)-errorcount;
  http_average /= (num*ps_num)-errorcount;

  printf("tcp-average:%10.10f\nhttp-average:%10.10f\ntotal-time:%10.10f\n",tcp_average,http_average,total_max-start_time);

  printf("session error ratio is %1.3f\n",(double)g_error_count / (double) num*ps_num);
}
