#ifndef _MIYUKI_PTRDEFINES_H
#define _MIYUKI_PTRDEFINES_H

// Defined operation replated to pointer

#include <memory>

#define DEFINE_SHARED_PTR(type) class type; typedef std::shared_ptr<type> type##Ptr;
#define DEFINE_WEAK_PTR(type)   class type; typedef std::weak_ptr<type> type##PtrW;
#define DEFINE_UNIQUE_PTR(type) class type; typedef std::unique_ptr<type> type##Ptr;
#define DEFINE_RAW_PTR(type) class type; typedef type* type##Ptr;

#endif
