/*
 * the header is my lab2 perceptronBP's header
 * StuID: 2200012927
 */

#ifndef __CPU_PRED_LAB2_BP_HH__
#define __CPU_PRED_LAB2_BP_HH__

#include <vector>

#include "base/sat_counter.hh"
#include "base/types.hh"
#include "cpu/pred/bpred_unit.hh"
#include "params/Lab2PerceptronBP.hh"

namespace gem5 
{

namespace branch_prediction 
{

class Lab2PerceptronBP : public BPredUnit {

  public:
    Lab2PerceptronBP(const Lab2PerceptronBPParams &params);

    bool lookup(ThreadID tid, Addr branch_addr, void *&bp_history) override;
    void updateHistories(ThreadID tid, Addr pc, bool uncond, bool taken,
                         Addr target, void *&bp_history) override;
    void update(ThreadID tid, Addr pc, bool taken, void *&bp_history,
                bool squashed, const StaticInstPtr &inst, Addr target) override;
    void squash(ThreadID tid, void *&bp_history) override;

  private:
    void updateGHR(ThreadID tid, bool taken);
    inline unsigned getPerceptronIndex(ThreadID tid, Addr & pc);

    struct Perceptron{
      int weight[30] = {};
    }perceptrons[8192];

    // the table of Global History Register
    std::vector<unsigned> ghr;

    // the threshold for perceptron to be confident
    unsigned threshold;

    // the number of weights each perceptron
    unsigned weightNum;

    // the number of perceptrons, it should be a power of 2
    unsigned perceptronNum;
    unsigned perceptronBits;
    unsigned perceptronMask;

    struct History{
      int yield;
      unsigned tempGHR;
    };
};

} // namespace branch_prediction
} // namespace gem5

#endif // __CPU_PRED_LAB2_BP_HH__
