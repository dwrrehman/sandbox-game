//
//  space.cpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#include "space.hpp"
#include "parameters.hpp"


size_t count_alive(std::vector<size_t> neighbors) {
    size_t count = 0;
    for (auto unit : neighbors) if (unit) count++;
    return count;
} 

int are_equal(std::vector<size_t> des, std::vector<size_t> ns) {
    if (ns.size() != des.size()) return 0;
    for (int i = 0; i < ns.size(); i++) {
        if (des[i] != ns[i]) return 0;
    }
    return 1;
}

//space last_output {M};

bool is_special_case(std::vector<size_t> ns) {
    std::vector<size_t> rule1 = {0, 0, 0, 0, 0, 0, 1};
    std::vector<size_t> rule2 = {0, 0, 0, 0, 1, 1, 1};
    std::vector<size_t> rule3 = {0, 0, 1, 1, 1, 1, 1};
    return are_equal(rule1, ns) || are_equal(rule2, ns) || are_equal(rule3, ns);
}

void evolve(space& current, space& output) {  ///// 3d version of rule 110,
    auto& cur = current.S; 
    auto copy = cur;
    
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            for (int z = 0; z < size; z++) {
                
                std::vector<size_t> neighbors = {};
                for (int i = -1; i <= 1; i++)
                    for (int j = -1; j <= 1; j++)
                        for (int k = -1; k <= 1; k++)
                            if ((not i and j and not k) or 
                                (not i and not j and k) or
                                (i and not j and not k) or
                                (not i and not j and not k))
                                    neighbors.push_back(cur[(x + i + size) % size] [(y + j + size) % size] [(z + k + size) % size]);                                                
                
                copy[x][y][z] = count_alive(neighbors) and count_alive(neighbors) < 7 and not is_special_case(neighbors); 
            }
        }
    }
//    
//    space output_copy = output;
//    space output1 {M};
//    for (int x = 0; x < M; x++) {
//        for (int y = 0; y < M; y++) {
//            for (int z = 0; z < M; z++) {
//                output1.S[x][y][z] = (cur[x][y][z] == copy[x][y][z]);
//            }
//        }
//    }
//    
//       
//    for (int x = 0; x < M; x++) {
//        for (int y = 0; y < M; y++) {
//            for (int z = 0; z < M; z++) {
//                output.S[x][y][z] = (last_output.S[x][y][z] == output1.S[x][y][z]);
//            }
//        }
//    }
//    
//    last_output = output;
    
    cur = copy;
    output = current;
}





void game_of_life_evolve(space& current) {
    auto& cur = current.S;
    auto copy = cur;
    
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            for (int z = 0; z < size; z++) {
                
                std::vector<size_t> neighbors = {};
                neighbors.reserve(27);
                for (int i = -1; i <= 1; i++)
                    for (int j = -1; j <= 1; j++)
                        for (int k = -1; k <= 1; k++)
                            if (i or j or k) neighbors.push_back(cur
                                                [(x + i + size) % size]
                                                [(y + j + size) % size]
                                                [(z + k + size) % size]);
                
                const size_t alive = cur[x][y][z];
                
                if (alive and not (count_alive(neighbors) >= 4
                               and count_alive(neighbors) <= 5)
                    ) copy[x][y][z] = 0;
                
                else if (not alive and (count_alive(neighbors) <= 5 and
                                        count_alive(neighbors) >= 5))
                    copy[x][y][z] = 1;
                                
            }
        }
    }
    cur = copy;
}
