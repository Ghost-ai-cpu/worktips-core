# Copyright (c) 2014-2018, The Monero Project
# 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification, are
# permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this list of
#    conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
#    of conditions and the following disclaimer in the documentation and/or other
#    materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its contributors may be
#    used to endorse or promote products derived from this software without specific
#    prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
# THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers


# We always compile if we are building statically to reduce static dependency issues...
# ...except for FreeBSD, because FreeBSD is a special case that doesn't play well with
# others.

if(NOT STATIC AND NOT BUILD_STATIC_DEPS)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(MINIUPNPC miniupnpc>=2.1)
  pkg_check_modules(WORKTIPSMQ libworktipsmq>=1.1.4)
endif()

if(MINIUPNPC_FOUND)
  message(STATUS "Found miniupnpc")
  link_dep_libs(miniupnpc INTERFACE "${MINIUPNPC_LIBRARY_DIRS}" ${MINIUPNPC_LIBRARIES})
  target_include_directories(miniupnpc INTERFACE ${MINIUPNPC_INCLUDE_DIRS})
else()
  message(STATUS "Using in-tree miniupnpc")
  add_subdirectory(miniupnp/miniupnpc)
  if(MSVC)
    target_compile_options(libminiupnpc-static PRIVATE -wd4244 -wd4267)
  else()
    target_compile_options(libminiupnpc-static PRIVATE -Wno-undef -Wno-unused-result -Wno-unused-value)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES "NetBSD")
    target_compile_definitions(libminiupnpc-static PRIVATE _NETBSD_SOURCE)
  endif()

  target_link_libraries(miniupnpc INTERFACE libminiupnpc-static)
endif()


if(NOT WORKTIPSMQ_FOUND)
  message(STATUS "Using in-tree worktipsmq")
  if (DEPENDS)
    # The depends system builds a static libzmq so set up a target worktips-mq can use to avoid building
    add_library(libzmq STATIC IMPORTED GLOBAL)
    set_target_properties(libzmq PROPERTIES IMPORTED_LOCATION ${ZMQ_LIBRARIES})
    target_link_libraries(libzmq INTERFACE sodium)
    target_include_directories(libzmq INTERFACE ${ZMQ_INCLUDE_DIRS})
    message(STATUS "${ZMQ_INCLUDE_DIRS} ${ZMQ_LIBRARIES}")
  endif()
  add_subdirectory(worktips-mq)
else()
  add_library(worktipsmq INTERFACE)
  link_dep_libs(worktipsmq INTERFACE "${WORKTIPSMQ_LIBRARY_DIRS}" ${WORKTIPSMQ_LIBRARIES})
  target_include_directories(worktipsmq INTERFACE ${WORKTIPSMQ_INCLUDE_DIRS})
  add_library(worktipsmq::worktipsmq ALIAS worktipsmq)
  message(STATUS "Found libworktipsmq ${WORKTIPSMQ_VERSION}")

  # TODO: Temporary fix for also linking libzmq; this can go when the direct zmq code (for zmq
  # server) gets replaced with worktipsmq.
  pkg_check_modules(LIBZMQ libzmq)
  if (LIBZMQ_FOUND)
    link_dep_libs(worktipsmq INTERFACE "${LIBZMQ_LIBRARY_DIRS}" ${LIBZMQ_LIBRARIES})
    target_include_directories(worktipsmq INTERFACE ${LIBZMQ_INCLUDE_DIRS})
  endif()
endif()

add_subdirectory(db_drivers)
add_subdirectory(easylogging++)
add_subdirectory(randomx EXCLUDE_FROM_ALL)
