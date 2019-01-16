#include "common.h"
#include "ADPCMDecoder.h"
int16_t covertTo16Int(uint8_t msb, uint8_t  lsb)
{
    int16_t res = lsb | (msb << 8);
    return res;
}

int adpcm_decode(unsigned char * adpcm_buf,  int len, short * pcm_buf)
{
	int i =0;
	for (i=0; i<len; i++) {
		uint8_t nib1 =  (adpcm_buf[i] & 0x0f);
		uint8_t nib2 =  (adpcm_buf[i] & 0xf0)>>4;

		pcm_buf[i * 2 ] = ADPCMDecoder(nib1) * 3;
		pcm_buf[i * 2 + 1] = ADPCMDecoder(nib2) * 3;
	}
	return len * 2;
}

int default_audio_main(int fd)
{
    int   len;
    short pcm_buf[1024];
    int   offset = 0;
    unsigned char buf[1024];
    int total_cnt = 0;
    int i;
    /*
    while ( (len = read (fd, buf, 1024 )) >=0)
    {
        if (stream_started && stream_started && buf[0] == GEN_INPUT_REPORT_ID && buf[1] == 0x00)
        {
            stream_started = false;
            fprintf(stderr, "Audio KEY released, stop aduio\n");
            close_upcm_dev();
        }

        if (buf[0] == VOICE_CONTROL_PCM_DATA_REPORT_ID)
        {
            unsigned char * pdu = buf + 1;
            len -= 1;
            if (!stream_started)
            {
                fprintf(stderr, "Audio stream started\n");
                adpcm_decoder_init(pdu[0], pdu[1]);
                stream_started = true;
                pdu += 2;
                len -= 2;
                open_upcm_dev();
            }

            pcmDataSize = 0;
            adpcm_decoder_decode(pdu, len, pcmData, &pcmDataSize);
            write_upcm_dev((unsigned char *)pcmData, pcmDataSize * 2);
        }
    }*/

    int audio_fd = -1;
    while ( (len = read (fd, buf, 1024 )) >=0) {
    	
/*	int i=0; 
	
	if (buf[0] == 0x1F)
	{
		for (i=0; i<len; i++)
		{
			printf("%02X ", buf[i]);
		}

		printf("\n");
	}
	continue;
*/
	if (buf[0] == 0x1F) {
    		if (buf[1] == 0xFF && buf[2] == 0x01) {
    			 printf("Audio start\n");
    			 if (audio_fd < 0)
    			 	audio_fd = open("audio.pcm", O_WRONLY | O_CREAT);
    			
			open_upcm_dev();
		} else if (buf[1] == 0xFF && buf[2] == 0x00) {
			printf("Audio stopped\n");
			if (audio_fd >0) {
				close(audio_fd);
				audio_fd = -1;
			}
			close_upcm_dev();
		} else if (buf[1] == 0xFE ) {
			printf("Recv prevIndex and prevSample\n");
    			state.prevIndex = buf[2];
    			state.prevSample = covertTo16Int(buf[3], buf[4]);
    		}
    	} else if (buf[0] == 0x1E) {
    		int sample_cnt = adpcm_decode(buf+1, len-1, pcm_buf);
    		write(audio_fd, pcm_buf, sample_cnt * 2);
    		write_upcm_dev((unsigned char *)pcm_buf, sample_cnt * 2);
	}
    }

    if (audio_fd > 0 )
    	close(audio_fd);
}

struct device  dev_default =
{
    .name = "default",
    .vid = 0,
    .pid = 0,
    .desc_size = 220,
    .audio_main = default_audio_main
};
