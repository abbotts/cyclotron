#pragma once

enum class Location { GPU, HOST, STACK };

struct BufferOptions {
  Location loc;
  long offset, extra;

  BufferOptions():
    loc(Location::HOST),
    offset(0),
    extra(0)
  {}

  bool set(const char *arg);
};

struct Cyclotron {
  int barrier;
  int iters;
  BufferOptions ropt, sopt;

  Cyclotron():
    barrier(0),
    iters(1000)
  {}

  void run() const;
};
