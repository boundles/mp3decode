#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct {
    short channels;
    short bits;
    int rate;
}head_pama;

void wav_write_header(FILE* fp,head_pama pt,const int datasize);
int decidemp3format(const char* inputfilename);
int mp3decodefunc(const char *outfilename, const char *inputfilename);
int mp2decode(const char *outfilename, const char *inputfilename);