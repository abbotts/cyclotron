#pragma once

enum class Location { GPU, HOST, STACK };

struct BufferOptions {
  Location loc;
  long offset, delta, extra;

  BufferOptions():
    loc(Location::HOST),
    offset(0),
    delta(0),
    extra(0)
  {}

  bool set(const char *arg);
};

struct Cyclotron {
  int barrier;
  int iters;
  int switcheroo;
  std::string output_path;
  BufferOptions ropt, sopt;

  Cyclotron():
    barrier(0),
    iters(1000),
    switcheroo(0),
    output_path("")
  {}

  void run() const;
};
