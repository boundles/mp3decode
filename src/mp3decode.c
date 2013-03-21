#include "mp3decode.h"

void wav_write_header(FILE* fp,head_pama pt,const int datasize)
{
    int long_temp;
    short short_temp;
    short BlockAlign;
    char *data = NULL;

    data = "RIFF";
    fwrite(data, sizeof(char), 4, fp);

    long_temp=datasize*2+36;
	fwrite(&long_temp, sizeof(long_temp), 1, fp);

    data = "WAVE";
    fwrite(data, sizeof(char), 4, fp);

    data = "fmt ";
    fwrite(data, sizeof(char), 4, fp);

    long_temp = 16;
    fwrite(&long_temp, sizeof(long_temp), 1, fp);

    short_temp = 0x0001;
    fwrite(&short_temp, sizeof(short_temp), 1, fp);

    short_temp = pt.channels;
    fwrite(&short_temp, sizeof(short_temp), 1, fp);

    long_temp = pt.rate;
    fwrite(&long_temp, sizeof(long_temp), 1, fp);
    
    long_temp = ((pt.bits)/8) * (pt.channels) * (pt.rate);
    fwrite(&long_temp, sizeof(long_temp), 1, fp);

    BlockAlign = ((pt.bits)/8) * (pt.channels);
    fwrite(&BlockAlign, sizeof(BlockAlign), 1, fp);

    short_temp = pt.bits;
    fwrite(&short_temp, sizeof(short_temp), 1, fp);

    data = "data";
    fwrite(data, sizeof(char), 4, fp);

    long_temp = datasize*(pt.channels)*(pt.bits/8);
    fwrite(&long_temp, sizeof(long_temp), 1, fp);
}

int decidemp3format(const char* inputfilename)
{
	FILE* fp;
	char id[3];
	
	fp = fopen(inputfilename,"rb+");
	if(fp == NULL)
	{
		return 1;
	}
	fread(id,1,3,fp);

	if((id[0] =='I'||id[0] =='i')
		&&(id[1] =='D'||id[1] =='d')
		&&(id[2] == '3'))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int mp3decodefunc(const char *outfilename, const char *inputfilename)
{
	
	AVFormatContext *pFormatCtx;
    AVCodec *aCodec;
    AVCodecContext * aCodecCtx= NULL;

	short sample[2304],outsample[1152];
	short silsample[22050] = {0};
	int len, m;
    FILE *outfile;
	int out_size=AVCODEC_MAX_AUDIO_FRAME_SIZE * 100;
	uint8_t * outbuf = (uint8_t *)malloc(out_size);
	int frame = 0;
	int error = 0;
	unsigned int i = 0;
	int audioStream = 0;
	AVPacket packet;
	uint8_t *pktdata;
	int pktsize;

	head_pama pt;
	int data_size = 0;

	error =av_open_input_file(&pFormatCtx, inputfilename, NULL, 0, NULL);

	if(error !=0)
	{
		return 1;  
	}

	error = av_find_stream_info(pFormatCtx);

	if( error <0)
	{
		printf("Couldn't find stream information error :%d\n",error);
		return 1;
	}

	for(i=0; i< pFormatCtx->nb_streams; i++)
	{

		if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO && audioStream < 0)
		{
			audioStream = i;
		}
	}

	if(audioStream== -1)
	{
		return 1;
	}

	aCodecCtx=pFormatCtx->streams[audioStream]->codec;

	pt.bits = 16;
	pt.channels = 1;
	pt.rate = aCodecCtx->sample_rate;

	aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
	if(!aCodec) 
	{
		exit(1);
	}
	// Open adio codec
	if(avcodec_open(aCodecCtx, aCodec)<0)
	{
		return 1;
	}

	//分配输出内存
	outbuf = (uint8_t*)malloc(AVCODEC_MAX_AUDIO_FRAME_SIZE);
	outfile = fopen(outfilename, "wb");
	if (!outfile) 
	{
		av_free(aCodecCtx);
		return 1;
	}

	fseek(outfile,44,SEEK_SET);
	while(av_read_frame(pFormatCtx, &packet) >= 0) 
	{
	    if(packet.stream_index == audioStream)
	    {
			pktdata = packet.data;
			pktsize = packet.size;
			while(pktsize > 0)
			{
				out_size = AVCODEC_MAX_AUDIO_FRAME_SIZE *100;
				len = avcodec_decode_audio2(aCodecCtx,(short *)outbuf, &out_size,packet.data,packet.size); //解码
				if (len < 0) 
			    {
				    break;
			    }

				if (out_size > 0)
			    {
					memcpy(sample,outbuf,out_size);
					for(m=0;m<out_size/2;m+=2)
						outsample[m/2]=sample[m];
					fwrite(outsample, 2, out_size/4, outfile);
					data_size += out_size/4;
			    }
				pktsize -= len;
				pktdata += len;
			}
	    } 
		// Free the packet that was allocated by av_read_frame
		frame ++;
		av_free_packet(&packet);
	}

	fwrite(silsample,2,22050,outfile);
	data_size += 22050;
	fseek(outfile,0,SEEK_SET);
	wav_write_header(outfile,pt,data_size);

    fclose(outfile);
    free(outbuf);
    avcodec_close(aCodecCtx);
    av_free(aCodecCtx);
	return 0;
}

int mp3decode(const char *outfilename, const char *inputfilename)
{
	int result = decidemp3format(inputfilename);
	if(!result)
	{
		avcodec_init();
		av_register_all();
		avcodec_register_all();

		mp3decodefunc(outfilename, inputfilename);

		return 0;
	}
	else
	{
		return 1;
	}
}
int main()
{
    const char* src = "G:\\mp3decode\\北国之春之榕树下只故乡的雨.mp3";
	const char* dst = "G:\\mp3decode\\北国之春之榕树下只故乡的雨.wav";
	int resdec = mp3decode(dst, src);
	if(resdec)
		return 0;
	else
		return 1;
}