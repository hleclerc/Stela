#ifndef IPCALLINFO_H
#define IPCALLINFO_H

class SourceFile;

/**
  to maintain a call stack
*/
struct IpCallInfo {
    enum { IMPORT, CALL, IF };
    const SourceFile *sf;
    int offset;
    int reason;
};

#endif // IPCALLINFO_H
