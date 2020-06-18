#pragma once
#include <stdlib.h>
#include <stdint.h>

struct ch_buf_hdr
{
    uint64_t Cap;
    uint64_t Count;
};

#define ChBufHdr(Array) ((ch_buf_hdr *)Array - 1)
#define ChBufCount(Array) ChBufHdr(Array)->Count
#define ChBufLast(Array) (Array)[ChBufCount(Array)-1]
#define ChBufPush(Array, Elmt) ((Array) = (decltype(Array))__ChBufExtend(Array, sizeof(Elmt)), (Array)[ChBufCount(Array)-1] = Elmt)
#define ChBufInit(Count, Type) (Type *)__ChBufInit(Count, sizeof(Type))

static void
ChBufFree(void *Buf)
{
    if (!Buf) return;
    free(ChBufHdr(Buf));
}

static void *
__ChBufInit(uint64_t Count, size_t ElmtSize)
{
    void *HdrBuf = malloc(sizeof(ch_buf_hdr) + Count * ElmtSize);
    ch_buf_hdr *Hdr = (ch_buf_hdr *)HdrBuf;
    Hdr->Count = Count;
    Hdr->Cap = Count;
    
    void *Buf = Hdr + 1;
    return Buf;
}

static void *
__ChBufExtend(void *Buf, size_t ElmtSize)
{
    void *Result = 0;
    
    if (Buf == 0)
    {
        ch_buf_hdr Hdr = {};
        Hdr.Cap = 2;
        Hdr.Count = 1;
        
        ch_buf_hdr *BufWithHdr = (ch_buf_hdr *)malloc(sizeof(Hdr) + Hdr.Cap * ElmtSize);
        *BufWithHdr = Hdr;
        Result = BufWithHdr + 1;
    }
    else
    {
        ch_buf_hdr *Hdr = ChBufHdr(Buf);
        Hdr->Count += 1;
        
        if (Hdr->Count > Hdr->Cap)
        {
            uint64_t NewCap = (uint64_t)((float)Hdr->Cap * 1.5f) + 1;
            Hdr->Cap = NewCap;
            ch_buf_hdr *BufWithHdr = (ch_buf_hdr *)realloc(Hdr, sizeof(ch_buf_hdr) + NewCap * ElmtSize);
            Result = BufWithHdr + 1;
        }
        else
        {
            Result = Buf;
        }
    }
    
    return Result;
}