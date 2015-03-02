// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_util.h"

#import <Foundation/Foundation.h>
#include <copyfile.h>

#include "base/basictypes.h"
#include "base/files/file_path.h"
#include "base/mac/foundation_util.h"
#include "base/strings/string_util.h"
#include "base/threading/thread_restrictions.h"

namespace base {

bool CopyFile(const FilePath& from_path, const FilePath& to_path) {
  ThreadRestrictions::AssertIOAllowed();
  if (from_path.ReferencesParent() || to_path.ReferencesParent())
    return false;
  return (copyfile(from_path.value().c_str(),
                   to_path.value().c_str(), NULL, COPYFILE_DATA) == 0);
}

bool GetTempDir(base::FilePath* path) {
#ifndef MAC_APP_STORE
  NSString* tmp = NSTemporaryDirectory();
  if (tmp == nil)
    return false;
  *path = base::mac::NSStringToFilePath(tmp);
  return true;
#else // MAC_APP_STORE
  // Chromium uses temp directories to store a lot of UNIX socket files.
  // In sandboxed applications socket files can only be used if they are
  // inside of app container -
  // https://stackoverflow.com/questions/17753222/sandboxd-deny-network-bind-error-message-when-binds-socket
  // That is why it is easier to put the whole tmp dir on mac
  // in app container rather than fix it all over the places.
  base::FilePath cachesDir;
  if (!GetCachesDir(&cachesDir)) {
    return false;
  }
  cachesDir = cachesDir.Append("tmp");
  if (!DirectoryExists(cachesDir)) {
    if (mkdir(cachesDir.value().c_str(), 0700) != 0) {
      return false;
    }
  }
  *path = cachesDir;
  return true;
#endif
}

bool GetCachesDir(base::FilePath* path) {
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
  NSString *cachesDirectory = [paths objectAtIndex:0];
  if (cachesDirectory == nil)
    return false;
  *path = base::mac::NSStringToFilePath(cachesDirectory);
  return true;
}

FilePath GetHomeDir() {
  NSString* tmp = NSHomeDirectory();
  if (tmp != nil) {
    FilePath mac_home_dir = base::mac::NSStringToFilePath(tmp);
    if (!mac_home_dir.empty())
      return mac_home_dir;
  }

  // Fall back on temp dir if no home directory is defined.
  FilePath rv;
  if (GetTempDir(&rv))
    return rv;

  // Last resort.
  return FilePath("/tmp");
}

}  // namespace base
