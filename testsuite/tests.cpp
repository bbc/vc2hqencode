/*****************************************************************************
 * tests.cpp : Test Everything
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

#include <cstdio>

static bool HAS_SSE4_2 = false;
static bool HAS_AVX    = false;
static bool HAS_AVX2   = false;

int test_transforms(bool HAS_SSE4_2, bool HAS_AVX, bool HAS_AVX2);

void detect_cpu_features() {
  __builtin_cpu_init();

  HAS_SSE4_2 = __builtin_cpu_supports("sse4.2");
  HAS_AVX    = __builtin_cpu_supports("avx");
  HAS_AVX2   = __builtin_cpu_supports("avx2");

  printf("\n");
  printf("Processor Features:\n");
  if (HAS_SSE4_2)
    printf("  SSE4.2 [X]\n");
  else
    printf("  SSE4.2 [ ]\n");

  if (HAS_AVX)
    printf("  AVX    [X]\n");
  else
    printf("  AVX    [ ]\n");

  if (HAS_AVX2)
    printf("  AVX2   [X]\n");
  else
    printf("  AVX2   [ ]\n");
  printf("\n");
}

int main() {
  int r=0;

  printf("--------------------------------------------------------------------------------\n");
  printf("  VC2HQEncode Test Suite\n");
  printf("--------------------------------------------------------------------------------\n");

  detect_cpu_features();

  r = test_transforms(HAS_SSE4_2, HAS_AVX, HAS_AVX2);
  if (r) return r;

  return r;
}
