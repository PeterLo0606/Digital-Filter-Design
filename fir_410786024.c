#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>

#define FS 48000.0f
#define FL 1000.0f
#define FH 4000.0f
#define PI 3.141592653589793f


typedef struct _wav {
 int fs;
 char header[44];
 size_t length;
 short *LChannel;
 short *RChannel;
} wav;

int wav_read_fn(char *fn, wav *p_wav)
{
 //char header[44];
 short temp = 0;
 size_t i = 0;

 FILE *fp = fopen(fn, "rb");
 if(fp==NULL) {
  fprintf(stderr, "cannot read %s\n", fn);
  return 0;
 }
 fread(p_wav->header, sizeof(char), 44, fp);
 while( !feof(fp) ) {
  fread(&temp, sizeof(short), 1, fp);
  i++;
 }
 p_wav->length = i / 2;
 p_wav->LChannel = (short *) calloc(p_wav->length, sizeof(short));
 if( p_wav->LChannel==NULL ) {
  fprintf(stderr, "cannot allocate memory for LChannel in wav_read_fn\n");
  fclose(fp);
  return 0;
 }
 p_wav->RChannel = (short *) calloc(p_wav->length, sizeof(short));
 if( p_wav->RChannel==NULL ) {
  fprintf(stderr, "cannot allocate memory for RChannel in wav_read_fn\n");
  fclose(fp);
  return 0;
 }
 fseek(fp, 44, SEEK_SET);
 for(i=0;i<p_wav->length;i++) {
  fread(p_wav->LChannel+i, sizeof(short), 1, fp);
  fread(p_wav->RChannel+i, sizeof(short), 1, fp);
 }
 fclose(fp);
 return 1;
}

int wav_save_fn(char *fn, wav *p_wav)
{
 FILE *fp = fopen(fn, "wb");
 size_t i;
 if(fp==NULL) {
  fprintf(stderr, "cannot save %s\n", fn);
  return 0;
 }
 fwrite(p_wav->header, sizeof(char), 44, fp);
 for(i=0;i<p_wav->length;i++) {
  fwrite(p_wav->LChannel+i, sizeof(short), 1, fp);
  fwrite(p_wav->RChannel+i, sizeof(short), 1, fp);
 }
 fclose(fp);
 return 1;
}

int wav_init(size_t length, wav *p_wav)
{
 p_wav->length = length;
 p_wav->LChannel = (short *) calloc(p_wav->length, sizeof(short));
 if( p_wav->LChannel==NULL ) {
  fprintf(stderr, "cannot allocate memory for LChannel in wav_read_fn\n");
  return 0;
 }
 p_wav->RChannel = (short *) calloc(p_wav->length, sizeof(short));
 if( p_wav->RChannel==NULL ) {
  fprintf(stderr, "cannot allocate memory for RChannel in wav_read_fn\n");
  return 0;
 }
 return 1;
}

void wav_free(wav *p_wav)
{
 free(p_wav->LChannel);
 free(p_wav->RChannel);
}

/* hamming: for n=0,1,2,...N, length of N+1 */
float hamming(int N, int n)
{
 return 0.54 - 0.46 * cosf(2*PI*((float)(n))/((float)N));
}

/* low-pass filter coef: n=0,1,2...,2M */
float low_pass(int m, int n)
{
 float wc = 2*PI*FL/FS;
 if(n==m) {// L'Hopital's Rule
  return wc/PI;
 }
 else {
  return sinf(wc*((float)(n-m)))/PI/((float)(n-m)) * hamming(2*m+1, n);
 }
}

float band_pass(int m, int n)
{
 float wh = 2*PI*FH/FS;
    float wl = 2*PI*FL/FS;
 if(n==m) {// L'Hopital's Rule
  return 2.0*(wh/PI - wl/PI);
 }
 else {
  return 2.0*(sinf(wh*((float)(n-m)))-sinf(wl*((float)(n-m))))/PI/((float)(n-m)) * hamming(2*m+1, n);
 }
}

float band_stop(int m, int n)
{
 float wh = 2*PI*FH/FS;
    float wl = 2*PI*FL/FS;
 if(n==m) {// L'Hopital's Rule
  return (1.0)-(wh/PI - wl/PI);
 }
 else {
  return (-1.0)*(sinf(wh*((float)(n-m)))-sinf(wl*((float)(n-m))))/PI/((float)(n-m)) * hamming(2*m+1, n);
 }
}


int main(int argc, char **argv)
{
    wav wavin;
    wav wavout;

    int M=atoi(argv[1]); //M=8or32or1024
    char* hLn = argv[2];  //左聲道經過bandpass filter
    char* hRn = argv[3];  //右聲道經過bandstop filter
    char* YLn = argv[4];  //左聲道經過filter後30~30.025秒的值做DFT
    char* YRn = argv[5];  //右聲道經過filter後30~30.025秒的值做DFT
    char* fn_in = argv[6];  //輸入的音檔
    char* fn_out = argv[7];  //輸出的音檔

 float h_L[2500] = {0};
 float h_R[2500] = {0};
 int n = 0;
 float y = 0;
 int k;

 //建立並開啟這4個檔案
 FILE* hL=fopen(hLn,"w+");
 FILE* hR=fopen(hRn,"w+");
 FILE* YL=fopen(YLn,"w+");
 FILE* YR=fopen(YRn,"w+");

 // read wav
 if( wav_read_fn(fn_in, &wavin) == 0 ) {
  fprintf(stderr, "cannot read wav file %s\n", fn_in);
  exit(1);
 }

 // construct low-pass filter
 for(n=0;n<(2*M+1);n++) {
  h_L[n] = band_pass(M, n);
  h_R[n] = band_stop(M, n);
  fprintf(hL, "%.15e\n", h_L[n]);
  fprintf(hR, "%.15e\n", h_R[n]);
 }

 // filtering (convolution)
 if( wav_init(wavin.length, &wavout)==0 ) {
  exit(1);
 }

 for(n=0;n<wavin.length;n++) {
  y = 0;
  for(k=0;k<(2*M+1);k++) {
   if( (n-k)>=0 )
    y = y + h_L[k] * ((float)(wavin.LChannel[n-k]));
  }
  wavout.LChannel[n] = (short)(roundf(y));

  y = 0;
  for(k=0;k<(2*M+1);k++) {
   if( (n-k)>=0 )
    y = y + h_R[k] * ((float)(wavin.RChannel[n-k]));
  }
  wavout.RChannel[n] = (short)(roundf(y));
 }

 fclose(hL);
 fclose(hR);

 //YL&YR (30~30.025s DFT)
 float real=0,image=0,real1=0,image1=0,Xn=0,Xn1=0;
 float *X_L = calloc(1200,sizeof(float));
 float *X_R = calloc(1200,sizeof(float));

 for(k=0;k<1200;k++){
    X_L[k] = wavout.LChannel[30*48000+k]*hamming(1200,k);
    X_R[k] = wavout.RChannel[30*48000+k]*hamming(1200,k);
 }
 for(k=0;k<600;k++){
    real=0,image=0,real1=0,image1=0;
    for(n=0;n<1200;n++){
        real  += cos((2*M_PI/1200)*n*k)*X_L[n];
        image -= sin((2*M_PI/1200)*n*k)*X_L[n];
        real1 += cos((2*M_PI/1200)*n*k)*X_R[n];
        image1-= sin((2*M_PI/1200)*n*k)*X_R[n];
    }
    Xn = 20*log10(sqrt(real*real)+(image*image));
    Xn1= 20*log10(sqrt(real1*real1)+(image1*image1));

    fprintf(YL,"%.15e\n",Xn);
    fprintf(YR,"%.15e\n",Xn1);
 }
 memcpy(wavout.header, wavin.header, 44);
 fclose(YL);
 fclose(YR);

 // save wav
 if( wav_save_fn(fn_out, &wavout)==0) {
  fprintf(stderr, "cannot save %s\n", fn_out);
  exit(1);

 }
 wav_free(&wavin);
 wav_free(&wavout);
}
