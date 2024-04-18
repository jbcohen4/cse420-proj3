
#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_LRU_IPV_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_LRU_IPV_HH__

#include <cstdint>
#include <memory>
#include <vector>

#include "mem/cache/replacement_policies/base.hh"

struct LRUIPVRPParams; // when you find what the actual name is, make sure to do a cmd + f search in both files for this wrong thing

namespace ReplacementPolicy {

class LRUIPVRP : public Base
{
  private:
    // our version of 'PLRUTree' is the recency list, which we just use a std::vector<int>
    int numWays;
    std::vector<int> promotion_vector;
    uint64_t count; // used when instantiating entries to keep track of how many blocks in the current set we've allocated
    std::vector<int>* recency_list_instance; // holds the latest refrence created by instantiateEntry()
  protected:
    struct LRUIPVRP_Repl_Data : ReplacementData{ // Our version of 'TreePLRUReplData'
        // what goes in here?
        // I think this is where I put info that should be associated with each individual cache block
        const uint64_t index;
        std::shared_ptr<std::vector<int>> set_recency_list; // the recency list that corresponds to this cache set
        LRUIPVRP_Repl_Data(uint64_t index, std::shared_ptr<std::vector<int>> set_recency_list);
    };
  public:
    typedef LRUIPVRPParams Params;
    LRUIPVRP(const Params &p);
    ~LRUIPVRP() = default;

    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const override;
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const override;
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const override;
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const override;
    std::shared_ptr<ReplacementData> instantiateEntry() override;
};

} // namespace ReplacementPolicy

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_LRU_IPV_HH__
