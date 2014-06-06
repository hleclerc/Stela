#ifndef TODEL_H
#define TODEL_H

/**
A basic class to store objects to be deleted
*/
struct ToDel {
    virtual ~ToDel() {
    }
};

/**
specialization that will call delete
*/
template<class T>
struct ToDel_by_delete : ToDel {
    virtual ~ToDel_by_delete() {
        delete ptr;
    }
    T *ptr;
};


#endif // TODEL_H
