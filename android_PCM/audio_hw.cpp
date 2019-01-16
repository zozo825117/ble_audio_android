/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "audio_hw_ble"
#define LOG_NDEBUG 0

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>

#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include <tinyalsa/asoundlib.h>

#include <media/nbaio/MonoPipe.h>
#include <media/nbaio/MonoPipeReader.h>
#include <media/AudioBufferProvider.h>

#include <utils/String8.h>
#include <media/AudioParameter.h>

extern "C" {

namespace android {

#define MAX_PIPE_DEPTH_IN_FRAMES     (1024*8)
// The duration of MAX_READ_ATTEMPTS * READ_ATTEMPT_SLEEP_MS must be stricly inferior to
//   the duration of a record buffer at the current record sample rate (of the device, not of
//   the recording itself). Here we have:
//      3 * 5ms = 15ms < 1024 frames * 1000 / 48000 = 21.333ms
#define MAX_READ_ATTEMPTS            3
#define READ_ATTEMPT_SLEEP_MS        5 // 5ms between two read attempts when pipe is empty
#define DEFAULT_RATE_HZ              48000 // default sample rate
/*
struct ble_config {
    audio_format_t format;
    audio_channel_mask_t channel_mask;
    unsigned int rate; // sample rate for the device
    unsigned int period_size; // size of the audio pipe is period_size * period_count in frames
    unsigned int period_count;
};
*/
struct ble_audio_device {
    struct audio_hw_device device;
 //   bool output_standby;
    bool input_standby;
  //  ble_config config;
    
    int  upcm_card_num; 


    // Pipe variables: they handle the ring buffer that "pipes" audio:
    //  - from the submix virtual audio output == what needs to be played
    //    remotely, seen as an output for AudioFlinger
    //  - to the virtual audio source == what is captured by the component
    //    which "records" the submix / virtual audio source, and handles it as needed.
    // A usecase example is one where the component capturing the audio is then sending it over
    // Wifi for presentation on a remote Wifi Display device (e.g. a dongle attached to a TV, or a
    // TV with Wifi Display capabilities), or to a wireless audio player.
  //  sp<MonoPipe>       rsxSink;
 //   sp<MonoPipeReader> rsxSource;

    // device lock, also used to protect access to the audio pipe
    pthread_mutex_t lock;
};

struct ble_stream_out {
    struct audio_stream_out stream;
    struct ble_audio_device *dev;
};

struct ble_stream_in {
    struct audio_stream_in stream;
    struct ble_audio_device *dev;

    struct pcm_config config;
    struct pcm *pcm;

  //  bool output_standby; // output standby state as seen from record thread
    bool standby;
    // wall clock when recording starts
    struct timespec record_start_time;
    // how many frames have been requested to be read
    int64_t read_counter_frames;
};

/* audio HAL functions */

/** audio_stream_in implementation **/
static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
    const struct ble_stream_in *in = reinterpret_cast<const struct ble_stream_in *>(stream);
    //ALOGV("in_get_sample_rate() returns %u", in->dev->config.rate);
    return in->config.rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    return -ENOSYS;
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
    const struct ble_stream_in *in = reinterpret_cast<const struct ble_stream_in *>(stream);
    ALOGV("in_get_buffer_size() returns %u",
            in->config.period_size * audio_stream_frame_size(stream));
    return in->config.period_size * audio_stream_frame_size(stream);
}

static audio_channel_mask_t in_get_channels(const struct audio_stream *stream)
{
    return AUDIO_CHANNEL_IN_MONO;
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
    return AUDIO_FORMAT_PCM_16_BIT;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format)
{
    if (format != AUDIO_FORMAT_PCM_16_BIT) {
        return -ENOSYS;
    } else {
        return 0;
    }
}

static int in_standby(struct audio_stream *stream)
{
    ALOGI("in_standby()");
    struct ble_stream_in *in = reinterpret_cast< struct ble_stream_in *>(stream);

    pthread_mutex_lock(&in->dev->lock);
    if (!in->standby && in->pcm ) {
        pcm_close(in->pcm);
        in->pcm = NULL;
        in->standby = true;
    }
    pthread_mutex_unlock(&in->dev->lock);

    return 0;
}

static int in_dump(const struct audio_stream *stream, int fd)
{
    return 0;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    return 0;
}

static char * in_get_parameters(const struct audio_stream *stream,
                                const char *keys)
{
    return strdup("");
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
    return 0;
}

static ssize_t in_read(struct audio_stream_in *stream, void* buffer,
                       size_t bytes)
{
     int ret;
     struct ble_stream_in *in = (struct ble_stream_in *) stream;
     pthread_mutex_lock(&in->dev->lock);
     if (in->standby) {
      //  ALOGE("TRY TO PCM_OPEN") ;
        in->pcm = pcm_open(in->dev->upcm_card_num, 0, PCM_IN, &in->config); 
        if (in->pcm == NULL ) {
            ALOGE("PCM_OPEN FAILED"); 
        }

        in->standby = false;
    }
    
    ret = pcm_read(in->pcm, buffer, bytes);
  //  ALOGE("in read ret=%d bytes=%d", ret, bytes);
    pthread_mutex_unlock(&in->dev->lock);

    return bytes;
    /*
    //generate saw wave 
    unsigned char *data = (unsigned char *)buffer;
    int pcmreturn, l1, l2;
    short s1, s2;
  //  int frames;
    int num_frames = bytes / 4  -1;
      for(l2 = 0; l2 < num_frames; l2++) {
        s1 = (l2 % 128) * 100 - 5000;  
        s2 = (l2 % 256) * 100 - 5000;  
        data[4*l2] = (unsigned char)s1;
        data[4*l2+1] = s1 >> 8;
        data[4*l2+2] = (unsigned char)s2;
        data[4*l2+3] = s2 >> 8;
      }

    //memset(buffer, 0, bytes);
    usleep(bytes * 1000000 / audio_stream_frame_size(&stream->common) /
           in_get_sample_rate(&stream->common));
    return bytes; */

    
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
    return 0;
}

static int in_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    return 0;
}

static int in_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    return 0;
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs)
{
    return -ENOSYS;
}

static char * adev_get_parameters(const struct audio_hw_device *dev,
                                  const char *keys)
{
    return strdup("");;
}

static int adev_init_check(const struct audio_hw_device *dev)
{
    ALOGI("adev_init_check()");
    return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
{
    return -ENOSYS;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
{
    return -ENOSYS;
}

static int adev_get_master_volume(struct audio_hw_device *dev, float *volume)
{
    return -ENOSYS;
}

static int adev_set_master_mute(struct audio_hw_device *dev, bool muted)
{
    return -ENOSYS;
}

static int adev_get_master_mute(struct audio_hw_device *dev, bool *muted)
{
    return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode)
{
    return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
{
    return -ENOSYS;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
    return -ENOSYS;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config)
{
    //### TODO correlate this with pipe parameters
    return  4096;
}

static int find_upcm_card_num()
{
    /*
     char buf[100];
     char *p, *pp;
     int  len;
     FILE * fp = fopen("/proc/asound/CSRUPCM/pcm0c/info", "r"); 
     if (fp == NULL)  {
        ALOGE("find_upcm_card_num, failed to get upcm card num, upcm.ko not insmod?");
        return -1;
     }

     fgets(buf, 100, fp);
     buf[strlen(buf)-1] = '\0';

     for(p=buf; *p != '\0'; p++) {
        char c =  *p;
        if (c >='0' && c<='9') 
            break;
     }

     int ret = atoi(p);
     fclose(fp);
     return ret; */
     return 1;
}


static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in)
{
    ALOGI("adev_open_input_stream()");

    struct ble_audio_device *adev = (struct ble_audio_device *)dev;
    struct ble_stream_in *in;
    int ret;

    in = (struct ble_stream_in *)calloc(1, sizeof(struct ble_stream_in));
    if (!in) {
        ret = -ENOMEM;
        goto err_open;
    }

    //if (adev->upcm_card_num < 0)
    adev->upcm_card_num = find_upcm_card_num();
    if (adev->upcm_card_num < 0 ) 
        return -ENODEV;

    pthread_mutex_lock(&adev->lock);

    in->stream.common.get_sample_rate = in_get_sample_rate;
    in->stream.common.set_sample_rate = in_set_sample_rate;
    in->stream.common.get_buffer_size = in_get_buffer_size;
    in->stream.common.get_channels = in_get_channels;
    in->stream.common.get_format = in_get_format;
    in->stream.common.set_format = in_set_format;
    in->stream.common.standby = in_standby;
    in->stream.common.dump = in_dump;
    in->stream.common.set_parameters = in_set_parameters;
    in->stream.common.get_parameters = in_get_parameters;
    in->stream.common.add_audio_effect = in_add_audio_effect;
    in->stream.common.remove_audio_effect = in_remove_audio_effect;
    in->stream.set_gain = in_set_gain;
    in->stream.read = in_read;
    in->stream.get_input_frames_lost = in_get_input_frames_lost;

    config->channel_mask = AUDIO_CHANNEL_IN_MONO;
    config->sample_rate = 16000;  
    config->format = AUDIO_FORMAT_PCM_16_BIT;

    in->config.rate = 16000;
    in->config.format = PCM_FORMAT_S16_LE; 
    in->config.period_size = 1024;
    in->config.period_count = 4;
    in->config.channels = 1;

    in->standby = true;
    in->pcm = NULL;

    *stream_in = &in->stream;

    in->dev = adev;

   // in->read_counter_frames = 0;
   // in->output_standby = adev->output_standby;

    pthread_mutex_unlock(&adev->lock);

    return 0;

err_open:
    *stream_in = NULL;
    return ret;
}

static void adev_close_input_stream(struct audio_hw_device *dev,
                                   struct audio_stream_in *stream)
{
    ALOGV("adev_close_input_stream()");
    //struct ble_audio_device *adev = (struct ble_audio_device *)dev;
    struct ble_stream_in *in = reinterpret_cast<struct ble_stream_in *>(stream);


      pthread_mutex_lock(&in->dev->lock);

      if (!in->standby && in->pcm){
         pcm_close(in->pcm);
         in->pcm = NULL;
         in->standby = true;
      }

      pthread_mutex_unlock(&in->dev->lock);
/*
    MonoPipe* sink = adev->rsxSink.get();
    if (sink != NULL) {
        ALOGI("shutdown");
        sink->shutdown(true);
    }

    free(stream);
*/
  //  pthread_mutex_unlock(&adev->lock);
}

static int adev_dump(const audio_hw_device_t *device, int fd)
{
    return 0;
}

static int adev_close(hw_device_t *device)
{
    ALOGI("adev_close()");
    free(device);
    return 0;
}

static int adev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device)
{
    ALOGI("adev_open(name=%s)", name);
    struct ble_audio_device *adev;

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    adev = (ble_audio_device*) calloc(1, sizeof(struct ble_audio_device));
    if (!adev)
        return -ENOMEM;

    adev->device.common.tag = HARDWARE_DEVICE_TAG;
    adev->device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
    adev->device.common.module = (struct hw_module_t *) module;
    adev->device.common.close = adev_close;

    adev->device.init_check = adev_init_check;
    adev->device.set_voice_volume = adev_set_voice_volume;
    adev->device.set_master_volume = adev_set_master_volume;
    adev->device.get_master_volume = adev_get_master_volume;
    adev->device.set_master_mute = adev_set_master_mute;
    adev->device.get_master_mute = adev_get_master_mute;
    adev->device.set_mode = adev_set_mode;
    adev->device.set_mic_mute = adev_set_mic_mute;
    adev->device.get_mic_mute = adev_get_mic_mute;
    adev->device.set_parameters = adev_set_parameters;
    adev->device.get_parameters = adev_get_parameters;
    adev->device.get_input_buffer_size = adev_get_input_buffer_size;
  //  adev->device.open_output_stream = adev_open_output_stream;
   // adev->device.close_output_stream = adev_close_output_stream;
    adev->device.open_input_stream = adev_open_input_stream;
    adev->device.close_input_stream = adev_close_input_stream;
    adev->device.dump = adev_dump;
    //adev->input_standby = true;
    adev->upcm_card_num = -1;
 //   adev->output_standby = true;

    *device = &adev->device.common;

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    /* open */ adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    /* common */ {
        /* tag */                HARDWARE_MODULE_TAG,
        /* module_api_version */ AUDIO_MODULE_API_VERSION_0_1,
        /* hal_api_version */    HARDWARE_HAL_API_VERSION,
        /* id */                 AUDIO_HARDWARE_MODULE_ID,
        /* name */               "BLE audio HAL",
        /* author */             "The Android Open Source Project",
        /* methods */            &hal_module_methods,
        /* dso */                NULL,
        /* reserved */           { 0 },
    },
};

} //namespace android

} //extern "C"
