#include "mem/cache/replacement_policies/lru_ipv.hh"

#include <cmath>

#include "base/intmath.hh"
#include "base/logging.hh"
#include "params/LRUIPVRP.hh" // this uses the wrong name for the params struct
#include "stdio.h" // want to have printf



namespace ReplacementPolicy {

LRUIPVRP::LRUIPVRP(const Params &p) 
: Base(p), numWays(p.numWays), count(0)
{
    printf("hello from ohio\n");
    this->promotion_vector = {0, 1, 0, 3, 0, 1, 2, 1, 0, 5, 1, 0, 0, 1, 11, 13};
    assert(this->promotion_vector.size() == 17);
    // maybe print something in here?
    this->recency_list_instance = nullptr;
    // pass
}

static int index_of(int target, const std::vector<int> *vec){
    for(int i = 0; i < vec->size(); i++){
        if(vec->at(i) == target){
            return i;
        }
    }
    assert(false);
}

void LRUIPVRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data) const {
    printf("hello from ohio\n");
    assert(this->promotion_vector.size() == 17);
    // I don't think the paper actually says what we're supposed to do here. 
    // I'm going to guess that I put the block we're invalidating in the LRU position and move everything behind it up
    std::shared_ptr<LRUIPVRP_Repl_Data> block_data =
        std::static_pointer_cast<LRUIPVRP_Repl_Data>(replacement_data);
    std::vector<int>* recency_stack = block_data->set_recency_list.get();
    int old_recency_idx = index_of(block_data->index, recency_stack);
    assert(recency_stack->size() == 16); // we'll move this guy into position 15
    for(int i = old_recency_idx; i < recency_stack->size(); i++){
        recency_stack->at(i) = recency_stack->at(i + 1);
    }
    recency_stack->at(15) = block_data->index;
    // pass
}

void LRUIPVRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const {
    printf("hello from ohio\n");
    assert(this->promotion_vector.size() == 17);
    // I think touch means that the block is being read (or written)
    // We want to follow the Insertion-Promotion-Vector on how to do this
    // Step 0: cast the pointer
    std::shared_ptr<LRUIPVRP_Repl_Data> block_data = 
        std::static_pointer_cast<LRUIPVRP_Repl_Data>(replacement_data);
    // Step 1: get the recency stack
    std::vector<int>* recency_list = block_data->set_recency_list.get();
    // might want to print these guys
    uint64_t block_index = block_data->index;
    int idx_in_recency_stack = -1; 
    for(int i = 0; i < recency_list->size(); i++){
        if(recency_list->at(i) == block_index){
            idx_in_recency_stack = i;
            break;
        }
    }
    assert(idx_in_recency_stack != -1);
    int new_idx = this->promotion_vector[idx_in_recency_stack];
    assert(new_idx <= idx_in_recency_stack);
    for(int i = idx_in_recency_stack; i > new_idx; i--){
        recency_list->at(i) = recency_list->at(i - 1);
    }
    recency_list->at(new_idx) = block_index;
    // pass
}
void LRUIPVRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const {
    printf("hello from ohio\n");
    assert(this->promotion_vector.size() == 17);
    // I think reset means the block was invalidated, but now we are going to insert something into it
    std::shared_ptr<LRUIPVRP_Repl_Data> block_data =
        std::static_pointer_cast<LRUIPVRP_Repl_Data>(replacement_data);
    std::vector<int>* recency_stack = block_data->set_recency_list.get();
    int block_index = block_data->index;
    int old_recency_idx = -1;
    for(int i = 0; i < recency_stack->size(); i++){
        if(recency_stack->at(i) == block_index){
            old_recency_idx = i;
            break;
        }
    }
    assert(old_recency_idx != -1);
    int insertion_recency_idx = this->promotion_vector[16];
    for(int i = old_recency_idx; i > insertion_recency_idx; i--){
        recency_stack->at(i) = recency_stack->at(i - 1);
    }
    recency_stack->at(insertion_recency_idx) = block_index;
    // pass
}
ReplaceableEntry* LRUIPVRP::getVictim(const ReplacementCandidates& candidates) const {
    printf("hello from ohio\n");
    assert(candidates.size() == 16);
    // might want to print candidates size
    std::vector<int> *recency_list = 
        std::static_pointer_cast<LRUIPVRP_Repl_Data>(candidates[0]->replacementData)->set_recency_list.get();
    assert(recency_list->size() == 16);
    int index_to_be_victimed = recency_list->at(15); // the last item in the list
    return candidates[index_to_be_victimed];
    // pass
}


LRUIPVRP::LRUIPVRP_Repl_Data::LRUIPVRP_Repl_Data(const uint64_t index, std::shared_ptr<std::vector<int>> lst)
: index(index), set_recency_list(lst) {}

std::shared_ptr<ReplacementData> LRUIPVRP::instantiateEntry() { // is this right? Do I need this function at all?
    printf("hello from ohio\n");
    uint64_t index = this->count % this->numWays;
    if(index == 0){
        this->recency_list_instance = new std::vector<int>(this->numWays); // initialize a new vector with size numWays and all initialized to 0
        for(int i = 0; i < this->numWays; i++){
            this->recency_list_instance->at(i) = i;
        }
    }
    LRUIPVRP_Repl_Data* cache_block_data = new LRUIPVRP_Repl_Data(
        index, 
        std::shared_ptr<std::vector<int>>(this->recency_list_instance)
    );
    this->count++;
    return std::shared_ptr<ReplacementData>(cache_block_data);
}
}


