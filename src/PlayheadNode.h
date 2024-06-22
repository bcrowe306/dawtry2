#ifndef PLAYHEADNODE_H
#define FUNCTION_NODE_H
#include "LabSound/core/AudioBus.h"
#include "LabSound/core/AudioContext.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/SampledAudioNode.h"
#include "LabSound/extended/AudioContextLock.h"
#include "LabSound/extended/AudioFileReader.h"
#include "LabSound/extended/FunctionNode.h"
#include <memory>
using namespace lab;

class PlayheadNode : public FunctionNode {
public:
  std::shared_ptr<AudioContext> ac;
  int tpqn = 480;
  int counter = 0;
  int ticks = 0;
  int samplesPerTick;
  bool enabled = true;
  bool playing = true;
  unsigned int beatsPerBar = 4;
  unsigned int beatValue = 4;
  std::shared_ptr<SampledAudioNode> metronomeBeat;
  std::shared_ptr<AudioBus> hiHat;
  std::shared_ptr<AudioBus> rim;
  std::shared_ptr<SampledAudioNode> metronomeDownBeat;
  PlayheadNode(std::shared_ptr<AudioContext> ac) : FunctionNode( (*ac) ) {
    this->ac = ac;
    _setSamplesPerTick();
    metronomeBeat = std::make_shared<SampledAudioNode>((*ac));
    
    metronomeDownBeat = std::make_shared<SampledAudioNode>((*ac));


    hiHat = MakeBusFromFile(
        "/Users/bcrowe/Documents/Git/daw_try_2/assets/XF_Hat_1.wav", false,
        ac->sampleRate());
    rim = MakeBusFromFile(
        "/Users/bcrowe/Documents/Git/daw_try_2/assets/XF_Rim_11.wav", false,
        ac->sampleRate());
    metronomeBeat->setBus(hiHat);

    metronomeDownBeat->setBus(rim);
    ac->connect(ac->destinationNode(), metronomeBeat);
    ac->connect(ac->destinationNode(), metronomeDownBeat);
    this->setFunction(callback);
  };
  ~PlayheadNode(){};
  void onMetronomeBeat(bool isDownBeat){
    if(enabled){
      if (isDownBeat)
        metronomeDownBeat->schedule(0.f);
      else
        metronomeBeat->schedule(0.f);

    }
  };
  void setTempo(float tempo) {
    _tempo = tempo;
    this->_setSamplesPerTick();
    
  };
  float getTempo(){
    return _tempo;
  };
  bool isMod(int numerator, int denomenator) {
    if (numerator < denomenator) {
      return numerator == 0;
    }
    return numerator % denomenator == 0;
  };
  void setTimeSignature(){};
  static void callback(ContextRenderLock &r, FunctionNode *me, int channel,
                       float *buffer, int bufferSize) {
    auto playHeadNode = (PlayheadNode *)me;
    
    for (size_t i = 0; i < bufferSize; i++) {
      //  istick
      if( playHeadNode->isMod(playHeadNode->counter, (int)playHeadNode->samplesPerTick) ){

        // isMetronomebeat
        if (playHeadNode->isMod(playHeadNode->ticks, playHeadNode->tpqn) && playHeadNode->playing) {
          bool isDownBeat = playHeadNode->isMod(playHeadNode->ticks, playHeadNode->tpqn * playHeadNode->beatsPerBar);
          playHeadNode->onMetronomeBeat(isDownBeat);
        }

        // Only tick if playing is true
        if (playHeadNode->playing)
          playHeadNode->ticks++;
        else{
          playHeadNode->ticks = 0;
        }
      }
      playHeadNode->counter++;
    }
  }

private:
  void _setSamplesPerTick(){
    samplesPerTick = (int)(60.f / _tempo / tpqn * ac->sampleRate());
  };
  float _tempo = 120;
};

#endif // !PLAYHEADNODE_H
