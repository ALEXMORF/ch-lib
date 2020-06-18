#include "../ch_buf.h"
#include <assert.h>
#include <stdio.h>

int main()
{
    int *Data = 0;
    int Count = 1000;
    
    for (int I = 0; I < Count; ++I)
    {
        ChBufPush(Data, I);
    }
    
    assert(ChBufCount(Data) == Count);
    for (int I = 0; I < Count; ++I)
    {
        assert(Data[I] == I);
    }
    
    printf("OK\n");
    return 0;
}