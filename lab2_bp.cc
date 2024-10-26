/*
 * this is the lab2_bp.cc for lab2 work
 * StuID: 2200012927
 */

#include "cpu/pred/lab2_bp.hh"

#include "base/intmath.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "base/bitfield.hh"

namespace gem5 
{

namespace branch_prediction 
{

Lab2PerceptronBP::Lab2PerceptronBP(const Lab2PerceptronBPParams &params)
    : BPredUnit(params),
      ghr(params.numThreads, 0),
      threshold(params.threshold),
      weightNum(params.weightNum),
      perceptronNum(params.perceptronNum),
      perceptronBits(ceilLog2(perceptronNum)),
      perceptronMask(perceptronNum - 1)
{
  if (!isPowerOf2(perceptronNum)) {
    fatal("Invalid perceptron number!\n");
  }

}

bool 
Lab2PerceptronBP::lookup(ThreadID tid, Addr branch_addr, void *&bp_history) {
  unsigned pInd = getPerceptronIndex(tid, branch_addr);

  unsigned tempGHR = ghr[tid];
  int yield = perceptrons[pInd].weight[0];
  for (int i = 1; i < weightNum; i++) {
    int x_i = tempGHR & 1 ? 1 : -1;
    tempGHR >>= 1;
    yield += perceptrons[pInd].weight[i] * x_i;
  }

  History *history = new History;
  history->yield = yield;
  history->tempGHR = ghr[tid];

  bp_history = static_cast<void*>(history);

  if (yield >= 0) return true;
  else return false;
}

void 
Lab2PerceptronBP::updateHistories(ThreadID tid, Addr pc, bool uncond,
                                       bool taken, Addr target,
                                       void *&bp_history) {
  assert(uncond || bp_history);
  if (uncond) {
    History *history = new History;
    history->tempGHR = ghr[tid];
    history->yield = taken ? 1 : -1;
    bp_history = static_cast<History*>(history);
  }
  updateGHR(tid, taken);
}

void 
Lab2PerceptronBP::update(ThreadID tid, Addr branch_addr, bool taken,
                              void *&bp_history, bool squashed,
                              const StaticInstPtr &inst, Addr target) {
                              
  History *history = static_cast<History*>(bp_history);
  
  if (squashed) {
    ghr[tid] = (history->tempGHR << 1) | taken;
    return;
  }

  unsigned pInd = getPerceptronIndex(tid, branch_addr);
  unsigned rectGHR = history->tempGHR << 1 | taken;
  if ((history->yield >= 0) != taken || abs(history->yield) <= threshold)
    for (int i = 0; i < weightNum; i++){
      bool check = rectGHR & 1;
      rectGHR >>= 1;
      check = (check == taken);
      perceptrons[pInd].weight[i] += check ? 1 : -1;
    }

  delete history;
  bp_history = nullptr;
}                              

void 
Lab2PerceptronBP::squash(ThreadID tid, void *&bp_history) {
  History *history = static_cast<History*> (bp_history);
  ghr[tid] = history->tempGHR;
  
  delete history;
  bp_history = nullptr;
}

void 
Lab2PerceptronBP::updateGHR(ThreadID tid, bool taken){
  ghr[tid] = taken ? (ghr[tid] << 1) | 1 : ghr[tid] << 1;
}

inline
unsigned
Lab2PerceptronBP::getPerceptronIndex(ThreadID tid, Addr &branch_addr)
{
  return ((branch_addr >> instShiftAmt) ^ ghr[tid]) & perceptronMask;
}

} // namespace branch_prediction
} // namespace gem5
