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
    printf("In constructor.\n");
    this->promotion_vector = {0, 0, 1, 0, 3, 0, 1, 2, 1, 0, 5, 1, 0, 0, 1, 11, 13};
    assert(this->promotion_vector.size() == 17);
    this->recency_list_instance = nullptr;
    this->set_index = -1;
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
    printf("In invalidate.\n");
    // I don't think the paper actually says what we're supposed to do here. 
    // I'm going to guess that I put the block we're invalidating in the LRU position and move everything behind it up
    std::shared_ptr<LRUIPVRP_Repl_Data> block_data =
        std::static_pointer_cast<LRUIPVRP_Repl_Data>(replacement_data);
    std::vector<int>* recency_stack = block_data->set_recency_list.get();
    printf("    old shared state: ");
    for(int i = 0; i < recency_stack->size(); i++){
        printf("%d ", recency_stack->at(i));
    }
    int old_recency_idx = index_of(block_data->index, recency_stack);
    assert(recency_stack->size() == 16); // we'll move this guy into position 15
    for(int i = old_recency_idx; i < recency_stack->size(); i++){
        recency_stack->at(i) = recency_stack->at(i + 1);
    }
    recency_stack->at(15) = block_data->index;
    printf("    new shared state: ");
    for(int i = 0; i < recency_stack->size(); i++){
        printf("%d ", recency_stack->at(i));
    }
    printf("\n");
    // pass
}

void LRUIPVRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const {
    // I think touch is where you read the block and/or write to it
    printf("In touch.\n");
    printf("    promotion vector: ");
    for(int i = 0; i < this->promotion_vector.size(); i++){
        printf("%d ", this->promotion_vector[i]);
    }
    printf("\n");
    std::shared_ptr<LRUIPVRP_Repl_Data> block_data = 
        std::static_pointer_cast<LRUIPVRP_Repl_Data>(replacement_data);
    std::vector<int>* recency_list = block_data->set_recency_list.get();
    printf("    old shared state: ");
    for(int i = 0; i < recency_list->size(); i++){
        printf("%d ", recency_list->at(i));
    }
    
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
    printf("    new shared state: ");
    for(int i = 0; i < recency_list->size(); i++){
        printf("%d ", recency_list->at(i));
    }
    printf("\n");
    // pass
}
void LRUIPVRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const {
    printf("In reset.\n");
    
    // I think reset means the block was invalidated, but now we are going to insert something into it
    // JK, I think that's totally wrong. I have no idea what the fuck this thing is supposed to do
    // maybe it's a touch? It can't be a touch cuz it makes the shared state look fucking tiny
    std::shared_ptr<LRUIPVRP_Repl_Data> block_data =
        std::static_pointer_cast<LRUIPVRP_Repl_Data>(replacement_data);
    printf("    SetID: %ld, index: %ld\n", block_data->set_index, block_data->index);
    std::vector<int>* recency_stack = block_data->set_recency_list.get();
    printf("    old SharedState: ");
    for(int i = 0; i < recency_stack->size(); i++){
        printf("%d ", recency_stack->at(i));
    }

    int block_index = block_data->index;
    int old_recency_idx = index_of(block_index, recency_stack); // the index of the block index in the recency stack
    int insertion_recency_idx = this->promotion_vector[16];
    assert(old_recency_idx > insertion_recency_idx);
    for(int i = old_recency_idx; i > insertion_recency_idx; i--){
        recency_stack->at(i) = recency_stack->at(i - 1);
    }
    recency_stack->at(insertion_recency_idx) = block_index;
    printf("    new sharedState: ");
    for(int i = 0; i < recency_stack->size(); i++){
        printf("%d ", recency_stack->at(i));
    }
    printf("\n");
    // pass
}


ReplaceableEntry* LRUIPVRP::getVictim(const ReplacementCandidates& candidates) const {
    
    // assert(candidates.size() == 16);
    // might want to print candidates size
    const LRUIPVRP_Repl_Data* block_data = std::static_pointer_cast<LRUIPVRP_Repl_Data>(candidates[0]->replacementData).get();
    printf("In getVictim. SetID: %ld\n", block_data->set_index);
    printf("    candidate indicies: ");
    for(int i = 0; i < candidates.size(); i++){
        const LRUIPVRP_Repl_Data* curr = std::static_pointer_cast<LRUIPVRP_Repl_Data>(candidates[i]->replacementData).get();
        printf("%ld ", curr->index);
    }
    printf("\n");
    std::vector<int> *recency_list = 
        std::static_pointer_cast<LRUIPVRP_Repl_Data>(candidates[0]->replacementData)->set_recency_list.get();
    printf("    shared state: ");
    for(int i = 0; i < recency_list->size(); i++){
        printf("%d ", recency_list->at(i));
    }
    int index_to_be_victimed = recency_list->at(15); // the last item in the list
    printf("    Victim: %d\n", index_to_be_victimed);
    return candidates[index_to_be_victimed];
    // pass
}


LRUIPVRP::LRUIPVRP_Repl_Data::LRUIPVRP_Repl_Data(const uint64_t index, const int64_t set_index, std::shared_ptr<std::vector<int>> lst)
: index(index), set_recency_list(lst), set_index(set_index) {}

std::shared_ptr<ReplacementData> LRUIPVRP::instantiateEntry() { // is this right? Do I need this function at all?
    uint64_t index = this->count % this->numWays;
    if(index == 0){
        this->recency_list_instance = new std::vector<int>(this->numWays); // initialize a new vector with size numWays and all initialized to 0
        this->set_index += 1;
        for(int i = 0; i < this->numWays; i++){
            this->recency_list_instance->at(i) = i;
        }
    }
    printf("In instantiate entry. Count: %ld, set index: %ld, block index: %ld\n", this->count, this->set_index, index);
    LRUIPVRP_Repl_Data* cache_block_data = new LRUIPVRP_Repl_Data(
        index, 
        set_index,
        std::shared_ptr<std::vector<int>>(this->recency_list_instance)
    );
    this->count++;
    return std::shared_ptr<ReplacementData>(cache_block_data);
}

void LRUIPVRP::print_recency_list(const std::vector<int>* lst){
    if(lst->size() == 0){
        printf("[]\n");
    } else {
        printf("[");
        for(int i = 0; i < lst->size() - 1; i++){
            printf("%d, ", lst->at(i));
        }
        printf("%d]\n", lst->at(lst->size() - 1));
    }

}
}


