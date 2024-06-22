#include "LabSound/LabSound.h"
#include "LabSound/backends/AudioDevice_RtAudio.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioNodeDescriptor.h"
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/extended/AudioFileReader.h"
#include "LabSound/extended/FunctionNode.h"
#include "PlayheadNode.h"
#include "RtMidi.h"
#include <_types/_uint32_t.h>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string>
#include <termios.h>
#include <unistd.h>

using namespace lab;
// Function to set the terminal to raw mode
void set_raw_mode(termios &original) {
  termios raw = original;
  raw.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Function to restore the terminal to its original mode
void restore_terminal_mode(const termios &original) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

inline std::pair<AudioStreamConfig, AudioStreamConfig>
GetDefaultAudioDeviceConfiguration(const bool with_input = true) {
  // Get devices into
  const std::vector<AudioDeviceInfo> audioDevices =
      lab::AudioDevice_RtAudio::MakeAudioDeviceList();
  AudioDeviceInfo defaultOutputInfo, defaultInputInfo;
  for (auto &info : audioDevices) {
    if (info.is_default_output)
      defaultOutputInfo = info;
    if (info.is_default_input)
      defaultInputInfo = info;
  }

  // Create Output Stream Config with desired defaults
  AudioStreamConfig outputConfig;
  if (defaultOutputInfo.index != -1) {
    outputConfig.device_index = defaultOutputInfo.index;
    outputConfig.desired_channels =
        std::min(uint32_t(2), defaultOutputInfo.num_output_channels);
    outputConfig.desired_samplerate = defaultOutputInfo.nominal_samplerate;
  }

  // Create Input Stream Config with desired defaults
  AudioStreamConfig inputConfig;
  if (with_input) {
    if (defaultInputInfo.index != -1) {
      inputConfig.device_index = defaultInputInfo.index;
      inputConfig.desired_channels =
          std::min(uint32_t(1), defaultInputInfo.num_input_channels);
      inputConfig.desired_samplerate = defaultInputInfo.nominal_samplerate;
    } else {
      throw std::invalid_argument(
          "the default audio input device was requested but none were found");
    }
  }

  // RtAudio doesn't support mismatched input and output rates.
  // this may be a pecularity of RtAudio, but for now, force an RtAudio
  // compatible configuration
  if (defaultOutputInfo.nominal_samplerate !=
      defaultInputInfo.nominal_samplerate) {
    float min_rate = std::min(defaultOutputInfo.nominal_samplerate,
                              defaultInputInfo.nominal_samplerate);
    inputConfig.desired_samplerate = min_rate;
    outputConfig.desired_samplerate = min_rate;
    printf("Warning ~ input and output sample rates don't match, attempting to "
           "set minimum\n");
  }
  return {inputConfig, outputConfig};
}
int main(int, char **) {

  // Create input/output config
  AudioStreamConfig _inputConfig;
  AudioStreamConfig _outputConfig;
  auto config = GetDefaultAudioDeviceConfiguration(true);
  _inputConfig = config.first;
  _outputConfig = config.second;

  // Create audio device using RTAudio Backend
  std::shared_ptr<lab::AudioDevice_RtAudio> device(
      new lab::AudioDevice_RtAudio(_inputConfig, _outputConfig));

  // Create the audio context

  auto context = std::make_shared<lab::AudioContext>(false, true);
  auto metBeatNode = std::make_shared<SampledAudioNode>((*context));
  auto hiHat = MakeBusFromFile(
      "/Users/bcrowe/Documents/Git/daw_try_2/assets/XF_Hat_1.wav", false,
      48000);

  // create the destination node
  auto destinationNode =
      std::make_shared<lab::AudioDestinationNode>(*context.get(), device);
  device->setDestinationNode(destinationNode);
  context->setDestinationNode(destinationNode);
  auto ph = std::shared_ptr<PlayheadNode>(new PlayheadNode(context) );
  context->connect(destinationNode, ph);
  ph->start(0.f);
  context->synchronizeConnections();
  RtMidiIn midiIn;
  std::cout << midiIn.getPortCount() << std::endl;


  std::string msg;

  termios original;

  // Get the current terminal settings
  tcgetattr(STDIN_FILENO, &original);

  // Set the terminal to raw mode
  set_raw_mode(original);

  char c;
  

  while (true) {
    // Read a single character
    if (read(STDIN_FILENO, &c, 1) == -1) {
      perror("read");
      break;
    }

    // Print the ASCII value of the key pressed
    std::cout << "You pressed: " << c << " (ASCII: " << static_cast<int>(c)
              << ")\n";

    // Break the loop if 'q' is pressed
    if (c == 'p'){
      ph->playing = !ph->playing;
    }
    if (c == '='){
      ph->setTempo(ph->getTempo() + 1.0f);
      std::cout << ph->getTempo();
    }
    if (c == '-'){
      ph->setTempo(ph->getTempo() - 1.0f);
      std::cout << ph->getTempo();
    }
    if (c == 'q')
      break;
  }

  // Restore the original terminal settings
  restore_terminal_mode(original);

  std::cout << "Exiting...\n";
}
