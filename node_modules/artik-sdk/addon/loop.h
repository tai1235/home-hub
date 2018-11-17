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

#ifndef ADDON_LOOP_H_
#define ADDON_LOOP_H_

#include <glib.h>
#include <uv.h>

namespace artik {

class GlibLoop {
  public:
    static GlibLoop* Instance();
    void attach();
    void detach();
  private:
    GlibLoop();
    ~GlibLoop();

    static GlibLoop* m_instance;
    static int m_refcount;
    static uv_prepare_t m_prepare_h;
    static uv_idle_t m_idle_h;
};

}  // namespace artik

#endif  // ADDON_LOOP_H_
