/*
 * Copyright (c) 2020, 2022, Red Hat Inc.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef CGROUP_V2_SUBSYSTEM_LINUX_HPP
#define CGROUP_V2_SUBSYSTEM_LINUX_HPP

#include "cgroupSubsystem_linux.hpp"

class CgroupV2Controller: public CgroupController {
  public:
    using CgroupController::CgroupController;
};

class CgroupV2CpuController: public CgroupCpuController {
  private:
    CgroupV2Controller _reader;
    CgroupV2Controller* reader() { return &_reader; }
  public:
    CgroupV2CpuController(const CgroupV2Controller& reader) : _reader(reader) {
    }
    int cpu_quota() override;
    int cpu_period() override;
    int cpu_shares() override;
    bool is_read_only() override {
      return reader()->is_read_only();
    }
    void set_subsystem_path(const char *cgroup_path) { reader()->set_subsystem_path(cgroup_path); }
};

class CgroupV2MemoryController final: public CgroupMemoryController {
  private:
    CgroupV2Controller _reader;
    CgroupV2Controller* reader() { return &_reader; }
  public:
    CgroupV2MemoryController(const CgroupV2Controller& reader) : _reader(reader) {
    }

    jlong read_memory_limit_in_bytes(julong upper_bound) override;
    jlong memory_and_swap_limit_in_bytes(julong host_mem, julong host_swp) override;
    jlong memory_and_swap_usage_in_bytes(julong host_mem, julong host_swp) override;
    jlong memory_soft_limit_in_bytes(julong upper_bound) override;
    jlong memory_usage_in_bytes() override;
    jlong memory_max_usage_in_bytes() override;
    jlong rss_usage_in_bytes() override;
    jlong cache_usage_in_bytes() override;
    void print_version_specific_info(outputStream* st, julong host_mem) override;
    bool is_read_only() override {
      return reader()->is_read_only();
    }
    bool trim_path(size_t dir_count) override { return reader()->trim_path(dir_count); }
    void set_subsystem_path(const char *cgroup_path) { reader()->set_subsystem_path(cgroup_path); }
};

class CgroupV2Subsystem: public CgroupSubsystem {
  private:
    /* One unified controller */
    CgroupV2Controller _unified;
    /* Caching wrappers for cpu/memory metrics */
    CachingCgroupController<CgroupMemoryController>* _memory = nullptr;
    CachingCgroupController<CgroupCpuController>* _cpu = nullptr;

    CgroupV2Controller* unified() { return &_unified; }
    bool trim_path(size_t dir_count) override { return unified()->trim_path(dir_count); }

  public:
    CgroupV2Subsystem(CgroupV2MemoryController* memory,
                      CgroupV2CpuController* cpu,
                      CgroupV2Controller unified) :
        _unified(unified),
        _memory(new CachingCgroupController<CgroupMemoryController>(memory)),
        _cpu(new CachingCgroupController<CgroupCpuController>(cpu)) {
      initialize_hierarchy();
    }

    char * cpu_cpuset_cpus() override;
    char * cpu_cpuset_memory_nodes() override;
    jlong pids_max() override;
    jlong pids_current() override;

    bool is_containerized() override;

    const char * container_type() override {
      return "cgroupv2";
    }
    CachingCgroupController<CgroupMemoryController>* memory_controller() { return _memory; }
    CachingCgroupController<CgroupCpuController>* cpu_controller() { return _cpu; }
};

#endif // CGROUP_V2_SUBSYSTEM_LINUX_HPP
