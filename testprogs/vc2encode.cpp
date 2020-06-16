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

#include "tclap/CmdLine.h"

#define MIN(A,B) (((A)>(B))?(B):(A))


void generateMovingZonePlate(uint8_t *odata, int w, int h, int stride);

void usage() {
  printf("Usage: vc2encode [options] [ input_file [output_file] ]\n");
  printf("  If no input is specified use internal zoneplate generator\n");
  printf("  By default the output file will be the input file with .vc2 appended\n");
  printf("  All short program options must have a space between them and their parameter\n");
  printf("  All program options must have a parameter\n");
}

int main (int argc, char *argv[]) {
  int err = 0;
  VC2EncoderResult r;

  /* Initialise Library */
  vc2encode_init();

  int num_frames              = 1;
  float ratio                   = 2;
  int threads_number          = 1;
  std::string wavelet_string  = "legall";
  std::string speed_string    = "medium";
  int depth                   = 3;
  bool disable_output         = false;
  bool interlace              = false;
  bool V210                   = false;
  int width                   = 1920;
  int height                  = 1080;
  int slice_width             = 32;
  int slice_height            = 8;
  std::string input_filename;
  std::string output_filename;
  bool zoneplate              = false;
  int speed                   = VC2ENCODER_SPEED_SLOWEST;
  int fragment_size           = 0;

  try {
    TCLAP::CmdLine cmd("VC2 HQ profile Encoder Example\n"
                       "All input files must be yuv422p10le or v210\n"
                       "All output files will be vc2 streams\n"
                       "First file is input file, second is output, others are ignored", '=', "0.1", true);

    TCLAP::ValueArg<int> num_frames_arg          ("n", "num-frames",     "Number of frames to encode",          false, 1, "integer", cmd);
    TCLAP::ValueArg<float> ratio_arg               ("r", "ratio",          "r:1 compression ratio (float)",               false, 2, "float", cmd);
    TCLAP::ValueArg<int> num_threads_arg         ("t", "threads",        "Number of threads",                   false, 1, "integer", cmd);
    TCLAP::ValueArg<std::string> wavelet_arg     ("w", "wavelet",        "wavelet kernel: `fidelity', `deslauriers-debuc-9-7', `deslauriers-debuc-13-7', `legall', `haar0', or `haar1'", false, "legall", "string", cmd);
    TCLAP::ValueArg<int> depth_arg               ("d", "depth",          "wavelet depth",                       false, 3, "integer", cmd);
    TCLAP::ValueArg<int> slice_width_arg         ("", "slicewidth",    "slice width",                         false, 32, "integer", cmd);
    TCLAP::ValueArg<int> slice_height_arg        ("", "sliceheight",   "slice height",                        false, 8, "integer", cmd);
    TCLAP::SwitchArg     disable_output_arg      ("", "disable-output", "disable output",                                           cmd, false);
    TCLAP::ValueArg<std::string> speed_arg       ("", "speed",          "speed: slowest, slower, slow, medium (default), fast, faster, fastest", false, "medium", "string", cmd);
    TCLAP::SwitchArg     interlace_arg           ("", "interlace",      "interlaced output",                                        cmd, false);
    TCLAP::SwitchArg     V210_arg                ("", "V210",           "V210 input",                                               cmd, false);
    TCLAP::ValueArg<int> width_arg               ("", "width",          "frame width",                         false, 1920, "integer", cmd);
    TCLAP::ValueArg<int> height_arg              ("", "height",         "frame height",                        false, 1080, "integer", cmd);
    TCLAP::ValueArg<int> fragments_arg           ("f", "fragment",      "maximum size for picture fragments (default is don't fragment)",  false, 0,  "integer", cmd);
    
    TCLAP::UnlabeledMultiArg<std::string> file_args("input_file",   "encoded input file",                          false, "string", cmd);

    cmd.parse( argc, argv );

    num_frames          = num_frames_arg.getValue();
    ratio               = ratio_arg.getValue();
    threads_number      = num_threads_arg.getValue();
    wavelet_string      = wavelet_arg.getValue();
    depth               = depth_arg.getValue();
    slice_width         = slice_width_arg.getValue();
    slice_height        = slice_height_arg.getValue();
    disable_output      = disable_output_arg.getValue();
    speed_string        = speed_arg.getValue();
    interlace           = interlace_arg.getValue();
    V210                = V210_arg.getValue();
    width               = width_arg.getValue();
    height              = height_arg.getValue();
    fragment_size       = fragments_arg.getValue();

    std::vector<std::string> filenames = file_args.getValue();
    if (filenames.size() > 0) {
      input_filename  = filenames[0];
      if (filenames.size() > 1) {
        output_filename = filenames[1];
      } else {
        output_filename = input_filename + ".vc2";
      }
    } else {
      zoneplate = true;
    }

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
      return 1;
    }
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; return 1;
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
    return 1;
  }

  if (threads_number < 1)
    threads_number = 1;
  int stride = width;
  int pad_top = 8;
  int pad_bot = 8;

  if (zoneplate) {
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
  params.fragment_size = fragment_size;

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
      if (!f) {
        perror("Could not open input file");
        return 1;
      }

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
  printf("Setting bytes_per_picture=%d\n",bytes_per_picture);

  /* Get extra size needed for each frame to allow fragment headers */
  uint32_t pic_frag_size;
  r = vc2encode_get_fragment_headers_for_picture_size(encoder, bytes_per_picture, &pic_frag_size);
  if (r != VC2ENCODER_OK) {
    printf("Error in vc2encode_get_fragment_headers_for_picture_size\n");
    return 1;
  }

  char *odata = (char *)malloc(seq_start_size + num_output_pictures*(pic_start_size + bytes_per_picture + pic_frag_size) + seq_end_size);








  /* Start main loop */
  int total_frames_encoded = 0;
  uint64_t time_taken = 0;
  while (total_frames_encoded < num_frames) {
    /* Reset to start of output buffer */
    char *o = odata;
    uint32_t parse_offset = 0;

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

      r = vc2encode_encode_fragmented_data(encoder, ipictures[n], istride, &o, bytes_per_picture, parse_offset, &parse_offset);
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
    s = write(of, odata, seq_start_size + num_output_pictures*(pic_start_size + bytes_per_picture + pic_frag_size) + seq_end_size);
    if (s < 0)
      printf("Output error\n");
    else
      printf("Wrote %d pictures in %d bytes\n", num_output_pictures, seq_start_size + num_output_pictures*(pic_start_size + bytes_per_picture + pic_frag_size) + seq_end_size);
  }

  /* Destroy encoder */
  vc2encode_destroy(encoder);


  /* Delete buffers */
  for (int i = 0; i < readframes; i++)
    if (idata[i]) free(idata[i]);
  free(odata);

  return err;
}
