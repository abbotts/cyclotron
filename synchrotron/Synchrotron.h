#pragma once

struct Synchrotron {
  int barrier;
  int iters;

  Synchrotron():
    barrier(0),
    iters(1000)
  {}

  void run() const;
};
