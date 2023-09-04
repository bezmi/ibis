#ifndef SOLVER_H
#define SOLVER_H

#include <iostream>
#include <string.h>

class Solver {
public:
    int solve();

protected:
    virtual int initialise()=0;
    virtual int finalise()=0;
    virtual int take_step()=0;
    virtual bool print_this_step()=0;
    virtual bool plot_this_step()=0;
    virtual int plot_solution()=0;
    virtual int print_progress()=0;
    virtual bool stop()=0;
    virtual void print_stop_reason()=0;

private:
    unsigned int max_step_ = 0;
};

#endif
