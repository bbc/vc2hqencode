/*****************************************************************************
 * vc2encoder.cpp : Test Program
 *****************************************************************************
 * Copyright (C) 2014-2015 BBC
 *
 * Authors: James P. Weaver <james.barrett@bbc.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at ipstudio@bbc.co.uk.
 *****************************************************************************/

#include <string.h>
#include <vc2hqencode/vc2hqencode.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <sstream>

#include <string>

#include "program_options_lite.h"

#define MIN(A,B) (((A)>(B))?(B):(A))


void generateMovingZonePlate(uint8_t *odata, int w, int h, int stride);

void usage() {
  printf("Usage: vc2encode [options] [ input_file [output_file] ]\n");
  printf("  If no input is specified use internal zoneplate generator\n");
  printf("  By default the output file will be the input file with .vc2 appended\n");
  printf("  All short program options must have a space between them and their parameter\n");
}

int main (int argc, char *argv[]) {
  int err = 0;
  VC2EncoderResult r;

  /* Initialise Library */
  vc2encode_init();

  /* Program Option parsing */
  namespace po = df::program_options_lite;

  int num_frames;
  int ratio;
  int threads_number;
  bool help;
  std::string wavelet_string;
  std::string speed_string;
  int depth;
  bool disable_output;
  bool interlace;
  bool V210;
  int width;
  int height;
  int slice_width;
  int slice_height;
  std::string input_filename;
  std::string output_filename;
  bool zoneplate;

  po::Options opts;
  opts.addOptions()
    ("help,h",         help,           false,    "produce help message")
    ("num-frames,n",   num_frames,     1,        "number of frames to encode")
    ("ratio,r",        ratio,          2,        "reciprocal of compression ration (default 2)")
    ("threads,t",      threads_number, 1,        "number of threads (minimum 1)")
    ("wavelet,w",      wavelet_string, std::string("legall"), "wavelet kernel: `fidelity', `deslauriers-debuc-9-7', `deslauriers-debuc-13-7', `legall', `haar0', or `haar1'")
    ("depth,d",        depth,          3,        "wavelet depth: default is 3")
    ("slicewidth",     slice_width,    32,       "slice width: default is 32")
    ("sliceheight",    slice_height,   8,        "slice height: default is 8")
    ("disable-output", disable_output, false,    "disable output")
    ("speed",          speed_string,   std::string("medium"), "speed: slowest, slower, slow, medium (default), fast, faster, fastest")
    ("interlace",      interlace,      false,    "Encode interlaced instead of progressive (input video should still be in full frames)")
    ("V210",           V210,           false,    "Input file is V210 instead of yuv422p10le")
    ("width",          width,          1920,     "video width (default=1920)")
    ("height",         height,         1080,     "video height (default=1080)")
    ;

  po::setDefaults(opts);
  const std::list<const char*>& argv_unhandled = po::scanArgv(opts, argc, (const char**) argv);
  std::list<const char*>::const_iterator argv_unhandled_it = argv_unhandled.begin();

  if (help) {
    usage();
    po::doHelp(std::cout, opts, 81);
    return 0;
  }

  int speed = VC2ENCODER_SPEED_SLOWEST;
  if (speed_string == "slowest")
    speed = VC2ENCODER_SPEED_SLOWEST;
  else if (speed_string == "slower")
    speed = VC2ENCODER_SPEED_SLOWER;
  else if (speed_string == "slow")
    speed = VC2ENCODER_SPEED_SLOW;
  else if (speed_string == "medium")
    speed = VC2ENCODER_SPEED_MEDIUM;
  else if (speed_string == "fast")
    speed = VC2ENCODER_SPEED_FAST;
  else if (speed_string == "faster")
    speed = VC2ENCODER_SPEED_FASTER;
  else if (speed_string == "fastest")
    speed = VC2ENCODER_SPEED_FASTEST;
  else {
    printf("Inavlid speed selected\n\n");
    usage();
    po::doHelp(std::cout, opts, 81);
    return 1;
  }


  int wavelet;
  if (wavelet_string == "fidelity")
    wavelet = VC2ENCODER_WFT_FIDELITY;
  else if (wavelet_string == "deslauriers-debuc-9-7")
    wavelet = VC2ENCODER_WFT_DESLAURIERS_DUBUC_9_7;
  else if (wavelet_string == "deslauriers-debuc-13-7")
    wavelet = VC2ENCODER_WFT_DESLAURIERS_DUBUC_13_7;
  else if (wavelet_string == "legall")
    wavelet = VC2ENCODER_WFT_LEGALL_5_3;
  else if (wavelet_string == "haar0")
    wavelet = VC2ENCODER_WFT_HAAR_NO_SHIFT;
  else if (wavelet_string == "haar1")
    wavelet = VC2ENCODER_WFT_HAAR_SINGLE_SHIFT;
  else {
    printf("Inavlid wavelet selected\n\n");
    usage();
    po::doHelp(std::cout, opts, 81);
    return 1;
  }

  if (threads_number < 1)
    threads_number = 1;
  int stride = width;
  int pad_top = 8;
  int pad_bot = 8;

  if (argv_unhandled_it == argv_unhandled.end()) {
    zoneplate = true;
    {
      std::stringstream ss;
      ss << "zoneplate_" << width << "x" << height << "_" << wavelet_string << "_";
      if (depth == 2)
        ss << "8x8x2_";
      else
        ss << "32x8x3_";
      ss << speed_string << "_" << num_frames << "frames_" << threads_number << "threads" << ".vc2";
      output_filename = ss.str();
    }
  } else {
    zoneplate = false;
    input_filename  = std::string(*(argv_unhandled_it++));
    if (argv_unhandled_it == argv_unhandled.end())
      output_filename = input_filename + ".vc2";
    else
      output_filename = std::string(*(argv_unhandled_it++));
  }


  /* Create Encoder */
  VC2EncoderHandle encoder = vc2encode_create();

  /* Configure encoder */
  VC2EncoderParams params;
  memset(&params, 0, sizeof(params));

  if (width == 1920 && height == 1080) {
    if (!interlace) {
      params.video_format.base_video_format = VC2ENCODER_BVF_HD1080P_50;
      params.video_format.custom_frame_rate_flag = 1;
      params.video_format.frame_rate_index       = 3; // 25 mFPS
      params.picture_coding_mode            = VC2ENCODER_PCM_FRAME;
    } else {
      params.video_format.base_video_format = VC2ENCODER_BVF_HD1080I_50;
      params.video_format.custom_frame_rate_flag = 0;
      params.picture_coding_mode            = VC2ENCODER_PCM_FIELD;
    }
  } else if (width == 3840 && height == 2160) {
    params.video_format.base_video_format = VC2ENCODER_BVF_UHDTV4K_50;
    if (!interlace) {
      params.picture_coding_mode            = VC2ENCODER_PCM_FRAME;
    } else {
      params.video_format.custom_scan_format_flag = 1;
      params.video_format.source_sampling   = VC2ENCODER_PCM_FIELD;
      params.picture_coding_mode            = VC2ENCODER_PCM_FIELD;
    }
  } else {
    params.video_format.custom_dimensions_flag = 1;
    params.video_format.frame_width            = width;
    params.video_format.frame_height           = height;

    if (!interlace) {
      params.video_format.base_video_format = VC2ENCODER_BVF_HD1080P_50;
      params.video_format.custom_frame_rate_flag = 1;
      params.video_format.frame_rate_index       = 3; // 25 mFPS
      params.picture_coding_mode            = VC2ENCODER_PCM_FRAME;
    } else {
      params.video_format.base_video_format = VC2ENCODER_BVF_HD1080I_50;
      params.video_format.custom_frame_rate_flag = 0;
      params.picture_coding_mode            = VC2ENCODER_PCM_FIELD;
    }
  }
  params.transform_params.wavelet_index = wavelet;
  params.transform_params.wavelet_depth = depth;
  params.transform_params.slice_width   = slice_width;
  params.transform_params.slice_height  = slice_height;

  params.n_threads                      = threads_number;
  params.speed                          = speed;
  if (!V210)
    params.input_format                   = VC2ENCODER_INPUT_10P2;
  else
    params.input_format                   = VC2ENCODER_INPUT_V210;

  r = vc2encode_set_parameters(encoder, params);
  if (r != VC2ENCODER_OK) {
    printf("Error in vc2encode_set_parameters\n");
    return 1;
  }



  /* Get sequence start size */
  uint32_t seq_start_size;
  r = vc2encode_get_sequence_start_size(encoder, &seq_start_size);
  if (r != VC2ENCODER_OK) {
    printf("Error in vc2encode_get_sequence_start_size\n");
    return 1;
  }



  /* Get picture start size */
  uint32_t pic_start_size;
  r = vc2encode_get_coded_picture_start_size(encoder, &pic_start_size);
  if (r != VC2ENCODER_OK) {
    printf("Error in vc2encode_get_coded_picture_start_size\n");
    return 1;
  }



  /* Get sequence end size */
  uint32_t seq_end_size;
  r = vc2encode_get_sequence_end_size(encoder, &seq_end_size);
  if (r != VC2ENCODER_OK) {
    printf("Error in vc2encode_get_sequence_end_size\n");
    return 1;
  }


  /* Variables for storing input and output buffers */
  char *idata[num_frames];
  char *ipictures[num_frames*(interlace?2:1)][3];
  int istride[3];

  /* Read input files or generate zoneplate */
  /*
       This code also pads with bright colours around the edges of the
       input data, to make overrun detection easier.
  */
  int readframes = 0;
  if (!zoneplate) {
    if (V210) {
      /* V210 Input Format */
      pad_top = pad_bot = 8;
      stride = ((((width + 47)/48)*128 + 4095)/4096)*4096;

      int f = open(input_filename.c_str(), O_RDONLY);

      int length = stride*(pad_top + height + pad_bot)*sizeof(uint8_t);
      int linelength = ((width + 47)/48)*128;

      ssize_t s;
      for (int i = 0; i < num_frames; i++) {
        int linesread = 0;
        idata[i] = (char *)malloc(length);
        int y = 0;
        for (; y < pad_top; y++) {
          for (int x = 0; x < stride; x += 4) {
            *((uint32_t*)(idata[i] + y*stride + x)) = 0x3FFFFFFF;
          }
        }
        for (; y < pad_top + height; y++) {
          s = read(f, idata[i] + y*stride*sizeof(uint8_t), linelength);
          if (s < linelength)
            break;
          for (int x = linelength; x < stride; x += 4) {
            *((uint32_t*)(idata[i] + y*stride + x)) = 0x3FFFFFFF;
          }
          linesread++;
        }
        for (; y < pad_top + height + pad_bot; y++) {
          for (int x = 0; x < stride; x += 4) {
            *((uint32_t*)(idata[i] + y*stride + x)) = 0x3FFFFFFF;
          }
        }
        if (linesread < height) {
          free(idata[i]);
          idata[i] = NULL;
          break;
        }
        readframes = i + 1;
      }

      close(f);
      printf("Read %d frames in %dbytes of input data\n", readframes, readframes*height*linelength);

      if (interlace) {
        istride[0] = stride*2;
        istride[1] = 0;
        istride[2] = 0;

        for (int i =0; i < readframes; i++) {
          ipictures[2*i + 0][0] = idata[i] + pad_top*stride;
          ipictures[2*i + 0][1] = NULL;
          ipictures[2*i + 0][2] = NULL;

          ipictures[2*i + 1][0] = idata[i] + pad_top*stride + stride;
          ipictures[2*i + 1][1] = NULL;
          ipictures[2*i + 1][2] = NULL;
        }
      } else {
        istride[0] = stride;
        istride[1] = 0;
        istride[2] = 0;

        for (int i =0; i < readframes; i++) {
          ipictures[i][0] = idata[i] + pad_top*stride;
          ipictures[i][1] = NULL;
          ipictures[i][2] = NULL;
        }
      }
    } else {
      /* Planar input format */
      pad_top = pad_bot = 8;
      stride = ((width*2 + 4095)/4096)*2048;

      int f = open(input_filename.c_str(), O_RDONLY);
      int length = stride*(pad_top + height + pad_bot)*2*sizeof(uint16_t);
      int linelength = width*sizeof(uint16_t);

      int LUMA_SIZE   = stride*(pad_top + height + pad_bot)*sizeof(uint16_t);
      int CHROMA_SIZE = stride/2*(pad_top + height + pad_bot)*sizeof(uint16_t);

      ssize_t s;
      for (int i = 0; i < num_frames; i++) {
        int linesread = 0;
        idata[i] = (char *)malloc(length);

        for (int y = pad_top; y < height + pad_top; y++) {
          s = read(f, idata[i] + y*stride*sizeof(uint16_t), linelength);
          if (s < linelength)
            break;
          linesread++;
        }
        for (int y = pad_top; y < height + pad_top; y++) {
          s = read(f, idata[i] + LUMA_SIZE + y*stride/2*sizeof(uint16_t), linelength/2);
          if (s < linelength/2)
            break;
          linesread++;
        }
        for (int y = pad_top; y < height + pad_top; y++) {
          s = read(f, idata[i] + LUMA_SIZE + CHROMA_SIZE + y*stride/2*sizeof(uint16_t), linelength/2);
          if (s < linelength/2)
            break;
          linesread++;
        }
        if (linesread < 3*height) {
          free(idata[i]);
          idata[i] = NULL;
          break;
        }

        readframes = i + 1;
      }

      close(f);
      printf("Read %d frames in %dbytes of input data\n", readframes, readframes*height*linelength);

      for (int i = 0; i < readframes; i++) {
        for (int c = 0; c < 3; c++) {
          int OFFS = (c==0)?(0):((c == 1)?(LUMA_SIZE):(LUMA_SIZE + CHROMA_SIZE));
          int st = (c==0)?(stride):(stride/2);
          int w  = (c==0)?(width):(width/2);
          int y = 0;
          for (y = 0; y < pad_top; y++) {
            for (int x = 0; x < st; x ++) {
              ((uint16_t*)idata[i])[OFFS/2 + y*st + x] = 0x3FF;
            }
          }
          for (; y < height + pad_top; y++) {
            int x;
            for (x = w; x < st; x++) {
              ((uint16_t*)idata[i])[OFFS/2 + y*st + x] = 0x3FF;
            }
          }
          for (; y < pad_top + height + pad_bot; y++) {
            for (int x = 0; x < st; x ++) {
              ((uint16_t*)idata[i])[OFFS/2 + y*st + x] = 0x3FF;
            }
          }
        }
      }

      if (interlace) {
        istride[0] = stride*2;
        istride[1] = stride;
        istride[2] = stride;

        for (int i =0; i < readframes; i++) {
          ipictures[2*i + 0][0] = idata[i] + pad_top*stride*sizeof(uint16_t);
          ipictures[2*i + 0][1] = idata[i] + LUMA_SIZE + pad_top*stride/2*sizeof(uint16_t);
          ipictures[2*i + 0][2] = idata[i] + LUMA_SIZE + CHROMA_SIZE + pad_top*stride/2*sizeof(uint16_t);

          ipictures[2*i + 1][0] = idata[i] + pad_top*stride*sizeof(uint16_t) + stride*sizeof(uint16_t);
          ipictures[2*i + 1][1] = idata[i] + LUMA_SIZE + pad_top*stride/2*sizeof(uint16_t) + stride/2*sizeof(uint16_t);
          ipictures[2*i + 1][2] = idata[i] + LUMA_SIZE + CHROMA_SIZE + pad_top*stride/2*sizeof(uint16_t) + stride/2*sizeof(uint16_t);
        }
      } else {
        istride[0] = stride;
        istride[1] = stride/2;
        istride[2] = stride/2;

        for (int i =0; i < readframes; i++) {
          ipictures[i][0] = idata[i] + pad_top*stride*sizeof(uint16_t);
          ipictures[i][1] = idata[i] + LUMA_SIZE + pad_top*stride/2*sizeof(uint16_t);
          ipictures[i][2] = idata[i] + LUMA_SIZE + CHROMA_SIZE + pad_top*stride/2*sizeof(uint16_t);
        }
      }
    }
  } else {
    /* Generate some zoneplate */
    pad_top = pad_bot = 0;
    stride = width;
    int length = width*height*2*sizeof(uint16_t);

    for (int n = 0; n < num_frames; n++) {
      idata[n] = (char *)malloc(length);
      generateMovingZonePlate((uint8_t *)idata[n], width, height, width);
    }
    readframes = num_frames;

    const int LUMA_SIZE   = height*stride*sizeof(uint16_t);
    const int CHROMA_SIZE = height*stride/2*sizeof(uint16_t);

    if (interlace) {
      istride[0] = stride*2;
      istride[1] = stride;
      istride[2] = stride;

      for (int i =0; i < readframes; i++) {
        ipictures[2*i + 0][0] = idata[i];
        ipictures[2*i + 0][1] = idata[i] + LUMA_SIZE;
        ipictures[2*i + 0][2] = idata[i] + LUMA_SIZE + CHROMA_SIZE;

        ipictures[2*i + 0][0] = idata[i] + stride*sizeof(uint16_t);
        ipictures[2*i + 0][1] = idata[i] + LUMA_SIZE + stride/2*sizeof(uint16_t);
        ipictures[2*i + 0][2] = idata[i] + LUMA_SIZE + CHROMA_SIZE + stride/2*sizeof(uint16_t);
      }
    } else {
      istride[0] = stride;
      istride[1] = stride/2;
      istride[2] = stride/2;

      for (int i =0; i < readframes; i++) {
        ipictures[i][0] = idata[i];
        ipictures[i][1] = idata[i] + LUMA_SIZE;
        ipictures[i][2] = idata[i] + LUMA_SIZE + CHROMA_SIZE;
      }
    }
  }



  /* Allocate Output Buffer */
  int num_output_pictures = MIN(num_frames, readframes)*(interlace?2:1);
  if (disable_output) /* If output is disabled then only bother with space for one frame */
    num_output_pictures = (interlace?2:1);
  int bytes_per_picture = (width*height*2*10/(8*ratio*(interlace?2:1)));

  char *odata = (char *)malloc(seq_start_size + num_output_pictures*(pic_start_size + bytes_per_picture) + seq_end_size);








  /* Start main loop */
  int total_frames_encoded = 0;
  uint64_t time_taken = 0;
  while (total_frames_encoded < num_frames) {
    /* Reset to start of output buffer */
    char *o = odata;
    uint32_t parse_offset;

    /* Start Sequence */
    r = vc2encode_start_sequence(encoder, &o, &parse_offset);
    if (r != VC2ENCODER_OK) {
      printf("Failed to start sequence\n");
      err = 1;
      break;
    }

    int n = 0;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (; n < num_output_pictures && total_frames_encoded < num_frames; n++) {
      /* Start picture */
      r = vc2encode_start_picture(encoder, &o, parse_offset, n, bytes_per_picture, &parse_offset);
      if (r != VC2ENCODER_OK) {
        printf("Failed to start picture\n");
        err = 1;
        break;
      }

      r = vc2encode_encode_data(encoder, ipictures[n], istride, &o, bytes_per_picture);
      if (r != VC2ENCODER_OK) {
        printf("Failed to encode picture\n");
        err = 1;
        break;
      }

      if (!interlace || (n%2 == 1))
        total_frames_encoded++;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_taken += (end.tv_sec*1000000000 + end.tv_nsec) - (start.tv_sec*1000000000 + start.tv_nsec);

    if (err)
      break;

    if (n == num_output_pictures) {
      r = vc2encode_end_sequence(encoder, &o, parse_offset);
      if (r != VC2ENCODER_OK) {
        printf("Failed to end sequence\n");
        err = 1;
        break;
      }
    }
  }

  if (!err) {
    printf("Encoded %d frames in %5.3fs -- %5.3f fps\n", total_frames_encoded, time_taken/1000000000.0, total_frames_encoded*1000000000.0/time_taken);
  }


  /* Write output */
  if (!err && !disable_output) {
    ssize_t s;
    int of = open(output_filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 00777);
    s = write(of, odata, seq_start_size + num_output_pictures*(pic_start_size + bytes_per_picture) + seq_end_size);
    if (s < 0)
      printf("Output error\n");
    else
      printf("Wrote %d pictures in %d bytes\n", num_output_pictures, seq_start_size + num_output_pictures*(pic_start_size + bytes_per_picture) + seq_end_size);
  }

  /* Destroy encoder */
  vc2encode_destroy(encoder);


  /* Delete buffers */
  for (int i = 0; i < readframes; i++)
    if (idata[i]) free(idata[i]);
  free(odata);

  return err;
}
