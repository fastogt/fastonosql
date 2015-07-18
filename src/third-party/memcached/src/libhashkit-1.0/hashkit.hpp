/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  HashKit library
 *
 *  Copyright (C) 2011-2012 Data Differential, http://datadifferential.com/
 *  Copyright (C) 2006-2010 Brian Aker All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *      * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above
 *  copyright notice, this list of conditions and the following disclaimer
 *  in the documentation and/or other materials provided with the
 *  distribution.
 *
 *      * The names of its contributors may not be used to endorse or
 *  promote products derived from this software without specific prior
 *  written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#pragma once

#include <libhashkit-1.0/hashkit.h>
#include <string>

class Hashkit {

public:

  Hashkit()
  {
    hashkit_create(&self);
  }

  Hashkit(const Hashkit& source)
  {
    hashkit_clone(&self, &source.self);
  }

  Hashkit& operator=(const Hashkit& source)
  {
    hashkit_free(&self);
    hashkit_clone(&self, &source.self);

    return *this;
  }

  friend bool operator==(const Hashkit &left, const Hashkit &right)
  {
    return hashkit_compare(&left.self, &right.self);
  }

  uint32_t digest(std::string& str)
  {
    return hashkit_digest(&self, str.c_str(), str.length());
  }

  uint32_t digest(const char *key, size_t key_length)
  {
    return hashkit_digest(&self, key, key_length);
  }

  hashkit_return_t set_function(hashkit_hash_algorithm_t hash_algorithm)
  {
    return hashkit_set_function(&self, hash_algorithm);
  }

  hashkit_return_t set_distribution_function(hashkit_hash_algorithm_t hash_algorithm)
  {
    return hashkit_set_function(&self, hash_algorithm);
  }

  ~Hashkit()
  {
    hashkit_free(&self);
  }
private:

  hashkit_st self;
};
