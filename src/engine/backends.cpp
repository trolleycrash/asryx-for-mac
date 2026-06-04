#include "constants/constants.hpp"
#include "engine/engine.hpp"
#include "platform/process.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace engine {

pid_t start_recording(const std::string& wav_path, const std::string& err_path)
{
  std::vector<std::string> args;
#ifdef __APPLE__
  if (platform::command_exists("rec")) {
    args = {"rec", "-q", "-r", "16000", "-c", "1", "-b", "16", "-e", "signed-integer", wav_path};
  }
  else if (platform::command_exists("ffmpeg")) {
    args = {"ffmpeg", "-y",  "-f",    "avfoundation", "-i",  ":0",    "-ac",
            "1",      "-ar", "16000", "-sample_fmt",  "s16", wav_path};
  }
  else {
    throw std::runtime_error("No recorder tool found (need sox or ffmpeg: brew install sox)");
  }
#else
  if (platform::command_exists("pw-record")) {
    args = {"pw-record", "--format=s16", "--rate=16000", "--channels=1", wav_path};
  }
  else if (platform::command_exists("arecord")) {
    args = {"arecord", "-q", "-t", "wav", "-f", "S16_LE", "-c", "1", "-r", "16000", wav_path};
  }
  else {
    throw std::runtime_error("No recorder tool found (need pw-record or arecord)");
  }
#endif

  pid_t pid = platform::spawn_process_background(args, err_path);
  if (pid == -1) {
    throw std::runtime_error("Failed to start recorder process");
  }

  return pid;
}

bool copy_to_clipboard(const std::string& text)
{
#ifdef __APPLE__
  if (platform::command_exists("pbcopy")) {
    return platform::run_process_with_stdin({"pbcopy"}, text);
  }
  std::cerr << "Warning: pbcopy not found.\n";
  return false;
#else
  if (platform::command_exists("wl-copy")) {
    return platform::run_process_with_stdin({"wl-copy"}, text);
  }

  if (platform::command_exists("xclip")) {
    return platform::run_process_with_stdin({"xclip", "-selection", "clipboard"}, text);
  }

  std::cerr << "Warning: Neither wl-copy nor xclip is available to copy transcript.\n";
  return false;
#endif
}

bool send_notification(const std::string& message)
{
#ifdef __APPLE__
  return platform::run_process_blocking(
      {"osascript", "-e", "on run argv", "-e",
       "display notification (item 1 of argv) with title \"asryx\"", "-e", "end run", "--",
       message});
#else
  if (platform::command_exists("notify-send")) {
    return platform::run_process_blocking(
        {"notify-send", std::string(constants::app_name), message});
  }

  return false;
#endif
}

} // namespace engine
