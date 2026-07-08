#pragma once

void cyclotron(int iters = 1000,
               size_t recvExtra = 0,
               size_t recvOffset = 0,
               size_t sendExtra = 0,
               size_t sendOffset = 0,
               bool recvGPU = false,
               bool sendGPU = true,
               bool recvStack = false,
               bool sendStack = false,
               bool barrier = false
              );
