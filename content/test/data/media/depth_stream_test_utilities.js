// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function getConstraintsForDevice(deviceLabel) {
  return new Promise(function(resolve, reject) {
    navigator.mediaDevices.enumerateDevices()
    .then(function(devices) {
      for (var i = 0; i < devices.length; ++i) {
        if (deviceLabel == devices[i].label) {
          return resolve({video:{deviceId: {exact: devices[i].deviceId},
                                 width: {exact:96},
                                 height: {exact:96}}
                          });
        }
      }
      return reject("Expected to have a device with label:" + deviceLabel);
    })
  });
}

function getFake16bitStream() {
  return new Promise(function(resolve, reject) {
    getConstraintsForDevice("fake_device_1")
    .then(function(constraints) {
      if (!constraints)
        return reject("No fake device found");
      return navigator.mediaDevices.getUserMedia(constraints);
    }).then(function(stream) {
      return resolve(stream);
    });
  });
}

function getStreamOfVideoKind(constraint_kind) {
  var constraints = {
    video:{
      videoKind: constraint_kind
    }
  }
  return navigator.mediaDevices.getUserMedia(constraints);
}

// Data is RGBA array data and could be used with different formats:
// e.g. Uint8Array, Uint8ClampedArray, Float32Array...
// Value at point (row, column) is calculated as
// (top_left_value + (row + column) * step) % wrap_around. wrap_around is 255
// (for Uint8) or 1.0 for float. See FakeVideoCaptureDevice for details.
function verifyPixels(
    data, width, height, flip_y, step, wrap_around, tolerance, test_name) {
  var rowsColumnsToCheck = [[1, 1],
                            [0, width - 1],
                            [height - 1, 0],
                            [height - 1, width - 1],
                            [height - 3, width - 3]];

  // Same value is expected for all color components.
  if (data[0] != data[1] || data[0] != data[2]) {
    return Promise.reject(test_name + ": values " + data[0] + ", " + data[1] +
                          ", " + data[2] + " differ at top left.");
  }

  // Calculate all reference points based on top left and compare.
  for (var j = 0; j < rowsColumnsToCheck.length; ++j) {
    var row = rowsColumnsToCheck[j][0];
    var column = rowsColumnsToCheck[j][1];
    var i = (width * row + column) * 4;
    if (data[i] != data[i + 1] || data[i] != data[i + 2]) {
      return Promise.reject(test_name + ": values " + data[i] + ", " +
                            data[i + 1] + ", " + data[i + 2] +
                            " differ at index " + i);
    }
    var calculated = (data[0] + wrap_around +
                      step * ((flip_y ? -row : row) + column)) % wrap_around;
    if (Math.abs(calculated - data[i]) > tolerance) {
      return Promise.reject(test_name + ": reference value " + data[i] +
          " differs from calculated: " + calculated +
          " at index (row, column) " + i + " (" + row + ", " + column +
          "). TopLeft value:" + data[0] + ", step:" + step + ", flip_y:" +
          flip_y);
    }
  }
  return true;
}

// Although RED texture is bound to framebuffer, we read RGBA pixels to |data|.
// Value at point (row, column) is calculated as
// (top_left_value + (row + column) * step) % wrap_around. wrap_around is 255
// (for Uint8) or 1.0 for float. See FakeVideoCaptureDevice for details.
function verifyPixelsRed(
    data, width, height, flip_y, step, wrap_around, tolerance, test_name) {
  var rowsColumnsToCheck = [[1, 1],
                            [0, width - 1],
                            [height - 1, 0],
                            [height - 1, width - 1],
                            [height - 3, width - 3]];

  // Calculate all reference points based on top left and compare.
  for (var j = 0; j < rowsColumnsToCheck.length; ++j) {
    var row = rowsColumnsToCheck[j][0];
    var column = rowsColumnsToCheck[j][1];
    var i = (width * row + column) * 4;
    var calculated = (data[0] + wrap_around +
                      step * ((flip_y ? -row : row) + column)) % wrap_around;
    if (Math.abs(calculated - data[i]) > tolerance) {
      return Promise.reject(test_name + ": reference value " + data[i] +
          " differs from calculated: " + calculated +
          " at index (row, column) " + i + " (" + row + ", " + column +
          "). TopLeft value:" + data[0] + ", step:" + step + ", flip_y:" +
          flip_y);
    }
  }
  return true;
}