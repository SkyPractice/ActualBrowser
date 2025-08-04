#pragma once
#include "../SigmaInterpreter.h"
#include <memory>
#include <vector>

namespace Util {
    class SigmaInterpreterHelper {
    public:
        static void initializeStandardLibraries(std::shared_ptr<Scope>& target_scope);    

        static LambdaVal* evaluateLambda(std::shared_ptr<Scope>& target_scope, Statement* stmt);
        static ObjectVal* evaluateStruct(SigmaInterpreter* self,
            StructExpression* stmt);
        static RunTimeVal* evaluteIdentifier(SigmaInterpreter* self,
            IdentifierExpression* expr);

        // for if, elseif, else too
        static RunTimeVal* evaluateIfCodeBlock(SigmaInterpreter* self,
            std::vector<Statement*>& stmts);
        static RunTimeVal* evaluteLoopCodeBlock(SigmaInterpreter* self,
            std::vector<Statement*>& stmts, bool& request_break);
        static void verifyCompiledFunctionCallArgs(SigmaInterpreter* self,
            FunctionCallExpression* expr, std::vector<RunTimeVal*>& args,
            RunTimeVal* func);

        static std::vector<RunTimeVal*> evaluateExprVector(SigmaInterpreter* self,
            std::vector<Expression*>& expr_vec);
        static std::vector<RunTimeVal*> evaluateExprVectorForCompiledFunctions(
            SigmaInterpreter* self, std::vector<Expression*>& expr_vec
        );
        static void cvtToPrimitiveIfWrapper(RunTimeVal** val);
        static RunTimeVal* cvtToWrapperIfPossible(RunTimeVal* val);
    };
};