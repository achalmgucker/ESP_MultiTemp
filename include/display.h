/**
 * All display and measure work.
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <worker.h>
#include <Ticker.h>


class Display : public Worker
{
public:
    Display() {};
    ~Display() override {};

    void Setup() override;
    void Loop() override;

};



#endif
