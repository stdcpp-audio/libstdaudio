// libstdaudio
// Copyright (c) 2018 - Timur Doumler

#pragma once
#include <forward_list>
#include "device.h"

LIBSTDAUDIO_NAMESPACE_BEGIN

class device_list;

/** Returns the current default audio input device. */
device get_input_device();

/** Returns the current default audio output device. */
device get_output_device();

/** Returns the program-wide list of currently available audio input devices. */
device_list& get_input_device_list();

/** Returns the program-wide list of currently available audio output devices. */
device_list& get_output_device_list();

/** A singleton list of audio input and output devices.
 *  Users cannot construct a device_list and can only obtain access to one via the
 *  audio::get_input_device_list and audio::get_output_device_list free functions.
 */
class device_list {
private:
  using _underlying_container = std::forward_list<device>;

public:
  /** A forward iterator whose value type is audio::device. */
  using iterator = _underlying_container::iterator;

  /** Returns an iterator referring to the first audio::device in the list. */
  iterator begin() noexcept { return _dlist.begin(); }

  /** Returns the past-the-end iterator of the device list. Attempting to dereference
   *  this iterator results in undefined behavior.
   */
  iterator end() noexcept { return _dlist.end(); }

private:
  device_list(_underlying_container&& dlist) : _dlist(std::move(dlist)) {}
  friend device_list& get_input_device_list();
  friend device_list& get_output_device_list();
  _underlying_container _dlist;
};

LIBSTDAUDIO_NAMESPACE_END