//
//  space.cpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#include "space.hpp"
#include "parameters.hpp"
//
//size_t count_alive(std::vector<size_t> neighbors) {
//    size_t count = 0;
//    for (auto unit : neighbors) if (unit == 1) count++;
//    return count;
//}
//
//void game_of_life_evolve(space& current) {
//    auto& cur = current.S;
//    auto copy = cur;
//
//    for (int x = 0; x < size; x++) {
//        for (int y = 0; y < size; y++) {
//            for (int z = 0; z < size; z++) {
//
//                std::vector<size_t> neighbors = {};
//                neighbors.reserve(27);
//                for (int i = -1; i <= 1; i++)
//                    for (int j = -1; j <= 1; j++)
//                        for (int k = -1; k <= 1; k++)
//                            if (i or j or k) neighbors.push_back(cur
//                                                [(x + i + size) % size]
//                                                [(y + j + size) % size]
//                                                [(z + k + size) % size]);
//
//                const size_t alive = cur[x][y][z];
//
//                if (alive and not (count_alive(neighbors) >= 4
//                               and count_alive(neighbors) <= 5)
//                    ) copy[x][y][z] = 0;
//
//                else if (not alive and (count_alive(neighbors) <= 5 and
//                                        count_alive(neighbors) >= 5))
//                    copy[x][y][z] = 1;
//
//            }
//        }
//    }
//    cur = copy;
//}
