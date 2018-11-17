/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#include <unistd.h>

#include "loop.h"

namespace artik {

GlibLoop* GlibLoop::m_instance = nullptr;
int GlibLoop::m_refcount = 0;
uv_prepare_t GlibLoop::m_prepare_h;
uv_idle_t GlibLoop::m_idle_h;

static void prepare_cb(uv_prepare_t *handle) {
  /*
   * trick to use glib's own poll function and prepare/check/dispatch
   * in libuv's prepare time
   */
  g_main_context_iteration(NULL, FALSE);
}

static void _idle_cb(uv_idle_t *handle) {
  usleep(1);
}

GlibLoop::GlibLoop() {
}

GlibLoop::~GlibLoop() {
}

GlibLoop* GlibLoop::Instance() {
  if (!m_instance)
    m_instance = new GlibLoop();

  return m_instance;
}

void GlibLoop::attach() {
  if (++m_refcount > 1)
    return;

  /*
   * libuv use epoll, and assert if epoll fail (except EEXIST errno).
   * if we pass the regular file to uv_poll's fd, epoll fail with ENOPERM
   * and assert. (becasue regular file not support epoll)
   * so, use glib's own poll function in uv_prepare time.
   */
  uv_prepare_init(uv_default_loop(), &m_prepare_h);
  uv_prepare_start(&m_prepare_h, prepare_cb);

  /*
   * uv_poll use epoll with infinite timeout(-1) if there are no registered
   * event source (fd/timer/idle/...)
   * we do not add glib's poll fd to uv_poll, libuv don't know how many
   * events are registered.
   * so, add fake event source(idle) to libuv.
   */
  uv_idle_init(uv_default_loop(), &m_idle_h);
  uv_idle_start(&m_idle_h, _idle_cb);
}

void GlibLoop::detach() {
  if ((m_refcount == 0) || (--m_refcount > 0))
    return;

  uv_idle_stop(&m_idle_h);
  uv_prepare_stop(&m_prepare_h);
}

}  // namespace artik
