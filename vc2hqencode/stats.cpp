/*****************************************************************************
 * stats.cpp : Debug stats
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

#include "stats.hpp"

#include <string.h>
#include <stdio.h>

#define max(A,B) (((A) > (B))?(A):(B))

void accumulate_transformed_statistics(JobBase *_job) {
  JobData<int16_t> *job = dynamic_cast<JobData<int16_t> *>(_job);
  {
    uint32_t samples[2][4][32];
    memset(samples, 0, sizeof(samples));

    for (int c = 0; c < 3; c++) {
      for (int y = 0; y < job->video_data[c]->height; y++) {
        for (int x = 0; x < job->video_data[c]->width; x++) {
          uint32_t d = (uint32_t)abs(job->video_data[c]->data[y*job->video_data[c]->stride + x]);
          int l = (d==0)?0:(32 - __builtin_clz(d));

          switch(y%4) {
          case 0:
            switch (x%4) {
            case 0:
              samples[0][0][l]++;
              break;
            case 2:
              samples[0][1][l]++;
              break;
            case 1:
            case 3:
              samples[1][1][l]++;
              break;
            }
            break;
          case 2:
            switch (x%4) {
            case 0:
              samples[0][2][l]++;
              break;
            case 2:
              samples[0][3][l]++;
              break;
            case 1:
            case 3:
              samples[1][1][l]++;
              break;
            }
            break;
          case 1:
          case 3:
            switch (x%4) {
            case 0:
            case 2:
              samples[1][2][l]++;
              break;
            case 1:
            case 3:
              samples[1][3][l]++;
              break;
            }
            break;
          }
        }
      }
    }

    printf("--------------------------------------------------------------------------\n");
    printf("     Bit Frequency histograms by subband for job %d\n", job->job);
    printf("--------------------------------------------------------------------------\n");
    printf("  N    LL      |  LH      HL      HH      |  LH      HL      HH\n");
    for (int i = 0; i < 32; i++) {
      printf("  %2d:  %6u  |  %6u  %6u  %6u  |  %6u  %6u  %6u\n",
             i,
             samples[0][0][i],
             samples[0][1][i],
             samples[0][2][i],
             samples[0][3][i],
             samples[1][1][i],
             samples[1][2][i],
             samples[1][3][i]);
    }
    printf("--------------------------------------------------------------------------\n");
  }

  {
    uint32_t samples[32];
    memset(samples, 0, sizeof(samples));

    for (int y = 0; y < job->video_data[0]->height; y+=8) {
      for (int x = 0; x < job->video_data[0]->width; x+=8) {
        uint32_t d[3][2][4];
        int c;
        {
          c = 0;

          d[c][0][0] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x + 4]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x + 4]))
                        );
          
          d[c][0][1] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x + 6]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x + 6]))
                        );
          d[c][0][2] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x + 4]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 4]))
                        );
          d[c][0][3] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x + 6]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 6]))
                        );
          
          d[c][1][1] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x + 5]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x + 7]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x + 5]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x + 7]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x + 5]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x + 7]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 5]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 7]))
                        );
          d[c][1][2] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x + 4]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x + 6]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x + 4]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x + 6]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x + 4]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x + 6]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x + 4]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x + 6]))
                        );
          d[c][1][3] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x + 5]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x + 7]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x + 5]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x + 7]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x + 5]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x + 7]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x + 5]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x + 7]))
                        );
        }

        for (c=1; c < 3; c++) {
          d[c][0][0] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x/2 + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x/2 + 0]))
                        );
          
          d[c][0][1] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x/2 + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x/2 + 2]))
                        );
          d[c][0][2] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x/2 + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x + 0]))
                        );
          d[c][0][3] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x/2 + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x/2 + 2]))
                        );
          
          d[c][1][1] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x/2 + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 0)*job->video_data[c]->stride + x/2 + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x/2 + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 2)*job->video_data[c]->stride + x/2 + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x/2 + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 4)*job->video_data[c]->stride + x/2 + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x/2 + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 6)*job->video_data[c]->stride + x/2 + 3]))
                        );
          d[c][1][2] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x/2 + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x/2 + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x/2 + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x/2 + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x/2 + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x/2 + 2]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x/2 + 0]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x/2 + 2]))
                        );
          d[c][1][3] = (((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x/2 + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 1)*job->video_data[c]->stride + x/2 + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x/2 + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 3)*job->video_data[c]->stride + x/2 + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x/2 + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 5)*job->video_data[c]->stride + x/2 + 3]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x/2 + 1]))
                        | ((uint32_t)1+abs(job->video_data[c]->data[(y + 7)*job->video_data[c]->stride + x/2 + 3]))
                        );
        }

        int l[3][2][4];
        for (c = 0; c < 3; c++) {
          for (int lvl = 0; lvl < 2; lvl++) {
            for (int sb=0; sb < 4; sb++) {
              l[c][lvl][sb] = (d[c][lvl][sb]==0)?0:(32 - __builtin_clz(d[c][lvl][sb]));
              l[c][lvl][sb] = (l[c][lvl][sb]>10)?4*(l[c][lvl][sb] - 10):0;
            }
          }
          l[c][0][0] += (l[c][0][0] == 0)?0:4;
          l[c][0][1] += (l[c][0][1] == 0)?0:2;
          l[c][0][2] += (l[c][0][2] == 0)?0:2;
          l[c][0][3] += 0;
          l[c][1][1] += (l[c][1][1] == 0)?0:4;
          l[c][1][2] += (l[c][1][2] == 0)?0:4;
          l[c][1][3] += (l[c][1][3] == 0)?0:2;
          int L = 0;
          L = max(L, l[c][0][0]);
          L = max(L, l[c][0][1]);
          L = max(L, l[c][0][2]);
          L = max(L, l[c][0][3]);
          L = max(L, l[c][1][1]);
          L = max(L, l[c][1][2]);
          L = max(L, l[c][1][3]);
          samples[L]++;
        }
      }
    }

    printf("--------------------------------------------------------------------------\n");
    printf("     Quantisers needed by slice for job %d\n", job->job);
    printf("--------------------------------------------------------------------------\n");
    printf("  N  |  slices\n");
    for (int i = 0; i < 32; i++) {
      printf("  %2d |  %6u\n",
             i,
             samples[i]);
    }
    printf("--------------------------------------------------------------------------\n");
  }
}

void accumulate_encoded_statistics(CodedSlice<int16_t> *slices, int n_slices) {
  int samples[32];
  memset(samples, 0, sizeof(samples));

  printf("--------------------------------------------------------------------------\n");
  printf("Oversized Slices:\n");
  printf("--------------------------------------------------------------------------\n");
  for (int i = 0; i < n_slices; i++) {
    int l = 0;
    for (int n = 0; n < 64; n++)
      l = max(l, slices[i].wordlengths[0][n]);
    for (int c = 1; c < 3; c++)
      for (int n = 0; n < 64; n++)
        l = max(l, slices[i].wordlengths[c][n]);
    samples[l]++;

    if (l > 18)
      printf("%d, ", i);
  }
  printf("\n");
  printf("--------------------------------------------------------------------------\n");


  printf("--------------------------------------------------------------------------\n");
  printf("     Coded word lengths by slice\n");
  printf("--------------------------------------------------------------------------\n");
  printf("  N  |  slices\n");
  for (int i = 0; i < 32; i++) {
    printf("  %2d |  %6u\n",
           i,
           samples[i]);
  }
  printf("--------------------------------------------------------------------------\n");
}
