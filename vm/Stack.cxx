#include "vm/Stack.h"

#include <cstdio>

namespace Miyuki::VM {

    Miyuki::VM::StackPagedItem::StackPagedItem() {
        st = new Byte[1024];
        sp = -1;
    }

    Miyuki::VM::StackPagedItem::~StackPagedItem() {
        delete[] st;
    }

    Stack::Stack() {
        memset((void*)pages, 0, sizeof(StackPagedItem*) * 128);
        allocPage(0);
    }

    Stack::~Stack() {
        for (StackPagedItem* e : pages)
            if (e) delete e;
    }

    StackPagedItem * Miyuki::VM::Stack::getOrAlloc(uint32_t pageID) {
        if (!pages[pageID]) allocPage(pageID);
        return pages[pageID];
    }

    Byte& Stack::get(uint32_t addr) {
        int pageID = addr / STACK_PAGE_SIZE;
        assert(pageID < 128 && "Stack overflow");
        //printf("Stack::get():  Page: %d  item: %d\n", pageID, addr % STACK_PAGE_SIZE);
        return getOrAlloc(pageID)->get(addr % STACK_PAGE_SIZE);
    }

    void Miyuki::VM::Stack::push(Byte B) {
        // Check if current page is full
        if ((*currentPage)->isFull()) {
            // alloc new page
            currentPage++;
            allocPage(currentPage - pages);
        }
        (*currentPage)->push(B);
    }

    Byte Miyuki::VM::Stack::top() {
        return (*currentPage)->top();
    }

    void Miyuki::VM::Stack::pop() {
        // check if currentPage is empty
        if ((*currentPage)->isEmpty()) {
            currentPage--;
        }
        (*currentPage)->pop();
    }

}