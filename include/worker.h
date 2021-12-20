/**
 * Virtual base class for all worker classes.
 */

#ifndef __WORKER_H__
#define __WORKER_H__

class Worker
{
public:
    /**
     * Destructor
     */
    virtual ~Worker() {};

    /**
     * One-time setup
     */
    virtual void Setup() = 0;

    /**
     * Periodic workload
     */
    virtual void Loop() = 0;
};



#endif
