#ifndef _MIYUKI_TYPENAME_H
#define _MIYUKI_TYPENAME_H

#include "common/defines.h"
#include <string>

namespace Miyuki::AST {

    DEFINE_SHARED_PTR(WithTypeName)

    using namespace std;

    class WithTypeName {
        string typeName ="";
    public:
        void setTypeName(string tn);
        const string& getTypeName();
        string getAndSetTypeNameFrom(WithTypeNamePtr WTN);
        string getPointerTypeName();
        string getConstTypeName();
        string getConstPointerTypeName();

        // Static functions
        static string getStructTypeName(string structName);
        static string getUnionTypeName(string unionName);
        static string getEnumTypeName();
        static string _getCompName(string name, string inWhich);
        template <class DesiredType> 
        static string getLLVMTypeName();
    };

}

#endif 