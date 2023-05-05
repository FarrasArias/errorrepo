#pragma once

#include <unordered_map>
#include <vector>

// START OF NAMESPACE
namespace enums {

std::unordered_map<int,std::vector<int>> DENSITY_QUANTILES = {
  {0,{2,3,4,5,6,8,10,12,17,1073741824}},
  {1,{3,4,6,7,9,11,12,16,20,1073741824}},
  {2,{3,4,5,6,8,10,12,16,20,1073741824}},
  {3,{2,4,5,6,8,9,12,15,19,1073741824}},
  {4,{3,3,4,6,6,8,9,12,15,1073741824}},
  {5,{3,3,4,5,6,8,9,12,15,1073741824}},
  {6,{1,2,3,4,5,7,9,12,16,1073741824}},
  {7,{1,1,2,2,3,4,6,8,13,1073741824}},
  {8,{1,2,3,4,5,6,8,10,15,1073741824}},
  {9,{1,2,2,3,4,4,6,8,11,1073741824}},
  {10,{1,2,3,4,5,6,7,8,12,1073741824}},
  {11,{1,2,3,4,4,5,6,7,10,1073741824}},
  {12,{3,4,6,6,8,10,12,15,18,1073741824}},
  {13,{2,3,4,5,6,8,10,12,16,1073741824}},
  {14,{1,1,1,2,2,3,4,5,8,1073741824}},
  {15,{2,3,4,6,7,8,12,16,18,1073741824}},
  {16,{2,3,3,4,5,6,8,10,15,1073741824}},
  {17,{2,3,3,4,5,6,8,12,16,1073741824}},
  {18,{2,3,3,4,5,6,8,10,15,1073741824}},
  {19,{2,2,3,4,5,6,8,10,15,1073741824}},
  {20,{2,2,3,4,5,6,7,9,12,1073741824}},
  {21,{2,3,4,5,6,8,9,12,16,1073741824}},
  {22,{1,2,3,3,4,5,6,7,9,1073741824}},
  {23,{2,3,4,5,6,8,9,12,16,1073741824}},
  {24,{2,4,5,6,8,9,11,14,19,1073741824}},
  {25,{4,6,7,8,11,13,16,21,28,1073741824}},
  {26,{2,3,4,5,6,6,8,10,15,1073741824}},
  {27,{3,4,5,6,8,9,12,14,19,1073741824}},
  {28,{3,4,6,7,8,9,12,15,16,1073741824}},
  {29,{2,3,4,5,6,8,10,13,18,1073741824}},
  {30,{2,3,4,6,7,9,12,15,20,1073741824}},
  {31,{1,2,2,3,4,5,6,8,12,1073741824}},
  {32,{1,2,2,3,4,4,4,5,7,1073741824}},
  {33,{2,3,3,4,4,5,6,8,8,1073741824}},
  {34,{2,3,4,4,6,6,8,8,10,1073741824}},
  {35,{1,2,3,4,4,4,5,6,8,1073741824}},
  {36,{2,3,4,5,5,6,7,8,10,1073741824}},
  {37,{2,3,4,4,5,6,7,8,10,1073741824}},
  {38,{3,4,5,6,8,8,9,12,16,1073741824}},
  {39,{2,3,4,5,6,7,8,8,11,1073741824}},
  {40,{1,2,3,4,4,5,6,8,11,1073741824}},
  {41,{1,2,3,3,4,4,6,7,10,1073741824}},
  {42,{1,1,2,3,3,4,5,6,8,1073741824}},
  {43,{1,1,2,2,3,4,4,6,8,1073741824}},
  {44,{1,2,2,3,4,5,6,8,12,1073741824}},
  {45,{1,2,3,4,5,6,8,12,16,1073741824}},
  {46,{2,3,4,5,6,8,8,12,16,1073741824}},
  {47,{1,1,2,2,3,4,5,8,14,1073741824}},
  {48,{1,2,2,3,4,4,6,8,11,1073741824}},
  {49,{1,2,2,3,3,4,4,6,8,1073741824}},
  {50,{1,2,3,3,3,4,5,6,8,1073741824}},
  {51,{1,2,3,3,3,4,5,6,8,1073741824}},
  {52,{1,2,2,3,4,4,6,7,10,1073741824}},
  {53,{1,2,3,4,4,5,6,8,10,1073741824}},
  {54,{1,2,3,3,4,4,6,7,9,1073741824}},
  {55,{1,1,2,2,3,4,6,7,11,1073741824}},
  {56,{1,2,3,3,4,4,6,7,9,1073741824}},
  {57,{1,2,2,3,4,4,5,6,8,1073741824}},
  {58,{1,2,2,3,3,4,4,6,8,1073741824}},
  {59,{1,2,3,3,4,5,6,7,9,1073741824}},
  {60,{1,2,2,3,3,4,5,6,8,1073741824}},
  {61,{2,2,3,4,5,6,8,10,14,1073741824}},
  {62,{2,3,4,4,6,6,8,10,15,1073741824}},
  {63,{1,2,3,4,4,6,6,8,12,1073741824}},
  {64,{1,2,3,3,4,4,5,6,8,1073741824}},
  {65,{1,2,3,3,4,5,6,6,8,1073741824}},
  {66,{1,2,3,3,4,5,6,6,8,1073741824}},
  {67,{1,2,2,3,3,4,5,6,8,1073741824}},
  {68,{1,2,2,3,4,4,5,6,8,1073741824}},
  {69,{1,2,2,3,4,4,5,6,8,1073741824}},
  {70,{1,2,2,3,3,4,5,6,8,1073741824}},
  {71,{1,2,3,3,4,5,6,7,9,1073741824}},
  {72,{1,2,3,4,4,5,6,7,10,1073741824}},
  {73,{1,2,3,3,4,5,6,7,9,1073741824}},
  {74,{1,2,3,3,4,5,5,7,8,1073741824}},
  {75,{1,2,3,4,4,5,6,7,8,1073741824}},
  {76,{1,2,3,4,5,6,6,8,11,1073741824}},
  {77,{1,2,3,3,4,5,6,6,8,1073741824}},
  {78,{1,2,2,3,4,4,5,6,8,1073741824}},
  {79,{1,2,3,3,4,5,6,8,10,1073741824}},
  {80,{2,3,3,4,5,6,8,9,14,1073741824}},
  {81,{2,3,4,5,6,8,9,12,16,1073741824}},
  {82,{1,2,3,4,4,5,6,7,9,1073741824}},
  {83,{1,2,3,4,4,6,6,8,12,1073741824}},
  {84,{2,2,3,4,5,6,8,10,15,1073741824}},
  {85,{1,2,3,3,4,4,5,6,8,1073741824}},
  {86,{1,2,3,3,4,5,6,8,12,1073741824}},
  {87,{2,3,4,5,6,8,8,12,16,1073741824}},
  {88,{1,2,3,3,4,5,6,7,9,1073741824}},
  {89,{1,2,2,3,3,3,4,5,6,1073741824}},
  {90,{2,3,3,4,5,6,8,10,15,1073741824}},
  {91,{1,2,3,3,4,4,5,6,8,1073741824}},
  {92,{1,2,2,3,3,4,5,6,8,1073741824}},
  {93,{1,1,2,2,3,3,4,6,9,1073741824}},
  {94,{1,2,3,3,4,4,5,7,9,1073741824}},
  {95,{1,1,2,2,3,3,4,5,7,1073741824}},
  {96,{2,3,3,4,5,6,8,12,16,1073741824}},
  {97,{1,1,1,2,3,3,4,6,9,1073741824}},
  {98,{1,2,2,3,4,5,6,8,11,1073741824}},
  {99,{2,3,3,4,5,6,8,9,14,1073741824}},
  {100,{1,2,3,3,4,5,6,7,10,1073741824}},
  {101,{1,1,1,2,2,3,4,6,9,1073741824}},
  {102,{1,2,3,3,4,5,6,8,11,1073741824}},
  {103,{1,2,3,3,4,5,6,7,10,1073741824}},
  {104,{1,2,3,4,5,6,8,9,13,1073741824}},
  {105,{4,6,6,8,8,10,12,16,18,1073741824}},
  {106,{2,4,5,7,8,10,12,16,18,1073741824}},
  {107,{2,3,4,5,6,8,9,12,18,1073741824}},
  {108,{3,4,6,7,8,10,12,15,18,1073741824}},
  {109,{1,2,3,3,4,5,6,8,12,1073741824}},
  {110,{2,3,4,4,6,6,7,8,12,1073741824}},
  {111,{1,2,3,4,4,5,6,8,11,1073741824}},
  {112,{1,2,3,4,4,6,8,10,16,1073741824}},
  {113,{1,2,3,4,5,6,8,10,14,1073741824}},
  {114,{2,4,4,5,6,8,10,12,16,1073741824}},
  {115,{2,2,3,4,5,6,8,10,15,1073741824}},
  {116,{1,2,2,3,4,4,6,8,12,1073741824}},
  {117,{1,2,3,4,4,6,7,9,13,1073741824}},
  {118,{1,2,2,3,4,4,5,7,10,1073741824}},
  {119,{1,1,1,1,1,1,2,2,4,1073741824}},
  {120,{1,1,1,1,2,3,4,8,14,1073741824}},
  {121,{1,2,3,4,5,7,8,12,20,1073741824}},
  {122,{1,1,1,1,1,2,2,4,6,1073741824}},
  {123,{1,1,2,2,3,4,5,7,15,1073741824}},
  {124,{1,1,1,2,2,3,5,8,15,1073741824}},
  {125,{1,1,1,2,2,3,4,6,10,1073741824}},
  {126,{1,1,2,2,3,4,4,6,11,1073741824}},
  {127,{1,1,2,2,3,4,6,8,13,1073741824}},
  {128,{2,3,5,8,10,12,15,18,26,1073741824}}
};

}
// END OF NAMESPACE