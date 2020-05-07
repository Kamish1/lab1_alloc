#include "StdAfx.h"
#include "Allocator.h"
#include <iostream>
using namespace std;
struct header {
    bool isBusy;
    size_t size;
    size_t prevSize;
};
Allocator::Allocator(size_t memorySize)
{
    this->memorySize = memorySize;
    memory = new char[this->memorySize];
    struct header* head = (header*)memory;
    head->isBusy = false;
    head->prevSize = 0;
    head->size = this->memorySize - sizeof(header);
}
Allocator::~Allocator(void)
{
    delete[] memory;
}
void mem_copy(size_t ptr, size_t ptrc, size_t length)
{
    size_t* p = (size_t*)ptr;
    size_t* cp = (size_t*)ptrc;
    for (int i = 0; i < (length / 4); i++) {
        *cp = *p;
        p++;
        cp++;
    }
}
void Allocator::mem_dump()
{
    cout << "Memory dump (Memory size: ";
    cout << memorySize;
    cout << " bytes):\n";
    struct header* head = (header*)memory;
    int i = 1;
    size_t sum = 0;
    while (head < ((header*)((size_t)memory + memorySize))) {
        cout << "Block #";
        cout << i;
        cout << ":\t";
        if (head->isBusy) {
            cout << "Busy";
        }
        else {
            cout << "Free";
        }
        cout << "\t";
        cout << head->size;
        cout << " bytes";
        cout << "\t";
        cout << ((void*)((size_t)head + 12));
        cout << "\n";
        sum = sum + head->size + sizeof(header);
        head = (header*)((size_t)head + head->size + sizeof(header));
        i++;
    }
    cout << "Real memory size: ";
    cout << sum;
    cout << "\n";
}
void* Allocator::mem_alloc(size_t size)
{
    size_t size4 = size;
    while ((size4 % 4) != 0) {
        size4++;
    }
    size_t* ptr = new size_t;
    *ptr = NULL;
    struct header* head = (header*)memory;
    while ((*ptr == NULL) && ((size_t)head < ((size_t)memory + memorySize))) {
        while (
            (((head->isBusy) && ((size_t)head < ((size_t)memory + memorySize))) || ((!head - > isBusy) && (head->size < size4))) && ((size_t)head < ((size_t)memory + memorySize))) {
            head = (header*)((size_t)head + head->size + sizeof(header));
        }
        if ((size_t)head < ((size_t)memory + memorySize)) {
            if (head->size >= (size4 + sizeof(header))) {
                struct header* newHead = (header*)((size_t)head + head->size - size4);
                struct header* nextHead = (header*)((size_t)head + head->size + sizeof(header));
                if (((size_t)nextHead) < ((size_t)memory) + memorySize) {
                    nextHead->prevSize = size4;
                }
                head->size = head->size - size4 - sizeof(header);
                newHead->isBusy = true;
                newHead->size = size4;
                newHead->prevSize = head->size;
                *ptr = (size_t)newHead + sizeof(header);
            }
            else {
                if (head->size == size4) {
                    head->isBusy = true;
                    *ptr = (size_t)head + sizeof(header);
                }
                else {
                    head = (header*)((size_t)head + head->size + sizeof(header));
                }
               
            }
        }
    }
    return (void*)*ptr;
}
void Allocator::mem_free(void* ptr)
{
    if (ptr == NULL) {
        return;
    }
    struct header* head = (header*)((size_t)ptr - sizeof(header));
    if (!head->isBusy) {
        return;
    }
    struct header* prevHead = (header*)((size_t)head - head->prevSize - sizeof(header));
    struct header* nextHead = (header*)((size_t)head + head->size + sizeof(header));
    if ((((size_t)prevHead) >= ((size_t)memory)) && (((size_t)nextHead) < ((size_t)memory + memorySize))) {
        if ((!prevHead->isBusy) && (!nextHead->isBusy)) {
            struct header* nextNextHead = (header*)((size_t)nextHead + nextHead->size + sizeof(header));
            prevHead->size = prevHead->size + head->size + nextHead->size + 2 * sizeof(header);
            if (((size_t)nextNextHead) < ((size_t)memory + memorySize)) {
                nextNextHead->prevSize = prevHead->size;
            }
        }
        else {
            if (!prevHead->isBusy) {
                prevHead->size = prevHead->size + head->size + sizeof(header);
                nextHead->prevSize = prevHead->size;
            }
            else {
                if (!nextHead->isBusy) {
                    struct header* nextNextHead = (header*)((size_t)nextHead + nextHead - > size + sizeof(header));
                    head->isBusy = false;
                    head->size = head->size + nextHead->size + sizeof(header);
                    if (((size_t)nextNextHead) < ((size_t)memory + memorySize)) {
                        nextNextHead->prevSize = head->size;
                    }
                }
                else {
                    head->isBusy = false;
                }
            }
        }
    }
    else {
        if (((size_t)prevHead) >= ((size_t)memory)) {
            if (!prevHead->isBusy) {
                prevHead->size = prevHead->size + head->size + sizeof(header);
            }
            else {
                head->isBusy = false;
            }
        }
        else {
            if (((size_t)nextHead) < ((size_t)memory + memorySize)) {
                if (!nextHead->isBusy) {
                    struct header* nextNextHead = (header*)((size_t)nextHead + nextHead - > size + sizeof(header));
                    head->isBusy = false;
                    head->size = head->size + nextHead->size + sizeof(header);
                    9 if (((size_t)nextNextHead) < ((size_t)memory + memorySize))
                    {
                        nextNextHead->prevSize = head->size;
                    }
                }
                else {
                    head->isBusy = false;
                }
            }
            else {
                head->isBusy = false;
            }
        }
    }
}

void* Allocator::mem_realloc(void* ptr, size_t size)
{
    size_t* rePtr = new size_t;
    *rePtr = NULL;
    if (ptr == NULL) {
        *rePtr = (size_t)mem_alloc(size);
    }
    else {
        size_t size4 = size;
        while ((size4 % 4) != 0) {
            size4++;
        }
        struct header* head = (header*)((size_t)ptr - sizeof(header));
        size_t prevSize = head->size;
        if (size4 != head->size) {
            struct header* prevHead = (header*)((size_t)head - head->prevSize - sizeof(header));
            struct header* nextHead = (header*)((size_t)head + head->size + sizeof(header));
            if ((((size_t)prevHead) >= ((size_t)memory)) && (((size_t)nextHead) < ((size_t)memory + memorySize))) {
                if ((!prevHead->isBusy) && (!nextHead->isBusy)) {
                    if ((prevHead->size + head->size + nextHead->size + 2 * sizeof(header)) >= size4) {
                        size_t sumSize = prevHead->size + head->size + nextHead->size + 2 * sizeof(header);
                        prevHead->isBusy = true;
                        prevHead->size = size4;
                        *rePtr = (size_t)prevHead + sizeof(header);
                        struct header* nextNextHead = (header*)((size_t)nextHead + nextHead->size + sizeof(header));
                        size_t copySize;
                        if (prevSize > size4) {
                            copySize = prevSize;
                        }
                        else {
                            copySize = size4;
                        }
                        mem_copy((size_t)ptr, *rePtr, copySize);
                        if (sumSize != size4) {
                            struct header* newHead = (header*)((size_t)prevHead + prevHead->size + sizeof(header));
                            newHead->isBusy = false;
                            newHead->prevSize = prevHead->size;
                            newHead->size = sumSize - newHead->prevSize - sizeof(header);
                            if ((size_t)nextNextHead < ((size_t)memory + memorySize)) {
                                nextNextHead->prevSize = newHead->size;
                            }
                        }
                        else {
                            if ((size_t)nextNextHead < ((size_t)memory + memorySize))
                                { nextNextHead->prevSize = prevHead->size; }
                        }
                    }
                    else {
                        *rePtr = (size_t)mem_alloc(size);
                        if (*rePtr != NULL) {
                            mem_copy((size_t)ptr, *rePtr, head->size);
                            mem_free(ptr);
                        }
                    }
                }
                else {
                    if (!prevHead->isBusy) {
                        if ((prevHead->size + head->size + sizeof(header)) >= size4) {
                            size_t sumSize = prevHead->size + head->size + sizeof(header);
                            prevHead->isBusy = true;
                            prevHead->size = size4;
                            *rePtr = (size_t)prevHead + sizeof(header);
                            size_t copySize;
                            if (prevSize > size4) {
                                copySize = prevSize;
                            }
                            else {
                                copySize = size4;
                            }
                            mem_copy((size_t)ptr, *rePtr, copySize);
                            if (sumSize != size4) {
                                struct header* newHead = (header*)((size_t)prevHead + prevHead->size + sizeof(header));
                                newHead->isBusy = false;
                                newHead->prevSize = prevHead->size;
                                newHead->size = sumSize - prevHead->size - sizeof(header);
                                nextHead->prevSize = newHead->size;
                            }
                            else {
                                nextHead->prevSize = prevHead->size;
                            }
                        }
                        else {
                            *rePtr = (size_t)mem_alloc(size);
                            if (*rePtr != NULL) {
                                mem_copy((size_t)ptr, *rePtr, head->size);
                                mem_free(ptr);
                            }
                        }
                    }
                    else {
                        if (!nextHead->isBusy) {
                            if ((head->size + nextHead->size + sizeof(header)) >= size4) {
                                size_t sumSize = head->size + nextHead->size + sizeof(header);
                                head->size = size4;
                                *rePtr = (size_t)head + sizeof(header);
                                struct header* nextNextHead = (header*)((size_t)nextHead + nextHead->size + sizeof(header));
                                if (sumSize != size4) {
                                    struct header* newHead = (header*)((size_t)head + head->size + sizeof(header));
                                    newHead->isBusy = false;
                                    newHead->prevSize = head->size;
                                    newHead->size = sumSize - head->size - sizeof(header);
                                    if (((size_t)nextNextHead) < ((size_t)memory + memorySize))
                                    {
                                        nextNextHead->prevSize = newHead - > size;
                                    }
                                }
                                else {
                                    if (((size_t)nextNextHead) < ((size_t)memory + memorySize)) {
                                        nextNextHead->prevSize = head - > size;
                                    }
                                }
                            }
                            else {
                                *rePtr = (size_t)mem_alloc(size);
                                if (*rePtr != NULL) {
                                    mem_copy((size_t)ptr, *rePtr, head - > size);
                                    mem_free(ptr);
                                }
                            }
                        }
                        else {
                            if (size4 > head->size) {
                                *rePtr = (size_t)mem_alloc(size);
                                if (*rePtr != NULL) {
                                    mem_copy((size_t)ptr, *rePtr, head - > size);
                                    mem_free(ptr);
                                }
                            }
                            else {
                                struct header* newHead = (header*)((size_t)head + size4 + sizeof(header));
                                newHead->isBusy = false;
                                newHead->size = head->size - size4 - sizeof(header);
                                head->size = size4;
                                newHead->prevSize = head->size;
                                nextHead->prevSize = newHead->size;
                                *rePtr = (size_t)ptr;
                            }
                        }
                    }
                }
            }
            else {
                if (((size_t)prevHead) >= ((size_t)memory)) {
                    if (!prevHead->isBusy) {
                        if ((prevHead->size + head->size + sizeof(header)) >= size4) {
                            size_t sumSize = prevHead->size + head->size + sizeof(header);
                            prevHead->isBusy = true;
                            prevHead->size = size4;
                            *rePtr = (size_t)prevHead + sizeof(header);
                            size_t copySize;
                            if (prevSize > size4) {
                                copySize = prevSize;
                            }
                            else {
                                copySize = size4;
                            }
                            mem_copy((size_t)ptr, *rePtr, copySize);
                            if (sumSize != size4) {
                                struct header* newHead = (header*)((size_t)prevHead + prevHead->size + sizeof(header));
                                newHead->isBusy = false;
                                newHead->prevSize = prevHead->size;
                                newHead->size = sumSize - prevHead->size - sizeof(header);
                            }
                        }
                        else {
                            *rePtr = (size_t)mem_alloc(size);
                            if (*rePtr != NULL) {
                                mem_copy((size_t)ptr, *rePtr, head->size);
                                mem_free(ptr);
                            }
                        }
                    }
                    else {
                        if (size4 > head->size) {
                            *rePtr = (size_t)mem_alloc(size);
                            if (*rePtr != NULL) {
                                mem_copy((size_t)ptr, *rePtr, head->size);
                                mem_free(ptr);
                            }
                        }
                        else {
                            struct header* newHead = (header*)((size_t)head + size4 + sizeof(header));
                            newHead->isBusy = false;
                            newHead->size = head->size - size4 - sizeof(header);
                            head->size = size4;
                            newHead->prevSize = head->size;
                            *rePtr = (size_t)ptr;
                        }
                    }
                }
                else {
                    if (((size_t)nextHead) < ((size_t)memory + memorySize)) {
                        if (!nextHead->isBusy) {
                            if ((head->size + nextHead->size + sizeof(header)) >= size4) {
                                size_t sumSize = head->size + nextHead->size + sizeof(header);
                                head->size = size4;
                                *rePtr = (size_t)head + sizeof(header);
                                struct header* nextNextHead = (header*)((size_t)nextHead + nextHead->size + sizeof(header));
                                if (sumSize != size4) {
                                    struct header* newHead = (header*)((size_t)head + head->size + sizeof(header));
                                    newHead->isBusy = false;
                                    newHead->prevSize = head->size;
                                    newHead->size = sumSize - head->size - sizeof(header);
                                    if (((size_t)nextNextHead) < ((size_t)memory + memorySize)) {
                                        nextNextHead->prevSize = newHead - > size;
                                    }
                                }
                                else {
                                    if (((size_t)nextNextHead) < ((size_t)memory + memorySize)) {
                                        nextNextHead->prevSize = head - > size;
                                    }
                                }
                            }
                            else
                               
                                {
                                    *rePtr = (size_t)mem_alloc(size);
                                    if (*rePtr != NULL) {
                                        mem_copy((size_t)ptr, *rePtr, head - > size);
                                        mem_free(ptr);
                                    }
                                }
                        }
                        else {
                            *rePtr = (size_t)mem_alloc(size);
                            if (*rePtr != NULL) {
                                mem_copy((size_t)ptr, *rePtr, head->size);
                                mem_free(ptr);
                            }
                        }
                    }
                    else {
                        if (size4 > head->size) {
                            *rePtr = NULL;
                        }
                        else {
                            struct header* newHead = (header*)((size_t)head + size4 + sizeof(header));
                            newHead->prevSize = size4;
                            newHead->isBusy = false;
                            newHead->size = head->size - size4 - sizeof(header);
                            head->size = size4;
                        }
                    }
                }
            }
        }
        else {
            *rePtr = (size_t)ptr;
        }
    }
    return (void*)*rePtr;
}