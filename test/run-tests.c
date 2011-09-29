/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string.h>

#include "runner.h"
#include "task.h"
#include "uv.h"

/* Actual tests and helpers are defined in test-list.h */
#include "test-list.h"

/* The time in milliseconds after which a single test times out. */
#define TEST_TIMEOUT  5000

static int maybe_run_test(int argc, char **argv);


int main(int argc, char **argv) {
  platform_init(argc, argv);

  switch (argc) {
  case 1: return run_tests(TEST_TIMEOUT, 0);
  case 2: return maybe_run_test(argc, argv);
  case 3: return run_test_part(argv[1], argv[2]);
  default:
    LOGF("Too many arguments.\n");
    return 1;
  }
}


static int ipc_helper() {
  /*
   * This is launched from test-ipc.c. stdin is a duplex channel that we
   * over which a handle will be transmitted. In this initial version only
   * data is transfered over the channel. XXX edit this comment after handle
   * transfer is added.
   */
  uv_pipe_t channel;
  uv_write_t write_req;
  int r;
  uv_buf_t buf;

  r = uv_pipe_init(uv_default_loop(), &channel, 1);
  ASSERT(r == 0);

  uv_pipe_open(&channel, 0);

  buf = uv_buf_init("hello\n", 6);
  r = uv_write(&write_req, (uv_stream_t*)&channel, &buf, 1, NULL);
  ASSERT(r == 0);

  r = uv_run(uv_default_loop());
  ASSERT(r == 0);

  return 0;
}


static int maybe_run_test(int argc, char **argv) {
  if (strcmp(argv[1], "--list") == 0) {
    print_tests(stdout);
    return 0;
  }

  if (strcmp(argv[1], "ipc_helper") == 0) {
    return ipc_helper();
  }

  if (strcmp(argv[1], "spawn_helper1") == 0) {
    return 1;
  }

  if (strcmp(argv[1], "spawn_helper2") == 0) {
    printf("hello world\n");
    return 1;
  }

  if (strcmp(argv[1], "spawn_helper3") == 0) {
    char buffer[256];
    fgets(buffer, sizeof(buffer) - 1, stdin);
    buffer[sizeof(buffer) - 1] = '\0';
    fputs(buffer, stdout);
    return 1;
  }

  if (strcmp(argv[1], "spawn_helper4") == 0) {
    /* Never surrender, never return! */
    while (1) uv_sleep(10000);
  }

  return run_test(argv[1], TEST_TIMEOUT, 0);
}
